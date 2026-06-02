# SPDX-License-Identifier: BSD-3-Clause
# logarithm kernel smoke-test build system (submodule-local layout)

VSPA_TOOL         ?= /opt/VSPA_Tools_vbeta_14_00_781
VSPA_CC            = $(VSPA_TOOL)/bin/fsvspacc
LM_LICENSE_FILE   ?= /environment/licenses/license.dat
export LM_LICENSE_FILE
AU       ?= 16
LOG_FACT ?= LOG2x1

KERNEL_DIR    = ..
VSPA_SDK_INC  = ../../../vspa-sdk/inc
REPO_ROOT     = ../../../../..
FRAMEWORK_DIR = $(REPO_ROOT)/framework
BUILD_DIR    ?= ../build/tests

VCFLAGS = \
    -arch vspa2 \
    -au_count $(AU) \
    -core_type sp \
    -ansi off \
    -O3 \
    -Os \
    -opt=noalias_by_type \
    -mvcpu \
    -msgstyle gcc \
    -env "$(VSPA_TOOL)" \
    -isystem $(VSPA_TOOL)/include \
    -I$(VSPA_SDK_INC) \
    -I$(KERNEL_DIR)/include \
    -I../../common/include \
    -DLOG_FACT=$(LOG_FACT)

TARGET = $(BUILD_DIR)/test_logarithm_vspa2_$(AU)au.elf

SRCS = test_logarithm.c $(KERNEL_DIR)/src/log_asm.sx

.PHONY: all generate test clean

all: $(TARGET)

generate:
	LOG_FACT=$(LOG_FACT) python3 gen_vectors.py

$(TARGET): $(SRCS) vectors/input.hex
	@mkdir -p $(BUILD_DIR)
	$(VSPA_CC) $(VCFLAGS) \
	    -mem $(VSPA_TOOL)/etc/vspa2_$(AU)au/lcf/vspa2_$(AU)au_sp.lcf \
	    $(VSPA_TOOL)/lib/vspa2/startup_au$(AU)_sp.eln \
	    $(SRCS) \
	    $(VSPA_TOOL)/lib/vspa2/libc_au$(AU)_sp.elb \
	    $(VSPA_TOOL)/lib/vspa2/librt_au$(AU)_sp.elb \
	    -o $@
	@echo "Built: $@"

vectors/input.hex:
	$(MAKE) generate

test: $(TARGET)
	@if [ -f "$(REPO_ROOT)/scripts/run_test.py" ]; then python3 $(REPO_ROOT)/scripts/run_test.py $(TARGET) vspa2_$(AU)au; else echo "INFO: run_test.py not in tree; ELF at $(TARGET)"; fi

clean:
	rm -f vectors/input.hex vectors/ref.hex $(TARGET)
