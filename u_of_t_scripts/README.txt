After creating a new Quartus project, perform the following steps:

	1) File->New...->Verilog HDL file. Call it "main.v". 
	
	2) Paste the contents of lab_template.v into your newly created
	   main.v
	
	3) Write your code
	
	4) Tools->Tcl Scripts...->Add to Project... and browse to the
	   compile_sim.tcl provided in this folder.
	
	5) While still inside the Tcl Scripts window, click once on
	   compile_sim.tcl to select it, then click Run at the bottom. If
	   you have any Verilog errors, they will appear in the Processing
	   window in Quartus.
	
	6) Copy fakefpga.vpi, tb.v, and run_sim_X from this folder into 
	   W:\Path\To\Your\Quartus\Project\simulation\modelsim. 
	   -> Select the correct run_sim_X for your platform:
	      -> run_sim_home.bat: Use this if you are on a Windows computer 
	         at home. YOU WILL NEED TO EDIT THE FIRST LINE so that the
	         path points to where Modelsim is installed
	         
	      -> run_sim_DESL.bat: Use this on DESL computers
	      
	      -> run_sim_ECF.bat: Use this on ECF computers
	      
	      -> run_sim_linux.sh: Use this on Linux computers. YOU WILL 
	         NEED TO ADD AN `export PATH=/path/to/modelsim` LINE IF 
	         MODELSIM IS NOT ALREADY ON YOUR PATH.
	         
	   -> By the way, this folder should contain a file called "main.vo".
	      If it does not, makes sure that you didn't get any errors in
	      Step 5. Otherwise, make sure you are copying the files into
	      the correct folder.
	
	7) If it is not already done, start the GUI.
	
	8) Double-click on run_sim_X
	

TROUBLESHOOTING:

	If nothing happens, try shift-right-clicking in the folder 
	containing main.vo, fakefpga.vpi, tb.v, and run_sim_X and clicking
	"Open Command window here...". Then, in that command prompt, type
	run_sim_X (where you've replaced X with the correct file name). This
	will allow you to read the error messages.

	Remember to re-run compile_sim.tcl whenever you change main.v
	
	
