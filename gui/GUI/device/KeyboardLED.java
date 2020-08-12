package GUI.device;

import javafx.scene.control.Button;

public class KeyboardLED extends Button {
    // true if LED is on, false if LED is off
    private boolean status;

    public KeyboardLED() {
        this.setDisable(true);
        this.status = false;
        this.setDisplay();
        this.setMaxSize(20,10);
        this.setMinSize(20,10);
    }

    private void setDisplay(){
        if(! this.status){
            this.setStyle(" -fx-border-color:black; -fx-background-color: aliceblue;");
        }else{
            this.setStyle(" -fx-background-color: red;");
        }
    }

    /**
     * Setters and Getters
     */
    public void setStatus(boolean status){
        this.status = status;
        this.setDisplay();
    }

}

