package GUI.devicecontainer;

import GUI.DeviceContainer;
import GUI.device.KEY;
import javafx.scene.layout.FlowPane;

public class KeyContainer extends DeviceContainer {


    private int keyNum = 4;
    private final KEY[] keys;
    private final FlowPane content;

    public KeyContainer() {
        super("Push Buttons", new FlowPane());

        content = new FlowPane(2, 2);
        keys = new KEY[keyNum];

        for (int i = keyNum - 1; i >= 0; i--) {
            keys[i] = new KEY(i);
            content.getChildren().add(keyNum - i - 1, keys[i]);
        }


        super.setContent(content);
    }


    /**
     * Reset all push buttons when simulation stops
     */

    public void stop() {
        for(int i=0; i < keyNum; i++){
            keys[i].setStop();
        }
    }

    /**
     * Setters and Getters
     */
    public void setKeyNum(int keyNum) {
        this.keyNum = keyNum;
    }
}
