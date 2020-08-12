package GUI.device;

import Controller.Device;
import GUI.Main;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.scene.control.CheckBox;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;
import javafx.scene.text.TextAlignment;

public class GPIO extends VBox {
    private static final int gpioNum = 32;
    private static final char[] portValue = new char[gpioNum];
    private static final boolean[] portIsOutput = new boolean[gpioNum];

    private final CheckBox checkBox;
    private final int index;

    public GPIO(int index){
        super();

        portValue[index] = '0';
        this.index = index;

        checkBox = new CheckBox();
        checkBox.setSelected(false);
        // set signal map
        portIsOutput[index] = false;

        checkBox.selectedProperty().addListener(new ChangeListener<>() {
            @Override
            public void changed(ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) {

                // update signal map
                portValue[index] = newValue ? '1' : '0';
                if (!portIsOutput[index]) {
                    // send signal
                    Main.connector.sendSignal(Device.GPIO + " " + getSignal());
                }
            }
        });

        Label label;
        if(index > 9){
            // one whitespace
            label = new Label(" " + index);
        }else {
            // two whitespaces
            label = new Label("  " + index);
        }
        label.setTextAlignment(TextAlignment.CENTER);
        this.getChildren().add(checkBox);
        this.getChildren().add(label);
    }



    private static String getSignal() {
        StringBuilder signal = new StringBuilder();
        for (int i = gpioNum-1; i >= 0; i--) {

            // sets output port to 'z'
            if(portIsOutput[i]) {
                signal.append('z');
            }else{
                signal.append(portValue[i]);
            }
        }
        return signal.toString();
    }


    public static void setIsOutput(int index, boolean isOutput){
        portIsOutput[index] = isOutput;
    }


    /**
     * Setters and Getters
     */
    public void setStatus(boolean status){
        this.checkBox.setSelected(status);
        // signal map
        portValue[this.index] = status ? '1' : '0';
    }


    public void setStop(){
        this.setStatus(false);
        portIsOutput[this.index] = false;
    }
}
