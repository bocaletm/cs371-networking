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
#include <netdb.h>  
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
// for sockaddr
#include <netinet/in.h>

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
 * returns client address info
 * ********************/
struct sockaddr_in  acceptConnection(int portNumber,struct socketFDs* socketInfoPtr) {
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

  printf("ftserver waiting for connection on port %d\n",portNumber);
  // Accept a connection, blocking if one is not available until one connects
  sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
  socketInfoPtr->connectionFD = accept(socketInfoPtr->listenFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
  if (socketInfoPtr->connectionFD < 0) {
    errorx("ERROR on accept");
  }
  printf("ftserver connected to client\n");
  return clientAddress;
}

/***********************
 * readSocket(): read command from client
 ***********************/
void readSocket(int establishedConnectionFD, char* message) {
  int charsRead = -1;
  int bufferSize = 256;
  char buffer[bufferSize];
  printf("ftserver trying to read from socket\n");
  while(strstr(message,"@") == NULL) {
    memset(buffer, '\0', bufferSize);
    charsRead = recv(establishedConnectionFD, buffer, bufferSize - 1, 0); // Read the client's message from the socket
    if (charsRead < 0) {
      error("ERROR reading from socket");
      break;
    } else {
      strcat(message,buffer);
    }
  }
  //null terminate message
  int end;
  end = strstr(message,"@") - message;
  if (end) {
    message[end] = '\0';
  }
}

/************************
 * execute(): try to execute the command
 * returns the result of the command as a string or an error
 * **********************/
void execute(char* message,char* result) {
  FILE* inputFile;
  long fileBytes;
  DIR* dir;
  struct dirent* directory;
  char* token = 0;
  char* tempDir = 0;
  const char space[2] = " ";


  tempDir = malloc(sizeof(char) * 256);
  memset(tempDir,'\0',256);

  if (message[1] == 'l') {
    dir = opendir(".");
    if (dir) {
      while ((directory = readdir(dir)) != NULL) {
        memset(tempDir,'\0',256);
        sprintf(tempDir,"%s ",directory->d_name);
        strcat(result,tempDir);
      }
    } else {
      strcpy(result,"Error: could not get directory list\n");
    }
  } else {
    token = strtok(message,space);
    token = strtok(NULL,space);
    inputFile = fopen(token, "r");
    if (inputFile < 0) {
      strcpy(result,"Error: could not open file\n");
    } else {
      fseek(inputFile, 0L, SEEK_END);
      fileBytes = ftell(inputFile);
      fseek(inputFile,0L,SEEK_SET);
      fread(result,sizeof(char),fileBytes,inputFile);
      fclose(inputFile);
    }
  }
}

/**********************
 * sendMessage(): in this function, this program is the client
 * ********************/
void sendMessage(char* hostName, int port, char* msg) {
  int socketFD;
  struct sockaddr_in serverAddress;
  struct hostent* serverHostInfo;

  // Set up the server address struct
  memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
  serverAddress.sin_family = AF_INET; // Create a network-capable socket
  serverAddress.sin_port = htons(port); // Store the port number
  serverHostInfo = gethostbyname(hostName); // Convert the machine name into a special form of address
  if (serverHostInfo == NULL) { 
    errorx("ERROR, no such host\n"); 
  }
  memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

  // create the socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0) { 
    error("ERROR opening socket\n");
  }

  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    char errorMsg[50];
    memset(errorMsg,'\0',50);
    sprintf(errorMsg,"Error: could not contact %s on port %d\n",hostName, port);
    fprintf(stderr,errorMsg);
    fflush(stderr);
  }

  //write to socket
  int totalSent = 0;
  int charsWritten = -5; 
  while (totalSent < strlen(msg)) {
    charsWritten = send(socketFD, msg, strlen(msg), 0); // Write to the server
    totalSent += charsWritten;
  }

  //close the socket
  close(socketFD); 
}

/**********************
 * readCommands()
 * returns 0 for errors
 **********************/
int readCommands(char* message, char* clientName, int transferPort) {
  int read = 1;
  char* result = 0;
  int maxLength = 10000;
  result = malloc(sizeof(char) * (maxLength + 1));
  memset(result,'\0',(maxLength));

  //check for valid path length and command
  if (message[0] != '-' && (message[1] != 'g' || message[1] != 'l')) { 
    read = 0;
  } else {
    execute(message,result);
    printf("ftserver command result: %s",result);
    sleep(2);
    printf("ftserver sending result to client\n");
    sleep(2);
    sendMessage(clientName,transferPort,result);
  }

  if (result) {
    free(result);
  }

  return read;
}

/************************
 * getClientName()
 * https://beej.us/guide/bgnet/html/multi/getnameinfoman.html
 * puts hostname in parameter
 ************************/
void getClientName(char* clientName, struct sockaddr_in clientAddress) {
  char service[20];
  char host[50];
  struct sockaddr* clientAddressPtr = (struct sockaddr*)&clientAddress; //must cast to regular sockaddr
  getnameinfo(clientAddressPtr, sizeof(clientAddress), host, sizeof(host), service, sizeof(service), 0);
  strcpy(clientName,host);
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
  struct sockaddr_in clientAddress = acceptConnection(atoi(argv[1]),&controlConnection);

  //read transfer port from client
  char portBuffer[10];
  memset(portBuffer,'\0',10);
  readSocket((controlConnection.connectionFD),portBuffer); 

  int transferPort; 
  char* command; 
  int cmdLength = 260;
  if (portBuffer[0] != '\0') { 
    transferPort = atoi(portBuffer);
    command = malloc(cmdLength * sizeof(char));
    memset(command,'\0',cmdLength);
  } else {
    printf("Error. Bad response from client\n");
    exit(1);
  }

  //get the client hostname
  char clientname[1025];
  memset(clientname,'\0',1025);
  getClientName(clientname,clientAddress);
  printf("ftserver read clientname as %s\n",clientname);


  //read the command from client
  readSocket((controlConnection.connectionFD),command);

  //CLOSE SOCKET HERE
  //close(controlConnection.connectionFD);

  printf("ftserver received command %s", command);

  char* error = 0;
  error = malloc(sizeof(char) * 50);
  memset(error,'\0',50);
  sprintf(error,"ftserver Error: invalid command - ");

  //get commands from client
  if (!readCommands(command,clientname,transferPort)) {
    printf("%s%s\n",error,command);
  }

  free(error);
  free(command);
}
