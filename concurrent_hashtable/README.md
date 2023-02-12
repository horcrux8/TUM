<p align="center">
  <h2 align="left"> Concurrent hash table server </h2>
  <p> A thread-safe hash table server which communicates with multiple clients through shm.
  <br/>
<br/>

### 1. Build & Run
#### 1.1 Binary generation
```
* Build all.	``` $ make all ```
* Build server.	``` $ make server.o ```
* Build client. ``` $ make client.o ```
* Clean binaries. ``` $ make clean ```
```
#### 1.2 Run 
* Command-line arguments for server.o
  * Hash table size: The maximum number of buckets in the hash table
* Client does not take any arguments
* Client reads input from terminal in the following format:
  * Operation key
  * Operation can be one of: 1:Insert, 2:Read, 3:Delete 4:To Quit.
  * Key is an integer > 0.

* The default implementation can handle maximum 10 clients at a time. It is defined in the shm_header.shas "MAX_CLIENTS".
* The shared memory buffer size: sizeof( global_t ) + MAX_CLIENTS * (sizeof (wait_t) + sizeof(req_t))
  * global_t has global information of the queue, viz. front, rear index and lock
  * wait_s is the waiting station for client waiting for a reply from server which it submitted
  * Queue is implemented using circular array of req_t. Queue size = MAX_CLIENTS
* Each client gets a free id on its invocation by looking up the wait station and locks it.
* Client waits at the station corresponding to its id upon successful submittion of request at the queue (enqueue).
* The hash table has one lock per bin. 
* Concurrent operations using reader-writers per bin

  

