package GUI.devicecontainer;

import GUI.DeviceContainer;
import GUI.device.Switch;
import javafx.scene.layout.FlowPane;


public class SwitchContainer extends DeviceContainer {

    private int switchNum = 10;
    private final Switch[] switches;
    private final FlowPane content;

    public SwitchContainer(){
        super("Switches",  new FlowPane());

        content = new FlowPane(2, 2);
        switches = new Switch[switchNum];

        for(int i=switchNum - 1; i >= 0; i--) {
            switches[i] = new Switch(i);
            content.getChildren().add(switchNum - i -1, switches[i]);
        }


        super.setContent(content);
    }


    public void stop(){
        for(int i=0; i < switchNum; i++){
            switches[i].setStop();
        }
    }

    /**
     *  Setters and Getters
    */
    public void setSwitchNum(int switchNum) {
        this.switchNum = switchNum;
    }
}
