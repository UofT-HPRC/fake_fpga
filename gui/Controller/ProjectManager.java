package Controller;

import GUI.Main;
import GUI.device.VGA;
import javafx.stage.DirectoryChooser;

import java.io.File;
import java.nio.file.Path;
import java.nio.file.Paths;

public class ProjectManager {
    private static String projDir;

    public static void openProj(String path) {
        File dir = new File(path);
        setProjDir(dir.getAbsolutePath());
        VGA.setProjDir(projDir);
        Main.messageBox.addMessage("Project: " + dir.getName() + " opened.", Message.INFO);
    }


    public static void openProj() {
        DirectoryChooser directoryChooser = new DirectoryChooser();
        directoryChooser.setInitialDirectory(new File(".."));
        // get the file selected
        File dir = directoryChooser.showDialog(Main.nodeMap.get("EditorView").getScene().getWindow());

        // if dir exists
        if (dir != null) {
            // check if it has ModelSim subfolder
            Path projPath = Paths.get(dir.getAbsolutePath());
            Path simPath = projPath.resolve("ModelSim");
            File f = new File(simPath.toString());

            // if exists, open project
            if (f.exists() && f.isDirectory()) {
                setProjDir(dir.getAbsolutePath());
                VGA.setProjDir(projDir);
                Main.messageBox.addMessage("Project: " + dir.getName() + " opened.", Message.INFO);
            } else {
                Main.messageBox.addMessage("Project: " + dir.getName() + " should contain ModelSim subfolder.", Message.ERROR);
            }
        } else {
            Main.messageBox.addMessage("Project directory not found.", Message.ERROR);
        }
    }

    public static void setProjDir(String dir) {
        projDir = dir;
    }

    public static String getProjDir() {
        return projDir;
    }

    public static void closeCurProj() {
        if (projDir != null) {
            projDir = null;
            Main.messageBox.addMessage("Closed current project.", Message.INFO);
        }
    }
}
