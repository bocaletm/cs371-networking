import java.net.*;
import java.io.*;

public class chatserve {
  public static void main(String[] args) throws IOException {
    if (args.length != 1) {
      System.err.println("Usage: java chatserve - argument count is not 1");
      System.exit(1);
    }

    int port = Integer.parseInt(args[0]);
    String name = "Error";
    String inputLine = "";
    boolean firstMessage = true;
    ServerSocket serverSocket = new ServerSocket(port);
    Socket clientSocket = serverSocket.accept();
    PrintWriter outputBuffer = new PrintWriter(clientSocket.getOutputStream(), true);
    BufferedReader inputBuffer = new BufferedReader(new InputStreamReader(clientSocket.getInputStream().readFully()));
    SpeakingThread myThread = new SpeakingThread(outputBuffer);
    //TODO: kill thread and exit loop when /quit is received
    while (true) {
      name = "Error";
      firstMessage = true;
      try {
        myThread.start();
        while ((inputLine = inputBuffer.readLine()) != null) {
          if (firstMessage) {
            name = inputLine.substring(0,Math.min(inputLine.length(),9));
            firstMessage = false;
          } else {
            System.out.println(name + "> " + inputLine);
            System.out.flush();
          }
        }
      } catch (IOException e) {
        System.out.println("Exception caught when trying to listen on port " + port + " or listening for a connection");
        System.out.println(e.getMessage());
      }
    }
  }
}
