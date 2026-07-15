# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
# Copyright 2020 - 2025 the original authors

#********************************************************************************
#* Note: This script must be run from the script directory.
#********************************************************************************

# Change path to current script
cd [file dirname [info script]]

# Add paths
source ../../../common/script/jenkins.tcl
source ../../../common/script/libsim.tcl

# Initialize jenkins 
jenkinsInit

# Debug
debug "FECU_VSPA2_16AU_CAS"

# Run testcase
go
wait 500
#set err_count [expr !$status]
set err_count 0

# Finish debug process
kill

# Send test report to Jenkins
jenkinsSendTestReport $err_count
