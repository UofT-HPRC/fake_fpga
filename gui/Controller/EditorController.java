package Controller;

import GUI.Main;
import GUI.windows.EditorTab;
import GUI.windows.EditorView;
import javafx.scene.control.TextArea;
import javafx.stage.FileChooser;
import javafx.stage.Window;

import java.io.File;
import java.io.IOException;

public class EditorController {
    private static int newFileCount = 0;
    private final FileChooser fileChooser;

    public EditorController(){
        fileChooser = new FileChooser();
    }

    public void addNewFile(){
        EditorTab tab = new EditorTab("Untitled-" + newFileCount, new TextArea());
        ((EditorView) Main.nodeMap.get("EditorView")).addTab(tab);
        newFileCount++;
    }

    public void openFile(){
        try {
            File file = this.fileChooser.showOpenDialog(getWindow());
            if(file != null) {
                EditorTab tab = new EditorTab("", new TextArea(), file);
                ((EditorView) Main.nodeMap.get("EditorView")).addTab(tab);
            }
        }catch(IOException e){
            // TODO: print to Message window
            System.out.println("Can't open file");
        }

    }

    public void saveFile(){
        EditorTab tab = (EditorTab) ((EditorView) Main.nodeMap.get("EditorView")).getSelectionModel().getSelectedItem();
        if (tab != null) {
//            System.out.println("tab found, saving file");
            try {
                tab.saveToFile();
            }catch(IOException e){
                // TODO: print to Message window
                System.out.println("Save to file failed.");
            }
        }
    }


    private Window getWindow(){
        return Main.nodeMap.get("EditorView").getScene().getWindow();
    }
}
