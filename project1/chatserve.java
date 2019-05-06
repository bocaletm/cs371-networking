/******************
 * Mario Bocaletti
 * OSU cs371 Networking
 * Basic Chat Server
 * ****************/

import java.net.*;
import java.io.*;
import java.io.DataInputStream;
import java.nio.ByteBuffer;
import java.lang.Object;

//Socket code based on example here: https://docs.oracle.com/javase/tutorial/displayCode.html?code=https://docs.oracle.com/javase/tutorial/networking/sockets/examples/EchoClient.java
public class chatserve {
  public static void main(String[] args) throws IOException {
    if (args.length != 1) {
      System.err.println("Usage: java chatserve - argument count is not 1");
      System.exit(1);
    }
    //message variables
    String clientPort = "Error";
    String inputLine = "";
    boolean firstMessage = true;
    //socket variables
    int port = Integer.parseInt(args[0]);
    ServerSocket serverSocket;
    Socket clientSocket;
    //socket IO buffers
    PrintWriter outputBuffer; 
    BufferedReader inputBuffer;  
    //thread to handle writing back to client
    SpeakingThread myThread; 
    while (true) {
      System.out.println("\nServer waiting for a connection on port " + port);
      serverSocket = new ServerSocket(port);
      clientSocket = serverSocket.accept();
      outputBuffer = new PrintWriter(clientSocket.getOutputStream(), true);
      inputBuffer = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
      myThread = new SpeakingThread(outputBuffer);
      try {
        while ((inputLine = inputBuffer.readLine()) != null) {
          if (firstMessage) {
            System.out.println("Now connected to " + inputLine + ". Ready to receive IMs...\n");
            System.out.flush();
            firstMessage = false;
            myThread.start();
          } else if (inputLine.contains("\\quit") || !myThread.isAlive()) {
            System.out.println("\nDisconnecting from client due to \\quit command...");
            myThread.interrupt();
            myThread = null;
            serverSocket.close();
            serverSocket = null;
            clientSocket.close();
            clientSocket = null;
            outputBuffer = null;
            inputBuffer = null;
            System.gc();
            firstMessage = true;
            break;
          } else {
            System.out.println();
            System.out.println(inputLine);
            //print the prompt again so incoming messages don't hide it
            System.out.print(Globals.SERVER_HANDLE + ": ");
          }
        }
      } catch (IOException e) {
        System.out.println("Exception caught when trying to listen on port " + port + " or listening for a connection");
        System.out.println(e.getMessage());
      }
    }
  }
}
