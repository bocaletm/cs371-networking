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
#include <pthread.h>

#define h_addr h_addr_list[0]   


//global message string limits
const int NAME_LIMIT = 10;
const int MSG_LIMIT = 512;
const int RAW_MSG_LIMIT = 500;
//pointer to the name that the thread can access
char* NAME = 0;
//flag the thread sets upon termination
int threadQuit = 0;

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

/******************
 * listeningThread()
 * reads from the socket and prints to stdout
 * sets global flag threadQuit if quit is received
 * ***************/
void* listeningThread(void* socketFD){
  int fd = *((int*)socketFD);
  char buffer[MSG_LIMIT + 20];
  int charsRead = 0;
  while(1) {
    // Get return message from server
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer for reuse
    while(strstr(buffer,"\n") == NULL) {
      charsRead = recv(fd, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
    }
    if (charsRead < 0) {
      close(fd); // Close the socket
      errorx("CLIENT: ERROR reading from socket\n");
    } else {
      if (strstr(buffer,"\\quit") != NULL) {
        threadQuit =  1;
        printf("\nReceived \\quit... Disconnecting from server...\n");
        break;
      } else {
        printf("\n%s", buffer);
        fflush(stdout);
        //reset the prompt
        printf("%s: ",NAME);
        fflush(stdout);
      }
    }
  }
  return NULL;
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
 * returns 1 if /quit is typed
 * ***********************/
int getMsg(char* name,char* msg) {
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
  snprintf(msg,(size_t)(MSG_LIMIT),"%s> %s",name,buffer);
  free(buffer);
  if (strstr(msg,"\\quit") != NULL) {
    return 1;
  } else {
    return 0;
  }
}

/************************
 * sendPort():  
 * sends port to the server
 * returns 1 if no error is found
 *************************/
int sendPort(int socketFD,int port) {
  int totalSent = 0; 
  int charsWritten = 0;
  int errorCode = 1;
    //convert port to string
  char stringPort[7];
  memset(stringPort,'\0',7);
  snprintf(stringPort,sizeof(stringPort),"%d\n",port);
  while (totalSent < strlen(stringPort)) {
      //write the port to the server
    charsWritten = send(socketFD, stringPort, strlen(stringPort), 0); 
    totalSent += charsWritten;
  }
  return errorCode;
}

/************************
 * socketConnect():  
 * try to connect to the server
 *************************/
void socketConnect(char* hostName, int port, char* name, char* msg) {
    //handles sending quit message
  int quit = 0;
  int socketFD, charsWritten;
  struct sockaddr_in serverAddress;
  struct hostent* serverHostInfo;

  // Set up the server address struct
  memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
  serverAddress.sin_family = AF_INET; // Create a network-capable socket
  serverAddress.sin_port = htons(port); // Store the port number
  serverHostInfo = gethostbyname(hostName); // Convert the machine name into a special form of address
  if (serverHostInfo == NULL) { 
    errorx("CLIENT: ERROR, no such host\n"); 
  }
  memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

  // create the socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
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
  int result_code = 1; //for thread creation
  pthread_t thread;
  if (successfulPeerConnect) {
    //read from peer in separate thread
    result_code = pthread_create(&thread,NULL,listeningThread,&socketFD);
    if (result_code != 0) {
      errorx("Error with listening thread\n");
    } 
    //write to peer
    while(1) {  
      //get the message 
      quit = getMsg(name,msg);
      // Send message to server
      int totalSent = 0; 
      while (totalSent < strlen(msg)) {
        charsWritten = send(socketFD, msg, strlen(msg), 0); // Write to the server
        totalSent += charsWritten;
      }
      if (quit || threadQuit) {
        //program will exit
        break;
      }
    }
  } else {
    printf("Could not connect to peer\n");
    fflush(stdout);
  }
    //close listening thread
  pthread_cancel(thread);
    //close the socket
  close(socketFD); 
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
  checkMem(name,222);

  char* msg = malloc((MSG_LIMIT + 1) * sizeof(char));
  memset(msg,'\0',(MSG_LIMIT + 1));
  checkMem(msg,230);

  getName(name);
  //make the name globally accessible
  NAME = name;
  printf("\nTrying to connect to %s on port %s...\n\n",argv[1],argv[2]);
  socketConnect(argv[1],atoi(argv[2]),name,msg);

  free(name);
  free(msg);
  
  printf("\nExiting successfully...\n");

  return 0;
}
