package GUI.devicecontainer;

import GUI.DeviceContainer;
import GUI.device.SevenSeg;
import javafx.scene.layout.FlowPane;

public class SevenSegContainer extends DeviceContainer {

    private int segNum = 6;
    private final SevenSeg[] sevenSegments;
    private final FlowPane content;

    public SevenSegContainer() {
        super("Seven-segment Displays", new FlowPane());

        content = new FlowPane(4, 2);
        sevenSegments = new SevenSeg[segNum];

        for (int i = segNum - 1; i >= 0; i--) {
            sevenSegments[i] = new SevenSeg();
            content.getChildren().add(segNum - i - 1, sevenSegments[i]);
        }


        super.setContent(content);
    }

    /**
     * Reset all seven seg when simulation stops
     */
    public void stop(){
        for(int index=0; index < segNum; index++) {
            sevenSegments[index].setColor(false, 0);
        }
    }


    /**
     * Setters and Getters
     */
    public void setSegNum(int segNum) {
        this.segNum = segNum;
    }


    public void setSevenSegments(int index, boolean signal, int status){
        if(index < 0 || index > segNum){
            throw new IndexOutOfBoundsException("Invalid 7-Seg index\n");
        }
        sevenSegments[index].setColor(signal, status);
    }

}

