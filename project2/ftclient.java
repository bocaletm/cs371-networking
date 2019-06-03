/**************************
 * Maintainer: Mario Bocaletti
 * Description: ftclient receives file contents
 * over a TCP socket connection
 * Class: cs371 Networking
 * Last Modified: 6-2-19 
 * ***********************/

//Socket code based on example here: https://docs.oracle.com/javase/tutorial/displayCode.html?code=https://docs.oracle.com/javase/tutorial/networking/sockets/examples/EchoClient.java

import java.net.*;
import java.io.*;
import java.io.DataInputStream;
import java.nio.ByteBuffer;
import java.lang.Object;

public class ftclient {
  public static void main(String[] args) throws IOException, InterruptedException {
    if (args.length < 4 || args.length > 5) {
      System.err.println("Usage: java ftclient - argument count incorrect");
      System.exit(1);
    }
    //socket variables
    String serverHost = args[0];
    int serverPort = Integer.parseInt(args[1]);
    String command = args[2];
    int transferPort;
    String fileName;
    String message;
    
    //handle different usage 
    if (args.length == 4) {
      transferPort = Integer.parseInt(args[3]);
      message = command;
    } else {
      fileName = args[3];
      transferPort = Integer.parseInt(args[4]);
      message = command + " " + fileName;
    }
    System.out.println("calling ConnectSocket");
    //handle server interaction
    ConnectSocket connection = new ConnectSocket(serverHost,serverPort,transferPort,message);
    System.out.println("calling connect");
    connection.connect();
  } 
}
