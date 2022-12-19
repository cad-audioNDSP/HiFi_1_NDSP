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
 * Test-engine add-on for matrix categories 
 */

#ifndef TESTENG_MTX_H__
#define TESTENG_MTX_H__

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Matrix functions API. */
#include "NatureDSP_Signal.h"
/* Test engine API. */
#include "testeng.h"

/* extraParam flags */
#define MTX_PLAIN      0    /* non-streaming data                        */
#define MTX_PLAIN_LSH  1    /* non-streaming data with aditional lsh/rsh     */
#define MTX_SCR16X16   2    /* use scratch SCRATCH_MTX_MPY16X16   */
#define MTX_SCR24X24   4    /* use scratch SCRATCH_MTX_MPY24x24   */

int te_loadFxn_mmlt( tTestEngContext * context ); /* Allocate vectors and load the data set for [c]matmmlt: * X[M][N], Y[N][P], Z[M][P] */
int te_loadFxn_mmlt_24( tTestEngContext * context ); /* Allocate vectors and load the data set for [c]matmmlt: * X[M][N], Y[N][P], Z[M][P] */
void te_processFxn_mmlt( tTestEngContext * context ); /* Apply the function under test to test case data set: [c]matmmlt */
void te_processFxn_vecmmlt( tTestEngContext * context ); /* Apply the function under test to test case data set: vecmpy */

/* Allocate vectors and load the data set for matrix operations:
 * X[L][N][N], Y[L][N][N], Z[L][N][N] */
int te_loadFxn_mtx_op0( tTestEngContext * context );
/* Allocate vectors and load the data set for matrix operations:
 * X[L][N][N], Z[L][N][N] */
int te_loadFxn_mtx_op1( tTestEngContext * context );
/* X[N][4], Z[N][9] */
int te_loadFxn_q2rot( tTestEngContext * context );

/* Apply the function under test to test case data set: 
X[L][N][N], Y[L][N][N], Z[L][N][N]  */
void te_processFxn_mtx_op0( tTestEngContext * context );
/* Apply the function under test to test case data set: 
X[L][N][N], Z[L][N][N]  */
void te_processFxn_mtx_op1( tTestEngContext * context );

/* Allocate vectors and load the data set for matrix operations:
 * X[L][N][N], Z[L] */
int te_loadFxn_mtx_op2( tTestEngContext * context );
/* Apply the function under test to test case data set: 
X[L][N][N] , Z[L] */
void te_processFxn_mtx_op2( tTestEngContext * context );


#endif
