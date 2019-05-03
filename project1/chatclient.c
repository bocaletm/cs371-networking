/*********************
 * Mario Bocaletti
 * cs371
 * chatclient client that encodes text
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define h_addr h_addr_list[0]   


//global message string limits
const int NAME_LIMIT = 10;
const int MSG_LIMIT = 512;
const int RAW_MSG_LIMIT = 500;

/******************
 * listeningThread()
 * reads from the socket and prints to stdout
 * ***************/
void* listeningThread(){
  char buffer[MSG_LIMIT];
  while(1) {
    // Get return message from server
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer for reuse
    while(strstr(buffer,"\n") == NULL) {
      charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
    }
    if (charsRead < 0) {
      close(socketFD); // Close the socket
      errorx("CLIENT: ERROR reading from socket\n");
    } else {
      printf("%s", buffer);
      fflush(stdout);
    }
  }
  return NULL;
}

/********************
 * checkMem(): checks if malloc
 * succeeded
 * ******************/
void checkMem(char* ptr,int line) {
  if (ptr == 0) {
    printf("Malloc error in line %d-ish.",line);
    exit(1);
  }
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

/**************************
 * getName()
 * ***********************/
void getName(char* name) {
  size_t charsEntered = 0;
  size_t bufferSize = 1000;
  char* buffer = malloc(bufferSize * sizeof(char));
  memset(buffer,'\0',bufferSize);

  printf("\t==> Welcome to chatclient...\n\t\tHandle: ");
  charsEntered = getline(&buffer, &bufferSize, stdin);
  //exit early if length is invalid
  while (charsEntered > NAME_LIMIT + 1) {
    printf("\n\tHandle too long. Try a handle up to 10 characters...\n\t\tHandle: ");
    charsEntered = getline(&buffer, &bufferSize, stdin);
  }
  //omit the newline entered
  snprintf(name,(size_t)(charsEntered),"%s",buffer);
  free(buffer);
}

/**************************
 * getMsg()
 * ***********************/
void getMsg(char* name,char* msg) {
  size_t charsEntered = 0;
  size_t bufferSize = 1000;
  char* buffer = malloc(bufferSize * sizeof(char));
  memset(buffer,'\0',bufferSize);
  //prompt
  printf("%s: ",name);
  charsEntered = getline(&buffer, &bufferSize, stdin);
  //exit early if length is invalid (+1 for newline)
  while (charsEntered > RAW_MSG_LIMIT + 1) {
    printf("\n\tMessage too long. Try a message up to %d characters...\n\n%s: ",RAW_MSG_LIMIT,name);
    charsEntered = getline(&buffer, &bufferSize, stdin);
  }
  //an extra char to null terminate
  snprintf(msg,(size_t)(MSG_LIMIT),"%s> %s",name,buffer);
}

/************************
 * sendPort():  
 * sends port to the server
 *************************/
int sendPort(int socketFD,int port) {
  int totalSent = 0; 
  int charsWritten = 0;
  int error = 0;
  while (totalSent < strlen(itoa(port))) {
    charsWritten = send(socketFD, itoa(port), strlen(itoa(port)), 0); // Write to the server
    totalSent += charsWritten;
    if (charsWritten < 0) {
      error("CLIENT: ERROR writing to socket");
      error = 1;
    }
    if (charsWritten < strlen(itoa(port))) {
      error("CLIENT: WARNING: Not all data written to socket!\n");
      error = 1;
    }
  }
  return error;
}

/************************
 * socketConnect():  
 * try to connect to the server
 *************************/
void socketConnect(char* hostName, int port, char* name, char* msg) {
  int socketFD, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  struct hostent* serverHostInfo;
  char buffer[MSG_LIMIT];

  // Set up the server address struct
  memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
  serverAddress.sin_family = AF_INET; // Create a network-capable socket
  serverAddress.sin_port = htons(port); // Store the port number
  serverHostInfo = gethostbyname(hostName); // Convert the machine name into a special form of address
  if (serverHostInfo == NULL) { 
    errorx("CLIENT: ERROR, no such host\n"); 
  }
  memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

  // Set up the socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
  if (socketFD < 0) { 
    error("CLIENT: ERROR opening socket\n");
  }

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    char errorMsg[50];
    memset(errorMsg,'\0',50);
    sprintf(errorMsg,"Error: could not contact %s on port %d\n",hostName, port);
    fprintf(stderr,errorMsg);
    fflush(stderr);
  }

  // Send the port 
  int successfulPeerConnect = sendPort(socketFD,port);

  if (successfulPeerConnect) {
    //read from peer in separate thread
    pthread_t thread;
    int result_code = 1;
    result_code = pthread_create(&thread,NULL,listeningThread,NULL);
    if (result_code != 0) {
      error("Error with listening thread\n");
    } 
    //write to peer
    while(1) {  
      //get the message 
      getMsg(name,msg);

      // Send message to server
      int totalSent = 0; 
      while (totalSent < strlen(msg)) {
        charsWritten = send(socketFD, msg, strlen(msg), 0); // Write to the server
        totalSent += charsWritten;
        if (charsWritten < 0) {
          error("CLIENT: ERROR writing to socket");
        }
        if (charsWritten < strlen(buffer)) {
          error("CLIENT: WARNING: Not all data written to socket!\n");
        }
      }
    }

  } else {
    printf("Could not connect to peer\n");
    fflush(stdout);
  }
  close(socketFD); // Close the socket
}

/**************************
 * MAIN
 * ************************/
int main(int argc, char *argv[]) {

  // Check usage & args
  if (argc < 3) { 
    errorx("chatclient: missing args\n"); 
  } else if (argc > 3) {
    errorx("chatclient: too many args\n"); 
  }

  //vars to hold messages
  char* name = malloc((NAME_LIMIT + 1) * sizeof(char));
  memset(name,'\0',(NAME_LIMIT + 1));
  checkMem(name,146);

  char* rawMsg = malloc((RAW_MSG_LIMIT + 1) * sizeof(char));
  memset(rawMsg,'\0',(RAW_MSG_LIMIT + 1));
  checkMem(rawMsg,152);

  char* msg = malloc((MSG_LIMIT + 1) * sizeof(char));
  memset(msg,'\0',(MSG_LIMIT + 1));
  checkMem(msg,154);

  while (1) {
    getName(name);
    printf("\nTrying to connect to %s on port %s...\n\n",argv[1],argv[2]);
    socketConnect(argv[1],atoi(argv[2]),name,msg);
  }
  return 0;
}
