package GUI.devicecontainer;

import GUI.DeviceContainer;
import GUI.device.GPIO;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.VBox;

public class GPIOContainer extends DeviceContainer {

    private int gpioNum = 32;
    private final GPIO[] ports;
    private final VBox content;

    public GPIOContainer() {
        super("Parallel Ports", new VBox());
        ports = new GPIO[gpioNum];

        FlowPane flowPane = new FlowPane(2, 2);
        FlowPane flowPane1 = new FlowPane(2, 2);
        content = new VBox();
        content.getChildren().addAll(flowPane, flowPane1);

        for (int i = gpioNum - 1; i >= gpioNum/2; i--) {
            ports[i] = new GPIO(i);
            flowPane.getChildren().add(ports[i]);
        }

        for (int i = gpioNum/2 - 1; i >= 0; i--) {
            ports[i] = new GPIO(i);
            flowPane1.getChildren().add(ports[i]);
        }

        super.setContent(content);
    }


    /**
     * Reset all push buttons when simulation stops
     */

    public void stop() {
        for(int i=0; i < gpioNum; i++){
            ports[i].setStop();
        }
    }

    public void setGPIO(int index, boolean signal){
        if(index < 0 || index > gpioNum){
            throw new IndexOutOfBoundsException("Invalid parallel port index\n");
        }
        ports[index].setStatus(signal);
    }

    /**
     * Setters and Getters
     */
    public void setGPIONum(int gpioNum) {
        this.gpioNum = gpioNum;
    }
}
