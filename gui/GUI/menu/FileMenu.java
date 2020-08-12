package GUI.menu;

import Controller.EditorController;
import Controller.ProjectManager;
import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuItem;
import javafx.scene.control.SeparatorMenuItem;


public class FileMenu extends Menu {

    public FileMenu(String text) {
        super(text);

        // New
        MenuItem newFileItem = new MenuItem("New");
        this.getItems().add(newFileItem);
        newFileItem.addEventHandler(ActionEvent.ACTION, new EventHandler<>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                EditorController editorController = new EditorController();
                editorController.addNewFile();
            }
        });

        // Open
        MenuItem openFileItem = new MenuItem("Open");
        this.getItems().add(openFileItem);
        openFileItem.addEventHandler(ActionEvent.ACTION, new EventHandler<>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                EditorController editorController = new EditorController();
                editorController.openFile();
            }
        });

        // Save
        MenuItem saveFileItem = new MenuItem("Save");
        this.getItems().add(saveFileItem);
        saveFileItem.addEventHandler(ActionEvent.ACTION, new EventHandler<>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                EditorController editorController = new EditorController();
                editorController.saveFile();
            }
        });
        // this.getItems().add(new MenuItem("Save As"));


        this.getItems().add(new SeparatorMenuItem());

        // this.getItems().add(new MenuItem("New Project"));

        // Open Project
        MenuItem openProjItem = new MenuItem("Open Project");
        this.getItems().add(openProjItem);
        openProjItem.addEventHandler(ActionEvent.ACTION, new EventHandler<>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                openProj();
            }
        });

        // this.getItems().add(new MenuItem("Open Recent Project"));


        MenuItem closeProjItem = new MenuItem("Close Project");
        this.getItems().add(closeProjItem);
        closeProjItem.addEventHandler(ActionEvent.ACTION, new EventHandler<>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                closeProj();
            }
        });
        this.getItems().add(new SeparatorMenuItem());

        // Exit
        MenuItem exitItem = new MenuItem("Exit");
        this.getItems().add(exitItem);
        exitItem.addEventHandler(ActionEvent.ACTION, new EventHandler<>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                Platform.exit();
            }
        });
    }


    private void openProj() {
        ProjectManager.openProj();

    }


    private void closeProj() {
        ProjectManager.closeCurProj();
    }
}

