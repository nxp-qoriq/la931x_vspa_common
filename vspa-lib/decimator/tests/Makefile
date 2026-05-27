# SPDX-License-Identifier: BSD-3-Clause
# NXP vspa-lib — decimator kernel test build system
#
# Usage:
#   make              # build with C + asm kernel (AU=16)
#   make AU=8         # different AU count
#   make generate     # regenerate test vectors from Python model
#   make test         # build + run in simulator → PASS/FAIL
#   make clean

VSPA_TOOL         ?= /environment/cwvspa.vbeta_14_00_781_vspa.linux/VSPA_Tools
VSPA_CC            = $(VSPA_TOOL)/bin/fsvspacc
LM_LICENSE_FILE   ?= /environment/licenses/license.dat
export LM_LICENSE_FILE

AU       ?= 16

# Paths relative to this Makefile (tests/ subdir)
KERNEL_DIR    = ..
VSPA_SDK_INC  = $(KERNEL_DIR)/../../../vspa-sdk/inc
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
    -I$(KERNEL_DIR)/src \
    -I$(FRAMEWORK_DIR) \
    -DTEST_DECIM_FLT_LEN=32

TARGET = $(BUILD_DIR)/test_decimator_vspa2_$(AU)au.elf

SRCS = test_decimator.c \
    $(KERNEL_DIR)/src/decimator_asm.sx \
    $(KERNEL_DIR)/src/decimator_alloc.c

.PHONY: all generate test clean

all: $(TARGET)

generate:
	python3 gen_vectors.py

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
	python3 $(REPO_ROOT)/scripts/run_test.py $(TARGET) vspa2_$(AU)au

clean:
	rm -rf $(BUILD_DIR)
	rm -f vectors/input.hex vectors/ref.hex
