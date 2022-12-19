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
    constants and definitions for testing MCPS performance of NatureDSP_Adp library
	Integrit, 2006-2015
*/
#ifndef MIPS_H__
#define MIPS_H__
#include "NatureDSP_Signal.h"
#include "profiler.h"
#include "utils.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "malloc16.h"
//#include "hwoptions.h"

typedef union tag_profiler_data
{
  uint8_t u8[16384 * 8];
  int8_t i8[16384*8];
  int16_t i16[8192*8];
  int32_t i32[4096*8];
  int64_t i64[4096*8];
  float32_t f32[4096*8];
  complex_float cf32[4096*4];
  complex_fract32 ci32[4096*4];
  complex_fract16 ci16[4096*8];
}
tProfiler_data;

#define OBJINSTANCE_SIZE  ( 10*4096)
#define SCRATCH_SIZE      (192*4096)

// external data (all aligned)
extern uint8_t objinstance_memory[OBJINSTANCE_SIZE];// PLACE_IN_DRAM1;
extern uint8_t scratch_memory    [SCRATCH_SIZE    ];// PLACE_IN_DRAM1;
extern tProfiler_data inp0,inp1,inp2,out0,out1,out2;

#define PROFILE_SIMPLE(_f_name, _f_arg, _file, _info, _fmt)       \
{                                                                 \
  if (NatureDSP_Signal_isPresent(_f_name))                        \
  {                                                               \
       PROFILE(_f_name _f_arg);                                   \
       perf_info(_file, "%-12s\t%-16s\t", #_f_name,_info);        \
       perf_info(_file, _fmt, profiler_clk);                      \
       perf_info(_file, "\n");                                    \
  }                                                               \
  else                                                            \
  {                                                               \
       perf_info(_file, "%-12s\t%-16s\t", #_f_name,_info);        \
       perf_info(_file, "NOT TESTED\n",#_f_name);                 \
  }                                                               \
}

#define PROFILE_INVERTED(_f_name, _f_arg, _file, _info, _fmt, _norm)   \
{                                                                 \
  float32_t __norm;                                               \
  if (NatureDSP_Signal_isPresent(_f_name))                        \
  {                                                               \
      PROFILE(_f_name _f_arg);                                    \
      __norm=(float32_t)(_norm)/profiler_clk;                     \
      perf_info(_file, "%-12s\t%-16s\t", #_f_name,_info);         \
      perf_info(_file, _fmt, profiler_clk,__norm);                \
      perf_info(_file, "\n");                                     \
  }                                                               \
  else                                                            \
  {                                                               \
       perf_info(_file, "%-12s\t%-16s\t", #_f_name,_info);        \
       perf_info(_file, "NOT TESTED\n",#_f_name);                 \
  }                                                               \
}

#define PROFILE_NORMALIZED(_f_name, _f_arg, _file, _info, _fmt, _norm)   \
{                                                                 \
  float32_t __norm;                                               \
  if (NatureDSP_Signal_isPresent(_f_name))                        \
  {                                                               \
  PROFILE(_f_name _f_arg);                                        \
  __norm=profiler_clk/(float32_t)(_norm);                         \
  perf_info(_file, "%-12s\t%-16s\t", #_f_name,_info);             \
  perf_info(_file, _fmt, profiler_clk,__norm);                    \
  perf_info(_file, "\n");                                         \
  }                                                               \
  else                                                            \
  {                                                               \
       perf_info(_file, "%-12s\t%-16s\t", #_f_name,_info);        \
       perf_info(_file, "NOT TESTED\n",#_f_name);                 \
  }                                                               \
}

#define OBJ_PROFILE_NORMALIZED(_objname, _a_arg, _i_arg, _p_arg, _file, _fmt, _norm)  \
{                                                                                     \
  int isPresent;                                                                      \
  isPresent =NatureDSP_Signal_isPresent(_objname##_alloc);                          \
  isPresent|=NatureDSP_Signal_isPresent(_objname##_init);                           \
  isPresent|=NatureDSP_Signal_isPresent(_objname##_process);                          \
  if (isPresent && (_objname##_alloc _a_arg <= sizeof(objinstance_memory)))           \
  {                                                                                     \
  _objname##_handle_t _objname;                                                           \
    _objname = _objname##_init _i_arg;                                                \
    if (_objname==NULL) isPresent=0;                                                  \
    if (isPresent)                                                                    \
    {                                                                                 \
    PROFILE(_objname##_process _p_arg);                                               \
    NORMALIZE(profiler_cpm, profiler_clk, _norm);                                     \
    profiler_cpm+=5;                                                                  \
    perf_info(_file, #_objname "_process\t" _fmt "\n",                                \
      profiler_clk, profiler_cpm/100, (profiler_cpm%100)/10);                         \
    }                                                                                 \
  }                                                                                   \
  if(!isPresent)                                                                      \
  {                                                                                   \
    perf_info(_file, #_objname "_process\t\tNOT TESTED\n");                           \
  }                                                                                   \
}

#define OBJ_PROFILE_INVERTED(_objname, _a_arg, _i_arg, _p_arg, _file,_info_,_fmt, _norm)\
{                                                                                       \
  int isPresent;                                                                        \
  _objname##_handle_t _objname;                                                         \
  isPresent =NatureDSP_Signal_isPresent(_objname##_alloc);                              \
  isPresent|=NatureDSP_Signal_isPresent(_objname##_init);                               \
  isPresent|=NatureDSP_Signal_isPresent(_objname##_process);                            \
  if (isPresent && (_objname##_alloc _a_arg <= sizeof(objinstance_memory)))             \
  {                                                                                     \
    _objname = _objname##_init _i_arg;                                                  \
    if (_objname==NULL) isPresent=0;                                                    \
    if (isPresent)                                                                      \
    {                                                                                   \
       PROFILE_INVERTED(_objname##_process, _p_arg, _file, _info_, _fmt, _norm)         \
    }                                                                                   \
  }                                                                                     \
  if(!isPresent)                                                                        \
  {                                                                                     \
    perf_info(_file, #_objname "_process\t\tNOT TESTED\n");                             \
  }                                                                                     \
}

#define PROFILE_INVERTED_FFT(_f_name,N, _f_arg, _file, _fmt, _norm) \
 if (NatureDSP_Signal_isPresent(_f_name))                         \
 {                                                                \
    PROFILE_INVERTED(_f_name, _f_arg, _file, "N="#N, _fmt, _norm) \
 }                                                                \
 else                                                             \
 {                                                                \
  perf_info(_file, #_f_name"\t\tNOT TESTED\n");                   \
 }

#define PROFILE_INVERTED_FFT_SC(_f_name,N, i, _f_arg, _file, _fmt, _norm)           \
if (NatureDSP_Signal_isPresent(_f_name))                                            \
 {                                                                                  \
    PROFILE_INVERTED(_f_name, _f_arg, _file, "N=" #N ", scaling=" #i , _fmt, _norm) \
 }                                                                                  \
 else                                                                               \
 {                                                                                  \
 perf_info(_file, #_f_name"\t\tNOT TESTED\n");                                      \
 }


/* perf_info
  Works as printf() but duplicates output into a file (if f is not NULL)
 */
void perf_info(FILE * f, const char * fmt, ...);

/* global constants */
extern const char* prf_ptscycle;
extern const char* prf_ptscycle2;
extern const char* prf_ptscycle3;
extern const char* prf_maccycle;
extern const char* prf_cyclesmtx;
#if 1
extern const char* prf_cyclespts;
extern const char* prf_samplecycle;
extern const char* prf_cyclessample;
extern const char* prf_cycle;
extern const char* prf_cyclesbqd;
extern const char* prf_bytescycle;
#endif

#endif
