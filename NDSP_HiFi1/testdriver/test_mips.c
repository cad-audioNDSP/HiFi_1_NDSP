/* ------------------------------------------------------------------------ */
/* Copyright (c) 2022 by Cadence Design Systems, Inc. ALL RIGHTS RESERVED.  */
/* These coded instructions, statements, and computer programs ('Cadence    */
/* Libraries') are the copyrighted works of Cadence Design Systems Inc.     */
/* Cadence IP is licensed for use with Cadence processor cores only and     */
/* must not be used for any other processors and platforms. Your use of the */
/* Cadence Libraries is subject to the terms of the license agreement you   */
/* have entered into with Cadence Design Systems, or a sublicense granted   */
/* to you by a direct Cadence license.                                      */
/* ------------------------------------------------------------------------ */
/*  IntegrIT, Ltd.   www.integrIT.com, info@integrIT.com                    */
/*                                                                          */
/* NatureDSP Signal Library                                                 */
/*                                                                          */
/* This library contains copyrighted materials, trade secrets and other     */
/* proprietary information of IntegrIT, Ltd. This software is licensed for  */
/* use with Cadence processor cores only and must not be used for any other */
/* processors and platforms. The license to use these sources was given to  */
/* Cadence, Inc. under Terms and Condition of a Software License Agreement  */
/* between Cadence, Inc. and IntegrIT, Ltd.                                 */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2009-2022 IntegrIT, Limited.                      */
/*                      All Rights Reserved.                                */
/* ------------------------------------------------------------------------ */

/*  
    test module for performance evaluation of NatureDSP_Signal library
*/

#include "NatureDSP_Signal.h"
#include "mips.h"


#if defined COMPILER_MSVC
  #define MEM_ALIGNER  __declspec(align(64))
#else
  #define MEM_ALIGNER __attribute__ ((aligned(64)))
#endif

#if defined (COMPILER_XTENSA) && defined(MEM_MODEL)
#define PLACE_IN_DRAM0 _attribute_ ((section(".dram0.data")))
#if XCHAL_HAVE_DATARAM1
#define PLACE_IN_DRAM1 _attribute_ ((section(".dram1.data")))
#else
#define PLACE_IN_DRAM1 _attribute_ ((section(".dram0.data")))
#endif
#else
#define PLACE_IN_DRAM0
#define PLACE_IN_DRAM1
#endif

uint8_t        MEM_ALIGNER objinstance_memory[OBJINSTANCE_SIZE] PLACE_IN_DRAM1;
uint8_t        MEM_ALIGNER scratch_memory    [SCRATCH_SIZE    ] PLACE_IN_DRAM1;
tProfiler_data MEM_ALIGNER inp0                                 PLACE_IN_DRAM0;
tProfiler_data MEM_ALIGNER out0                                 PLACE_IN_DRAM0;
tProfiler_data MEM_ALIGNER inp1                                 PLACE_IN_DRAM1;
tProfiler_data MEM_ALIGNER out1                                 PLACE_IN_DRAM1;

tProfiler_data MEM_ALIGNER inp2                                 PLACE_IN_DRAM1;
tProfiler_data MEM_ALIGNER out2                                 PLACE_IN_DRAM1;


/* global constants */
const char* prf_ptscycle ="%d (%.1f pts/cycle)";
const char* prf_ptscycle2 ="%d (%.2f pts/cycle)";
const char* prf_ptscycle3 ="%d (%.3f pts/cycle)";
const char* prf_maccycle ="%d (%.1f MACs/cycle)";
const char* prf_cyclesmtx="%d (%.1f cycles/matrix)";
const char* prf_samplecycle = "%d (%.1f samples/cycle)";
const char* prf_cyclespts="%d (%.1f cycles/pts)";
const char* prf_cyclessample = "%d (%.1f cycles/sample)";
const char* prf_cycle ="%d (cycles)";
const char* prf_cyclesbqd =" %d (%.1f cycles/(biquad*pts)";
const char* prf_bytescycle ="%d (%.1f bytes/cycle)";

/* perf_info
  Works as printf() but duplicates output into a file (if f is not NULL)
 */
void perf_info(FILE * f, const char * fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  if (f) vfprintf(f, fmt, arg);
  vprintf(fmt, arg);
  va_end(arg);
}


void main_mips( int phaseNum, int isFull, int isVerbose, int breakOnError )
{
    (void)phaseNum; 
    (void)isFull;
    (void)isVerbose;
    (void)breakOnError;
#ifdef COMPILER_XTENSA
#if XCHAL_HAVE_CCOUNT==0 || !defined(XT_RSR_CCOUNT) || defined(NO_XT_RSR_CCOUNT)
    printf("CCOUNT register not present in the configuration or XT_RSR_CCOUNT not defined! Cycle measurements might be inaccurate!\n");
#endif
#endif
  Rand_reset(RAND_RESET_A, RAND_RESET_B);
  Rand_i16(inp0.i16, sizeof(inp0.i16)/sizeof(*inp0.i16));
  Rand_i16(inp1.i16, sizeof(inp1.i16)/sizeof(*inp1.i16));

  Rand_i32(inp2.i32, sizeof(inp2.i32)/sizeof(*inp2.i32));

}

