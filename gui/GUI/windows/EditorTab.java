package GUI.windows;

import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.scene.control.Tab;
import javafx.scene.control.TextArea;

import java.io.*;

public class EditorTab extends Tab {
    private File file = null;
    private boolean saved = false;
    private final TextArea textArea;

    /**
     * New editor tab constructor for new file
     *
     * @param text     tab label
     * @param textArea text area to show file content
     */
    public EditorTab(String text, TextArea textArea) {
        super(text, textArea);

        this.textArea = textArea;
        textArea.textProperty().addListener(new ChangeListener<>() {
            @Override
            public void changed(ObservableValue<? extends String> observable, String oldValue, String newValue) {
                setSaved(false);
            }
        });

    }

    public EditorTab(String text, TextArea textArea, File file) throws IOException {
        super(text, textArea);
        this.file = file;
        this.saved = true;
        this.textArea = textArea;
        this.textArea.textProperty().addListener(new ChangeListener<>() {
            @Override
            public void changed(ObservableValue<? extends String> observable, String oldValue, String newValue) {
                setSaved(false);
            }
        });

        updateTab();
    }


    /**
     * Will save the contents of the TextArea to the associated File
     * if it is valid. If it is not valid, a prompt will ask for
     * file selection.
     *
     * @throws java.io.IOException throw exception when fail opening file
     */
    public void saveToFile() throws IOException {
        File file = this.getFile();
        if (file != null && file.isFile()) {
            writeToFile();
            updateTab();
        } else {
            // TODO: implement file chooser
            System.out.println("File not found");
//            saveToNewFile();
        }
    }


    /**
     * Updates the text area to match the file text and the
     * tab text to match the file name.
     */
    private void updateTab() throws IOException {
        File file = this.getFile();
        this.setText(file.getName());
        textArea.setText(getFileContent(file));
        this.setSaved(true);
    }


    private String getFileContent(File file) throws IOException {
        FileInputStream fis = new FileInputStream(file);
        BufferedInputStream bis = new BufferedInputStream(fis);
        StringBuilder sb = new StringBuilder();
        while (bis.available() > 0) {
            sb.append((char) bis.read());
        }
        return sb.toString();
    }

    /**
     * Saves the current text in the text area to the file and sets
     * the current saved state to true.
     */
    private void writeToFile() throws IOException {
        File file = this.getFile();
        FileOutputStream fos = new FileOutputStream(file);
        OutputStreamWriter osw = new OutputStreamWriter(fos);
        Writer writer = new BufferedWriter(osw);
        writer.write(textArea.getText());

        // Set saved state to true.
        this.setSaved(true);
    }


    /**
     * Setters and Getters
     */
    public void setSaved(boolean saved) {
        this.saved = saved;
    }

    public File getFile() {
        return this.file;
    }

}

