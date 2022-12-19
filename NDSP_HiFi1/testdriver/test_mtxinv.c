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
 * Test procedures for matrix inversion and related functions
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* DSP Library API:  */
#include "NatureDSP_Signal.h"
/* Test engine API. */
#include "testeng_mtxinv.h"
#include <string.h>

static const tTestEngDesc descr_mtx_invf =  { FMT_REAL | FMT_FLOAT32, MTXINV_PLAIN,TE_ARGNUM_1, TE_ALIGN_NO, NULL, NULL, &te_loadFxn_mtxinv, &te_processFxn_matinv };
static const tTestEngDesc descr_cmtx_invf =  { FMT_CPLX | FMT_FLOAT32, MTXINV_PLAIN,TE_ARGNUM_1, TE_ALIGN_NO, NULL, NULL, &te_loadFxn_mtxinv, &te_processFxn_matinv };

static const tTestMtxinv tests[] =
{

  { &descr_mtx_invf, (tTestEngTarget)&mtx_inv2x2f, 1,"mtx_inv2x2f_cond10.seq"    },
  { &descr_mtx_invf, (tTestEngTarget)&mtx_inv2x2f, 1,"mtx_inv2x2f_cond100.seq"   },
  { &descr_mtx_invf, (tTestEngTarget)&mtx_inv2x2f, 1,"mtx_inv2x2f_cond1000.seq"  },
                                                   
  { &descr_mtx_invf, (tTestEngTarget)&mtx_inv3x3f, 1,"mtx_inv3x3f_cond10.seq"    },
  { &descr_mtx_invf, (tTestEngTarget)&mtx_inv3x3f, 1,"mtx_inv3x3f_cond100.seq"   },
  { &descr_mtx_invf, (tTestEngTarget)&mtx_inv3x3f, 1,"mtx_inv3x3f_cond1000.seq"  },

  { &descr_mtx_invf, (tTestEngTarget)&mtx_inv4x4f, 1,"mtx_inv4x4f_cond10.seq"    },
  { &descr_mtx_invf, (tTestEngTarget)&mtx_inv4x4f, 1,"mtx_inv4x4f_cond100.seq"   },
  { &descr_mtx_invf, (tTestEngTarget)&mtx_inv4x4f, 1,"mtx_inv4x4f_cond1000.seq"  },
  { NULL }  /* end of list */
};

static const tTestMtxinv tests3[] =
{

  { &descr_cmtx_invf, (tTestEngTarget)&cmtx_inv2x2f, 1,"cmtx_inv2x2f_cond10.seq"    },
  { &descr_cmtx_invf, (tTestEngTarget)&cmtx_inv2x2f, 1,"cmtx_inv2x2f_cond100.seq"   },
  { &descr_cmtx_invf, (tTestEngTarget)&cmtx_inv2x2f, 1,"cmtx_inv2x2f_cond1000.seq"  },
                                                   
  { &descr_cmtx_invf, (tTestEngTarget)&cmtx_inv3x3f, 1,"cmtx_inv3x3f_cond10.seq"    },
  { &descr_cmtx_invf, (tTestEngTarget)&cmtx_inv3x3f, 1,"cmtx_inv3x3f_cond100.seq"   },
  { &descr_cmtx_invf, (tTestEngTarget)&cmtx_inv3x3f, 1,"cmtx_inv3x3f_cond1000.seq"  },

  { &descr_cmtx_invf, (tTestEngTarget)&cmtx_inv4x4f, 1,"cmtx_inv4x4f_cond10.seq"    },
  { &descr_cmtx_invf, (tTestEngTarget)&cmtx_inv4x4f, 1,"cmtx_inv4x4f_cond100.seq"   },
  { &descr_cmtx_invf, (tTestEngTarget)&cmtx_inv4x4f, 1,"cmtx_inv4x4f_cond1000.seq"  },
  { NULL }  /* end of list */
};


/* Perform all tests for matrix inversion API functions. */
int main_mtxinv( int phaseNum,  int isFull, int isVerbose, int breakOnError )
{
  int ret1=0,ret2=0;
    printf( "\nMatrix inversions:\n" );
    if ( phaseNum == 0 || phaseNum == 2 )
    {
        ret1  = te_ExecMtxInv(tests,isFull,isVerbose,breakOnError);
    }
    if ( phaseNum == 0 || phaseNum == 3 )
    {
        ret2  = te_ExecMtxInv(tests3,isFull,isVerbose,breakOnError);
    }
  return (ret1&&ret2);
} /* main_mtxinv() */
