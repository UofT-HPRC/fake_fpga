package GUI.menu;

import javafx.scene.control.Menu;
import javafx.scene.control.MenuItem;
import javafx.scene.control.SeparatorMenuItem;

public class EditMenu extends Menu {
    public EditMenu(String text){
        super(text);

        this.getItems().add(new MenuItem("Undo"));
        this.getItems().add(new MenuItem("Redo"));
        this.getItems().add(new SeparatorMenuItem());

        this.getItems().addAll(new MenuItem("Cut"), new MenuItem("Copy"), new MenuItem("Paste"));
        this.getItems().add(new SeparatorMenuItem());

        this.getItems().addAll(new MenuItem("Find"), new MenuItem("Replace"));
    }
}
