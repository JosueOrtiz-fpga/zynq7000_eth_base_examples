# ZYNQ 7000 Ethernet Base Examples
This repository implements several AMD ethernet examples for Zynq 7000 FPGAs. The Digilent Cora-z7-07s board was used for recreating the examples.
## Hardware Setup
The list of hardware needed:
1. A Cora-z7-07s board
2. Micro-USB cable
3. Ethernet Cable
## Environment Setup
Git Bash was used for development on Windows; Linux works just as well.  
This [guide](https://gist.github.com/evanwill/0207876c3243bbb6863e65ec5dc3f058) shows how to add _make_ to Git Bash  
Vivado and Vitis tool paths need to be provided in the make/scripts/env_vars.mak file. An optional SW_OUTPUT_DIR variable can be used to store the Vitis workspace in an alternate location.
## Build Steps
For each example, there is a corresponding top/ subdirectory. The make/Makefile must be used to first make the Vivado project (HW) and then the Vitis project (SW). The build steps are similar for all examples, the name - represented as _$(top_name)_ below - just needs to be changed.
```
cd make
make $(top_name)
make $(top_name).sw
```
To program the board, open the Vitis project in Vitis, select the application component, and "Run" it.
## Table of Contents
* [Z7 Echo Server](#z7_echo_server)  
* [Z7 Packet Redirection](#z7_pkt_redirect)
---
## Z7 Echo Server <a name="z7_echo_server"/>
This example configures the Z7 FPGA as a ethernet server that echos the user keyboard input. [Digilent](https://digilent.com/reference/programmable-logic/guides/zynq-servers) provides a step-by-step guide for building and running the design.

## Z7 Packet Redirection <a name="z7_pkt_redirect"/>
This example configures the Z7 FPGA as a ethernet server that echos the user keyboard input. [Digilent](https://digilent.com/reference/programmable-logic/guides/zynq-servers) provides a step-by-step guide for building and running the design.
