# How to Build and Run the Source Code in Linux environment
  * Get the latest or required version of NDSP HiFi1 Code from GitHub 
  * https://github.com/cad-audioNDSP/HiFi_1_NDSP/tree/main/NDSP_HiFi1

## The source code is organized as follows.
  * **build** - contains the make file 
  * **library** - contains the optimized kernel functions for the HiFi core 
  * **testdriver** - contains the demo driver code to tun the library   

### It is assumed that the required HiFi core configurations and the Xtensa toolchain are set up in the Linux environment.
 An example .cshrc file  that sets up the build environment accordingly is provided for reference 

## Setting up the environment 
  * A typical way is to place this .cshrc file in your home directory. 
  * source ~/.cshrc 
  * setenv XTENSA_CORE CORE_NAME     
    Ex: setenv XTENSA_CORE AE_HiFi1e_LE5_AO_FP  

## Compiling the Source Code: 
  * Navigate to the testdriver directory:   …/ NDSP_HiFi1/build/project/xtclang/testdriver
  * **CLEAN:**  make clean -j -e LANG=LLVM  
  * **BUILD:**  make all -j -e LANG=LLVM 


## Running the executable: 
  ### Navigate to the bin directory: …/ NDSP_HiFi1/build/bin
  ### Performance tests:
  * xt-run testdriver-AE_HiFi1e_LE1_AO_FP_llvm-Xtensa-release -mips -sanity         
  * xt-run testdriver-AE_HiFi1e_LE1_AO_FP_llvm-Xtensa-release -mips -brief 
  * xt-run testdriver-AE_HiFi1e_LE1_AO_FP_llvm-Xtensa-release -mips -full   
  ###	Functional tests:
  * xt-run --turbo testdriver-AE_HiFi1e_LE1_AO_FP_llvm-Xtensa-release -func -sanity
  * xt-run --turbo testdriver-AE_HiFi1e_LE1_AO_FP_llvm-Xtensa-release -func -brief
  * xt-run --turbo testdriver-AE_HiFi1e_LE1_AO_FP_llvm-Xtensa-release -func -full
  * xt-run --turbo testdriver-AE_HiFi1e_LE1_AO_FP_llvm-Xtensa-release -func -sanity -verbose 
  * xt-run --turbo testdriver-AE_HiFi1e_LE1_AO_FP_llvm-Xtensa-release -func -sanity -fir -verbose 
  * xt-run --turbo testdriver-AE_HiFi1e_LE1_AO_FP_llvm-Xtensa-release -func -brief -fir -iir -fft
