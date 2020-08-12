package GUI.devicecontainer;

import GUI.DeviceContainer;
import GUI.device.Keyboard;


public class KeyboardContainer extends DeviceContainer {
    private final Keyboard keyboard;
    public KeyboardContainer() {
        super("PS/2 Keyboard", new Keyboard());
        keyboard = new Keyboard();
        this.setContent(keyboard);
    }

    public void setLockLED(int index, boolean status){
        keyboard.setLockLED(index, status);
    }

    public void stop(){
        keyboard.clearInput();
        keyboard.clearFIFO();
        // turn off lock LEDs
        for(int i=0; i<3; i++){
            keyboard.setLockLED(i, false);
        }
    }


}
