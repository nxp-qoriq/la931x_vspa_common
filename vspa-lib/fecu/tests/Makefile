# SPDX-License-Identifier: BSD-3-Clause
# fecu kernel end-to-end test build system

VSPA_TOOL         ?= /opt/VSPA_Tools_vbeta_14_00_781
VSPA_CC            = $(VSPA_TOOL)/bin/fsvspacc
LM_LICENSE_FILE   ?= /environment/licenses/license.dat
export LM_LICENSE_FILE

include ../../tools/checks.mk
AU        ?= 16

# Minimum VSPA simulator version required to run the FECU testbench.
# Versions older than this have a known issue in the FECU block, so the
# test is skipped (not failed) when an older simulator is detected.
MIN_SIM_VERSION ?= lin_1_0_63_4


KERNEL_DIR    = ..
VSPA_SDK_INC  = ../../../vspa-sdk/inc
BUILD_DIR    ?= ../build/tests

VCFLAGS = \
    -arch vspa2 \
    -au_count $(AU) \
    -core_type sp \
    -ansi off \
    -g -O3 \
    -cwd source \
    -Os \
    -opt=noalias_by_type \
    -mvcpu \
    -msgstyle gcc \
    -env "$(VSPA_TOOL)" \
    -isystem $(VSPA_TOOL)/include \
    -I$(VSPA_SDK_INC) \
    -I$(KERNEL_DIR)/include \
    -Xlnk "-Map $(BUILD_DIR)/test_fecu_vspa2_$(AU)au.map"

TARGET = $(BUILD_DIR)/test_fecu_vspa2_$(AU)au.eld
SRCS   = test_fecu.c \
         $(KERNEL_DIR)/src/fecu.c \
         $(KERNEL_DIR)/src/fecu_p2p.c

VECTORS = vectors/input_tc0.hex vectors/input_tc1.hex \
          vectors/input_tc2.hex vectors/input_tc3.hex \
          vectors/input_tc4.hex \
          vectors/ref_tc0.hex   vectors/ref_tc1.hex   \
          vectors/ref_tc2.hex   vectors/ref_tc3.hex   \
          vectors/ref_tc4.hex   \
          vectors/enc_tc0.hex   vectors/enc_tc1.hex   \
          vectors/enc_tc2.hex   vectors/enc_tc3.hex   \
          vectors/enc_tc4.hex   vectors/enc_tc5.hex

.PHONY: all generate test clean

all: $(TARGET)

generate:
	python3 gen_vectors.py

$(VECTORS):
	$(MAKE) generate

$(TARGET): check-vspa $(SRCS) $(VECTORS)
	@mkdir -p $(BUILD_DIR)
	$(VSPA_CC) $(VCFLAGS) \
	    -mem $(VSPA_TOOL)/etc/vspa2_$(AU)au/lcf/vspa2_$(AU)au_sp.lcf \
	    $(VSPA_TOOL)/lib/vspa2/startup_au$(AU)_sp.eln \
	    $(SRCS) \
	    $(VSPA_TOOL)/lib/vspa2/libc_au$(AU)_sp.elb \
	    $(VSPA_TOOL)/lib/vspa2/librt_au$(AU)_sp.elb \
	    -o $@
	@echo "Built: $@"

# Run the FECU testbench, but first verify the simulator is recent enough.
# Simulators older than MIN_SIM_VERSION have a known FECU block issue, so
# the test is skipped (exit 0) rather than reported as a failure.
test: check-vspa check-sim $(TARGET)
	@ver=`"$(SIMULATOR_PATH)" -v 2>&1 | grep -oE 'lin_[0-9]+_[0-9]+_[0-9]+_[0-9]+' | head -n1`; \
	if [ -z "$$ver" ]; then \
	    echo "WARNING: could not determine simulator version from '$(SIMULATOR_PATH) -v'; running FECU testbench anyway."; \
	else \
	    cur_n=`echo "$$ver" | sed 's/^lin_//; s/_/./g'`; \
	    min_n=`echo "$(MIN_SIM_VERSION)" | sed 's/^lin_//; s/_/./g'`; \
	    oldest=`printf '%s\n%s\n' "$$cur_n" "$$min_n" | sort -V | head -n1`; \
	    if [ "$$cur_n" != "$$min_n" ] && [ "$$oldest" = "$$cur_n" ]; then \
	        echo "SKIP: FECU testbench skipped - simulator $$ver < $(MIN_SIM_VERSION) (known FECU block issue)."; \
	        exit 0; \
	    fi; \
	fi; \
	python3 ../../tools/run_sim.py $(TARGET) vspa2_$(AU)au $(CYCLES)

clean:
	rm -f $(VECTORS)
	rm -rf $(BUILD_DIR)
