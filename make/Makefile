# get the name of the top level directories
TOPS:=$(foreach dir,$(wildcard ../top/*), $(notdir $(basename $(dir))))
# get the name for each top level directory with a 
TOPS_SW:=$(foreach dir,$(wildcard ../top/*/sw/config.tcl), $(notdir $(abspath $(dir)/../..)).sw)
# AMD executables
vivado=/c/AMD/Vivado/2024.1/bin/vivado
xsct=/c/AMD/Vitis/2024.1/bin/xsct
# default dir for sw - for windows, ninja builder has a 260 path length limit
sw_dir=/c/projects
.DEFAULT_GOAL=help
.PHONY: $(TOPS) $(TOPS_SW)
help:
	@echo "The following targets exists"
	@echo $(TOPS)
	@echo $(TOPS_SW);

clean:
	@rm -rf $(TOPS);
	@rm -rf $(sw_dir)/$(TOPS_SW);

$(TOPS): 
	mkdir -p ./$@
	cd ./$@; $(vivado) -mode batch -source ../scripts/viv_build.tcl -tclargs $@;

$(TOPS_SW):
	CONFIG_FILE=$(abspath ../top/$(basename $(TOPS_SW))/sw/config.tcl)
	SCRIPT_FILE=$(abspath ./scripts/vitis_build.tcl)
	@echo $(CONFIG_FILE)
	@echo $(SCRIPT_FILE)
	mkdir -p $(sw_dir)/$@
	cd $(sw_dir)/$@; $(xsct) $(SCRIPT_FILE)