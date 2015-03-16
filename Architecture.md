# Discovery #

Each node in the network listens to local network broadcasts to locate peer nodes. Each node knows which nodes it can directly connect to. The node set also behaves as a mesh network, which allows messages to be routed through nodes to other networks. This is especially useful for disaster tolerance and recovery.

## Query Processor ##

As each query processor starts up, it announces itself on the mesh network. Each query processor will receive announcements about available schema. Each query processor stores it's own view of the schema, which allows it to plan query execution without being chatty.

## Compute Cell ##

As each compute cell starts up, it announces itself on the mesh network. It then waits to receive configuration commands that will tell it what parts of the database it is responsible for. On a system with no storage cells, the database schema will have to be recreated on each restart, and the data reloaded using the ETL tools.

After being configured (assuming the presence of storage cells), a compute cell will begin to request data from storage cells.

## Storage Cell ##

As each storage cell starts up, it announces itself on the mesh network. It listens for compute cell announcements, and builds an internal hash map for each table that it has stored. After waiting a reasonable amount of time, the storage cell begins sending configuration commands to compute cells to tell them what section of the data they will be managing.

## Coordinator Cell ##

As each coordinator starts up, it listens to the compute configuration announcements and creates a view of the local configuration.

# Function #

Each unit in the cluster has a particular function. This section describes the functions of these units.

## Compute Cell ##

Each compute cell stores data in memory or disk. A compute cell that stores data on disk is called a cold compute cell. To be completely correct, a cold compute cell does not actually store data, but instead stores indexes that keep track of the data persisted to storage units.

Each compute cell processes predicates and passes them on to query processors for final assembly and processing. Each compute cell may also provide partial ordering for queries which require that.

Depending on size and frequency of use, tables may be managed by a single compute cell, many compute cells, or all compute cells. Very small tables are managed by a single compute cell, but may have read-only copies on many or all compute cells. As tables grow they may be hashed across any number of compute cells. The RDBMS tries to automatically tune the location of tables depending on how large and how used they are.

Depending on the configuration of the cluster, each node may provide one or more replica compute cells that only store data. They are primary used to provide durability for the rapid commit-to-memory architecture. They are also generally responsible for ensuring that updates get to a storage cell.

## Storage Cell ##

The storage cell is the persistence mechanism. Each storage cell contains a certain section of the data, as well as complete schema structure and one or more data replicas.

## Coordinator Cell ##

The coordinator cell tracks usage for each node. They also communicate with each other to create a cohesive view of cluster load. The coordinators may send configuration commands to compute cells to move data to other cells or nodes.