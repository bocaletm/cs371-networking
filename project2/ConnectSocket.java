/**************************
 * Maintainer: Mario Bocaletti
 * Description: ftclient receives file contents
 * over a TCP socket connection
 * Class: cs371 Networking
 * Last Modified: 6-2-19 
 * ***********************/
//SOURCE FOR SLEEP METHOD: https://stackoverflow.com/questions/24104313/how-do-i-make-a-delay-in-java

//Socket code based on example here: https://docs.oracle.com/javase/tutorial/displayCode.html?code=https://docs.oracle.com/javase/tutorial/networking/sockets/examples/EchoClient.java

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.concurrent.TimeUnit;

public class ConnectSocket {
  private String serverHost;
  private int serverPort;
  private int transferPort;
  private String message;
  public ConnectSocket(String sh,int sp,int tp,String msg) {
    this.serverHost = sh;
    this.serverPort = sp;
    this.transferPort = tp;
    this.message = msg;
  }
  public void connect() throws UnknownHostException, IOException, InterruptedException {
    try( Socket controlSocket = new Socket(serverHost,serverPort);
        PrintWriter out = new PrintWriter(controlSocket.getOutputStream(), true);
        BufferedReader in = new BufferedReader(new InputStreamReader(controlSocket.getInputStream()));
       ) {
      ReceiveResponse responseGetter;
      String serverResponse = "";
      //send the transfer port to the server
      out.println(transferPort + "@");
      //give the server a second to process errors
      TimeUnit.SECONDS.sleep(2);
      out.println(message + "@");
      //only wait for data if there was no error with the request
      responseGetter = new ReceiveResponse(transferPort);
      responseGetter.startListening();
      controlSocket.close();
    } catch (UnknownHostException e) {
      System.out.println("Error. Unknown host trying to connect to " + serverHost);
    } catch (IOException e) {
      System.out.println("Error. IO after connection to " + serverHost);
    } catch (InterruptedException e) {
      System.out.println("Error. Interrupted while waiting.");
    } 
  }
}
