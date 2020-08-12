vlib work
vlog hello.v
vlog tb.v
vsim -pli ../simulation/fakefpga.vpi -c -do "run -all" tb
