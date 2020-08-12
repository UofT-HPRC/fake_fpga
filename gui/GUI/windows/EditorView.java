package GUI.windows;

import javafx.scene.control.TabPane;

public class EditorView extends TabPane {

    public EditorView(){
        super();
    }

    /**
     * create tab for new file
     */
    public void addTab(EditorTab tab){
        this.getTabs().add(tab);
    }


}
