package GUI.device;

import Controller.Message;
import GUI.Main;
import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Button;
import javafx.scene.image.PixelWriter;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;


public class VGA extends VBox {

    private final PixelWriter pixelWriter;
    private final Canvas canvas;
    private final Button nextFrameBtn;
    private static String projDir;
    private int screenHeight;
    private int screenWidth;
    
    public VGA() {

        super();
        
        this.screenHeight = 480;
        this.screenWidth = 640;
        //----------------------------------------------------------------------------------
        nextFrameBtn = new Button("Next Frame");
        nextFrameBtn.setOnAction(new EventHandler<>() {
            @Override
            public void handle(ActionEvent e) {
                Main.connector.sendSignal("next");
                // proceed one frame at a time
                nextFrameBtn.setDisable(true);

            }
        });

        //----------------------------------------------------------------------------------
        canvas = new Canvas();
        this.getChildren().addAll(nextFrameBtn, canvas);
        canvas.setHeight(screenHeight);
        canvas.setWidth(screenWidth);
        zoomCanvas((float) 0.5);

        GraphicsContext graphicsContext = canvas.getGraphicsContext2D();
        graphicsContext.setFill(Color.BLACK);
        graphicsContext.fillRect(0, 0, screenWidth, screenHeight);

        pixelWriter = graphicsContext.getPixelWriter();
    }


    public void updateFrame() {
        try {
            File f = new File(projDir, "ModelSim/demo.txt");
            FileReader imgFile = new FileReader(f);

            int pixel;
            int xCounter = 0, yCounter = 0;

            // TODO: parameter: number of color bits
            while ((pixel = imgFile.read()) != -1) {
                pixel = pixel - '0';
                Color color = Color.rgb(255 * ((pixel >> 2) & 1), 255 * ((pixel >> 1) & 1), 255 * (pixel & 1));
                pixelWriter.setColor(xCounter, yCounter, color);
                xCounter++;

                if (xCounter == screenWidth) {
                    xCounter = 0;
                    yCounter++;
                }
            }

        } catch (FileNotFoundException e) {
            try {
                Main.messageBox.addMessage("VGA file not found.", Message.WARNING);
            }catch(IllegalStateException err){
                Platform.runLater(()->{
                    Main.messageBox.addMessage("VGA file not found. Please check your project directory.", Message.WARNING);
                });
            }
//            System.out.println("File not found: [path to project dir]/ModelSim/demo.txt, please check project path");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    public void set_pixel_160x120(int x, int y, char pixel) {
        //This assumes that the Verilog code is using 160x120 resolution,
        //so it automatically scales up to the 640x480 canvas
        int col = pixel - '0';
        Color color = Color.rgb(255 * ((col >> 2) & 1), 255 * ((col >> 1) & 1), 255 * (col & 1));

        x *= 4; y*= 4;
		
		
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                pixelWriter.setColor(x+i, y+j, color);
            }
        }
    }


    // TODO: Add button for VGA screen rescaling
    public void zoomCanvas(float scale) {
        canvas.setScaleX(scale);
        canvas.setScaleY(scale);
        if (scale < 1) {
            canvas.setTranslateX(320 * (scale - 1) + 20);
            canvas.setTranslateY(240 * (scale - 1) + 20);
        }
    }



    public void setDisableNextFrame(boolean status) {
        nextFrameBtn.setDisable(status);
    }

    public static void setProjDir(String path) {
        projDir = path;
    }

    public void setScreenHeight(int screenHeight){
        this.screenHeight = screenHeight;
    }

    public void setScreenWidth(int screenWidth){
        this.screenWidth = screenWidth;
    }
}
