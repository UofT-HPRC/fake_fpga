package Controller;

import GUI.Main;
import GUI.device.GPIO;
import GUI.devicecontainer.*;
import javafx.application.Platform;


/**
 * To parse incoming signals and control corresponding devices
 */
public class SignalParser {
	private final VGAContainer vgaContainer;
	
    public SignalParser() {
		vgaContainer = (VGAContainer) Main.nodeMap.get("VGAContainer");
    }

    public void processSignal(String line) {
        if (line.startsWith("h")) {
            processHEXSignal(line);
        } else if (line.startsWith("l")) {
            processLEDSignal(line);
        } else if (line.startsWith("g")) {
            processGPIOSignal(line);
        } else if (line.startsWith("p")) {
            processPS2Lock(line);
        } else if (line.startsWith("#")) {
            Platform.runLater(
                    () -> {
                        // Update message box
                        Main.messageBox.addMessage(line, Message.ModelSim);
                    }
            );
        } else if (line.startsWith("frame")) {
            processVGASignal();
        } else if (line.startsWith("c")) {
            /* Very crappy parser for individual pixel colours */
            /* Expected syntax: c ### ### #, where the first three
             * digits are the x coordinate, zero-padded if needed, the
             * next three are the y coordinate, and the last is the
             * colour, a digit from 0 to 7 */
            //VGAContainer vgaContainer = (VGAContainer) Main.nodeMap.get("VGAContainer");
            int x = Integer.parseInt(line.substring(2,5));
            int y = Integer.parseInt(line.substring(6,9));
            vgaContainer.set_pixel_160x120(x, y, line.charAt(10));
        }


    }

    private void processVGASignal() {
        //VGAContainer vgaContainer = (VGAContainer) Main.nodeMap.get("VGAContainer");
        vgaContainer.updateFrame();
        vgaContainer.enableNextFrame();

    }

    private void processPS2Lock(String line) {
        KeyboardContainer keyboardContainer = (KeyboardContainer) Main.nodeMap.get("KeyboardContainer");
        try {
            String binary_string = line.strip().substring(1);
            int length = binary_string.length();
            for (int i = 0; i < 3; i++) {
                keyboardContainer.setLockLED(i, binary_string.charAt(length - i - 1) == '1');
            }
        }catch (StringIndexOutOfBoundsException ignored){}
    }

    private void processLEDSignal(String line) {
        try {
            LEDContainer container = (LEDContainer) Main.nodeMap.get("LEDContainer");
            String binary_string = line.strip().substring(1);
            int length = binary_string.length();
            for (int i = 0; i < 10; i++) {
                container.setLED(i, i < length && binary_string.charAt(length - i - 1) == '1');

            }
        }catch (StringIndexOutOfBoundsException ignored){}
    }

    // TODO
    private void processGPIOSignal(String line) {
        try {
            GPIOContainer container = (GPIOContainer) Main.nodeMap.get("GPIOContainer");
            String binary_string = line.strip().substring(1);
            int length = binary_string.length();
            for (int i = 0; i < 32; i++) {
                char gpio = binary_string.charAt(length - i - 1);
                if (gpio != 'z') {
                    if (gpio == 'x') {
                        GPIO.setIsOutput(i, true);
                    }
                    container.setGPIO(i, i < length && binary_string.charAt(length - i - 1) == '1');
                }
            }
        }catch(StringIndexOutOfBoundsException ignored){}
    }

    private void processHEXSignal(String line) {
        try {
            SevenSegContainer container = (SevenSegContainer) Main.nodeMap.get("SevenSegContainer");
            String hex_string = line.strip().substring(1);
//        System.out.println(hex_string);

            for (int i = 0; i < hex_string.length() / 2; i++) {
                if (hex_string.charAt(2 * i) == 'z' || hex_string.charAt(2 * i) == 'x') {
                    container.setSevenSegments(i, false, 0);
                } else {
                    int val = 127 - Integer.parseInt(hex_string.substring(2 * i, 2 * i + 2), 16);
                    container.setSevenSegments(i, true, val);
                }
            }
        }catch(StringIndexOutOfBoundsException ignored){}

    }

}


