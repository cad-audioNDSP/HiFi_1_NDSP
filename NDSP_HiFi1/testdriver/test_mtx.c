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
 * Test procedures for matrix functions
 */
#include <string.h>

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Matrix functions API. */
#include "NatureDSP_Signal.h"
/* Test engine API. */
#include "testeng_mtx.h"

#define MAX_FUNC_NUM   10
/* Initializer for a function pointer array, appends NULL to a sequence of pointers. */
#define FUNC_LIST(...) { __VA_ARGS__, NULL }
/* Initializer for a test description structure. */
#define TEST_DESC( fmt, extra, argNum, align, loadFxn, procFxn ) { (fmt),extra,(argNum),(align),NULL,NULL,(loadFxn),(procFxn) }

/* vec API test definitions. */
static const struct 
{
tTestEngTarget   funcList[MAX_FUNC_NUM];
tTestEngDesc     testDesc;
}
testDefTbl[] =
{
    /*
     * Stage 1
     */
    { FUNC_LIST( (tTestEngTarget)&mtx_mpy16x16 ),
      TEST_DESC( FMT_REAL|FMT_FRACT16, MTX_PLAIN_LSH | MTX_SCR16X16,TE_ARGNUM_4, TE_ALIGN_NO, &te_loadFxn_mmlt, &te_processFxn_mmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_vecmpy16x16 ),
      TEST_DESC( FMT_REAL|FMT_FRACT16, MTX_PLAIN_LSH,TE_ARGNUM_4, TE_ALIGN_NO, &te_loadFxn_mmlt, &te_processFxn_vecmmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_mpy16x16_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT16, MTX_PLAIN_LSH | MTX_SCR16X16,TE_ARGNUM_4, TE_ALIGN_YES, &te_loadFxn_mmlt, &te_processFxn_mmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_vecmpy16x16_fast ),
      TEST_DESC( FMT_REAL|FMT_FRACT16, MTX_PLAIN_LSH,TE_ARGNUM_4, TE_ALIGN_YES, &te_loadFxn_mmlt, &te_processFxn_vecmmlt ) },

    { FUNC_LIST( (tTestEngTarget)&mtx_mpy24x24 ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN_LSH | MTX_SCR24X24,TE_ARGNUM_4, TE_ALIGN_NO, &te_loadFxn_mmlt_24, &te_processFxn_mmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_vecmpy24x24 ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN_LSH,TE_ARGNUM_4, TE_ALIGN_NO, &te_loadFxn_mmlt_24, &te_processFxn_vecmmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_mpy24x24_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN_LSH,TE_ARGNUM_4, TE_ALIGN_YES, &te_loadFxn_mmlt_24, &te_processFxn_mmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_vecmpy24x24_fast ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN_LSH,TE_ARGNUM_4, TE_ALIGN_YES, &te_loadFxn_mmlt_24, &te_processFxn_vecmmlt ) },

    /*
     * Stage 2
     */
    { FUNC_LIST( (tTestEngTarget)&mtx_mpyf ),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, MTX_PLAIN,TE_ARGNUM_4, TE_ALIGN_NO, &te_loadFxn_mmlt, &te_processFxn_mmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_vecmpyf ),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, MTX_PLAIN,TE_ARGNUM_4, TE_ALIGN_NO, &te_loadFxn_mmlt, &te_processFxn_vecmmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_mpyf_fast),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, MTX_PLAIN,TE_ARGNUM_4, TE_ALIGN_YES, &te_loadFxn_mmlt, &te_processFxn_mmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_vecmpyf_fast ),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, MTX_PLAIN,TE_ARGNUM_4, TE_ALIGN_YES, &te_loadFxn_mmlt, &te_processFxn_vecmmlt ) },

    /*
     * Stage 3
     */
    { FUNC_LIST( (tTestEngTarget)&mtx_mpy32x32 ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN_LSH | MTX_SCR24X24,TE_ARGNUM_4, TE_ALIGN_NO, &te_loadFxn_mmlt, &te_processFxn_mmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_vecmpy32x32 ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN_LSH,TE_ARGNUM_4, TE_ALIGN_NO, &te_loadFxn_mmlt, &te_processFxn_vecmmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_mpy32x32_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN_LSH,TE_ARGNUM_4, TE_ALIGN_YES, &te_loadFxn_mmlt, &te_processFxn_mmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_vecmpy32x32_fast ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN_LSH,TE_ARGNUM_4, TE_ALIGN_YES, &te_loadFxn_mmlt, &te_processFxn_vecmmlt ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_add2x2_16x16,(tTestEngTarget)&mtx_add3x3_16x16,(tTestEngTarget)&mtx_add4x4_16x16,
                 (tTestEngTarget)&mtx_sub2x2_16x16,(tTestEngTarget)&mtx_sub3x3_16x16,(tTestEngTarget)&mtx_sub4x4_16x16),
      TEST_DESC( FMT_REAL|FMT_FRACT16, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op0, &te_processFxn_mtx_op0 ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_mul2x2_16x16,(tTestEngTarget)&mtx_mul3x3_16x16,(tTestEngTarget)&mtx_mul4x4_16x16),
      TEST_DESC( FMT_REAL|FMT_FRACT16, MTX_PLAIN_LSH,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op0, &te_processFxn_mtx_op0 ) },
      { FUNC_LIST( (tTestEngTarget)&mtx_add2x2_32x32,(tTestEngTarget)&mtx_add3x3_32x32,(tTestEngTarget)&mtx_add4x4_32x32,
                 (tTestEngTarget)&mtx_sub2x2_32x32,(tTestEngTarget)&mtx_sub3x3_32x32,(tTestEngTarget)&mtx_sub4x4_32x32),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op0, &te_processFxn_mtx_op0 ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_mul2x2_32x32,(tTestEngTarget)&mtx_mul3x3_32x32,(tTestEngTarget)&mtx_mul4x4_32x32),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN_LSH,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op0, &te_processFxn_mtx_op0 ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_add2x2f,(tTestEngTarget)&mtx_add3x3f,(tTestEngTarget)&mtx_add4x4f,
                 (tTestEngTarget)&mtx_sub2x2f,(tTestEngTarget)&mtx_sub3x3f,(tTestEngTarget)&mtx_sub4x4f,
                 (tTestEngTarget)&mtx_mul2x2f,(tTestEngTarget)&mtx_mul3x3f,(tTestEngTarget)&mtx_mul4x4f),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op0, &te_processFxn_mtx_op0 ) },
    { FUNC_LIST( (tTestEngTarget)&cmtx_add2x2_16x16,(tTestEngTarget)&cmtx_add3x3_16x16,(tTestEngTarget)&cmtx_add4x4_16x16,
                 (tTestEngTarget)&cmtx_sub2x2_16x16,(tTestEngTarget)&cmtx_sub3x3_16x16,(tTestEngTarget)&cmtx_sub4x4_16x16),
      TEST_DESC( FMT_CPLX|FMT_FRACT16, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op0, &te_processFxn_mtx_op0 ) },
    { FUNC_LIST( (tTestEngTarget)&cmtx_mul2x2_16x16,(tTestEngTarget)&cmtx_mul3x3_16x16,(tTestEngTarget)&cmtx_mul4x4_16x16),
      TEST_DESC( FMT_CPLX|FMT_FRACT16, MTX_PLAIN_LSH,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op0, &te_processFxn_mtx_op0 ) },
    { FUNC_LIST( (tTestEngTarget)&cmtx_add2x2_32x32,(tTestEngTarget)&cmtx_add3x3_32x32,(tTestEngTarget)&cmtx_add4x4_32x32,
                 (tTestEngTarget)&cmtx_sub2x2_32x32,(tTestEngTarget)&cmtx_sub3x3_32x32,(tTestEngTarget)&cmtx_sub4x4_32x32),
      TEST_DESC( FMT_CPLX|FMT_FRACT32, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op0, &te_processFxn_mtx_op0 ) },
    { FUNC_LIST( (tTestEngTarget)&cmtx_mul2x2_32x32,(tTestEngTarget)&cmtx_mul3x3_32x32,(tTestEngTarget)&cmtx_mul4x4_32x32),
      TEST_DESC( FMT_CPLX|FMT_FRACT32, MTX_PLAIN_LSH,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op0, &te_processFxn_mtx_op0 ) },
    { FUNC_LIST( (tTestEngTarget)&cmtx_add2x2f,(tTestEngTarget)&cmtx_add3x3f,(tTestEngTarget)&cmtx_add4x4f,
                 (tTestEngTarget)&cmtx_sub2x2f,(tTestEngTarget)&cmtx_sub3x3f,(tTestEngTarget)&cmtx_sub4x4f,
                 (tTestEngTarget)&cmtx_mul2x2f,(tTestEngTarget)&cmtx_mul3x3f,(tTestEngTarget)&cmtx_mul4x4f),
      TEST_DESC( FMT_CPLX|FMT_FLOAT32, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op0, &te_processFxn_mtx_op0 ) },

    { FUNC_LIST( (tTestEngTarget)&mtx_tran2x2_16x16,(tTestEngTarget)&mtx_tran3x3_16x16,(tTestEngTarget)&mtx_tran4x4_16x16),
      TEST_DESC( FMT_REAL|FMT_FRACT16, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op1, &te_processFxn_mtx_op1 ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_tran2x2_32x32,(tTestEngTarget)&mtx_tran3x3_32x32,(tTestEngTarget)&mtx_tran4x4_32x32),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op1, &te_processFxn_mtx_op1 ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_tran2x2f,(tTestEngTarget)&mtx_tran3x3f,(tTestEngTarget)&mtx_tran4x4f),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op1, &te_processFxn_mtx_op1 ) },
    { FUNC_LIST( (tTestEngTarget)&cmtx_tran2x2_16x16,(tTestEngTarget)&cmtx_tran3x3_16x16,(tTestEngTarget)&cmtx_tran4x4_16x16),
      TEST_DESC( FMT_CPLX|FMT_FRACT16, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op1, &te_processFxn_mtx_op1 ) },
    { FUNC_LIST( (tTestEngTarget)&cmtx_tran2x2_32x32,(tTestEngTarget)&cmtx_tran3x3_32x32,(tTestEngTarget)&cmtx_tran4x4_32x32),
      TEST_DESC( FMT_CPLX|FMT_FRACT32, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op1, &te_processFxn_mtx_op1 ) },
    { FUNC_LIST( (tTestEngTarget)&cmtx_tran2x2f,(tTestEngTarget)&cmtx_tran3x3f,(tTestEngTarget)&cmtx_tran4x4f),
      TEST_DESC( FMT_CPLX|FMT_FLOAT32, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op1, &te_processFxn_mtx_op1 ) },

    { FUNC_LIST( (tTestEngTarget)&mtx_det2x2_16x16,(tTestEngTarget)&mtx_det3x3_16x16,(tTestEngTarget)&mtx_det4x4_16x16),
      TEST_DESC( FMT_REAL|FMT_FRACT16, MTX_PLAIN_LSH,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op2, &te_processFxn_mtx_op2 ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_det2x2_32x32,(tTestEngTarget)&mtx_det3x3_32x32,(tTestEngTarget)&mtx_det4x4_32x32),
      TEST_DESC( FMT_REAL|FMT_FRACT32, MTX_PLAIN_LSH,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op2, &te_processFxn_mtx_op2 ) },
    { FUNC_LIST( (tTestEngTarget)&mtx_det2x2f,(tTestEngTarget)&mtx_det3x3f,(tTestEngTarget)&mtx_det4x4f),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op2, &te_processFxn_mtx_op2 ) },
    { FUNC_LIST( (tTestEngTarget)&cmtx_det2x2_16x16,(tTestEngTarget)&cmtx_det3x3_16x16,(tTestEngTarget)&cmtx_det4x4_16x16),
      TEST_DESC( FMT_CPLX|FMT_FRACT16, MTX_PLAIN_LSH,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op2, &te_processFxn_mtx_op2 ) },
    { FUNC_LIST( (tTestEngTarget)&cmtx_det2x2_32x32,(tTestEngTarget)&cmtx_det3x3_32x32,(tTestEngTarget)&cmtx_det4x4_32x32),
      TEST_DESC( FMT_CPLX|FMT_FRACT32, MTX_PLAIN_LSH,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op2, &te_processFxn_mtx_op2 ) },
    { FUNC_LIST( (tTestEngTarget)&cmtx_det2x2f,(tTestEngTarget)&cmtx_det3x3f,(tTestEngTarget)&cmtx_det4x4f),
      TEST_DESC( FMT_CPLX|FMT_FLOAT32, MTX_PLAIN,TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_mtx_op2, &te_processFxn_mtx_op2 ) },

    { FUNC_LIST( (tTestEngTarget)&q2rotf),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, 0,TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_q2rot, &te_processFxn_vZvX ) },
    { FUNC_LIST( (tTestEngTarget)&q2rot_16x16),
      TEST_DESC( FMT_REAL|FMT_FRACT16, 0,TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_q2rot, &te_processFxn_vZvX ) },
    { FUNC_LIST( (tTestEngTarget)&q2rot_32x32),
      TEST_DESC( FMT_REAL|FMT_FRACT32, 0,TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_q2rot, &te_processFxn_vZvX ) },

    { FUNC_LIST( NULL ), TEST_DESC(  0,0, 0, 0, NULL, NULL ) } /* End of table */
};

/* Test executive function. Performs the specified test on a brief or full version
 * of the designated SEQ-file. Return the test result (non-zero if passed). */
static int testExec( tTestEngTarget   targetFxn, const char * seqName,
                     int isFull, int isVerbose, int breakOnError )
{
    return te_Exec(testDefTbl,sizeof(testDefTbl)/sizeof(testDefTbl[0]),MAX_FUNC_NUM,targetFxn, seqName,isFull, isVerbose, breakOnError);
}


/* Perform all tests for mat API functions. */
int main_mtx( int phaseNum, int isFull, int isVerbose, int breakOnError )
{
  int res = 1;

  printf( "\nMatrix functions:\n" );

  #define DO_TEST(fxn, seqFile) \
    if ( res || !breakOnError ) res &= ( 0 != testExec((tTestEngTarget)(fxn), (seqFile), \
                                         isFull, isVerbose, breakOnError ) )

  /*
   * Stage 1
   */

  if ( phaseNum == 0 || phaseNum == 1 )
  {
      DO_TEST( &mtx_mpy16x16        , "mtx_mpy16x16.seq"         );
      DO_TEST( &mtx_mpy16x16_fast   , "mtx_mpy16x16_fast.seq"    );
      DO_TEST( &mtx_vecmpy16x16     , "mtx_vecmpy16x16.seq"      );
      DO_TEST( &mtx_vecmpy16x16_fast, "mtx_vecmpy16x16_fast.seq" );
      DO_TEST( &mtx_mpy24x24        , "mtx_mpy24x24.seq"         );
      DO_TEST( &mtx_mpy24x24_fast   , "mtx_mpy24x24_fast.seq"    );
      DO_TEST( &mtx_vecmpy24x24     , "mtx_vecmpy24x24.seq"      );
      DO_TEST( &mtx_vecmpy24x24_fast, "mtx_vecmpy24x24_fast.seq" );
  }

  /*
   * Stage 2
   */

  if ( phaseNum == 0 || phaseNum == 2 )
  {
      DO_TEST( &mtx_mpyf        , "mtx_mpyf.seq"         );
      DO_TEST( &mtx_mpyf_fast   , "mtx_mpyf_fast.seq"    );
      DO_TEST( &mtx_vecmpyf     , "mtx_vecmpyf.seq"      );
      DO_TEST( &mtx_vecmpyf_fast, "mtx_vecmpyf_fast.seq" );
  }


  /*
   * Stage 3
   */

  if ( phaseNum == 0 || phaseNum == 3 )
  {
      DO_TEST( &mtx_mpy32x32        , "mtx_mpy32x32.seq"         );
      DO_TEST( &mtx_mpy32x32_fast   , "mtx_mpy32x32_fast.seq"    );
      DO_TEST( &mtx_vecmpy32x32     , "mtx_vecmpy32x32.seq"      );
      DO_TEST( &mtx_vecmpy32x32_fast, "mtx_vecmpy32x32_fast.seq" );

      DO_TEST( &mtx_add2x2_16x16   , "mtx_add2x2_16x16.seq"   );
      DO_TEST( &mtx_add3x3_16x16   , "mtx_add3x3_16x16.seq"   );
      DO_TEST( &mtx_add4x4_16x16   , "mtx_add4x4_16x16.seq"   );
      DO_TEST( &mtx_add2x2_32x32   , "mtx_add2x2_32x32.seq"   );
      DO_TEST( &mtx_add3x3_32x32   , "mtx_add3x3_32x32.seq"   );
      DO_TEST( &mtx_add4x4_32x32   , "mtx_add4x4_32x32.seq"   );
      DO_TEST( &mtx_add2x2f        , "mtx_add2x2f.seq"        );
      DO_TEST( &mtx_add3x3f        , "mtx_add3x3f.seq"        );
      DO_TEST( &mtx_add4x4f        , "mtx_add4x4f.seq"        );
      DO_TEST( &cmtx_add2x2_16x16  , "cmtx_add2x2_16x16.seq"  );
      DO_TEST( &cmtx_add3x3_16x16  , "cmtx_add3x3_16x16.seq"  );
      DO_TEST( &cmtx_add4x4_16x16  , "cmtx_add4x4_16x16.seq"  );
      DO_TEST( &cmtx_add2x2_32x32  , "cmtx_add2x2_32x32.seq"  );
      DO_TEST( &cmtx_add3x3_32x32  , "cmtx_add3x3_32x32.seq"  );
      DO_TEST( &cmtx_add4x4_32x32  , "cmtx_add4x4_32x32.seq"  );
      DO_TEST( &cmtx_add2x2f       , "cmtx_add2x2f.seq"       );
      DO_TEST( &cmtx_add3x3f       , "cmtx_add3x3f.seq"       );
      DO_TEST( &cmtx_add4x4f       , "cmtx_add4x4f.seq"       );
      DO_TEST( &mtx_sub2x2_16x16   , "mtx_sub2x2_16x16.seq"   );
      DO_TEST( &mtx_sub3x3_16x16   , "mtx_sub3x3_16x16.seq"   );
      DO_TEST( &mtx_sub4x4_16x16   , "mtx_sub4x4_16x16.seq"   );
      DO_TEST( &mtx_sub2x2_32x32   , "mtx_sub2x2_32x32.seq"   );
      DO_TEST( &mtx_sub3x3_32x32   , "mtx_sub3x3_32x32.seq"   );
      DO_TEST( &mtx_sub4x4_32x32   , "mtx_sub4x4_32x32.seq"   );
      DO_TEST( &mtx_sub2x2f        , "mtx_sub2x2f.seq"        );
      DO_TEST( &mtx_sub3x3f        , "mtx_sub3x3f.seq"        );
      DO_TEST( &mtx_sub4x4f        , "mtx_sub4x4f.seq"        );
      DO_TEST( &cmtx_sub2x2_16x16  , "cmtx_sub2x2_16x16.seq"  );
      DO_TEST( &cmtx_sub3x3_16x16  , "cmtx_sub3x3_16x16.seq"  );
      DO_TEST( &cmtx_sub4x4_16x16  , "cmtx_sub4x4_16x16.seq"  );
      DO_TEST( &cmtx_sub2x2_32x32  , "cmtx_sub2x2_32x32.seq"  );
      DO_TEST( &cmtx_sub3x3_32x32  , "cmtx_sub3x3_32x32.seq"  );
      DO_TEST( &cmtx_sub4x4_32x32  , "cmtx_sub4x4_32x32.seq"  );
      DO_TEST( &cmtx_sub2x2f       , "cmtx_sub2x2f.seq"       );
      DO_TEST( &cmtx_sub3x3f       , "cmtx_sub3x3f.seq"       );
      DO_TEST( &cmtx_sub4x4f       , "cmtx_sub4x4f.seq"       );
      DO_TEST( &mtx_mul2x2_16x16   , "mtx_mul2x2_16x16.seq"   );
      DO_TEST( &mtx_mul3x3_16x16   , "mtx_mul3x3_16x16.seq"   );
      DO_TEST( &mtx_mul4x4_16x16   , "mtx_mul4x4_16x16.seq"   );
      DO_TEST( &mtx_mul2x2_32x32   , "mtx_mul2x2_32x32.seq"   );
      DO_TEST( &mtx_mul3x3_32x32   , "mtx_mul3x3_32x32.seq"   );
      DO_TEST( &mtx_mul4x4_32x32   , "mtx_mul4x4_32x32.seq"   );
      DO_TEST( &mtx_mul2x2f        , "mtx_mul2x2f.seq"        );
      DO_TEST( &mtx_mul3x3f        , "mtx_mul3x3f.seq"        );
      DO_TEST( &mtx_mul4x4f        , "mtx_mul4x4f.seq"        );
      DO_TEST( &cmtx_mul2x2_16x16  , "cmtx_mul2x2_16x16.seq"  );
      DO_TEST( &cmtx_mul3x3_16x16  , "cmtx_mul3x3_16x16.seq"  );
      DO_TEST( &cmtx_mul4x4_16x16  , "cmtx_mul4x4_16x16.seq"  );
      DO_TEST( &cmtx_mul2x2_32x32  , "cmtx_mul2x2_32x32.seq"  );
      DO_TEST( &cmtx_mul3x3_32x32  , "cmtx_mul3x3_32x32.seq"  );
      DO_TEST( &cmtx_mul4x4_32x32  , "cmtx_mul4x4_32x32.seq"  );
      DO_TEST( &cmtx_mul2x2f       , "cmtx_mul2x2f.seq"       );
      DO_TEST( &cmtx_mul3x3f       , "cmtx_mul3x3f.seq"       );
      DO_TEST( &cmtx_mul4x4f       , "cmtx_mul4x4f.seq"       );
      DO_TEST( &mtx_tran2x2_16x16  , "mtx_tran2x2_16x16.seq"  );
      DO_TEST( &mtx_tran3x3_16x16  , "mtx_tran3x3_16x16.seq"  );
      DO_TEST( &mtx_tran4x4_16x16  , "mtx_tran4x4_16x16.seq"  );
      DO_TEST( &mtx_tran2x2_32x32  , "mtx_tran2x2_32x32.seq"  );
      DO_TEST( &mtx_tran3x3_32x32  , "mtx_tran3x3_32x32.seq"  );
      DO_TEST( &mtx_tran4x4_32x32  , "mtx_tran4x4_32x32.seq"  );
      DO_TEST( &mtx_tran2x2f       , "mtx_tran2x2f.seq"       );
      DO_TEST( &mtx_tran3x3f       , "mtx_tran3x3f.seq"       );
      DO_TEST( &mtx_tran4x4f       , "mtx_tran4x4f.seq"       );
      DO_TEST( &cmtx_tran2x2_16x16 , "cmtx_tran2x2_16x16.seq" );
      DO_TEST( &cmtx_tran3x3_16x16 , "cmtx_tran3x3_16x16.seq" );
      DO_TEST( &cmtx_tran4x4_16x16 , "cmtx_tran4x4_16x16.seq" );
      DO_TEST( &cmtx_tran2x2_32x32 , "cmtx_tran2x2_32x32.seq" );
      DO_TEST( &cmtx_tran3x3_32x32 , "cmtx_tran3x3_32x32.seq" );
      DO_TEST( &cmtx_tran4x4_32x32 , "cmtx_tran4x4_32x32.seq" );
      DO_TEST( &cmtx_tran2x2f      , "cmtx_tran2x2f.seq"      );
      DO_TEST( &cmtx_tran3x3f      , "cmtx_tran3x3f.seq"      );
      DO_TEST( &cmtx_tran4x4f      , "cmtx_tran4x4f.seq"      );
      DO_TEST( &mtx_det2x2_16x16   , "mtx_det2x2_16x16.seq"   );
      DO_TEST( &mtx_det3x3_16x16   , "mtx_det3x3_16x16.seq"   );
      DO_TEST( &mtx_det4x4_16x16   , "mtx_det4x4_16x16.seq"   );
      DO_TEST( &mtx_det2x2_32x32   , "mtx_det2x2_32x32.seq"   );
      DO_TEST( &mtx_det3x3_32x32   , "mtx_det3x3_32x32.seq"   );
      DO_TEST( &mtx_det4x4_32x32   , "mtx_det4x4_32x32.seq"   );
      DO_TEST( &mtx_det2x2f        , "mtx_det2x2f.seq"        );
      DO_TEST( &mtx_det3x3f        , "mtx_det3x3f.seq"        );
      DO_TEST( &mtx_det4x4f        , "mtx_det4x4f.seq"        );
      DO_TEST( &cmtx_det2x2_16x16  , "cmtx_det2x2_16x16.seq"  );
      DO_TEST( &cmtx_det3x3_16x16  , "cmtx_det3x3_16x16.seq"  );
      DO_TEST( &cmtx_det4x4_16x16  , "cmtx_det4x4_16x16.seq"  );
      DO_TEST( &cmtx_det2x2_32x32  , "cmtx_det2x2_32x32.seq"  );
      DO_TEST( &cmtx_det3x3_32x32  , "cmtx_det3x3_32x32.seq"  );
      DO_TEST( &cmtx_det4x4_32x32  , "cmtx_det4x4_32x32.seq"  );
      DO_TEST( &cmtx_det2x2f       , "cmtx_det2x2f.seq"       );
      DO_TEST( &cmtx_det3x3f       , "cmtx_det3x3f.seq"       );
      DO_TEST( &cmtx_det4x4f       , "cmtx_det4x4f.seq"       );
      DO_TEST( &q2rotf             , "q2rotf.seq"             );
      DO_TEST( &q2rot_16x16        , "q2rot_16x16.seq"        );
      DO_TEST( &q2rot_32x32        , "q2rot_32x32.seq"        );

  }

  return (res);

} /* main_mtx() */
