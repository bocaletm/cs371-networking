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

//Socket code based on example here: https://docs.oracle.com/javase/tutorial/displayCode.html?code=https://docs.oracle.com/javase/tutorial/networking/sockets/examples/EchoClient.java
public class ftclient {
  public static void main(String[] args) throws IOException {
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

    if (args.length == 4) {
      transferPort = Integer.parseInt(args[3]);
    } else {
      fileName = args[3];
      transferPort = Integer.parseInt(args[4]);
    }
   
    try ( 
      PrintWriter out = new PrintWriter(serverSocket.getOutputStream(), true);
      BufferedReader in = new BufferedReader(new InputStreamReader(serverSocket.getInputStream()));
      BufferedReader stdIn = new BufferedReader(new InputStreamReader(System.in));
      Socket serverSocket = new Socket(serverHost,serverPort);
    ) {
      out.println(command + " " + fileName);
      waitForResponse(
    }
  } 
}
