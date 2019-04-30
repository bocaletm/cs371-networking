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
  int charsEntered = 0;
  int bufferSize = 1000;
  char buffer[bufferSize];
  memset(buffer,'\0',bufferSize);

  printf("==>\tWelcome to chatclient...\n\t\tHandle: ");
  charsEntered = getline(&buffer, &bufferSize, stdin);
    /*exit early if length is invalid*/
  while (charsEntered > NAME_LIMIT) {
    printf("\n\tHandle too long. Try a handle up to 10 characters...\n\t\tHandle: ");
    charsEntered = getline(&buffer, &bufferSize, stdin);
  }
  snprintf(name,(size_t)NAME_LIMIT,"%s",buffer);
}

/**************************
 * getMsg()
 * ***********************/
void getMsg(char* name,char* msg) {
  int charsEntered = 0;
  int bufferSize = 1000;
  char buffer[bufferSize];
  memset(buffer,'\0',bufferSize);

  printf("%s: ",name);
  charsEntered = getline(&buffer, &bufferSize, stdin);
    /*exit early if length is invalid*/
  while (charsEntered > RAW_MSG_LIMIT) {
    printf("\n\tMessage too long. Try a message up to 10 characters...\n\t\t%s: ",name);
    charsEntered = getline(&buffer, &bufferSize, stdin);
  }
  //an extra char to null terminate
  snprintf((msg,(size_t)(MSG_LIMIT + 1)),"%s> %s",name,buffer);
}

/************************
 * socketConnect():  
 * try to connect to the server
 *************************/
void socketConnect(int port, char* msg) {
  int socketFD, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  struct hostent* serverHostInfo;
  char buffer[MSG_LIMIT];

  // Set up the server address struct
  memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
  serverAddress.sin_family = AF_INET; // Create a network-capable socket
  serverAddress.sin_port = htons(port); // Store the port number
  serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
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
    sprintf(errorMsg,"Error: could not contact otp_enc_d on port %d\n",port);
    fprintf(stderr,errorMsg);
    fflush(stderr);
  }
  
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

  // Get return message from server
  memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse

  while(strstr(buffer,"@") == NULL) {
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
  }

  if (charsRead < 0) {
    close(socketFD); // Close the socket
    errorx("CLIENT: ERROR reading from socket\n");
  }
  if (buffer[0] == '@') {
    close(socketFD); // Close the socket
    error("CLIENT: server could not encrypt message\n");
    exit(2);
  } else {
    //remove end of message and print
    buffer[strcspn(buffer, "@")] = '\0';
    printf("%s\n", buffer);
    fflush(stdout);
  }

  close(socketFD); // Close the socket
}

/**************************
 * MAIN
 * ************************/
int main(int argc, char *argv[]) {

  // Check usage & args
  if (argv[2] == NULL) { 
    errorx("chatclient: wrong number of args\n"); 
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
    getMsg(name,msg);
    printf("\n%s : %s",name,msg);
    //socketConnect(name,rawMsg,msg,atoi(argv[1]),argv[2]);
  }
  //compose msg including authentication prefix
  //snprintf(msg,(size_t)msgLimit,"e%s;%s@",key,plaintext);
  
  return 0;
}
