ifeq ($(OS),Windows_NT)
	JFX_PATH=windows_openjfx
	PATH_SEP=\
else
	JFX_PATH=linux_openjfx
	PATH_SEP=/
endif 

desim.jar:
	javac --module-path "$(JFX_PATH)$(PATH_SEP)lib" --add-modules=javafx.swing,javafx.graphics,javafx.fxml,javafx.media,javafx.web --add-reads javafx.graphics=ALL-UNNAMED -cp "dockfx/dockfx.jar" $$(find -name "*.java")
	jar -cvfe desim.jar Top $$(find -name "*.class")

clean:
	rm -rf desim.jar
	rm -rf $$(find -name "*.class")
	rm -rf out
