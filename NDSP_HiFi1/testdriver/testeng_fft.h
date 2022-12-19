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
 * Test engine extension for FFT tests
 */

#ifndef __TESTENG_FFT_H
#define __TESTENG_FFT_H

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Test engine API. */ 
#include "testeng.h"

/* Test case types related to FFT. */
#define TE_FFT_TESTCASE_FORWARD    6
#define TE_FFT_TESTCASE_INVERSE    7
#define TE_FFT_TESTCASE_RECONST    8

/* tTestEngDesc::extraParam options for FFT tests. */
#define TE_FFT_REAL             0x01 /* Forward/inverse real-valued FFT               */
#define TE_FFT_CPLX             0x02 /* Forward/inverse complex-valued FFT            */
#define TE_FFT_BLOCKWISE        0x04 /* Blockwise FFT                                 */
#define TE_FFT_FULL_SPECTRUM    0x08 /* Real FFT forming full symmetric spectrum      */
#define TE_FFT_OPT_SCALE_METH   0x10 /* Scale method support (fixed point only)       */
#define TE_FFT_OPT_INPLACE      0x20 /* Input/outbut buffers may be aliased           */
#define TE_FFT_OPT_REUSE_INBUF  0x40 /* FFT reuses input buffer for intermediate data */
#define TE_DCT                  0x80 /* DCT type II                                   */
#define TE_FFT_TWD16            0x100 /* use 16-bit twiddle tables                    */
#define TE_FFT_PACKED24         0x200 /* packed 24-bit inputs/outputs                 */
#define TE_FFT_32X32            0x400 /* Forward/inverse FFT with 32 bits data and 32 bits twiddles */
#define TE_INVFFT_32X32         0x800 /* Inverse FFT with 32 bits data and 32 bits twiddles */

/*
 * Test data frame processing for a standalone test. Apply an FFT routine to 16-bit
 * fixed point data read from the input file and convert the result to 64-bit
 * floating point samples.
 */

typedef int tTestEngFrameProcFxn_fft_stnd( tTestEngContext * context,
                                     const fract16         * in,  /* 16-bit fixed point complex    */
                                           float64_t       * out, /* 64-bit floating point complex */
                                     tVec * xVec, tVec * yVec );  /* In/out vectors for target FFT */

tTestEngFrameProcFxn_fft_stnd te_frameProc_stnd_fft;     /* <r|c>[i]fft[d], <r|c>[i]fftf[d|_fr16|_fr32] */
tTestEngFrameProcFxn_fft_stnd te_frameProc_stnd_scl_fft; /* <r|c>[i]fft_fr<16|32>                       */
tTestEngFrameProcFxn_fft_stnd te_frameProc_stnd_blkfft;  /* blk<r|c>[i]fft[_fr16|_fr32]                 */

/* FFT test target */
typedef void (*tFrwTransFxn)( const void * x, void * y, const void * twdTbl,
                     int twdStep, int N, int * bexp, int scl_mtd );
typedef void (*tInvTransFxn)( const void * x, void * y, const void * twdTbl,
                     int twdStep, int N, int * bexp, int scl_mtd );

typedef struct tagTestEngDesc_fft
{
    tTestEngDesc desc;
    tFrwTransFxn frwTransFxn;  /* Forward transform function */
    tInvTransFxn invTransFxn;  /* Inverse transform function */

} tTestEngDesc_fft;

/* FFT test context; is accessible through tTestEngContext::target::handle */
typedef struct tagTestEngContext_fft
{
  char fInName [64]; /* Input data filename                                   */
  char fRefName[64]; /* Reference data filename                               */ 
  int  scale_method; /* Scale method (for feature rich fixed point FFTs only) */
  int  frameCnt;     /* Frame counter                                         */

} tTestEngContext_fft;

/*
 * Test engine methods for FFT tests.
 */
 
/* Create a target algorithm instance and set tTestEngContext::target fields.
 * In particular, load twiddle factor tables. Return zero if failed. */
int te_create_fft( tTestEngContext * context );

/* Destroy the target algorithm instance and free memory block(s) allocated
 * for the target object. Return zero whenever any violation of memory area
 * bounds is detected. */
int te_destroy_fft( tTestEngContext * context );

/* Allocate in/out vectors for the next test case, and load the data set
 * from the SEQ-file. Return zero if failed. */
int te_load_fft( tTestEngContext * context );

/* Return a pointer to twiddle factor table. If step parameter
 * is non-zero, then the table is selected from the set of available
 * tables in dependence of the test frame counter, with appropriate
 * stride amount returned through step. If step is zero, return the
 * twiddle table such that stride is 1. Return zero if found no table
 * for the requested FFT size. */
void * te_get_twd_tbl( tTestEngContext * context, int fftSize, int * step );


#endif /* __TESTENG_FFT_H */
