package GUI;

import Controller.Connector;
import Controller.Message;
import GUI.devicecontainer.*;
import GUI.windows.EditorView;
import GUI.windows.MessageBox;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;
import org.dockfx.DockNode;
import org.dockfx.DockPane;
import org.dockfx.DockPos;
import org.dockfx.demo.DockFX;

import java.util.HashMap;


public class Main extends Application {

    // TODO: better implementation for global static variables
    public static final Connector connector = new Connector();
    public static final HashMap<String, Node> nodeMap = new HashMap<>();
    public static final MessageBox messageBox = new MessageBox();


    public static void main(String[] args) {
        Thread connThread = new Thread(connector);
        connThread.start();
        launch(args);
    }


    //    @SuppressWarnings("unchecked")
    @Override
    public void start(Stage primaryStage) {
        primaryStage.setTitle("DESim");

        // create a dock pane that will manage our dock nodes and handle the layout
        DockPane dockPane = new DockPane();

        // create the editor view at the center of the dock area
        EditorView editor = new EditorView();

        // load an image to caption the dock nodes
        Image dockImage = new Image(DockFX.class.getResource("docknode.png").toExternalForm());

        // create and dock some prototype dock nodes to the middle of the dock pane
        // the preferred sizes are used to specify the relative size of the node
        // to the other nodes
//        DockNode tabsDock = new DockNode(editor, "Editor", new ImageView(dockImage));
//        tabsDock.setMinSize(400, 300);
//        tabsDock.dock(dockPane, DockPos.TOP);
//
//        DockNode treeDock = new DockNode(generateFileTree(), "Project Files", new ImageView(dockImage));
//        treeDock.setMinSize(100, 100);
//        treeDock.dock(dockPane, DockPos.LEFT);

        ScrollPane messagePane = new ScrollPane(messageBox);
        messagePane.setFitToWidth(true);
        messagePane.vvalueProperty().bind(messageBox.heightProperty());
        DockNode messageDock = new DockNode(messagePane, "Messages", new ImageView(dockImage));
        messageDock.setMinSize(400, 100);
        messageDock.dock(dockPane, DockPos.BOTTOM);


        //-----------------------------------------------------------------------------------------------
        // Devices

        VBox devices = new VBox();
        LEDContainer ledContainer = new LEDContainer();
        devices.getChildren().add(ledContainer);

        SwitchContainer switchContainer = new SwitchContainer();
        devices.getChildren().add(switchContainer);

        KeyContainer keyContainer = new KeyContainer();
        devices.getChildren().add(keyContainer);

        SevenSegContainer sevenSegContainer = new SevenSegContainer();
        devices.getChildren().add(sevenSegContainer);

        KeyboardContainer keyboardContainer = new KeyboardContainer();
        devices.getChildren().add(keyboardContainer);

//        GPIOContainer gpioContainer = new GPIOContainer();
//        devices.getChildren().add(gpioContainer);

        VGAContainer vgaContainer = new VGAContainer();
        devices.getChildren().add(vgaContainer);

        ScrollPane devicePane = new ScrollPane(devices);
        devicePane.setFitToWidth(true);
        DockNode deviceDock = new DockNode(devicePane, "Devices", new ImageView(dockImage));
        deviceDock.setMinSize(400, 100);
        deviceDock.dock(dockPane, DockPos.RIGHT);


        // --------------------------------------------------------------------------------------
        // Menu and Tools
//        final FileMenu menu1 = new FileMenu("File");
//        final EditMenu menu2 = new EditMenu("Edit");
//        final Menu menu3 = new Menu("Action");
//        final Menu menu4 = new Menu("Windows");
//        final HelpMenu menu5 = new HelpMenu("Help");

//        MenuBar menuBar = new MenuBar();
//        menuBar.getMenus().addAll(menu1, menu2, menu3, menu4, menu5);
//        menuBar.getMenus().addAll(menu1, menu5);

//        Button runButton = new Button("Run");
//        runButton.setOnAction(new EventHandler<>() {
//            @Override
//            public void handle(ActionEvent event) {
//                String projDir = ProjectManager.getProjDir();
//
//                // clear previous signals
//                ledContainer.stop();
//                sevenSegContainer.stop();
//                // reset controls
//                keyContainer.stop();
//                switchContainer.stop();
//                keyboardContainer.stop();
//                vgaContainer.enableNextFrame();
////                gpioContainer.stop();
//
//                if (projDir == null) {
//                    messageBox.addMessage("No opened project, used default.", Message.WARNING);
//                } else {
//                    // send project path
//                    connector.sendSignal("PATH " + projDir);
//                }
//
//                // start simulation
//                try {
//                    connector.sendStartSignal();
//                    messageBox.addMessage("Start simulation...", Message.INFO);
//                } catch (Exception e) {
//                    messageBox.addMessage("Simulator not connected.", Message.ERROR);
//                }
//            }
//
//        });

        Button clearButton = new Button("Reset Signals");
        clearButton.setOnAction(new EventHandler<>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                ledContainer.stop();
                sevenSegContainer.stop();
                // reset controls
                keyContainer.stop();
                switchContainer.stop();
                keyboardContainer.stop();
                vgaContainer.enableNextFrame();
//                gpioContainer.stop();
            }
        });

        Button stopButton = new Button("Stop Simulation");
        stopButton.setOnAction(new EventHandler<>() {
            @Override
            public void handle(ActionEvent e) {
                connector.sendSignal("end");
                messageBox.addMessage("End simulation.", Message.INFO);
            }
        });


//        Button openButton = new Button("Open Project");
//        openButton.setOnAction(new EventHandler<>() {
//            @Override
//            public void handle(ActionEvent e) {
//                ProjectManager.openProj();
//
//            }
//        });

        ToolBar toolBar = new ToolBar(
//                new Button("Settings"),
//                openButton,
//                new Button("New"),
//                new Separator(),
//                runButton,
                clearButton,
                new Separator(),
                stopButton
        );

        //-------------------------------------------------------------------------------------


        // TODO: clear up
        nodeMap.put("LEDContainer", ledContainer);
        nodeMap.put("SevenSegContainer", sevenSegContainer);
        nodeMap.put("VGAContainer", vgaContainer);
        nodeMap.put("EditorView", editor);
        nodeMap.put("KeyboardContainer", keyboardContainer);
//        nodeMap.put("GPIOContainer", gpioContainer);


        VBox vbox = new VBox();
//        vbox.getChildren().addAll(menuBar, toolBar, dockPane);
        vbox.getChildren().addAll(toolBar, dockPane);
        VBox.setVgrow(dockPane, Priority.ALWAYS);

        primaryStage.setScene(new Scene(vbox, 1000, 750));
        primaryStage.sizeToScene();

        primaryStage.show();

        primaryStage.setOnCloseRequest(e -> Platform.exit());


        // test the look and feel with both Caspian and Modena
        Application.setUserAgentStylesheet(Application.STYLESHEET_MODENA);
        // initialize the default styles for the dock pane and undocked nodes using the DockFX
        // library's internal Default.css stylesheet
        // unlike other custom control libraries this allows the user to override them globally
        // using the style manager just as they can with internal JavaFX controls
        // this must be called after the primary stage is shown
        // https://bugs.openjdk.java.net/browse/JDK-8132900
        DockPane.initializeDefaultUserAgentStylesheet();

        // TODO: StyleManager class
    }


    private TreeView<String> generateFileTree() {
        // create a demonstration tree view to use as the contents for a dock node
        TreeItem<String> root = new TreeItem<>("Root");
        TreeView<String> treeView = new TreeView<>(root);
        treeView.setShowRoot(false);

        TreeItem<String> treeItem = new TreeItem<>("Folder");
        root.getChildren().add(treeItem);
        for (int i = 1; i < 5; i++) {
            TreeItem<String> childItem = new TreeItem<>("File " + i);
            treeItem.getChildren().add(childItem);
        }

        return treeView;
    }
}
