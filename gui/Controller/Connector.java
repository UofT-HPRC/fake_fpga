package Controller;

import GUI.Main;
import javafx.application.Platform;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Connector implements Runnable {

    private Socket socket;

    /**
     * Run server
     */
    @Override
    public void run() {
        // starts the server
        try (ServerSocket listener = new ServerSocket(54321)) {
            Main.messageBox.addMessage("The server is running...", Message.INFO);

            // set # of clients to 1
            ExecutorService pool = Executors.newFixedThreadPool(1);

            // listen for connection
            while (true) {
                socket = listener.accept();
                pool.execute(new SigReceiver(socket));
            }
        } catch (IOException e) {
            Main.messageBox.addMessage("Server setup failed.", Message.ERROR);
        }

    }

    public void sendStartSignal() throws Exception{
        PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
        // send signal
        out.println("start");
    }


    public void sendSignal(String signal) {
        try {
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);

            // send signal
            out.println(signal);
            System.out.println("Sent signal: " + signal);

        }
        // stop Signal Receiver if socket closed
        catch (IOException | NullPointerException ignored) {
        }
    }


    /**
     * Receive and process simulator outputs
     */
    private static class SigReceiver implements Runnable {
        private final Socket socket;

        SigReceiver(Socket socket) {
            this.socket = socket;
        }

        SignalParser signalParser = new SignalParser();

        @Override
        public void run() {
            try {
                Scanner in = new Scanner(socket.getInputStream());
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true);

                out.println("Server waiting for output ...");

                Platform.runLater(
                        () -> {
                            // Update message box
                            Main.messageBox.addMessage("Connected to simulator.", Message.INFO);
                        }
                );

                while (in.hasNextLine()) {
                    signalParser.processSignal(in.nextLine());
                }
            } catch (Exception e) {
                e.printStackTrace();
                System.out.println("Connection is broken.");
            } finally {
                try {
                    socket.close();
                } catch (IOException ignored) {
                }
                System.out.println("Disconnect " + socket);
                Platform.runLater(
                        () -> {
                            // Update message box
                            Main.messageBox.addMessage("Disconnected from simulator.", Message.WARNING);
                        }
                );
            }
        }
    }
}
