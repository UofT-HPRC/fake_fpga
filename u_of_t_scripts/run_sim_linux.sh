#!/bin/bash

vlib work
vlog tb.v
vlog main.vo
vsim -pli fakefpga.vpi -novopt -c -t 1ps -L cyclonev_ver -L altera_ver -L altera_mf_ver -L 220model_ver -L sgate_ver -L altera_lnsim_ver tb -voptargs="+acc" -do "run -all"
