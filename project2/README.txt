Mario Bocaletti - Simple File Tranfer

0. Environment
  - flip1 was used to test this program.
  - Unzip the submission with the following command:
     $ unzip MarioBocaletti_project2.zip

1. Compilation
  - A makefile is included in the submission
  - Type make to compile both the java  and the C code
     $ make
  - Type make clean to clear the directory after testing (Optional)
     $ make clean

2. Usage
  - After compilation, open the server program by typing:
      $ ./ftserver <PORT> 
  - Open the client program with the following command:
      $ java ftclient <HOST> <PORT> <COMMAND> <TRANSFER PORT> 
        EXAMPLE: $ java ftclient localhost 30066 -l 30065

  - Use the port of your choice, but ports in the 36000s are suggested
  - <COMMAND> must be -l or -g <FILENAME>
