# Introduction #

cql-db is a self-organizing, scalable relational database system. It is designed to scale seamlessly as new nodes become available, or are removed.

Each node runs one or more services, including query processing, compute, and storage. A complete system needs one of each running somewhere.

# Goals #

While having many laudable qualities, most modern database servers are not horizontally scalable. In addition, many of them require complex configuration. Disaster recovery is expensive, and very difficult to scale. Management is a significant cost factor, and is often very complex.

The primary goals of cql-db are to allow the system to require as little configuration as possible. It should scale up and down transparently, and self-optimize for data access. It should allow long-running queries to co-exist with short, ad-hoc queries without causing unnecessary drain on system resources.
