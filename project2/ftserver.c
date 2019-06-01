/**************************
 * Maintainer: Mario Bocaletti
 * Description: ftserver serves file contents
 * over a TCP socket connection
 * Class: cs371 Networking
 * Last Modified: 6-2-19 
 * ***********************/

/*
 *
 *  NOTE: Socket code based on my own cs344 project (bocaletm on GitHub), some of which is based on Professor Brewster's cs344 lectures.
 *  source for reading file http://www.fundza.com/c4serious/fileIO_reading_all/
 *
 */

//for signals
#include <signal.h>
//for general stuff
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//for sockets
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
//for forking
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
// for mutex
#include <pthread.h>
// for EBUSY
#include <errno.h>
// to read directory
#include <dirent.h>

//mutex to control access to the socket
pthread_mutex_t socketLock;

const int MAX_FORKS = 5;
int caught_signal = -5;

/***********************
 * Struct for socket file descriptors   
 * ********************/
struct socketFDs {
  int listenFD;
  int connectionFD;
};

/************************
 * handler: broken pipes in child process 
 * **********************/
void pipeHandler(int signo) {
  char* message = "error broken pipe\n";
  write(STDERR_FILENO,message,18);
  fflush(stderr);
}

/************************
 * handler: do not exit on signal from child
 * **********************/
void handler(int signo) {
  caught_signal = signo;
}

/************************
 *  Error function used for reporting major issues
 *************************/
void errorx(const char *msg) { 
  perror(msg);
  exit(1); 
}

/************************
 *  Error function used for reporting minor issues
 *************************/
void error(const char *msg) { 
  fprintf(stderr,msg);
}

/************************
 *  encrypt(): encrypts param2 using param1 key
 *************************/
void encrypt(char* key, char* msg, int n) {
  printf("this is a function\n");
}

/**********************
 * acceptConnection(): hangs on listen
 * puts file descriptors in parameter struct
 * ********************/
void acceptConnection(int portNumber,struct socketFDs* socketInfoPtr) {
  socketInfoPtr->listenFD = -5;
  socketInfoPtr->connectionFD = -5;
  socklen_t sizeOfClientInfo;
  struct sockaddr_in serverAddress, clientAddress;

  // Set up the address struct for this process (the server)
  memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
  serverAddress.sin_family = AF_INET; // Create a network-capable socket
  serverAddress.sin_port = htons(portNumber); // Store the port number
  serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

  // Set up the socket
  socketInfoPtr->listenFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
  if (socketInfoPtr->listenFD < 0) {
    errorx("ERROR opening socket");
  }

  // Enable the socket to begin listening
  if (bind(socketInfoPtr->listenFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    errorx("ERROR on binding");
  }

  listen(socketInfoPtr->listenFD,1); // Flip the socket on 

  // Accept a connection, blocking if one is not available until one connects
  sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
  socketInfoPtr->connectionFD = accept(socketInfoPtr->listenFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
  if (socketInfoPtr->connectionFD < 0) {
    errorx("ERROR on accept");
  }
}

/***********************
 * readSocket(): read command from client
 ***********************/
void readSocket(int establishedConnectionFD, char* message) {
  int midx = 0, i, charsRead = -1;
  int bufferSize = 100000;
  char longbuffer[bufferSize];
  if (establishedConnectionFD > 0) {
    memset(longbuffer, '\0', bufferSize);
    charsRead = recv(establishedConnectionFD, longbuffer, bufferSize - 1, 0); // Read the client's message from the socket
    if (charsRead < 0) {
      error("ERROR reading from socket\n");
    } 

    for (i = 0; i < bufferSize; i++) {
      message[midx] = longbuffer[i];
      midx++;
    }

    while(strstr(longbuffer,"@") == NULL) {
      charsRead = recv(establishedConnectionFD, longbuffer, bufferSize - 1, 0); // Read the client's message from the socket
      if (charsRead < 0) {
        error("ERROR reading from socket");
      }
      strcat(message,longbuffer);
    }
  }
}

/**********************
 * spawnThreads()
 **********************/
void spawnThreads(pid_t* spawnpid) {
  int i;
  //spawn worker processes
  for (i = 0; i < MAX_FORKS; i++) {
    spawnpid[i] = fork();
    if (spawnpid[i] == -1) {
      errorx("Fork error. Exiting.\n");
      /*********************************************
       * CHILD PROCESSES
       * *******************************************/
    } else if (spawnpid[i] == 0) {
      int mutexStatus;

      while (1) {
        //handler to wake up on sigcont
        signal(SIGCONT,handler);

        //do not let broken pipe kill process
        signal(SIGPIPE,SIG_IGN);

        //sleep until parent signals
        pause();

        //try to lock the mutex

        mutexStatus = pthread_mutex_trylock(&socketLock);

        //loop back if another process picked up the job
        if (mutexStatus == EBUSY) {
          continue;
        }

        //unlock mutex
        pthread_mutex_unlock(&socketLock);
      }
      exit(0);
    }
  }
}

/************************
 * execute(): try to execute the command
 * returns the result of the command as a string or an error
 * **********************/
char* execute(char* message) {
  char* result;
  int maxLength = 1000000;
  FILE* inputFile;
  long fileBytes;
  DIR* directory;
  struct dirent *directory;
  result = malloc(sizeof(char) * (maxLength + 1));
  memset(result,'\0',(maxLength));

  if (message[1] == 'l') {
    directory = opendir(".");
    if (directory != 0) {
      strcpy(result,directory->dir_name,maxLength);
    } else {
      strcpy(result,"Error: could not get directory list\n");
    }
  } else {
    inputFile = open(FILEPATH, O_RDONLY, 0600);
    if (fileDescriptor < 0) {
      strcpy(result,"Error: could not open file\n");
    } else {
      fseek(inputFile, 0L, SEEK_END);
      fileBytes = ftell(inputFile);
      fseek(inputFile,0L,SEEK_SET);
      fread(result,sizeof(char),fileBytes,inputFile);
      fclose(inputFile);
    }
  }
  return result;
}

/**********************
 * readCommands()
 **********************/
void readCommands(int FD,int transferPort) {
  char* result = 0;
  int maxLength = 260; //this is the max length of a unix path + command + space + newline
  char* message = 0;
  while (1) {
    message = malloc(sizeof(char) * (maxLength + 1));
    memset(message,'\0',(maxLength + 1));
    readSocket(FD,message);
    //check for valid path length and command
    if (message[260] != '\0' || message[0] != '-' || message[1] != 'g' || message[1] != 'l') { 
      sendMessage(transferPort,"e\0");
    } else {
      result = execute(message);
      sendMessage(tranferPort,result);
    }
    free(message);
    message = 0;
    if (result != 0) {
      free(result);
      result = 0;
    }
  }
}

/************************
 * main()
 * *********************/
int main(int argc, char *argv[]) {
  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); 
  }

  struct socketFDs controlConnection;

  //accept connection passing the port
  acceptConnection(atoi(argv[1]),&controlConnection);

  //read transfer port from client
  char portBuffer[10];
  memset(portBuffer,'\0',10);
  readSocket(controlConnection->connectionFD,portBuffer); 
  int transferPort = atoi(portBuffer);

  //get commands from client
  readCommands(controlConnection->connectionFD,transferPort);

  //  pid_t spawnpid[MAX_FORKS];
  //spawnThreads(spawnpid);

  /*********************************************
   * PARENT PROCESS
   * *******************************************/

  //wake up children
  //  int i;
  //  for (i = 0; i < MAX_FORKS; i++) {
  //    kill(spawnpid[i],SIGCONT);
  //  }
}