package GUI.windows;

import Controller.Message;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.text.Text;


public class MessageBox extends VBox {


    public MessageBox() {
        super.setSpacing(2.0);
    }


    public void addMessage(String msg, Message priority) {
        Text text = new Text(msg);
        text.setStyle("-fx-font-size: 14;");

        switch (priority) {
            case ERROR:
                text.setText("Error: " + text.getText());
                text.setFill(Color.RED);
                break;
            case WARNING:
                text.setText("Warning: " + text.getText());
                text.setFill(Color.ORANGE);
                break;
            case INFO:
                text.setFill(Color.GREEN);
                break;
            case ModelSim:
                text.setText("  |  " + text.getText());
                text.setFill(Color.ORANGERED);

            default:
        }

        this.getChildren().add(text);
    }

}
