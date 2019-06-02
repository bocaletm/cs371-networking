/**************************
 * Maintainer: Mario Bocaletti
 * Description: ftclient receives file contents
 * over a TCP socket connection
 * Class: cs371 Networking
 * Last Modified: 6-2-19 
 * ***********************/
//SOURCE: My own chatserve.java from Program 1 (bocaletm at GitHub)
import java.net.*;
import java.io.*;
import java.io.DataInputStream;
import java.nio.ByteBuffer;
import java.lang.Object;

public class ReceiveResponse {
  private int transferPort;
  public ReceiveResponse(int tp) {
    this.transferPort = tp;
  }
  public void startListening() throws IOException {
    //the client acts like a server to accept data from ftserver
    ServerSocket serverSocket;
    Socket clientSocket;
    //socket IO buffers
    PrintWriter outputBuffer; 
    BufferedReader inputBuffer;  
    serverSocket = new ServerSocket(transferPort);
    clientSocket = serverSocket.accept();
    outputBuffer = new PrintWriter(clientSocket.getOutputStream(), true);
    inputBuffer = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
    String inputLine = "";
    try {
      while ((inputLine = inputBuffer.readLine()) != null) {
        System.out.println();
        System.out.println(inputLine);
      }
    } catch (IOException e) {
      System.out.println("java ReceiveResponse IO Exception");
      System.out.println(e.getMessage());
    }
    clientSocket.close();
    serverSocket.close();
  } 
}
