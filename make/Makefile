# get the name of the top level directories
TOPS:=$(foreach dir,$(wildcard ../top/*), $(notdir $(basename $(dir))))
# get the name for each top level directory with a 
TOPS_SW:=$(foreach dir,$(wildcard ../top/*/sw/config.py), $(notdir $(abspath $(dir)/../..)).sw)
# AMD executables
vivado=/c/AMD/Vivado/2024.1/bin/vivado
vitis=/c/AMD/Vitis/2024.1/bin/vitis
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
	$(eval SW_PATH:=$(abspath ../top/$(basename $@)/sw/))
	$(eval XSA_FILE:=$(abspath $(wildcard ./$(basename $@)/*.xsa)))
	$(eval SCRIPT_FILE:=$(abspath ./scripts/vitis_build.py))
	@echo $(SW_PATH)
	@echo $(XSA_FILE)
	@echo $(SCRIPT_FILE)
	rm -rf $(sw_dir)/$@
	mkdir $(sw_dir)/$@
	cd $(sw_dir)/$@; $(vitis) -s $(SCRIPT_FILE) $(SW_PATH) $(XSA_FILE)