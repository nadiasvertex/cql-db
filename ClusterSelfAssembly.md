# Introduction #

One of the most annoying parts of bringing up a cluster involves all the configuration needed to let each of the nodes know about other nodes. This is problematic in many situations, from installation to upgrade, to recovery.

The point of self assembly is to allow the user to simply install the software on a node, and the node automatically joins the cluster. The cluster automatically re-organizes itself to accept and utilize the new node.

By default a node will come up with compute and query processing units. If no data storage unit is explicitly configured, the nodes will elect some of their members to perform the job.

The intent is for the cluster to always keep the data safe. Therefore, it may make conservative choices that reduce potential performance in order to ensure that the cluster's data remains available and intact.

# Configuration Message Processing #

There are two types of messages in the system, generally speaking. The kind we are considering on this page are configuration messages. They change the state of the system, but don't directly affect data. The other kind are data operations, which may involve reads or writes.

Every configuration message is processed like this:

  * The number of nodes known to each node is _n_.
  * The number of tolerable faulty nodes is _f_ = (_n_-1)/3.
  * Each full sequence of PRE-PREPARE -> PREPARE -> COMMIT must finish within a bounded period of time. If it does not, then the sequence number is retired and all nodes ignore further messages with that sequence number. The time allowed varies based on the operation to perform.
  * Sequence numbers are allocated from a circular pool, and must be at least h values greater than the last, but no more than H values greater. h and H are generally compile time constants that serve to bound a small random number used as the increment.

  1. The originator of the configuration message sends the message as the payload of a PRE-PREPARE message to all nodes. Each message has a sequence number, and the payload is signed with an HMAC.
  1. The receivers of the configuration message validate the HMAC. If it is invalid, the message is discarded.
  1. If the receiver has already received this message with the same sequence number, the message is discarded.
  1. Otherwise the receiver generates an HMAC of the PRE-PREPARE message and sends it in a PREPARE message to all other nodes. (This message has the same sequence number as the PRE-PREPARE message.)
  1. Once the receiver receives _2f_ PREPARE messages consistent with the PRE-PREPARE message it sent out, it sends a COMMIT message to all other nodes.
  1. Once the originator and all receivers receive _2f+1_ COMMIT messages that are consistent with the PRE-PREPARE message, the nodes all apply the configuration command.

# Node Startup #

When a node starts, it binds to UDP port 32462. This is the cluster presence announce channel. Once a second each node in the cluster indicates its presence on this channel.

The presence announcement provides the node's UUID, which is generated fresh by each node each time it is started. This allows the cluster not to worry about nodes going away and coming back. If a node goes away, its gone. It never comes back. Instead, a new node appears which goes through the normal startup process.

# Peer Recognition #

After a node binds to the broadcast channel, it will start to receive presence announcements. It uses the presence announcements to map node addresses (UUIDs) to IP addresses.

When it notices a node that it has not seen before, it subscribes to the node's publishing channel. It also makes a point-to-point connection to the node's command reply channel.

Using the node's command reply channel it advertises its own capabilities, and asks for that node's capabilities. It also asks the node for a list of known nodes in the cluster, as well as the cluster's "state".

Note that each node engages in Diffie-Helman key exchange when setting up the point-to-point channel. Commands are always signed with an HMAC utilizing the negotiated key. The receiver always verifies the response via the HMAC. Failures are always discarded. If the failure rate passes a threshold, the offending node is marked as faulty and requests from it are ignored.

# The Breath of Life #

At this point, the new node is in a state called "PASSIVE". If this is a new cluster, then all nodes will be "PASSIVE", and the cluster's state will be "PASSIVE". On the other hand, it may be that this node needed to be rebooted for a maintenance cycle. If that is the case, then the cluster will already be active when this node comes up.

Therefore, the startup sequence splits into two paths at this point.

## Joining an Active Cluster ##

The cluster uses a series of configuration commands to understand state. When a new node announces in a cluster, all active nodes will see it. They will all send it a configuration command. It will accept the first one, and ignore all further configuration commands.

The node will then process the command like this:

  1. It will take the known configuration sent to it by one of the active nodes, and add itself.
  1. The new node will then propose the new configuration as a cluster-wide configuration command, with itself as the originator. (The steps in "Configuration Message Processing" above are followed.)
  1. If the reconfiguration does not commit within the allowed time limit, the node will request the current configuration at random from one of the nodes in the list it has made by listening to announcements. Go to step 1
  1. If the reconfiguration of the cluster commits, the node goes ACTIVE.

## Joining a Passive Cluster ##

When joining a passive cluster, all nodes will be in the same boat. That is, no configuration will automatically be sent to them. There is a timer which will expire when no configuration information is received.

  1. The node will gather the announcements it has heard about, and use them to generate a new configuration.
  1. The new configuration is proposed.
  1. If the configuration does not commit, the node will wait for a short period of time.
    1. If a configuration command is received before the timer expires, it will participate in that configuration event.
    1. Otherwise it will go to step 1
  1. If the configuration commits, then the node goes ACTIVE.

### Persistence ###

If a node is configured to not care about persistence, then it will not wait for storage cells to become active, and it won't request storage election. However, it will still participate in storage elections if some other node requests it; and if storage cells exist it will utilize them.
e
By default, any cluster over a single node will require a replication factor of 2. The required replication factor can be reduced or increased by configuration.

Therefore, if a cluster comes up in passive mode, and no storage cells were explicitly configured (or insufficient cells were configured) for the replication, then storage election will occur.

During storage election, each node examines its disk environment. By default, on Linux the data will go into /var/cql. On Windows it will go on whatever drive is largest. A folder named "cql" will be created in the root of the drive. If the process does not have permission to write to the root of the drive, it will fall back to the largest drive it can write to. Ultimately, it will fallback for the public data path and create a folder called "cql" there.

The process will then write a series of data blocks of various sizes to the chosen location in order to test data throughput. The file system capacity, as well as the performance results will be shared with all other nodes in the cluster.

Once every node has reported in, all nodes will evaluate the results and pick the largest, fastest k nodes to run storage cells. Both size and capacity are normalized w/r to the largest and fastest volumes in the cluster. Both size and capacity are given equal weight.

Once k nodes are chosen as storage cells, they permanently configure themselves as storage nodes.

Note that any node can be configured with storage, and the user can migrate roles at will. In all cases, the user has complete control over the configuration _if desired_. However, the defaults are sane and safe.

#### Storage Cell Election ####

  1. A node sends a storage cell election configuration message to all other nodes.
  1. Each receiver evaluates its storage cell configuration.
    1. Each node may choose to contact the known storage nodes if it has not done so recently.
  1. If there are sufficient storage nodes, and the node has contacted them recently, the node does _not_ send a PREPARE response.
  1. Otherwise the node sends a PREPARE response to all other nodes.
  1. If the election configuration does not COMMIT, this is a failed election.
  1. If the election configuration COMMITS, each node begins the election process:
    1. The node evaluates file systems it has permissions to, and finds the largest one.
    1. The node performs a throughput test on its chosen file system. (It may also utilize cached results if they are recent.)
    1. If the node has received PRE-PREPARE configuration commands proposing other nodes as storage cells, it checks its own statistics against those received:
      1. If one of the received configurations is better than its own, it sends a PREPARE message for that configuration.
      1. If the node has a better configuration than any of the receipts, it creates a new configuration proposing itself as a storage node and issues a PRE-PREPARE with that configuration.
    1. Otherwise the node creates a new configuration proposing itself as a storage node and issues a PRE-PREPARE with that configuration.
    1. If the configuration COMMITS, the node spins up a storage cell and begins handling storage traffic.
    1. The election process repeats until sufficient storage cells have been elected to satisfy the required replication factor.

# Extinction Level Events #

In an ideal world, a cluster would only experience planned events that allow the cluster to stay in simple recovery states. However, in the real world complex systems experience complex failures. Natural disasters, accidents, and poorly understood configurations can cause the system to fail in ways that it was not designed to anticipate. These kind of failures are called Byzantine faults.

This RDBMS is designed from the ground up to anticipate complex failure scenarios. It does this by implementing a fault tolerance algorithm based on Byzantine fault tolerance. It is not, however, a strictly BFT algorithm.

## Membership ##

Since announcements are made every second, it stands to reason that the sudden loss of a beacon would indicate the node has gone dark. A node is given a grace period to recover. If the node does not recover, all nodes in the cluster who notice will issue a reconfiguration command removing the node from the cluster. The first command to commit will cause all others to be discarded.

This same process will be followed if up to _f_ nodes fail during the epoch (where _f_ is the fault tolerance level calculated by the number of nodes and the replication factor in the system.)

This very simple process will cause new replicas to be formed and copies to be made, thus maintaining the integrity of the system.

If more than _f_ nodes fail in each epoch, the system has detected an extinction level event.

## Split Brain ##

If the system detects an ELE, it strives to avoid split-brain syndrome. It does this by following a process that ensures that one and only one group of nodes operates in write mode.

  1. For each table, if _p_ - _f_ partition primaries are available, and at least one replica for each missing primary, the table remains read/write.
  1. In all other cases, the table is read-only, best-effort. The server will indicate that the cluster is in a degraded state, so the client can take whatever steps are deemed necessary.

This process allows the system to avoid data corruption, while continuing to offer the best service possible.

For example, if a 20 node cluster with a replication factor of two becomes partitioned into exactly two pieces, a table with 20 partitions will only have 10 primaries available. This is less than the 14 primaries necessary, so neither side will have write access to this table. (Remember that _f_ = (_n_-1)/3. So in this case, _f_ = 6.)

On the other hand, if a table is only partitioned over 10 of the nodes, then it needs to ensure that 7 primaries survived, and that it has replicas for the other 3 that might not have. If it finds that to be true, it may continue read/write service to the table.

It is not possible for both sides to find the condition to be true, since the algorithm requires a clear majority of primaries. It is possible, as we've shown, for both sides to find the condition to be false.

The algorithm holds even in the presence of multiple failures. For example, if the cluster became divided into three units it would be less likely that any one of them could serve a table in read/write mode; however if the algorithm indicated it was safe it would indeed _be_ safe.

## Recovery ##

The simplest way to recover from an ELE, is just to choose to restart all but one of the cluster partitions. For example, if a cluster was divided in two by a network failure, and the failure has been corrected, simply restart all the nodes on one side or the other.

Then issue a "RECOVER" command to the cluster, which will reset the state of all tables to READ/WRITE.