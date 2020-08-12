package GUI.devicecontainer;

import GUI.DeviceContainer;
import GUI.device.VGA;
import javafx.application.Platform;

public class VGAContainer extends DeviceContainer{
    private final VGA vga;

    public VGAContainer() {
        super("VGA Display", new VGA());
        vga = new VGA();
        this.setContent(vga);
    }


    public void set_pixel_160x120(int x, int y, char pixel) {
		//Why must this be so complicated?
		final int xx = x;
		final int yy = y;
		final char cc = pixel;
		Platform.runLater(() -> vga.set_pixel_160x120(xx,yy,cc));
    }

    public void updateFrame(){
        vga.updateFrame();

    }


    public void enableNextFrame(){
        vga.setDisableNextFrame(false);
    }
}
