# fake_fpga
A GUI pretending to be a DE1 for visualizing Verilog sims

# Compiling

To compile the GUI, run `make` in the `GUI` folder. There are 
instructions in GUI/README.txt to help you get started.

To compile the Modelsim plugin, follow the instructions in 
`simulation/include/README.txt` and `simulation/lib/README.txt` which 
ask you to copy come files from the modelsim install folder. Then, run
`make` in the `simulation` folder.

# Running

To run the GUI, just double-click `run.bat` in the `GUI` folder. Do 
this before starting the simulation. By the way, this batch file can be 
sourced from inside `bash` on a Linux system.

Likewise, you can just double-click `demo.bat` in the `demo` folder to 
run the demo. Note that this script uses relative paths to find 
`fakefpga.vpi` (the thing you make when you run `make` in the 
`simulation` directory), so you may need to edit the last line if your 
files are in different folders. By the way, `demo.bat` can also be 
sourced on Linux.

# Acknowledgements

We would like to thank Ruiqi Wang for helping with the GUI.

# Problems?

Contact me at marco.merlini@utoronto.ca

