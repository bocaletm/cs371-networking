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
    byte[] message = new byte[256];
    boolean endMessage = false;
    boolean firstMessage = true;
    int bytesToRead = 0;
    int bytesReceived = -1;
      //socket variables
    int port = Integer.parseInt(args[0]);
    ServerSocket serverSocket = new ServerSocket(port);
    Socket clientSocket = serverSocket.accept();
      //socket IO buffers
    PrintWriter outputBuffer = new PrintWriter(clientSocket.getOutputStream(), true);
    DataInputStream inputBuffer = new DataInputStream(clientSocket.getInputStream());
      //thread to handle writing back to client
    SpeakingThread myThread = new SpeakingThread(outputBuffer);
    myThread.start();
    while (true) {
      try {
          // the client sends the length of its message in first two bytes
        message[0] = inputBuffer.readByte();
        message[1] = inputBuffer.readByte();
        ByteBuffer byteBuffer = ByteBuffer.wrap(message, 0, 2);
        bytesToRead = byteBuffer.getShort();
        System.out.println("received " + bytesToRead);
        while (bytesToRead > bytesReceived) {
          bytesReceived = inputBuffer.read(message);
          inputLine += new String(message,0,bytesReceived);
        }
        if (firstMessage) {
          System.out.println("Now connected to " + inputLine + ". Ready to receive IMs...");
          firstMessage = false;
        } else if (inputLine.contains("/quit")) {
          myThread.interrupt();
          System.exit(0);
        } else {
          System.out.println(inputLine);
        }
      } catch (IOException e) {
        System.out.println("Exception caught when trying to listen on port " + port + " or listening for a connection");
        System.out.println(e.getMessage());
      }
    }
  }
}
