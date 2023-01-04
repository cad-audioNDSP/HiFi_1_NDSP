
# HiFi_1_NDSP
NatureDSP Library for HiFi 1 DSP cores

# The repo is organized as follows.

## xws:
  * Last stable release version of the NDSP containing two xws files.

  * An xws each, for the library-kernels and the test-driver.
    Ex : HiFi1_VFPU_Library_v1_1_0.xws & HiFi1_VFPU_Demo_v1_1_0.xws

  * Building and executing the xws in Xtensa Xplorer is described in the API Reference Document. 
  * Detailed release documentation can be extracted from lib.xws/doc folder.

### Release v1.1.0 Brief: 
  * Release Date : 30-Oct-2022.  
  * This release contains numerous kernels that are optimized for either performance or code size, 
    for the updated HiFi 1 core in RI-2022.9 tool chain.
  * This update also includes optimizations for the LP (Low latency FMA/MAC, Low Power) version of the 
    HiFi1 core available in RI-2022.9 tools and onwards. 
  * The source code is protected such that HiFi1 NDSP Library is backward compatible with the HiFi1 core 
    with older hardware versions (Pre LX7.1.9 HW versions). 
    The same source code can be built for both HiFi1 cores, with and without LP option enabled. 
    

## NDSP_HiFi1
This contains the source code along with make files that will build in linux environment.  

## doc folder
This contains help documentation on how to build the source code in linux and run the performance and functional regressions. 
