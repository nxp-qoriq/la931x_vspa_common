# SPDX-License-Identifier: BSD-3-Clause
# Shared toolchain sanity checks for VSPA kernel test builds.
#
# Include this from a kernel tests/Makefile with:
#     include ../../tools/checks.mk
#
# It expects VSPA_CC to be defined by the including Makefile
# (typically $(VSPA_TOOL)/bin/fsvspacc) and provides two phony
# targets:
#     check-vspa  verifies the VSPA compiler is installed
#     check-sim   verifies the VSPA simulator is installed
#
# Add check-vspa as a prerequisite of the build/link target, and
# check-vspa + check-sim as prerequisites of the test target.

# Default standalone VSPA simulator location; override on the
# command line with: make ... SIMULATOR_PATH=/path/to/runsim
SIMULATOR_PATH ?= /opt/ccssim2/bin/runsim

.PHONY: check-vspa check-sim

check-vspa:
	@if [ ! -x "$(VSPA_CC)" ]; then \
	echo "ERROR: VSPA_TOOL is not set and /opt/VSPA_Tools_vbeta_14_00_781 not found."; \
        echo " Set VSPA_TOOL to your toolchain root, e.g.:";\
	echo " VSPA_TOOL=/path/to/VSPA_Tools"; \
	exit 1; \
	fi

check-sim:
	@if [ ! -x "$(SIMULATOR_PATH)" ]; then \
	    echo "ERROR: SIMULATOR_PATH is not set and /opt/ccssim2/bin/runsim not found:"; \
	    echo "  Set SIMULATOR_PATH to your  simulator executable, e.g.:";\
	    echo "  SIMULATOR_PATH=/path/to/ccssim2/bin/runsim"; \
	    exit 1; \
	fi
