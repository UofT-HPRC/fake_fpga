package GUI.menu;

import Controller.ProjectManager;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuItem;
import javafx.scene.control.SeparatorMenuItem;

public class HelpMenu extends Menu {

    public HelpMenu(String text){
        super(text);

        MenuItem sampleItem = new MenuItem("Sample Projects");
        this.getItems().add(sampleItem);
        this.getItems().add(new SeparatorMenuItem());

        MenuItem ledItem = new MenuItem("LED test");
        MenuItem ps2Item = new MenuItem("PS/2 test");
        this.getItems().addAll(ledItem, ps2Item);

        ledItem.addEventHandler(ActionEvent.ACTION, new EventHandler<>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                ProjectManager.openProj("../led_demo");
            }
        });

        ps2Item.addEventHandler(ActionEvent.ACTION, new EventHandler<>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                ProjectManager.openProj("../ps2_demo");
            }
        });


    }
}
