Linux Multi-threaded Client-Server Using Shared Memory

This project demonstrates a multi-threaded client-server architecture in Linux, using shared memory for communication between the server and its clients. The server can handle multiple clients simultaneously, logging all interactions to a file.

Features
* Multi-threaded Server: The server can handle multiple clients concurrently.
* Shared Memory: Communication between the server and clients is facilitated through shared memory.
* Semaphores: Synchronization is achieved using semaphores to ensure data integrity in the shared memory.
* Logging: All interactions are logged to a file named shared_memory_log.txt.

Getting Started

Prerequisites
* A Linux-based system
* GCC compiler
* POSIX thread library
* POSIX semaphore library
  
Compilation
To compile the server and client programs, use the following commands:

gcc -pthread -o server server.c -lrt
gcc -o client client.c -lrt

Running the Server
Start the server using:

./server
The server will listen on port 4567 for incoming client connections.

Running the Client
Start a client using:

./client
You can run multiple clients to test the multi-threaded functionality of the server.

Project Structure

* server.c: The server code, which handles incoming client connections, updates shared memory, and logs interactions.
* client.c: The client code, which connects to the server, sends messages, and reads responses.
* shared_memory_log.txt: The log file where all interactions are recorded.

How It Works

Server

1. Initializes shared memory and semaphores.
2. Listens for incoming client connections.
3. For each client, creates a new thread to handle communication.
4. Updates shared memory with client messages and server responses.
5.Logs all interactions to shared_memory_log.txt.

Client

1. Connects to the server.
2. Sends messages to the server.
3. Receives responses from the server.
4. Reads and displays the contents of the shared memory.
   
Example Usage

1. Start the server:

./server

2. Start a client:

./client

3. Enter messages in the client terminal. The server will respond, and the interactions will be logged and displayed.

Notes

Ensure that the server is running before starting any clients.
To stop the server, use Ctrl+C.
To exit a client, type exit and press Enter.


Author
[RAM BABU GUPTA]
