# SPDX-License-Identifier: BSD-3-Clause
# crc8 kernel end-to-end test build system (submodule-local layout)

VSPA_TOOL         ?= /opt/VSPA_Tools_vbeta_14_00_781
VSPA_CC            = $(VSPA_TOOL)/bin/fsvspacc
LM_LICENSE_FILE   ?= /environment/licenses/license.dat
export LM_LICENSE_FILE

include ../../tools/checks.mk
AU        ?= 16

KERNEL_DIR    = ..
VSPA_SDK_INC  = ../../../vspa-sdk/inc
BUILD_DIR    ?= ../build/tests
REPO_ROOT     = ../../../../..

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
    -I$(KERNEL_DIR)/inc \
    -Xlnk "-Map $(BUILD_DIR)/test_crc8_vspa2_$(AU)au.map"

TARGET = $(BUILD_DIR)/test_crc8_vspa2_$(AU)au.eld
SRCS = test_crc8.c $(KERNEL_DIR)/src/CRC8.c

.PHONY: all generate test clean

all: $(TARGET)

generate:
	python3 gen_vectors.py

$(TARGET): check-vspa $(SRCS) vectors/data1.hex vectors/data2.hex vectors/size.hex vectors/ref.hex
	@mkdir -p $(BUILD_DIR)
	$(VSPA_CC) $(VCFLAGS) \
	    -mem $(VSPA_TOOL)/etc/vspa2_$(AU)au/lcf/vspa2_$(AU)au_sp.lcf \
	    $(VSPA_TOOL)/lib/vspa2/startup_au$(AU)_sp.eln \
	    $(SRCS) \
	    $(VSPA_TOOL)/lib/vspa2/libc_au$(AU)_sp.elb \
	    $(VSPA_TOOL)/lib/vspa2/librt_au$(AU)_sp.elb \
	    -o $@
	@echo "Built: $@"

vectors/data1.hex vectors/data2.hex vectors/size.hex vectors/ref.hex:
	$(MAKE) generate

test: check-vspa check-sim $(TARGET)
	python3 ../../tools/run_sim.py $(TARGET) vspa2_$(AU)au $(CYCLES)

clean:
	rm -f vectors/data1.hex vectors/data2.hex vectors/size.hex vectors/ref.hex $(TARGET)
