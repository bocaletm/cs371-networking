import java.io.*;
import java.lang.Object;

public class SpeakingThread extends Thread {
  private PrintWriter buffer;
  public SpeakingThread(PrintWriter buffer) {
    this.buffer = buffer;
  }
  @Override
  public void run() {
    try (
      BufferedReader stdIn = new BufferedReader(new InputStreamReader(System.in))
    ) {
      String userInput;
      System.out.print(Globals.SERVER_HANDLE + ": ");
      while ((userInput = stdIn.readLine()) != null) {
        this.buffer.println(Globals.SERVER_HANDLE + "> " + userInput);
        System.out.print(Globals.SERVER_HANDLE + ": ");
      }
    } catch (IOException e) {
      System.err.println("SpeakingThread IO error");
    }
  }
}

