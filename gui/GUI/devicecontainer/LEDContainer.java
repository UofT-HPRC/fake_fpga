package GUI.devicecontainer;

import GUI.DeviceContainer;
import GUI.device.LED;
import javafx.scene.layout.FlowPane;

public class LEDContainer extends DeviceContainer {


    private int ledNum = 10;
    private final LED[] leds;
    private final FlowPane content;

    public LEDContainer(){
        super("LEDs",  new FlowPane());

        content = new FlowPane(2, 2);
        leds = new LED[ledNum];

        for(int i=ledNum - 1; i >= 0; i--) {
            leds[i] = new LED();
            content.getChildren().add(ledNum - i -1, leds[i]);
        }


        super.setContent(content);
    }


    /**
     * Reset all LEDs when simulation stops
     */
    public void stop(){
        for(int index = 0; index < ledNum; index++){
            setLED(index, false);
        }
    }

    /**
     *  Setters and Getters
     */
    public void setLEDNum(int ledNum) {
        this.ledNum = ledNum;
    }

    public void setLED(int index, boolean signal){
        if(index < 0 || index > ledNum){
            throw new IndexOutOfBoundsException("Invalid LED index\n");
        }
        leds[index].setStatus(signal);
    }

}
