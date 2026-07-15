# VSPA Common Repository

vspa_common_la9310 repository contains all the vspa kernel components for
enabling LA9310 VSPA firmware development. It includes vspa-sdk providing
basic support of la9310 SOC support, and vspa-lib which provides a set
of basic optimized VSPA kernels.


# Libraries

This repository includes 2 libraries for VSPA development:
- `vspa-sdk` library provides a set of include and source files to use VSPA
  peripherals, adds support for missing intrinsics, and targets VSPA3-based
  system-on-chips. 
- `vspa-lib` library provides optimized implementations of generic DSP functions
  not specific to any particular application. 


# linux environment (default)

Under linux, the library uses standalone linux vspa toolchain for compilation 
and python to generate test vectors. Standalone linux vspa simulator is used for testing.
For runtime debugging on simulator or target, CodeWarrior can be used (windows only) 

## Prerequisites

- Standalone VSPA toolchain 
Default install directory `/opt/VSPA_Tools_vbeta_14_00_781` unless defined with VSPA_TOOL env variable 
Download location `wget https://www.nxp.com/lgfiles/sdk/la1224/imx-la9310-sdk-10/cwvspa.vbeta_14_00_781_vspa.linux.tgz`

- Standalone VSPA Simulator
Default install `/opt/ccssim2/bin/runsim` unless defined with SIMULATOR_PATH env variable
Download location `wget https://www.nxp.com/lgfiles/sdk/la1224/imx-la9310-sdk-10/ccssim2_linux_gcc4-64_lin_1_0_63_4.tar.gz`

## Compile and run regression

```sh
# Compile a kernel and its test app
make -C vspa-lib/<kernel>/tests

# Run a kernel test
make -C vspa-lib/<kernel>/tests test

# Compile and run all tests
bash tools/regression_cycles_all.sh
```

# Run test binary on simulator
```sh
runsim -d vspa2_16au vspa-lib/build/<kernel>/tests/test_<kernel>_<TC_num>>.eld

ex:
/opt/ccssim2/bin/runsim -d vspa2_16au  build/build/atan/tests/test_atan_TC001_vspa2_16au.eld
KERNEL_CYCLES: 69
PASS
```

# with trace
```sh
runsim -d vspa2_16au -showpc vspa-lib/build/<kernel>/tests/test_<kernel>_<TC_num>>.eld

ex:
/opt/ccssim2/bin/runsim -d vspa2_16au -showpc build/build/atan/tests/test_atan_TC001_vspa2_16au.eld
PC:    0(0000) CC:0x0 CYC:     1 OP:680b000000000000   setB.creg 0x16(22), 0x0(0); // op_flag = 0x2
PC:    2(0002) CC:0x0 CYC:     2 OP:4200000010000000   jsr 0x10(16); // op_flag = 0x4
PC:    4(0004) CC:0x0 CYC:     3 OP:800000001f0d1c00   mv sp, 0x34700(214784); // op_flag = 0x10
PC:    6(0006) CC:0x0 CYC:     4 OP:6802000000000000   setB.creg 0x4(4), 0x0(0); // op_flag = 0x2
PC:   16(0010) CC:0x0 CYC:     5 OP:44b00004c4000000   mv a0, 0x4c4(1220); // op_flag = 0x4
...
```

# Windows environment 

Under Windows, the library relies on CodeWarrior projects for compilation/simulation/debug. 
Matlab is used to generate test vectors. 

## Prerequisites

- CodeWarrior for VSPA 10.3.0

## Generate test vectors using matlab

To generate VSPA kernel test vectors execute matlab script under related CW project
- run `${ProjDirPath}/[..]/vspa-lib/<kernel>/<kernel>_cwproj/matlab`

## Execute regression testing tcl scripts 

- Import existing project in CW `${ProjDirPath}`
- Compile project 
- open Debugger Shell 
-  cd `<kernel>_cwproj`
-  source ./scripts/<testBench>.tcl


