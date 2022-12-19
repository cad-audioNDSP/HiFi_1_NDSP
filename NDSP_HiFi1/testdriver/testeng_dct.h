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
 * Test engine extension for DCT tests
 */

#ifndef __TESTENG_DCT_H
#define __TESTENG_DCT_H

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Test engine API. */ 
#include "testeng.h"
#include "testeng_fft.h"

/*
 * Test data frame processing for a standalone test. Apply an DCT routine to 16-bit
 * fixed point data read from the input file and convert the result to 64-bit
 * floating point samples.
 */

tTestEngFrameProcFxn_fft_stnd te_frameProc_stnd_dct;     /* <r|c>[i]DCT[d], <r|c>[i]DCTf[d|_fr16|_fr32] */
tTestEngFrameProcFxn_fft_stnd te_frameProc_stnd_scl_dct; /* <r|c>[i]DCT_fr<16|32>                       */

typedef struct tagTestEngDesc_dct
{
    tTestEngDesc desc;
    tFrwTransFxn frwTransFxn;  /* Forward transform function */
    tInvTransFxn invTransFxn;  /* Inverse transform function */

} tTestEngDesc_dct;

/* DCT test context; is accessible through tTestEngContext::target::handle */
typedef struct tagTestEngContext_DCT
{
  char fInName [64]; /* Input data filename                                   */
  char fRefName[64]; /* Reference data filename                               */ 
  int  scale_method; /* Scale method (for feature rich fixed point DCTs only) */
  int  frameCnt;     /* Frame counter                                         */

} tTestEngContext_dct;

/*
 * Test engine methods for DCT tests.
 */
 
/* Create a target algorithm instance and set tTestEngContext::target fields.
 * In particular, load twiddle factor tables. Return zero if failed. */
int te_create_dct( tTestEngContext * context );

/* Destroy the target algorithm instance and free memory block(s) allocated
 * for the target object. Return zero whenever any violation of memory area
 * bounds is detected. */
int te_destroy_dct( tTestEngContext * context );

/* Allocate in/out vectors for the next test case, and load the data set
 * from the SEQ-file. Return zero if failed. */
int te_load_dct( tTestEngContext * context );


#endif /* __TESTENG_DCT_H */
