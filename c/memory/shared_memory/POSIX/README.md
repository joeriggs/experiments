
shm_client.c and shm_server.c show how you can use shared memory to communicate
between processes.  Start shm_server first.  Then, in another window, start
shm_client.

shm_server will create a shared memory space (you can see it in /dev/shm/my_shm).
Then it starts writing to that shared memory space.

shm_client watches the same shared memory space and reports whenever it sees
the data change.

shm_fork is similar to shm_server and shm_client, except it works in a parent/
child relationship.  It calls fork() to create a child, and then the parent
and child communicate via the shared memory space.

