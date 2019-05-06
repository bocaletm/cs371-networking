Mario Bocaletti - Simple Chat Client

0. Environment
  - flip1 was used to test this program.
  - Unzip the submission with the following command:
     $ unzip MarioBocaletti_project1.zip

1. Compilation
  - A makefile is included in the submission
  - Type make to compile both the java chatserve and the C chatclient
     $ make
  - Type make clean to clear the directory after testing (Optional)
     $ make clean

2. Usage
  - After compilation, open the server program by typing:
      $ ./chatserve <PORT>
  - Open the client program with the following command:
      $ ./chatclient localhost <PORT> 
  - Use the port of your choice, but ports in the 36000s are suggested
  - Enter your desired handle on the client. This will initiate the connection.
  - Enter messages on either program to communicate across the programs.
  - When satisfied with testing, enter \quit on either program to terminate.
     $ \quit
    NOTE: The client will wait for input from the user even if it receives
      \quit from the server. This is a design choice to make the program thread safe.

3. Extra Credit
  - Programs do not need to take turns to communicate. Any user
    can type a message at any time.
  - Both programs are multi-threaded. The client uses a thread to receive 
    messages from the server, and the server uses a thread to send messages to 
    the client.
