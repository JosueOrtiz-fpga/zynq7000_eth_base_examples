# proj_name passed as an arg
set proj_name [lindex $argv 0]
# set the output dirctory
set outputDir ./

# create the project and set the part/board
create_project -force $proj_name $outputDir
set_property board_part digilentinc.com:cora-z7-07s:part0:1.1 [current_project]

# import the hdl files - if empty, does nothing
set src_path "../../top/$proj_name/src/"
add_files -fileset sources_1 [ 
    glob -nocomplain $src_path/*.v\
    $src_path/*.vhd $src_path/*.sv] -quiet
# recreate any block design .tcl scripts
set bd_path "$src_path/bd/"
foreach bd [glob -nocomplain $bd_path/*.tcl] {
    source $bd
}

# import the constraint files - if empty, does nothing
set xdc_path "../../top/$proj_name/xdc/"
add_files -fileset constrs_1  [ glob -nocomplain $xdc_path/*.xdc] -quiet

# update the compile order
update_compile_order

# launch the runs
launch_runs synth_1
wait_on_run synth_1
launch_runs impl_1 -to_step write_bitstream
wait_on_run impl_1

# export the hardware file
append xsa_path $proj_name .xsa
write_hw_platform -fixed -include_bit -force -file $xsa_path




