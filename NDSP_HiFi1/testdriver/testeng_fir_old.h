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
 * Test-engine add-on for older FIR API
 */
#ifndef TESTENG__FIR_OLD_H__
#define TESTENG__FIR_OLD_H__

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"
/* Test data vectors tools and SEQ-file reader. */
#include "vectools.h"
#include "testeng.h"

typedef size_t tFirOldFxnAllocD(int D,int M);
typedef void * tFirOldFxnInitD (void * objmem, int D,int M, const void* h);
typedef size_t tFirOldFxnAlloc(int M);
typedef void * tFirOldFxnInit (void * objmem, int M, const void* h);
typedef void   tFirOldFxnProcess (void *handle, void * y, const void * x , int N);

typedef struct
{
    tFirOldFxnAlloc   *alloc;
    tFirOldFxnInit    *init;
    tFirOldFxnProcess *process;
}
tFirOldDescr;

/* function reads IR from file and creates FIR structure. returns 0 if failed */
int te_create_fir_old(tTestEngContext * context);
int te_create_fir_old_24(tTestEngContext * context);

/* function destroys FIR structure, returns 0 if failed */
int te_destroy_fir_old(tTestEngContext * context);

/* 
   Allocate vectors and load the data set for FIR:
*  vector X (in), vector Z (out) */
int te_loadFxn_fir_old(tTestEngContext * context);
int te_loadFxn_fir_old_24(tTestEngContext * context);

/* Apply FIR function to the test case data set.
*  vector X (in), vector Z (out) */
void te_processFxn_fir_old(tTestEngContext * context);

#endif
