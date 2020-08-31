set_global_assignment -name EDA_SIMULATION_TOOL "ModelSim-Altera (Verilog)"
set_global_assignment -name EDA_TIME_SCALE "1 ps" -section_id eda_simulation
set_global_assignment -name EDA_OUTPUT_DATA_FORMAT "VERILOG HDL" -section_id eda_simulation
set_global_assignment -name EDA_GENERATE_FUNCTIONAL_NETLIST ON -section_id eda_simulation
set_global_assignment -name EDA_MAINTAIN_DESIGN_HIERARCHY ON -section_id eda_simulation

load_package flow

execute_module -tool map
execute_module -tool eda

file copy "simulation/modelsim/$quartus(project).vo" "simulation/modelsim/main.vo"
