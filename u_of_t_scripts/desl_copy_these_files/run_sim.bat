REM https://stackoverflow.com/a/34182234/2737696
cls
@pushd %~dp0

PATH C:\DESL\Quartus18\modelsim_ase\win32aloem;%PATH%

vlib work
vlog tb.v
vlog main.vo
vsim -pli fakefpga.vpi +nowarn3116 -c -t 1ps -L cyclonev_ver -L altera_ver -L altera_mf_ver -L 220model_ver -L sgate_ver -L altera_lnsim_ver tb -voptargs="+acc" -do "run -all"

REM https://stackoverflow.com/a/34182234/2737696
@popd
