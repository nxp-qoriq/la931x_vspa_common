# SPDX-License-Identifier: BSD-3-Clause
# ATAN2 kernel test build system (submodule-local layout)

VSPA_TOOL         ?= /opt/VSPA_Tools_vbeta_14_00_781
VSPA_CC            = $(VSPA_TOOL)/bin/fsvspacc
LM_LICENSE_FILE   ?= /environment/licenses/license.dat
export LM_LICENSE_FILE

include ../../tools/checks.mk

AU       ?= 16
TC       ?= TC001

KERNEL_DIR    = ..
VSPA_SDK_INC  = ../../../vspa-sdk/inc
REPO_ROOT     = ../../../../..
FRAMEWORK_DIR = $(REPO_ROOT)/framework
BUILD_DIR    ?= ../build/tests

TC_VECTOR_DIR = vectors/$(TC)
TC_PARAMS     = $(TC_VECTOR_DIR)/tc_params.h

NUM_COEFF := $(shell grep '^#define TC_NUM_COEFF' $(TC_PARAMS) 2>/dev/null | awk '{print $$3}')
ifeq ($(NUM_COEFF),)
    $(error Cannot read TC_NUM_COEFF from $(TC_PARAMS) — run: make generate TC=$(TC))
endif

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
    -I$(KERNEL_DIR)/src \
    -I../../common/include \
    -I$(TC_VECTOR_DIR) \
    -DTEST_ATAN2_NUM_COEFF=$(NUM_COEFF) \
    -Xlnk "-Map $(BUILD_DIR)/test_atan_$(TC)_vspa2_$(AU)au.map"

TARGET = $(BUILD_DIR)/test_atan_$(TC)_vspa2_$(AU)au.eld

SRCS = test_atan.c \
       $(KERNEL_DIR)/src/atan_asm.sx \
       $(KERNEL_DIR)/src/atan_alloc.c

.PHONY: all generate generate-all test clean

all: $(TARGET)

generate:
	python3 gen_vectors.py --tc $(TC)

generate-all:
	python3 gen_vectors.py --all

$(TARGET): check-vspa $(SRCS) $(TC_PARAMS)
	@mkdir -p $(BUILD_DIR)
	$(VSPA_CC) $(VCFLAGS) \
	    -mem $(VSPA_TOOL)/etc/vspa2_$(AU)au/lcf/vspa2_$(AU)au_sp.lcf \
	    $(VSPA_TOOL)/lib/vspa2/startup_au$(AU)_sp.eln \
	    $(SRCS) \
	    $(VSPA_TOOL)/lib/vspa2/libc_au$(AU)_sp.elb \
	    $(VSPA_TOOL)/lib/vspa2/librt_au$(AU)_sp.elb \
	    -o $@
	@echo "Built: $@"

$(TC_PARAMS):
	$(MAKE) generate TC=$(TC)

test: check-vspa check-sim $(TARGET)
	python3 ../../tools/run_sim.py $(TARGET) vspa2_$(AU)au $(CYCLES)

clean:
	rm -f $(BUILD_DIR)/test_atan_*.eld
