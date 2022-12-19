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
 * Test procedures for arithmetic and logic functions on data vectors.
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* DSP Library API: arithmetic and logic functions on data vectors. */
#include "NatureDSP_Signal.h"
/* Test engine API. */
#include "testeng.h"
#include <string.h>

#define MIN(a,b)   ( (a)<(b) ? (a) : (b) )
#define MAX(a,b)   ( (a)>(b) ? (a) : (b) )

/* load data to the vec_polyxxx functions */
static int te_loadFxn_poly( tTestEngContext * context )
{
  int M, N;
  int nElemX, nElemY, nElemZ, res;

  ASSERT( context && context->seqFile );

  M = MAX( 0, context->args.N );
  N = MAX( 0, context->args.L );

  nElemX = M;
  nElemY = N;
  nElemZ = M;

  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate data vectors memory. */
  res=1;
  res &= ( 1 == vecsAlloc( context->desc->isAligned, context->desc->fmt,
                         &context->dataSet.X  , nElemX, 0 ) );
  res &= ( 1 == vecsAlloc( context->desc->isAligned, context->desc->fmt,
                         &context->dataSet.Y  , nElemY, 0 ) );
  res &= ( 1 == vecsAlloc( context->desc->isAligned, FMT_INT32,
                         &context->dataSet.Wlo , 1, 0 ) );
  res &= ( 3 == vecsAlloc( context->desc->isAligned, context->desc->fmt,
                         &context->dataSet.Z  , nElemZ,
                         &context->dataSet.Zlo, nElemZ,
                         &context->dataSet.Zhi, nElemZ, 0 ) );
  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &context->dataSet.X,
                                  &context->dataSet.Y,
                                  &context->dataSet.Wlo,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 0 ) ) )
    {
      printf( "te_loadFxn_poly(): failed to read vectors data; \n" );
    }
  }
  else
  {
    printf( "te_loadFxn_poly(): failed to allocate vectors\n");
  }
  /* Free vectors data if failed. */
  if ( !res ) freeVectors(context);
  return (res);
} /* te_loadFxn_poly() */

/* process data by the vec_polyxxx functions  */
void te_processFxn_poly( tTestEngContext * context )
{
  typedef void tFxn_fr16  ( fract16   * z, const fract16   * x, const fract16   *y, int lsh, int N );
  typedef void tFxn_fr32  ( fract32   * z, const fract32   * x, const fract32   *y, int lsh, int N );
  typedef void tFxn_fl32  ( float32_t * z, const float32_t * x, const float32_t *y, int N );

  tTestEngTarget   fxn;
  void *X, *Y, *Z, *W;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  W = vecGetElem( &context->dataSet.Wlo, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = context->target.fut;

  switch ( context->desc->fmt )
  {
  case FMT_REAL|FMT_FRACT16: ( (tFxn_fr16 *)fxn )((fract16  *)Z, (const fract16   *)X,(const fract16   *)Y, *(fract16        *)W,  N ); break;
  case FMT_REAL|FMT_FRACT32: ( (tFxn_fr32 *)fxn )((fract32  *)Z, (const fract32   *)X,(const fract32   *)Y, *(fract32        *)W,  N ); break;
  case FMT_REAL|FMT_FLOAT32: ( (tFxn_fl32 *)fxn )((float32_t*)Z, (const float32_t *)X,(const float32_t *)Y,  N ); break;
  default: ASSERT( 0 );
  }

} /* te_processFxn_poly() */

/****************************/
/* Apply the target function to the test case data set:
* vector X (in), scalar F0 (in), scalar F1 (in), scalar Y (in), vector Z (out) */
void te_processFxn_vXsF0sF1sYvZ(tTestEngContext * context)
{
  typedef void tFxn_fr16(const fract16         * x, fract16         y, fract16         * z, int N);
  typedef void tFxn_fr32(const fract32         * x, fract32         y, fract32         * z, int N);
  typedef void tFxn_fl32(float32_t       * z, const float32_t       * x, float32_t       y, float32_t       f0, float32_t       f1, int N);
  typedef void tFxn_fl64(const float64_t       * x, float64_t       y, float64_t       * z, int N);
  typedef void tFxn_fr16c(const complex_fract16 * x, complex_fract16 y, complex_fract16 * z, int N);
  typedef void tFxn_fr32c(const complex_fract32 * x, complex_fract32 y, complex_fract32 * z, int N);
  typedef void tFxn_fl32c(const complex_float   * x, complex_float   y, complex_float   * z, int N);
  typedef void tFxn_fl64c(const complex_double  * x, complex_double  y, complex_double  * z, int N);

  tTestEngTarget   fxn;
  void *X, *Y, *Z, *F0, *F1;
  int N;

  ASSERT(context && context->target.fut);

  X = vecGetElem(&context->dataSet.X, 0);
  Y = vecGetElem(&context->dataSet.Y, 0);
  F0 = vecGetElem(&context->dataSet.Wlo, 0);
  F1 = vecGetElem(&context->dataSet.Whi, 0);
  Z = vecGetElem(&context->dataSet.Z, 0);


  N = context->args.N;
  fxn = context->target.fut;

  switch (context->desc->fmt)
  {
  case FMT_REAL | FMT_FRACT16: ((tFxn_fr16 *)fxn)((const fract16        *)X, *(fract16        *)Y, (fract16         *)Z, N); break;
  case FMT_REAL | FMT_FRACT32: ((tFxn_fr32 *)fxn)((const fract32        *)X, *(fract32        *)Y, (fract32         *)Z, N); break;
  case FMT_REAL | FMT_FLOAT32: ((tFxn_fl32 *)fxn)((float32_t       *)Z, (const float32_t      *)X, *(float32_t      *)Y, *(float32_t      *)F0, *(float32_t      *)F1, N); break;
  case FMT_REAL | FMT_FLOAT64: ((tFxn_fl64 *)fxn)((const float64_t      *)X, *(float64_t      *)Y, (float64_t       *)Z, N); break;
  case FMT_CPLX | FMT_FRACT16: ((tFxn_fr16c*)fxn)((const complex_fract16*)X, *(complex_fract16*)Y, (complex_fract16 *)Z, N); break;
  case FMT_CPLX | FMT_FRACT32: ((tFxn_fr32c*)fxn)((const complex_fract32*)X, *(complex_fract32*)Y, (complex_fract32 *)Z, N); break;
  case FMT_CPLX | FMT_FLOAT32: ((tFxn_fl32c*)fxn)((const complex_float  *)X, *(complex_float  *)Y, (complex_float   *)Z, N); break;
  case FMT_CPLX | FMT_FLOAT64: ((tFxn_fl64c*)fxn)((const complex_double *)X, *(complex_double *)Y, (complex_double  *)Z, N); break;
  default: ASSERT(0);
  }

} /* te_processFxn_vXsF0sF1sYvZ() */
/******************************/

/****************/
/* Allocate vectors and load the data set:
* vector X (in), scalar F0 (in), scalar F1 (in), scalar Y (in), vector Z (out) */
int te_loadFxn_vXsF0sF1sYvZ(tTestEngContext * context)
{
  int M, N, L;
  int nElemX, nElemY, nElemZ, nElemW, res;

  ASSERT(context && context->seqFile);

  M = MAX(0, context->args.M);
  N = MAX(0, context->args.N);
  L = MAX(0, context->args.L);

  nElemX = M*N*L;
  nElemY = L;
  nElemW = L;
  nElemZ = M*N*L;

  memset(&context->dataSet, 0, sizeof(context->dataSet));

  /* Allocate data vectors memory. */
  res = (7 == vecsAlloc(context->desc->isAligned, context->desc->fmt,
    &context->dataSet.X, nElemX,
    &context->dataSet.Y, nElemY,
    &context->dataSet.Z, nElemZ,
    &context->dataSet.Wlo, nElemW,
    &context->dataSet.Whi, nElemW,
    &context->dataSet.Zlo, nElemZ,
    &context->dataSet.Zhi, nElemZ, 0));
  if (res)
  {
    /* Load vectors data from the SEQ-file. */
    if (!(res = seqFileReadVecs(context->seqFile,
      &context->dataSet.X,
      &context->dataSet.Y,
      &context->dataSet.Wlo,
      &context->dataSet.Whi,
      &context->dataSet.Zlo,
      &context->dataSet.Zhi, 0)))
    {
      printf("te_loadFxn_vXsF0sF1sYvZ(): failed to read vectors data; "
        "fmt = 0x%02x, nElemX = %d, nElemY = %d,  nElemW = %d, nElemZ = %d\n",
        (unsigned)context->desc->fmt, nElemX, nElemY, nElemW, nElemZ);
    }
  }
  else
  {
    printf("te_loadFxn_vXsF0sF1sYvZ(): failed to allocate vectors; "
      "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemW = %d, nElemZ = %d\n",
      (unsigned)context->desc->fmt, nElemX, nElemY, nElemW, nElemZ);
  }

  /* Free vectors data if failed. */
  if (!res) freeVectors(context);

  return (res);

} /* te_loadFxn_vXsF0sF1sYvZ() */


/* Test executive function. Performs the specified test on a brief or full version
 * of the designated SEQ-file. Return the test result (non-zero if passed). */
static int testExec( tTestEngTarget   targetFxn, const char * seqName,
                     int isFull, int isVerbose, int breakOnError );

/* Perform all tests for vec API functions. */
int main_vec( int phaseNum, int isFull, int isVerbose, int breakOnError )
{
  int res = 1;

  printf( "\nVector operations:\n" );

  #define DO_TEST(fxn, seqFile) \
    if ( res || !breakOnError ) res &= ( 0 != testExec( (tTestEngTarget)(fxn), (seqFile), \
                                                        isFull, isVerbose, breakOnError ) )

  /*
   * Stage 1
   */

  if ( phaseNum == 0 || phaseNum == 1 )
  {
        DO_TEST( &vec_dot16x16        , "vec_dot16x16.seq"        );
        DO_TEST( &vec_dot24x24        , "vec_dot24x24.seq"        );
        DO_TEST( &vec_dot32x16        , "vec_dot32x16.seq"        );
        DO_TEST( &vec_dot16x16_fast   , "vec_dot16x16_fast.seq"   );
        DO_TEST( &vec_dot24x24_fast   , "vec_dot24x24_fast.seq"   );
        DO_TEST( &vec_dot32x16_fast   , "vec_dot32x16_fast.seq"   );
        DO_TEST( &vec_add32x32        , "vec_add32x32.seq"        );
        DO_TEST( &vec_add24x24        , "vec_add24x24.seq"        );
        DO_TEST( &vec_add16x16        , "vec_add16x16.seq"        );
        DO_TEST( &vec_add32x32_fast   , "vec_add32x32_fast.seq"   );
        DO_TEST( &vec_add24x24_fast   , "vec_add24x24_fast.seq"   );
        DO_TEST( &vec_add16x16_fast   , "vec_add16x16_fast.seq"   );
        DO_TEST( &vec_power32x32      , "vec_power32x32.seq"      );
        DO_TEST( &vec_power24x24      , "vec_power24x24.seq"      );
        DO_TEST( &vec_power16x16      , "vec_power16x16.seq"      );
        DO_TEST( &vec_power32x32_fast , "vec_power32x32_fast.seq" );
        DO_TEST( &vec_power24x24_fast , "vec_power24x24_fast.seq" );
        DO_TEST( &vec_power16x16_fast , "vec_power16x16_fast.seq" );
        DO_TEST( &vec_shift32x32      , "vec_shift32x32.seq"      );
        DO_TEST( &vec_shift24x24      , "vec_shift24x24.seq"      );
        DO_TEST( &vec_shift16x16      , "vec_shift16x16.seq"      );
        DO_TEST( &vec_scale32x24      , "vec_scale32x24.seq"      );
        DO_TEST( &vec_scale24x24      , "vec_scale24x24.seq"      );
        DO_TEST( &vec_scale16x16      , "vec_scale16x16.seq"      );
        DO_TEST( &vec_shift32x32_fast , "vec_shift32x32_fast.seq" );
        DO_TEST( &vec_shift24x24_fast , "vec_shift24x24_fast.seq" );
        DO_TEST( &vec_shift16x16_fast , "vec_shift16x16_fast.seq" );
        DO_TEST( &vec_scale32x24_fast , "vec_scale32x24_fast.seq" );
        DO_TEST( &vec_scale24x24_fast , "vec_scale24x24_fast.seq" );
        DO_TEST( &vec_scale16x16_fast , "vec_scale16x16_fast.seq" );
        DO_TEST( &vec_min32x32        , "vec_min32x32.seq"        );
        DO_TEST( &vec_min24x24        , "vec_min24x24.seq"        );
        DO_TEST( &vec_min16x16        , "vec_min16x16.seq"        );
        DO_TEST( &vec_max32x32        , "vec_max32x32.seq"        );
        DO_TEST( &vec_max24x24        , "vec_max24x24.seq"        );
        DO_TEST( &vec_max16x16        , "vec_max16x16.seq"        );
        DO_TEST( &vec_min32x32_fast   , "vec_min32x32_fast.seq"   );
        DO_TEST( &vec_min24x24_fast   , "vec_min24x24_fast.seq"   );
        DO_TEST( &vec_min16x16_fast   , "vec_min16x16_fast.seq"   );
        DO_TEST( &vec_max32x32_fast   , "vec_max32x32_fast.seq"   );
        DO_TEST( &vec_max24x24_fast   , "vec_max24x24_fast.seq"   );
        DO_TEST( &vec_max16x16_fast   , "vec_max16x16_fast.seq"   );
        DO_TEST( &vec_poly4_32x32     , "vec_poly4_32x32.seq"     );
        DO_TEST( &vec_poly8_32x32     , "vec_poly8_32x32.seq"     );
        DO_TEST( &vec_poly4_24x24     , "vec_poly4_24x24.seq"     );
        DO_TEST( &vec_poly8_24x24     , "vec_poly8_24x24.seq"     );
  }
  /*
   * Stage 2
   */
  if ( phaseNum == 0 || phaseNum == 2 )
  {
        DO_TEST( &vec_dotf          , "vec_dotf.seq"           );
        DO_TEST( &vec_addf          , "vec_addf.seq"           );
        DO_TEST( &vec_powerf        , "vec_powerf.seq"         );
        DO_TEST( &vec_shiftf        , "vec_shiftf.seq"         );
        DO_TEST( &vec_scalef        , "vec_scalef.seq"         );
        DO_TEST (&vec_scale_sf      , "vec_scale_sf.seq"       );
        DO_TEST( &vec_minf          , "vec_minf.seq"           );
        DO_TEST( &vec_maxf          , "vec_maxf.seq"           );
        DO_TEST( &vec_poly4f        , "vec_poly4f.seq"         );
        DO_TEST( &vec_poly8f        , "vec_poly8f.seq"         );
  }

  /*
   * Stage 3
   */
  if ( phaseNum == 0 || phaseNum == 3 )
  {
        DO_TEST( &vec_dot32x32        , "vec_dot32x32.seq"        );
        DO_TEST( &vec_dot32x32_fast   , "vec_dot32x32_fast.seq"   );
        DO_TEST( &vec_poly4_16x16     , "vec_poly4_16x16.seq"     );
        DO_TEST( &vec_poly8_16x16     , "vec_poly8_16x16.seq"     );
  }

  return (res);

} /* main_vec() */

/* Test executive function. Performs the specified test on a brief or full version
 * of the designated SEQ-file. Return the test result (non-zero if passed). */
int testExec( tTestEngTarget   targetFxn, const char * seqName, 
              int isFull, int isVerbose, int breakOnError )
{
  #define MAX_FUNC_NUM   10
  /* Initializer for a function pointer array, appends NULL to a sequence of pointers. */
  #define FUNC_LIST(...) { __VA_ARGS__, NULL }
  /* Initializer for a test description structure. */
  #define TEST_DESC( fmt, argNum, align, loadFxn, procFxn ) { (fmt),0,(argNum),(align),NULL,NULL,(loadFxn),(procFxn) }

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

    {
      FUNC_LIST( (tTestEngTarget)&vec_dot16x16 ),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXvYsZ64, &te_processFxn_sZ64vXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_dot24x24 ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXvYsZ64_24, &te_processFxn_sZ64vXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_dot24x24_fast ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXvYsZ64_24, &te_processFxn_sZ64vXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_dot32x16 ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXvY16sZ64, &te_processFxn_sZ64vXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_dot32x16_fast ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXvY16sZ64, &te_processFxn_sZ64vXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_dot16x16_fast ),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXvYsZ32, &te_processFxn_sZ32vXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_add32x32, (tTestEngTarget)&vec_add24x24 ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXvYvZ, &te_processFxn_vZvXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_add32x32_fast, (tTestEngTarget)&vec_add24x24_fast ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXvYvZ, &te_processFxn_vZvXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_add16x16 ),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXvYvZ, &te_processFxn_vZvXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_add16x16_fast ),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXvYvZ, &te_processFxn_vZvXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_power16x16 ),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsY32sZ64, &te_processFxn_vXsY32sZ64 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_power16x16_fast ),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXsY32sZ64, &te_processFxn_vXsY32sZ64 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_power32x32 ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsY32sZ64, &te_processFxn_vXsY32sZ64 ) },
	{
      FUNC_LIST( (tTestEngTarget)&vec_power32x32_fast ), 
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXsY32sZ64, &te_processFxn_vXsY32sZ64 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_power24x24_fast ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXsY32sZ64_24, &te_processFxn_vXsY32sZ64 ) },
	{
      FUNC_LIST( (tTestEngTarget)&vec_power24x24 ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsY32sZ64_24, &te_processFxn_vXsY32sZ64 ) },
   	{ 
      FUNC_LIST( (tTestEngTarget)&vec_min32x32,(tTestEngTarget)&vec_min24x24,
                 (tTestEngTarget)&vec_max32x32,(tTestEngTarget)&vec_max24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsZ  , &te_processFxn_vXsZ ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_min32x32_fast,(tTestEngTarget)&vec_min24x24_fast,
                 (tTestEngTarget)&vec_max32x32_fast,(tTestEngTarget)&vec_max24x24_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXsZ  , &te_processFxn_vXsZ ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_min16x16, (tTestEngTarget)&vec_max16x16),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsZ  , &te_processFxn_vXsZ ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_min16x16_fast, (tTestEngTarget)&vec_max16x16_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXsZ  , &te_processFxn_vXsZ ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_scale32x24, (tTestEngTarget)&vec_scale24x24,
                 (tTestEngTarget)&vec_shift32x32, (tTestEngTarget)&vec_shift24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsYvZ, &te_processFxn_vZvXsY ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_scale32x24_fast, (tTestEngTarget)&vec_scale24x24_fast,
                 (tTestEngTarget)&vec_shift32x32_fast, (tTestEngTarget)&vec_shift24x24_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXsYvZ, &te_processFxn_vZvXsY ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_scale16x16), 
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsYvZ, &te_processFxn_vZvXsY ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_scale16x16_fast), 
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXsYvZ, &te_processFxn_vZvXsY ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_shift16x16), 
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsY32vZ, &te_processFxn_vZvXsY32 ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_shift16x16_fast), 
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXsY32vZ, &te_processFxn_vZvXsY32 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_poly4_32x32,(tTestEngTarget)&vec_poly8_32x32 ,
                 (tTestEngTarget)&vec_poly4_24x24,(tTestEngTarget)&vec_poly8_24x24 ),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_poly, &te_processFxn_poly ) },

    /*
     * Stage 2
     */
#if 1
    {
      FUNC_LIST( (tTestEngTarget)&vec_dotf ),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXvYsZ, &te_processFxn_vXvYsZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_addf),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXvYvZ, &te_processFxn_vZvXvY ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_scalef ),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsYvZ, &te_processFxn_vZvXsY ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_shiftf), 
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsY32vZ, &te_processFxn_vZvXsY32 ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_minf, (tTestEngTarget)&vec_maxf),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsZ  , &te_processFxn_vXsZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_powerf ),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsZ, &te_processFxn_vXsZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_poly4f,(tTestEngTarget)&vec_poly8f ),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_poly, &te_processFxn_poly ) },
    {
      FUNC_LIST((tTestEngTarget)&vec_scale_sf),
      TEST_DESC(FMT_REAL | FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsF0sF1sYvZ, &te_processFxn_vXsF0sF1sYvZ) },

#endif
      /* Phase 3*/
    {
      FUNC_LIST( (tTestEngTarget)&vec_dot32x32),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXvYsZ64, &te_processFxn_sZ64vXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_dot32x32_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXvYsZ64, &te_processFxn_sZ64vXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_poly4_16x16,(tTestEngTarget)&vec_poly8_16x16 ),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_poly, &te_processFxn_poly ) },

    { 
      FUNC_LIST( NULL ), TEST_DESC(  0, 0, 0, NULL, NULL ) } /* End of table */
  };

      return te_Exec(testDefTbl,sizeof(testDefTbl)/sizeof(testDefTbl[0]),MAX_FUNC_NUM,targetFxn, seqName,isFull, isVerbose, breakOnError);


} /* testExec() */
