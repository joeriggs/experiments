
The projects in this directory communicate via a shared memory space.  You can
see the shared memory space in /dev/shm/my_shm.

* shm_client and shm_server show how you can use shared memory to communicate
between unrelated processes.  Start shm_server first.  Then, in another window,
start shm_client.  They will chat back and forth.

* shm_fork_semaphore is similar to shm_server and shm_client, except it works in a
parent/child relationship.  It calls fork() to create a child, and then the parent
and child communicate via the shared memory space while using a semaphore as a
synchronization mechanism.

* shm_fork_rwlock is very similar to shm_fork_semaphore, except it uses a shared
read-write lock to communicate between parent (write lock) and children (read
locks).

