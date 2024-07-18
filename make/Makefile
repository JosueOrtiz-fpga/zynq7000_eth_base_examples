# get the name of the top level directories
TOPS:=$(foreach dir,$(wildcard ../top/*), $(shell basename $(dir)))

# AMD executables
vivado=/c/AMD/Vivado/2024.1/bin/vivado
xsct=/c/AMD/Vitis/2024.1/bin/xsct

.DEFAULT_GOAL=help
.PHONY: $(TOPS)
help:
	@echo "The following targets exists"
	@echo $(TOPS)

clean:
	@rm -rf $(TOPS)

$(TOPS): 
	mkdir -p ./$@
	cd ./$@; $(vivado) -mode batch -source ../scripts/viv_build.tcl -tclargs $@