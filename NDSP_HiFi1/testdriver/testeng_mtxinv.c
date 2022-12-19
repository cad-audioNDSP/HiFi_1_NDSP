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
 * Test-engine add-on for matrix inversion categories 
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* DSP Library API:  */
#include "NatureDSP_Signal.h"
/* Test engine API. */
#include "testeng_mtxinv.h"
#include <string.h>

#define MAX(a,b)   ( (a)>(b) ? (a) : (b) )


int te_loadFxn_mtxinv(tTestEngContext * context)
{
    int ret,N;
    N=context->args.N;
    if(N>0) context->args.N=N*N; /* load and process N^2 elements */
    ret=te_loadFxn_vXvZ(context);
    context->args.N=N; /* get back N */
    return ret;
}

/* Apply the target function to the test case data set:
 * vector X (in), vector Z (out) */
void te_processFxn_matinv( tTestEngContext * context )
{
  typedef void tFxn      (const void * X, int N );
  void *X, *Z;
  ASSERT( context && context->target.fut );
  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );
  memcpy(Z,X,vecGetSize(&context->dataSet.X));
  switch(context->desc->extraParam)
  {
  case MTXINV_PLAIN:    ((tFxn*)context->target.fut)( Z, context->args.N ); break;
  default:              ASSERT(0);
  }
} /* te_processFxn_matinv() */


/* Perform all tests for matrix inversion API functions. */
int te_ExecMtxInv(const tTestMtxinv* pTbl, int isFull, int isVerbose, int breakOnError )
{
    int res = 1;
    for (;pTbl->pFunDescr;pTbl++)
    {
        if (isFull | pTbl->isFull)
        {
            res = TestEngRun(pTbl->fxns, pTbl->pFunDescr, pTbl->seqFile, isFull, isVerbose, breakOnError);
            if (res == 0 && breakOnError) break;
        }
    }
    return (res);
} /* te_ExecMtxInv() */
