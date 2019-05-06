import java.io.*;
import java.util.Scanner;
import java.lang.Object;

public class SpeakingThread extends Thread {
  private PrintWriter buffer;
  public SpeakingThread(PrintWriter buffer) {
    this.buffer = buffer;
  }
  @Override
  public void run() {
    Scanner scan = new Scanner(System.in);
    try {
      String userInput;
      System.out.print(Globals.SERVER_HANDLE + ": ");
      while ((!Thread.currentThread().isInterrupted()) && ((userInput = scan.nextLine()) != null)) {
        if (userInput.length() <= 500) {
          this.buffer.println(Globals.SERVER_HANDLE + "> " + userInput);
          if (userInput.contains("\\quit")) {
            break;
          }
        } else {
          System.out.println("\tMessage too long--not sent. Keep it under 500 characters.");
        }
        System.out.print(Globals.SERVER_HANDLE + ": ");
      }
    } finally {
      System.gc();
      return;
    }
  } 
}

