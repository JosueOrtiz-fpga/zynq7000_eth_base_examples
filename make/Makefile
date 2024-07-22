#include env variables
include ./scripts/env_vars.mak
# get the name of the top level directories
TOPS:=$(foreach dir,$(wildcard ../top/*), $(notdir $(basename $(dir))))
# get the name for each top level directory with a 
TOPS_SW:=$(foreach dir,$(wildcard ../top/*/sw/config.py), $(notdir $(abspath $(dir)/../..)).sw)

.DEFAULT_GOAL=help
.PHONY: $(TOPS) $(TOPS_SW)
help:
	@echo "The following targets exists"
	@echo $(TOPS)
	@echo $(TOPS_SW);

clean:
	$(foreach dir, $(TOPS), $(shell rm -rf $(dir)))
	rm -rf 

$(TOPS): 
	mkdir -p ./$@
	cd ./$@; $(VIVADO) -mode batch -source ../scripts/viv_build.tcl -tclargs $@;

$(TOPS_SW):
ifeq ($(SW_OUTPUT_DIR),)
	$(eval SW_OUTPUT_DIR:= ./$(basename $@)/)
endif
	$(eval SW_PATH:=$(abspath ../top/$(basename $@)/sw/))
	$(eval XSA_FILE:=$(abspath $(wildcard ./$(basename $@)/*.xsa)))
	$(eval SCRIPT_FILE:=$(abspath ./scripts/vitis_build.py))
	@echo $(SW_PATH)
	@echo $(XSA_FILE)
	@echo $(SCRIPT_FILE)
	# if SW_OUTPUT_DIR is left empty choose the default
	rm -rf $(SW_OUTPUT_DIR)/$@
	mkdir $(SW_OUTPUT_DIR)/$@
	cd $(SW_OUTPUT_DIR)/$@; $(VITIS) -s $(SCRIPT_FILE) $(SW_PATH) $(XSA_FILE)