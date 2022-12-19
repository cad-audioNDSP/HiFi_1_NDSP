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
 * Test-engine add-on for FIR categories 
 */
#ifndef TESTENG__FIR_H__
#define TESTENG__FIR_H__

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"
/* Test data vectors tools and SEQ-file reader. */
#include "vectools.h"
#include "testeng.h"

/* FIR types (bits 9:8 from extraParam): */
#define TE_FIR_FIR (0<<8)
#define TE_FIR_DN  (1<<8) /* downsampler */
#define TE_FIR_UP  (2<<8) /* upsampler */


#define TE_FIR_OLD (16<<8)   /* legacy API */

/* function reads IR from file and creates FIR structure. returns 0 if failed */
int te_create_fir(tTestEngContext * context);

/* function destroys FIR structure, returns 0 if failed */
int te_destroy_fir(tTestEngContext * context);

/* 
   Allocate vectors and load the data set for FIR:
*  vector X (in), vector Z (out) */
int te_loadFxn_fir(tTestEngContext * context);

/* Apply FIR function to the test case data set.
*  vector X (in), vector Z (out) */
void te_processFxn_fir(tTestEngContext * context);

#endif
