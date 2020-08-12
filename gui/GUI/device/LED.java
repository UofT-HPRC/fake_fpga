package GUI.device;

import javafx.scene.Node;
import javafx.scene.shape.Rectangle;
import javafx.scene.paint.Color;

public class LED extends Rectangle {
    // true if LED is on, false if LED is off
    private boolean status;

    public LED() {
		super(15,15);
		this.setStroke(Color.GRAY);
		this.setArcHeight(4);
		this.setArcWidth(4);
        this.setDisable(true);
        this.status = false;
        this.setDisplay();
    }

    private void setDisplay(){
        if(! this.status){
            this.setFill(Color.ALICEBLUE);
        }else{
            this.setFill(Color.RED);
        }
    }

    /**
     * Setters and Getters
     */
    public void toggle(){
        this.status = !this.status;
        this.setDisplay();
    }

    public void setStatus(boolean status){
        this.status = status;
        this.setDisplay();
    }

    public boolean getStatus(){
        return this.status;
    }

}
