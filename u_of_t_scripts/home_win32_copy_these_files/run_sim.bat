PATH C:\Program Files\Quartus18\modelsim_ase\win32aloem;%PATH%
REM Edit the path before the semicolon!



REM You can delete this big block after you're done editing the path
@echo off
echo Rememember to fix the PATH command! 
echo Edit the first line of this file to point to the correct folder
echo where Modelsim is installed (and it should say something like
echo win32aloem at the end of it). Afterwards you can delete these
echo annoying echo statements
pause




REM Don't touch this
vlib work
vlog tb.v
vlog main.vo
vsim -pli fakefpga.vpi +nowarn3116 -c -t 1ps -L cyclonev_ver -L altera_ver -L altera_mf_ver -L 220model_ver -L sgate_ver -L altera_lnsim_ver tb -voptargs="+acc" -do "run -all"




REM you can delete this single "pause" if you want
pause
