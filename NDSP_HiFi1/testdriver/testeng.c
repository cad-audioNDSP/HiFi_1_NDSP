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
 * Test engine implementation
 */

#include <stdio.h>
#include <string.h>

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"
/* Test data vectors tools and SEQ-file reader. */
#include "vectools.h"
/* Utility functions. */
#include "utils.h"
/* Test engine API. */
#include "testeng.h"

 /* SEQ-file location for brief and full tests */
#ifndef TE_BRIEF_VECTOR_DIR
#define TE_BRIEF_VECTOR_DIR   "./../../vectors_brief/"
#endif
#ifndef TE_FULL_VECTOR_DIR
#define TE_FULL_VECTOR_DIR    "./../../vectors_full/"
#endif
#ifndef TE_SANITY_VECTOR_DIR
#define TE_SANITY_VECTOR_DIR   "./vectors_sanity/"
#endif


#define MIN(a,b)   ( (a)<(b) ? (a) : (b) )
#define MAX(a,b)   ( (a)>(b) ? (a) : (b) )

/* Test case semantic types. */
static const char * testCaseTypeStr[] = {
  "ACCURACY"                    , /*   0 */
  "KNOWN DATA"                  , /*   1 */
  "BAD SIZE/PARAM"              , /*   2 */
  "SOME AMOUNT OF OUTLIERS"     , /*   3 */
  "EDGE CASES"                  , /*   4 */
  "SPECIAL NUMBERS"             , /*   5 */
  "FORWARD FFT TEST"            , /*   6 */
  "INVERSE FFT TEST"            , /*   7 */
  "FFT RECONSTRUCTION TEST"     , /*   8 */
  "2D FFT ROW-WISE OVERFLOW"    , /*   9 */
  "2D FFT COLUMN-WISE OVERFLOW" , /*  10 */
  "2D FFT WGN"                  , /*  11 */
  "2D FFT SPECTRAL SPOTS"       , /*  12 */
  "UNKNOWN"                     , /* >12 */
};

/* Validate the test result. In the case of a failure optionally prints
 * a detailed report and returns zero. */
static int validateTestResult( tTestEngContext * context, int isVerbose ); 
/* Free vectors allocated for the test case. Return zero if guard memory areas check fails. */
int freeVectors( tTestEngContext * context );

/* Test executive function. Performs the specified test on a brief or full version
 * of the designated SEQ-file. Return the test result (non-zero if passed). */
int TestEngRun( tTestEngTarget   targetFxn, const tTestEngDesc * desc,
                const char * seqName, int isFull, int isVerbose, int breakOnError )
{
  tTestEngContext context;
  tSeqFile sf;

  const char * seqDir;
  char fnameBuf[256];
  int n, res = 1, testResult = 1, tcaseResult;
  int M, N, P, L;

  const int caseTypeLim = (int)sizeof(testCaseTypeStr)/sizeof(testCaseTypeStr[0]) - 1;

  ASSERT( /*targetFxn && */desc && seqName );

  Rand_reset( RAND_RESET_A, RAND_RESET_B );

  /*
   * Open the SEQ-file.
   */

  /* Select either full or brief variant of the SEQ-file. */
  //seqDir = ( isFull ? TE_FULL_VECTOR_DIR : TE_BRIEF_VECTOR_DIR );
  seqDir = ((isFull == 0) ? TE_FULL_VECTOR_DIR : (isFull == 2) ? TE_SANITY_VECTOR_DIR : TE_BRIEF_VECTOR_DIR);

  ASSERT( strlen(seqDir)+strlen(seqName) < sizeof(fnameBuf) );
  sprintf( fnameBuf, "%s%s", seqDir, seqName );
  
  printf( "%s... ", fnameBuf );

  res = ( 0 != ( sf = seqFileOpen( fnameBuf ) ) );

  if ( isVerbose ) printf( "\n" );

  /*
   * Prepare the test case context structure.
   */

  memset( &context, 0, sizeof(context) );
  context.desc       = desc;
  context.seqFile    = sf;
  context.isVerbose  = isVerbose;
  context.target.fut = targetFxn;

  ASSERT( strlen(seqDir) < sizeof(context.seqDir) );
  strcpy( context.seqDir, seqDir );

  if ( res )
  {
    ASSERT( TE_ARGNUM_1 <= desc->argNum && desc->argNum <= TE_ARGNUM_4 );

    if ( desc->argNum < TE_ARGNUM_2 ) L = 1;
    if ( desc->argNum < TE_ARGNUM_3 ) M = 1;
    if ( desc->argNum < TE_ARGNUM_4 ) P = 1;

    /* Initialize the Error Handling verification support. */
    te_errh_init( &context );
  }

  /*
   * Create the target algorithm instance (optional).
   */
  if(!NatureDSP_Signal_isPresent(targetFxn) )
  {
      res = -1;
  }
  if ( desc->targetCreateFxn && res>0 )
  {
    res = desc->targetCreateFxn( &context );
  }

  /*
   * Perform test cases until:
   *   A) SEQ-file is exhausted or
   *   B) test engine fails to perform a test case (e.g. because of a memory allocation error)
   *   C) some test fails, and breakOnError is true or isVerbose is false (there is no sense
   *      to continue testing of once failed function whenever isVerbose is false).
   */

  if ( !isVerbose ) breakOnError = 1;

  while ( sf && !feof(sf) && res>0 && ( testResult || !breakOnError ) )
  {
    /* Read the test case number and semantic type. */
    if ( !( res = ( 2 == seqFileScanf( sf, "%d %d",
                                       &context.args.caseNum,
                                       &context.args.caseType ) ) ) )
    {
      printf( "bad SEQ-file format (1)\n" );
      break;
    }

    if ( isVerbose )
    {
      n = MIN( context.args.caseType, caseTypeLim );
      printf( "#%-3d <%s> ", context.args.caseNum, testCaseTypeStr[n] );
    }

    /* Read dimensional parameters. */
    switch ( desc->argNum )
    {
    case TE_ARGNUM_1: n = seqFileScanf( sf, "%d"         , &N             ); break;
    case TE_ARGNUM_2: n = seqFileScanf( sf, "%d %d"      , &N, &L         ); break;
    case TE_ARGNUM_3: n = seqFileScanf( sf, "%d %d %d"   , &M, &N, &L     ); break;
    case TE_ARGNUM_4: n = seqFileScanf( sf, "%d %d %d %d", &M, &N, &P, &L ); break;
    }

    /* Verify that dimensional parameters have been read successfully. */
    if ( !( res = ( n == desc->argNum ) ) )
    {
      printf( "bad SEQ-file format (2)\n" );
      break;
    }

    context.args.M = M; context.args.N = N;
    context.args.P = P; context.args.L = L;

    /* Allocate data vectors and load test set data from the SEQ-file. */
    if ( !( res = desc->testCaseLoadFxn( &context ) ) ) break;
    /* Load reference error states, if any. */
    if ( !( res = te_errh_loadRef( &context ) ) ) break;

    /* Apply the target function. */
    desc->testCaseProcessFxn( &context );

    /* Validate the test case result. */
    tcaseResult = ( 0 != validateTestResult( &context, isVerbose ) );
    /* Attach the result of Error Handling validation, if applicable. */
    if ( context.errh.isEnabled ) tcaseResult &= ( 0 != context.errh.isPassed );
    /* Update the overall test result. */
    testResult &= tcaseResult;

    /* Free reference error states. */
    te_errh_freeRef( &context );
    /* Free test vectors. */
    if ( !freeVectors( &context ) ) res = 0;

    /* Print the test case result. */
    if ( isVerbose )
    {
      if (res == -1) printf("NOT TESTED\n");
      else  { //      printf((res && testResult) ? "OK\n" : "FAIL\n");
      if ( res && tcaseResult ) printf( "OK\n"   );
      else if ( !breakOnError ) printf( "FAIL\n" );
      }
    }
  }

  /* Destroy the algorithm instance. */
  if ( desc->targetDestroyFxn )
  {
    desc->targetDestroyFxn( &context );
  }

  /* Close the SEQ-file. */
  if ( sf ) seqFileClose( sf );

  /* Print the overall test result. */
  if (res==-1) printf( "NOT TESTED\n" );
  else         printf( ( res && testResult ) ? "OK\n" : "FAIL\n" );
  fflush(stdout);
  return ( res && testResult );

} /* TestEngRun() */

/* Validate the test result. In the case of a failure optionally prints
 * a detailed report and returns zero. */
static int validateTestResult( tTestEngContext * context, int isVerbose )
{
  int failIx, ix, resZ = 1, resW = 1;
  int M, N, P, L;
  char buf[64];

  const tVec *X, *Y, *Z, *W;
  const tVec *Zlo, *Zhi, *Wlo, *Whi;

  ASSERT( context );

  X   = &context->dataSet.X;
  Y   = &context->dataSet.Y;
  Z   = &context->dataSet.Z;
  W   = &context->dataSet.W;
  Zlo = &context->dataSet.Zlo;
  Zhi = &context->dataSet.Zhi;
  Wlo = &context->dataSet.Wlo;
  Whi = &context->dataSet.Whi;

  if ( Z->nElem > 0 ) resZ = ( 0 != vecCheckRange( Z, Zlo, Zhi, &failIx ) );
  if ( W->nElem > 0 ) resW = ( 0 != vecCheckRange( W, Wlo, Whi, &failIx ) );

  if ( isVerbose && ( !resZ || !resW ) )
  {
    const char * legend[] = { "s Z,W", " Z", " W", "" };

    printf( "\n##### start of error report #####\n" );

    M = context->args.M; N = context->args.N;
    P = context->args.P; L = context->args.L;

    switch ( context->desc->argNum )
    {
    case TE_ARGNUM_1: printf( "Dimensions: N=%d\n"               , N          ); break;
    case TE_ARGNUM_2: printf( "Dimensions: N=%d L=%d\n"          , N, L       ); break;
    case TE_ARGNUM_3: printf( "Dimensions: M=%d N=%d L=%d\n"     , M, N, L    ); break;
    case TE_ARGNUM_4: printf( "Dimensions: M=%d N=%d P=%d L=%d\n", M, N, P, L ); break;
    default: ASSERT(0);
    }

    printf( "Test result validation failed for vector%s at index %d\n", legend[(resZ<<1)|resW], failIx );
    printf( "Input: " );

    if ( X->nElem > 0 )    
    {
      ix = MIN( X->nElem-1, failIx );
      vecPrintElem( buf, X, ix );
      printf( "x[%d] = %s; ", ix, buf );
    }

    if ( Y->nElem > 0 )    
    {
      ix = MIN( Y->nElem-1, failIx );
      vecPrintElem( buf, Y, ix );
      printf( "y[%d] = %s; ", ix, buf );
    }

    if ( Z->nElem > 0 )
    {
      printf( "\nResult range: " );

      ix = MIN( Zlo->nElem-1, failIx );
      vecPrintElem( buf, Zlo, ix );
      printf( "zlo[%d] = %s; ", ix, buf );

      ix = MIN( Zhi->nElem-1, failIx );
      vecPrintElem( buf, Zhi, ix );
      printf( "zhi[%d] = %s; ", ix, buf );

      printf( "\nActual result: " );

      ix = MIN( Z->nElem-1, failIx );
      vecPrintElem( buf, Z, ix );
      printf( "z[%d] = %s; ", ix, buf );
    }

    if ( W->nElem > 0 )
    {
      printf( "\nResult range: " );

      ix = MIN( Wlo->nElem-1, failIx );
      vecPrintElem( buf, Wlo, ix );
      printf( "wlo[%d] = %s; ", ix, buf );

      ix = MIN( Whi->nElem-1, failIx );
      vecPrintElem( buf, Whi, ix );
      printf( "whi[%d] = %s; ", ix, buf );

      printf( "\nActual result: " );

      ix = MIN( W->nElem-1, failIx );
      vecPrintElem( buf, W, ix );
      printf( "w[%d] = %s; ", ix, buf );
    }

    printf( "\n##### end of error report #####\n" );
  }

  return ( resZ && resW );

} /* validateTestResult() */

/* Free vectors allocated for the test case. Return zero if guard memory areas check fails. */
int freeVectors( tTestEngContext * context )
{
  ASSERT( context );

  return ( ( !context->dataSet.X  .szBulk || 0 != vecFree( &context->dataSet.X   ) ) &
           ( !context->dataSet.Y  .szBulk || 0 != vecFree( &context->dataSet.Y   ) ) &
           ( !context->dataSet.W  .szBulk || 0 != vecFree( &context->dataSet.W   ) ) &
           ( !context->dataSet.Wlo.szBulk || 0 != vecFree( &context->dataSet.Wlo ) ) &
           ( !context->dataSet.Whi.szBulk || 0 != vecFree( &context->dataSet.Whi ) ) &
           ( !context->dataSet.Z  .szBulk || 0 != vecFree( &context->dataSet.Z   ) ) &
           ( !context->dataSet.Zlo.szBulk || 0 != vecFree( &context->dataSet.Zlo ) ) &
           ( !context->dataSet.Zhi.szBulk || 0 != vecFree( &context->dataSet.Zhi ) ) );

} /* freeVectors() */

/* Allocate vectors and load the data set:
 * vector X (in), scalar Y (in), vector Z (out) */
int te_loadFxn_vXsYvZ( tTestEngContext * context )
{
  int M, N, L;
  int nElemX, nElemY, nElemZ, res;

  ASSERT( context && context->seqFile );

  M = MAX( 0, context->args.M );
  N = MAX( 0, context->args.N );
  L = MAX( 0, context->args.L );

  nElemX = M*N*L;
  nElemY = L;
  nElemZ = M*N*L;

  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate data vectors memory. */
  res = ( 5 == vecsAlloc( context->desc->isAligned, context->desc->fmt,
                         &context->dataSet.X  , nElemX,
                         &context->dataSet.Y  , nElemY,
                         &context->dataSet.Z  , nElemZ,
                         &context->dataSet.Zlo, nElemZ,
                         &context->dataSet.Zhi, nElemZ, 0 ) );
  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &context->dataSet.X,
                                  &context->dataSet.Y,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 0 ) ) )
    {
      printf( "te_loadFxn_vXsYvZ(): failed to read vectors data; "
              "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
              (unsigned)context->desc->fmt, nElemX, nElemY, nElemZ );
    }
  }
  else
  {
    printf( "te_loadFxn_vXsYvZ(): failed to allocate vectors; "
            "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemY, nElemZ );
  }

  /* Free vectors data if failed. */
  if ( !res ) freeVectors(context);

  return (res);

} /* te_loadFxn_vXsY32vZ() */


/* vector X (in), scalar int32 Y (in), vector Z (out) */
static int te_loadFxn_vXsYvZ_helper( tTestEngContext * context, int fmtX, int fmtY, int fmtZ )
{
  int M, N, L;
  int nElemX, nElemY, nElemZ, res;

  ASSERT( context && context->seqFile );

  M = MAX( 0, context->args.M );
  N = MAX( 0, context->args.N );
  L = MAX( 0, context->args.L );

  nElemX = M*N*L;
  nElemY = L;
  nElemZ = M*N*L;

  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate data vectors memory. */
  res=1;
  res &= ( 1 == vecsAlloc( context->desc->isAligned, fmtX,
                         &context->dataSet.X  , nElemX, 0 ) );
  res &= ( 1 == vecsAlloc( context->desc->isAligned, fmtY,
                         &context->dataSet.Y  , nElemY, 0 ) );
  res &= ( 3 == vecsAlloc( context->desc->isAligned, fmtZ,
                         &context->dataSet.Z  , nElemZ,
                         &context->dataSet.Zlo, nElemZ,
                         &context->dataSet.Zhi, nElemZ, 0 ) );
  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &context->dataSet.X,
                                  &context->dataSet.Y,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 0 ) ) )
    {
      printf( "te_loadFxn_vXsYvZ_helper(): failed to read vectors data; "
              "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
              (unsigned)context->desc->fmt, nElemX, nElemY, nElemZ );
    }
  }
  else
  {
    printf( "te_loadFxn_vXsYvZ_helper(): failed to allocate vectors; "
            "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemY, nElemZ );
  }
  /* Free vectors data if failed. */
  if ( !res ) freeVectors(context);
  return (res);

} /* te_loadFxn_vXsYvZ_helper() */

/* vector X (in), scalar int32 Y (in), vector Z (out) */
int te_loadFxn_vXsY32vZ( tTestEngContext * context )
{
    return te_loadFxn_vXsYvZ_helper(context,context->desc->fmt,FMT_FRACT32,context->desc->fmt);
}
/* vector int32 X (in), scalar int32 Y (in), vector Z (out) */
int te_loadFxn_vX32sY32vZ( tTestEngContext * context )
{
    return te_loadFxn_vXsYvZ_helper(context,FMT_FRACT32,FMT_FRACT32,context->desc->fmt);
}
/* vector X (in), scalar int32 Y (in), vector int32 Z (out) */
int te_loadFxn_vXsY32vZ32( tTestEngContext * context )
{
    return te_loadFxn_vXsYvZ_helper(context,context->desc->fmt,FMT_FRACT32,FMT_FRACT32);
}

/* Allocate vectors and load the data set:
 * vector X (in), vector Y (in), vector Z (out) */
static int te_loadFxn_vXvYvZ_helper( tTestEngContext * context, int fmtX, int fmtY, int fmtZ )
{
  int M, N, L;
  int nElem, res;

  ASSERT( context && context->seqFile );

  M = MAX( 0, context->args.M );
  N = MAX( 0, context->args.N );
  L = MAX( 0, context->args.L );

  nElem = M*N*L;

  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate data vectors memory. */
  res = ( 1 == vecsAlloc( context->desc->isAligned, fmtX,
                         &context->dataSet.X  , nElem, 0 ) );
  res&= ( 1 == vecsAlloc( context->desc->isAligned, fmtY,
                         &context->dataSet.Y  , nElem, 0 ) );
  res&= ( 3 == vecsAlloc( context->desc->isAligned, fmtZ,
                         &context->dataSet.Z  , nElem,
                         &context->dataSet.Zlo, nElem,
                         &context->dataSet.Zhi, nElem, 0 ) );

  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &context->dataSet.X,
                                  &context->dataSet.Y,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 0 ) ) )
    {
      printf( "te_loadFxn_vXvYvZ_helper(): failed to read vectors data; "
              "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
              (unsigned)context->desc->fmt, nElem, nElem, nElem );
    }
  }
  else
  {
    printf( "te_loadFxn_vXvYvZ_helper(): failed to allocate vectors; "
            "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElem, nElem, nElem );
  }

  /* Free vectors data if failed. */
  if ( !res ) freeVectors(context);

  return (res);

} /* te_loadFxn_vXvYvZ_helper() */


/* Allocate vectors and load the data set:
 * vector X (in), vector Y (in), vector Z (out) */
int te_loadFxn_vXvYvZ( tTestEngContext * context )
{
    return te_loadFxn_vXvYvZ_helper(context, context->desc->fmt,context->desc->fmt,context->desc->fmt);
}
int te_loadFxn_vXvYvZf( tTestEngContext * context )
{
    return te_loadFxn_vXvYvZ_helper(context, context->desc->fmt,context->desc->fmt,FMT_FLOAT32);
}
int te_loadFxn_vXvYvZd( tTestEngContext * context )
{
    return te_loadFxn_vXvYvZ_helper(context, context->desc->fmt,context->desc->fmt,FMT_FLOAT64);
}


/* Allocate vectors and load the data set:
 * vector X (in), vector Y (in), scalar Z (out) */
int te_loadFxn_vXvYsZ( tTestEngContext * context )
{
  int M, N, L;
  int nElemX, nElemY, nElemZ, res;

  ASSERT( context && context->seqFile );

  M = MAX( 0, context->args.M );
  N = MAX( 0, context->args.N );
  L = MAX( 0, context->args.L );

  nElemX = M*N*L;
  nElemY = M*N*L;
  nElemZ = L;

  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate data vectors memory. */
  res = ( 5 == vecsAlloc( context->desc->isAligned, context->desc->fmt,
                         &context->dataSet.X  , nElemX,
                         &context->dataSet.Y  , nElemY,
                         &context->dataSet.Z  , nElemZ,
                         &context->dataSet.Zlo, nElemZ,
                         &context->dataSet.Zhi, nElemZ, 0 ) );
  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &context->dataSet.X,
                                  &context->dataSet.Y,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 0 ) ) )
    {
      printf( "te_loadFxn_vXvYsZ(): failed to read vectors data; "
              "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
              (unsigned)context->desc->fmt, nElemX, nElemY, nElemZ );
    }
  }
  else
  {
    printf( "te_loadFxn_vXvYsZ(): failed to allocate vectors; "
            "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemY, nElemZ );
  }

  /* Free vectors data if failed. */
  if ( !res ) freeVectors(context);
  return (res);

} /* te_loadFxn_vXvYsZ() */

/* Allocate vectors and load the data set:
 * vector X (in), vector Z (out) */
static int te_loadFxn_vXvZ_helper( tTestEngContext * context, int fmtX, int fmtZ )
{
  int M, N, L;
  int nElem, res;

  ASSERT( context && context->seqFile );

  M = MAX( 0, context->args.M );
  N = MAX( 0, context->args.N );
  L = MAX( 0, context->args.L );

  nElem = M*N*L;

  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate data vectors memory. */
  res = ( 1 == vecsAlloc( context->desc->isAligned, fmtX,
                         &context->dataSet.X  , nElem, 0 ) );
  res&= ( 3 == vecsAlloc( context->desc->isAligned, fmtZ,
                         &context->dataSet.Z  , nElem,
                         &context->dataSet.Zlo, nElem,
                         &context->dataSet.Zhi, nElem, 0 ) );
  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &context->dataSet.X,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 0 ) ) )
    {
      printf( "te_loadFxn_vXvZ_helper(): failed to read vectors data; "
              "fmt = 0x%02x, nElemX = %d, nElemZ = %d\n",
              (unsigned)fmtX, nElem, nElem );
    }
  }
  else
  {
    printf( "te_loadFxn_vXvZ_helper(): failed to allocate vectors; "
            "fmt = 0x%02x, nElemX = %d, nElemZ = %d\n",
            (unsigned)fmtZ, nElem, nElem );
  }

  /* Free vectors data if failed. */
  if ( !res ) freeVectors(context);
  return (res);

} /* te_loadFxn_vXvZ_helper() */

/* Allocate vectors and load the data set:
 * vector X (in), vector Z (out) */
int te_loadFxn_vXvZ( tTestEngContext * context )
{
    return te_loadFxn_vXvZ_helper(context,context->desc->fmt,context->desc->fmt);
} /* te_loadFxn_vXvZ() */

/* Allocate vectors and load the data set:
 * vector complex X (in), vector Z (out) */
int te_loadFxn_vXcvZ( tTestEngContext * context )
{
    return te_loadFxn_vXvZ_helper(context,context->desc->fmt | FMT_CPLX,context->desc->fmt);
} /* te_loadFxn_vXvZ() */

int te_loadFxn_vXvZf( tTestEngContext * context )
{
    return te_loadFxn_vXvZ_helper(context,context->desc->fmt,FMT_FLOAT32);
} /* te_loadFxn_vXvZf() */
int te_loadFxn_vXvZd( tTestEngContext * context )
{
    return te_loadFxn_vXvZ_helper(context,context->desc->fmt,FMT_FLOAT64);
} /* te_loadFxn_vXvZd() */

/* Allocate vectors and load the data set:
 * vector X (in), scalar int32 Z (out) */
static int te_loadFxn_vXsZ_helper( tTestEngContext * context, int fmtX, int fmtZ )
{
  int M, N, L;
  int nElemX, nElemZ, res = 0;

  ASSERT( context && context->seqFile );

  M = MAX( 0, context->args.M );
  N = MAX( 0, context->args.N );
  L = MAX( 0, context->args.L );

  nElemX = M*N*L;
  nElemZ = L;

  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate input data vector memory. */
  if ( !vecAlloc( &context->dataSet.X, nElemX, 
                   context->desc->isAligned, 
                   fmtX, 0 ) )
  {
    printf( "te_loadFxn_vXsZ_helper(): failed to allocate vector X; "
            "fmt = 0x%02x, nElemX = %d\n", (unsigned)context->desc->fmt, nElemX );
  }
  /* Allocate output and reference data vectors memory. */
  else if ( 3 != vecsAlloc( context->desc->isAligned, fmtZ,
                           &context->dataSet.Z  , nElemZ,
                           &context->dataSet.Zlo, nElemZ,
                           &context->dataSet.Zhi, nElemZ, 0 ) )
  {
    printf( "te_loadFxn_vXsZ_helper(): failed to allocate vectors Z/Zlo/Zhi; "
            "fmt = 0x%02x, nElemZ = %d\n", FMT_INT32, nElemZ );
  }
  /* Load vectors data from the SEQ-file. */
  else if ( !seqFileReadVecs( context->seqFile,
                             &context->dataSet.X,
                             &context->dataSet.Zlo,
                             &context->dataSet.Zhi, 0 ) )
  {
    printf( "te_loadFxn_vXsZ_helper(): failed to read vectors data; "
            "fmt = 0x%02x, nElemX = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemZ );
  }
  else
  {
    res = 1;
  }

  /* Free vectors data if failed. */
  if ( !res ) freeVectors(context);
  return (res);

} /* te_loadFxn_vXsZ32_helper() */

int te_loadFxn_vXsZ( tTestEngContext * context )
{
    return te_loadFxn_vXsZ_helper(context, context->desc->fmt, context->desc->fmt);
}

int te_loadFxn_vXsZ32( tTestEngContext * context )
{
    return te_loadFxn_vXsZ_helper(context, context->desc->fmt, FMT_REAL|FMT_INT32);
}

int te_loadFxn_vX32sZ( tTestEngContext * context )
{
    return te_loadFxn_vXsZ_helper(context, FMT_REAL|FMT_INT32, context->desc->fmt);
}

int te_loadFxn_vXsZ16( tTestEngContext * context )
{
    return te_loadFxn_vXsZ_helper(context, context->desc->fmt, FMT_REAL|FMT_INT16);
}


/* Allocate vectors and load the data set:
 * vector X (in), scalar int32 Y (in), scalar int64 Z (out) */
int te_loadFxn_vXsY32sZ64( tTestEngContext * context )
{
  int M, N, L;
  int nElemX, nElemZ, res = 0;

  ASSERT( context && context->seqFile );

  M = MAX( 0, context->args.M );
  N = MAX( 0, context->args.N );
  L = MAX( 0, context->args.L );

  nElemX = M*N*L;
  nElemZ = L;

  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate input data vector memory. */
  if ( !vecAlloc( &context->dataSet.X, nElemX, 
                   context->desc->isAligned, 
                   context->desc->fmt, 0 ) )
  {
    printf( "te_loadFxn_vXsZ32(): failed to allocate vector X; "
            "fmt = 0x%02x, nElemX = %d\n", (unsigned)context->desc->fmt, nElemX );
  }
  /* Allocate scalar input memory. */
  if ( !vecAlloc( &context->dataSet.Y, 1, TE_ALIGN_YES,
                  FMT_INT32, 0 ) )
  {
    printf( "te_loadFxn_vXsZ32(): failed to allocate scalar Y; "
            "fmt = 0x%02x\n", FMT_INT32 );
  }
  /* Allocate output and reference data vectors memory. */
  else if ( 3 != vecsAlloc( context->desc->isAligned, FMT_REAL|FMT_INT64,
                           &context->dataSet.Z  , nElemZ,
                           &context->dataSet.Zlo, nElemZ,
                           &context->dataSet.Zhi, nElemZ, 0 ) )
  {
    printf( "te_loadFxn_vXsZ32(): failed to allocate vectors Z/Zlo/Zhi; "
            "fmt = 0x%02x, nElemZ = %d\n", FMT_INT64, nElemZ );
  }
  /* Load vectors data from the SEQ-file. */
  else if ( !seqFileReadVecs( context->seqFile,
                             &context->dataSet.X,
                             &context->dataSet.Y,
                             &context->dataSet.Zlo,
                             &context->dataSet.Zhi, 0 ) )
  {
    printf( "te_loadFxn_vXsZ32(): failed to read vectors data; "
            "fmt = 0x%02x, nElemX = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemZ );
  }
  else
  {
    res = 1;
  }

  /* Free vectors data if failed. */
  if ( !res ) freeVectors(context);
  return (res);

} /* te_loadFxn_vXsY32sZ64() */
/*function similar to te_loadFxn_vXsY32sZ64() but the lower 8 bits are zeroed for each element in input buffers*/ 
int te_loadFxn_vXsY32sZ64_24( tTestEngContext * context )
{
  int M, N, L, i;
  int nElemX, nElemZ, res = 0;

  ASSERT( context && context->seqFile );

  M = MAX( 0, context->args.M );
  N = MAX( 0, context->args.N );
  L = MAX( 0, context->args.L );

  nElemX = M*N*L;
  nElemZ = L;

  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate input data vector memory. */
  if ( !vecAlloc( &context->dataSet.X, nElemX, 
                   context->desc->isAligned, 
                   context->desc->fmt, 0 ) )
  {
    printf( "te_loadFxn_vXsZ32(): failed to allocate vector X; "
            "fmt = 0x%02x, nElemX = %d\n", (unsigned)context->desc->fmt, nElemX );
  }
  /* Allocate scalar input memory. */
  if ( !vecAlloc( &context->dataSet.Y, 1, TE_ALIGN_YES,
                  FMT_INT32, 0 ) )
  {
    printf( "te_loadFxn_vXsZ32(): failed to allocate scalar Y; "
            "fmt = 0x%02x\n", FMT_INT32 );
  }
  /* Allocate output and reference data vectors memory. */
  else if ( 3 != vecsAlloc( context->desc->isAligned, FMT_REAL|FMT_INT64,
                           &context->dataSet.Z  , nElemZ,
                           &context->dataSet.Zlo, nElemZ,
                           &context->dataSet.Zhi, nElemZ, 0 ) )
  {
    printf( "te_loadFxn_vXsZ32(): failed to allocate vectors Z/Zlo/Zhi; "
            "fmt = 0x%02x, nElemZ = %d\n", FMT_INT64, nElemZ );
  }
  /* Load vectors data from the SEQ-file. */
  else if ( !seqFileReadVecs( context->seqFile,
                             &context->dataSet.X,
                             &context->dataSet.Y,
                             &context->dataSet.Zlo,
                             &context->dataSet.Zhi, 0 ) )
  {
    printf( "te_loadFxn_vXsZ32(): failed to read vectors data; "
            "fmt = 0x%02x, nElemX = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemZ );
  }
  else
  {
	for(i=0;i<(context->dataSet.X.nElem);i++) ((int32_t *)((uint8_t*)context->dataSet.X.ptr.aligned + (context->dataSet.X.offset )*context->dataSet.X.szElem))[i]&=0xFFFFFF00;
    res = 1;
  }

  /* Free vectors data if failed. */
  if ( !res ) freeVectors(context);
  return (res);

}

/* vector X (in), vector Y (in), scalar int64 Z (out) */
static int te_loadFxn_vXvYsZ_helper( tTestEngContext * context, int fmtY, int fmtZ )
{
  int M, N, L;
  int nElemX, nElemZ, res = 0;

  ASSERT( context && context->seqFile );

  M = MAX( 0, context->args.M );
  N = MAX( 0, context->args.N );
  L = MAX( 0, context->args.L );

  nElemX = M*N*L;
  nElemZ = L;

  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate input data vector memory. */
  if ( !vecAlloc( &context->dataSet.X, nElemX, 
                   context->desc->isAligned, 
                   context->desc->fmt, 0 ) )
  {
    printf( "te_loadFxn_vXvYsZ64(): failed to allocate vector X; "
            "fmt = 0x%02x, nElemX = %d\n", (unsigned)context->desc->fmt, nElemX );
  }
  /* Allocate input data vector memory. */
  if ( !vecAlloc( &context->dataSet.Y, nElemX, 
                   context->desc->isAligned, 
                   fmtY, 0 ) )
  {
    printf( "te_loadFxn_vXvYsZ64(): failed to allocate vector Y; "
            "fmt = 0x%02x, nElemX = %d\n", (unsigned)context->desc->fmt, nElemX );
  }
  /* Allocate output and reference data vectors memory. */
  else if ( 3 != vecsAlloc( context->desc->isAligned, fmtZ,
                           &context->dataSet.Z  , nElemZ,
                           &context->dataSet.Zlo, nElemZ,
                           &context->dataSet.Zhi, nElemZ, 0 ) )
  {
    printf( "te_loadFxn_vXvYsZ64(): failed to allocate vectors Z/Zlo/Zhi; "
            "fmt = 0x%02x, nElemZ = %d\n", FMT_INT64, nElemZ );
  }
  /* Load vectors data from the SEQ-file. */
  else if ( !seqFileReadVecs( context->seqFile,
                             &context->dataSet.X,
                             &context->dataSet.Y,
                             &context->dataSet.Zlo,
                             &context->dataSet.Zhi, 0 ) )
  {
    printf( "te_loadFxn_vXvYsZ64(): failed to read vectors data; "
            "fmt = 0x%02x, nElemX = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemZ );
  }
  else
  {
    res = 1;
  }

  /* Free vectors data if failed. */
  if ( !res ) freeVectors(context);
  return (res);

} /* te_loadFxn_vXvYsZ64() */
/*similar to te_loadFxn_vXvYsZ64() but the lower 8 bits in elements of input buffers are zeroed*/
static int te_loadFxn_vXvYsZ_helper_24( tTestEngContext * context, int fmtY, int fmtZ )
{
  int M, N, L, i;
  int nElemX, nElemZ, res = 0;

  ASSERT( context && context->seqFile );

  M = MAX( 0, context->args.M );
  N = MAX( 0, context->args.N );
  L = MAX( 0, context->args.L );

  nElemX = M*N*L;
  nElemZ = L;

  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate input data vector memory. */
  if ( !vecAlloc( &context->dataSet.X, nElemX, 
                   context->desc->isAligned, 
                   context->desc->fmt, 0 ) )
  {
    printf( "te_loadFxn_vXvYsZ64(): failed to allocate vector X; "
            "fmt = 0x%02x, nElemX = %d\n", (unsigned)context->desc->fmt, nElemX );
  }
  /* Allocate input data vector memory. */
  if ( !vecAlloc( &context->dataSet.Y, nElemX, 
                   context->desc->isAligned, 
                   fmtY, 0 ) )
  {
    printf( "te_loadFxn_vXvYsZ64(): failed to allocate vector Y; "
            "fmt = 0x%02x, nElemX = %d\n", (unsigned)context->desc->fmt, nElemX );
  }
  /* Allocate output and reference data vectors memory. */
  else if ( 3 != vecsAlloc( context->desc->isAligned, fmtZ,
                           &context->dataSet.Z  , nElemZ,
                           &context->dataSet.Zlo, nElemZ,
                           &context->dataSet.Zhi, nElemZ, 0 ) )
  {
    printf( "te_loadFxn_vXvYsZ64(): failed to allocate vectors Z/Zlo/Zhi; "
            "fmt = 0x%02x, nElemZ = %d\n", FMT_INT64, nElemZ );
  }
  /* Load vectors data from the SEQ-file. */
  else if ( !seqFileReadVecs( context->seqFile,
                             &context->dataSet.X,
                             &context->dataSet.Y,
                             &context->dataSet.Zlo,
                             &context->dataSet.Zhi, 0 ) )
  {
    printf( "te_loadFxn_vXvYsZ64(): failed to read vectors data; "
            "fmt = 0x%02x, nElemX = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemZ );
  }
  else
  {
    for(i=0;i<(context->dataSet.X.nElem);i++) ((int32_t *)((uint8_t*)context->dataSet.X.ptr.aligned + (context->dataSet.X.offset )*context->dataSet.X.szElem))[i]&=0xFFFFFF00;
    for(i=0;i<(context->dataSet.Y.nElem);i++) ((int32_t *)((uint8_t*)context->dataSet.Y.ptr.aligned + (context->dataSet.Y.offset )*context->dataSet.Y.szElem))[i]&=0xFFFFFF00;
    res = 1;
  }

  /* Free vectors data if failed. */
  if ( !res ) freeVectors(context);
  return (res);

}

/* vector X (in), vector Y (in), scalar int64 Z (out) */
int te_loadFxn_vXvYsZ64( tTestEngContext * context )
{
    return te_loadFxn_vXvYsZ_helper(context,context->desc->fmt,FMT_REAL|FMT_INT64);
}
/* vector X (in), vector Y (in), scalar int64 Z (out) */
int te_loadFxn_vXvYsZ64_24( tTestEngContext * context )
{
    return te_loadFxn_vXvYsZ_helper_24(context,context->desc->fmt,FMT_REAL|FMT_INT64);
}
/* vector X (in), vector Y (in), scalar int64 Z (out) */
int te_loadFxn_vXvYsZ32( tTestEngContext * context )
{
    return te_loadFxn_vXvYsZ_helper(context,context->desc->fmt,FMT_REAL|FMT_FRACT32);
}
/* vector X (in), vector int16 Y (in), scalar int64 Z (out) */
int te_loadFxn_vXvY16sZ64( tTestEngContext * context )
{
    return te_loadFxn_vXvYsZ_helper(context,FMT_REAL|FMT_FRACT16,FMT_REAL|FMT_INT64);
}


/* Apply the target function to the test case data set:
 * vector X (in), scalar Y (in), vector Z (out) */
void te_processFxn_vXsYvZ( tTestEngContext * context )
{
  typedef void tFxn_fr16  ( const fract16         * x, fract16         y, fract16         * z, int N );
  typedef void tFxn_fr32  ( const fract32         * x, fract32         y, fract32         * z, int N );
  typedef void tFxn_fl32  ( const float32_t       * x, float32_t       y, float32_t       * z, int N );
  typedef void tFxn_fl64  ( const float64_t       * x, float64_t       y, float64_t       * z, int N );
  typedef void tFxn_fr16c ( const complex_fract16 * x, complex_fract16 y, complex_fract16 * z, int N );
  typedef void tFxn_fr32c ( const complex_fract32 * x, complex_fract32 y, complex_fract32 * z, int N );
  typedef void tFxn_fl32c ( const complex_float   * x, complex_float   y, complex_float   * z, int N );
  typedef void tFxn_fl64c ( const complex_double  * x, complex_double  y, complex_double  * z, int N );

  tTestEngTarget   fxn;
  void *X, *Y, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = context->target.fut;

  switch ( context->desc->fmt )
  {
  case FMT_REAL|FMT_FRACT16: ( (tFxn_fr16 *)fxn )( (const fract16        *)X, *(fract16        *)Y, (fract16         *)Z, N ); break;
  case FMT_REAL|FMT_FRACT32: ( (tFxn_fr32 *)fxn )( (const fract32        *)X, *(fract32        *)Y, (fract32         *)Z, N ); break;
  case FMT_REAL|FMT_FLOAT32: ( (tFxn_fl32 *)fxn )( (const float32_t      *)X, *(float32_t      *)Y, (float32_t       *)Z, N ); break;
  case FMT_REAL|FMT_FLOAT64: ( (tFxn_fl64 *)fxn )( (const float64_t      *)X, *(float64_t      *)Y, (float64_t       *)Z, N ); break;
  case FMT_CPLX|FMT_FRACT16: ( (tFxn_fr16c*)fxn )( (const complex_fract16*)X, *(complex_fract16*)Y, (complex_fract16 *)Z, N ); break;
  case FMT_CPLX|FMT_FRACT32: ( (tFxn_fr32c*)fxn )( (const complex_fract32*)X, *(complex_fract32*)Y, (complex_fract32 *)Z, N ); break;
  case FMT_CPLX|FMT_FLOAT32: ( (tFxn_fl32c*)fxn )( (const complex_float  *)X, *(complex_float  *)Y, (complex_float   *)Z, N ); break;
  case FMT_CPLX|FMT_FLOAT64: ( (tFxn_fl64c*)fxn )( (const complex_double *)X, *(complex_double *)Y, (complex_double  *)Z, N ); break;
  default: ASSERT( 0 );
  }

} /* te_processFxn_vXsYvZ() */

/* Apply the target function to the test case data set:
 * vector Z (out), vector X (in), scalar Y (in)  */
void te_processFxn_vZvXsY( tTestEngContext * context )
{
  typedef void tFxn_fr16  ( fract16         * z, const fract16         * x, fract16         y, int N );
  typedef void tFxn_fr32  ( fract32         * z, const fract32         * x, fract32         y, int N );
  typedef void tFxn_fl32  ( float32_t       * z, const float32_t       * x, float32_t       y, int N );
  typedef void tFxn_fl64  ( float64_t       * z, const float64_t       * x, float64_t       y, int N );
  typedef void tFxn_fr16c ( complex_fract16 * z, const complex_fract16 * x, complex_fract16 y, int N );
  typedef void tFxn_fr32c ( complex_fract32 * z, const complex_fract32 * x, complex_fract32 y, int N );
  typedef void tFxn_fl32c ( complex_float   * z, const complex_float   * x, complex_float   y, int N );
  typedef void tFxn_fl64c ( complex_double  * z, const complex_double  * x, complex_double  y, int N );

  tTestEngTarget   fxn;
  void *X, *Y, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = context->target.fut;

  switch ( context->desc->fmt )
  {
  case FMT_REAL|FMT_FRACT16: ( (tFxn_fr16 *)fxn )((fract16         *)Z, (const fract16        *)X, *(fract16        *)Y,  N ); break;
  case FMT_REAL|FMT_FRACT32: ( (tFxn_fr32 *)fxn )((fract32         *)Z, (const fract32        *)X, *(fract32        *)Y,  N ); break;
  case FMT_REAL|FMT_FLOAT32: ( (tFxn_fl32 *)fxn )((float32_t       *)Z, (const float32_t      *)X, *(float32_t      *)Y,  N ); break;
  case FMT_REAL|FMT_FLOAT64: ( (tFxn_fl64 *)fxn )((float64_t       *)Z, (const float64_t      *)X, *(float64_t      *)Y,  N ); break;
  case FMT_CPLX|FMT_FRACT16: ( (tFxn_fr16c*)fxn )((complex_fract16 *)Z, (const complex_fract16*)X, *(complex_fract16*)Y,  N ); break;
  case FMT_CPLX|FMT_FRACT32: ( (tFxn_fr32c*)fxn )((complex_fract32 *)Z, (const complex_fract32*)X, *(complex_fract32*)Y,  N ); break;
  case FMT_CPLX|FMT_FLOAT32: ( (tFxn_fl32c*)fxn )((complex_float   *)Z, (const complex_float  *)X, *(complex_float  *)Y,  N ); break;
  case FMT_CPLX|FMT_FLOAT64: ( (tFxn_fl64c*)fxn )((complex_double  *)Z, (const complex_double *)X, *(complex_double *)Y,  N ); break;
  default: ASSERT( 0 );
  }

} /* te_processFxn_vZvXsY() */

/* Apply the target function to the test case data set:
 * vector Z (out), vector X (in), scalar int32 Y (in)  */
void te_processFxn_vZvXsY32( tTestEngContext * context )
{
  typedef void tFxn_fr16  ( fract16         * z, const fract16         * x, int32_t y, int N );
  typedef void tFxn_fr32  ( fract32         * z, const fract32         * x, int32_t y, int N );
  typedef void tFxn_fl32  ( float32_t       * z, const float32_t       * x, int32_t y, int N );
  typedef void tFxn_fl64  ( float64_t       * z, const float64_t       * x, int32_t y, int N );
  typedef void tFxn_fr16c ( complex_fract16 * z, const complex_fract16 * x, int32_t y, int N );
  typedef void tFxn_fr32c ( complex_fract32 * z, const complex_fract32 * x, int32_t y, int N );
  typedef void tFxn_fl32c ( complex_float   * z, const complex_float   * x, int32_t y, int N );
  typedef void tFxn_fl64c ( complex_double  * z, const complex_double  * x, int32_t y, int N );

  tTestEngTarget   fxn;
  void *X, *Y, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = context->target.fut;

  switch ( context->desc->fmt )
  {
  case FMT_REAL|FMT_FRACT16: ( (tFxn_fr16 *)fxn )((fract16         *)Z, (const fract16        *)X, *(int32_t*)Y,  N ); break;
  case FMT_REAL|FMT_FRACT32: ( (tFxn_fr32 *)fxn )((fract32         *)Z, (const fract32        *)X, *(int32_t*)Y,  N ); break;
  case FMT_REAL|FMT_FLOAT32: ( (tFxn_fl32 *)fxn )((float32_t       *)Z, (const float32_t      *)X, *(int32_t*)Y,  N ); break;
  case FMT_REAL|FMT_FLOAT64: ( (tFxn_fl64 *)fxn )((float64_t       *)Z, (const float64_t      *)X, *(int32_t*)Y,  N ); break;
  case FMT_CPLX|FMT_FRACT16: ( (tFxn_fr16c*)fxn )((complex_fract16 *)Z, (const complex_fract16*)X, *(int32_t*)Y,  N ); break;
  case FMT_CPLX|FMT_FRACT32: ( (tFxn_fr32c*)fxn )((complex_fract32 *)Z, (const complex_fract32*)X, *(int32_t*)Y,  N ); break;
  case FMT_CPLX|FMT_FLOAT32: ( (tFxn_fl32c*)fxn )((complex_float   *)Z, (const complex_float  *)X, *(int32_t*)Y,  N ); break;
  case FMT_CPLX|FMT_FLOAT64: ( (tFxn_fl64c*)fxn )((complex_double  *)Z, (const complex_double *)X, *(int32_t*)Y,  N ); break;
  default: ASSERT( 0 );
  }

} /* te_processFxn_vZvXsY32() */

/* Apply the target function to the test case data set:
 * vector X (in), vector Y (in), vector Z (out) */
void te_processFxn_vXvYvZ( tTestEngContext * context )
{
  typedef void tFxn( const void * X, const void * Y, void * Z, int N );

  tFxn *fxn;
  void *X, *Y, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;

  fxn( X, Y, Z, N );

} /* te_processFxn_vXvYvZ() */

/* Apply the target function to the test case data set:
 * vector Z (out), vector X (in), vector Y (in) */
void te_processFxn_vZvXvY( tTestEngContext * context )
{
  typedef void tFxn( const void * X, const void * Y, void * Z, int N );

  tFxn *fxn;
  void *X, *Y, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;

  fxn( Z, X, Y, N );

} /* te_processFxn_vZvXvY() */

/* Apply the target function to the test case data set:
 * vector Y (in), vector X (in), vector Z (out) */
void te_processFxn_vYvXvZ( tTestEngContext * context )
{
  typedef void tFxn( const void * Y, const void * X, void * Z, int N );

  tFxn *fxn;
  void *X, *Y, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;

  fxn( Y, X, Z, N );

} /* te_processFxn_vYvXvZ() */

/* Apply the target function to the test case data set:
 * vector Z (out), vector Y (in), vector X (in) */
void te_processFxn_vZvYvX( tTestEngContext * context )
{
  typedef void tFxn( const void * Y, const void * X, void * Z, int N );

  tFxn *fxn;
  void *X, *Y, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;

  te_errh_resetStates( context );
  fxn( Z, Y, X, N );
  te_errh_verifyStates( context, -1 );

} /* te_processFxn_vZvYvX() */

/* Apply the target function to the test case data set:
 * vector X (in), vector Y (in), scalar Z (out) */
void te_processFxn_vXvYsZ( tTestEngContext * context )
{
  typedef fract16         tFxn_fr16  ( const fract16         * x, const fract16         * y, int N );
  typedef fract32         tFxn_fr32  ( const fract32         * x, const fract32         * y, int N );
  typedef float32_t       tFxn_fl32  ( const float32_t       * x, const float32_t       * y, int N );
  typedef float64_t       tFxn_fl64  ( const float64_t       * x, const float64_t       * y, int N );
  typedef complex_fract16 tFxn_fr16c ( const complex_fract16 * x, const complex_fract16 * y, int N );
  typedef complex_fract32 tFxn_fr32c ( const complex_fract32 * x, const complex_fract32 * y, int N );
  typedef complex_float   tFxn_fl32c ( const complex_float   * x, const complex_float   * y, int N );
  typedef complex_double  tFxn_fl64c ( const complex_double  * x, const complex_double  * y, int N );

  tTestEngTarget   fxn;
  void *X, *Y, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = context->target.fut;

  switch ( context->desc->fmt )
  {
  case FMT_REAL|FMT_FRACT16: *(fract16        *)Z = ( (tFxn_fr16 *)fxn )( ( const fract16         *)X, ( const fract16         *)Y, N ); break;
  case FMT_REAL|FMT_FRACT32: *(fract32        *)Z = ( (tFxn_fr32 *)fxn )( ( const fract32         *)X, ( const fract32         *)Y, N ); break;
  case FMT_REAL|FMT_FLOAT32: *(float32_t      *)Z = ( (tFxn_fl32 *)fxn )( ( const float32_t       *)X, ( const float32_t       *)Y, N ); break;
  case FMT_REAL|FMT_FLOAT64: *(float64_t      *)Z = ( (tFxn_fl64 *)fxn )( ( const float64_t       *)X, ( const float64_t       *)Y, N ); break;
  case FMT_CPLX|FMT_FRACT16: *(complex_fract16*)Z = ( (tFxn_fr16c*)fxn )( ( const complex_fract16 *)X, ( const complex_fract16 *)Y, N ); break;
  case FMT_CPLX|FMT_FRACT32: *(complex_fract32*)Z = ( (tFxn_fr32c*)fxn )( ( const complex_fract32 *)X, ( const complex_fract32 *)Y, N ); break;
  case FMT_CPLX|FMT_FLOAT32: *(complex_float  *)Z = ( (tFxn_fl32c*)fxn )( ( const complex_float   *)X, ( const complex_float   *)Y, N ); break;
  case FMT_CPLX|FMT_FLOAT64: *(complex_double *)Z = ( (tFxn_fl64c*)fxn )( ( const complex_double  *)X, ( const complex_double  *)Y, N ); break;
  default: ASSERT( 0 );
  }

} /* te_processFxn_vXvYsZ() */

/* Apply the target function to the test case data set:
 * vector X (in), vector Z (out) */
void te_processFxn_vXvZ( tTestEngContext * context )
{
  typedef void tFxn( const void * X, void * Z, int N );

  tFxn *fxn;
  void *X, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;

  fxn( X, Z, N );

} /* te_processFxn_vXvZ() */

void te_processFxn_vZvX( tTestEngContext * context )
{
  typedef void tFxn( const void * X, void * Z, int N );

  tFxn *fxn;
  void *X, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;

  te_errh_resetStates( context );
  fxn( Z, X, N );
  te_errh_verifyStates( context, -1 );

} /* te_processFxn_vZvX() */

/* Apply the target function to the test case data set:
 * vector X (in), scalar Z (out) */
void te_processFxn_vXsZ( tTestEngContext * context )
{
  typedef fract16         tFxn_fr16  ( const fract16         * x, int N );
  typedef fract32         tFxn_fr32  ( const fract32         * x, int N );
  typedef float32_t       tFxn_fl32  ( const float32_t       * x, int N );
  typedef float64_t       tFxn_fl64  ( const float64_t       * x, int N );
  typedef complex_fract16 tFxn_fr16c ( const complex_fract16 * x, int N );
  typedef complex_fract32 tFxn_fr32c ( const complex_fract32 * x, int N );
  typedef complex_float   tFxn_fl32c ( const complex_float   * x, int N );
  typedef complex_double  tFxn_fl64c ( const complex_double  * x, int N );

  tTestEngTarget   fxn;
  void *X, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = context->target.fut;

  switch ( context->desc->fmt )
  {
  case FMT_REAL|FMT_FRACT16: *(fract16        *)Z = ( (tFxn_fr16 *)fxn )( (const fract16         *)X, N ); break;
  case FMT_REAL|FMT_FRACT32: *(fract32        *)Z = ( (tFxn_fr32 *)fxn )( (const fract32         *)X, N ); break;
  case FMT_REAL|FMT_FLOAT32: *(float32_t      *)Z = ( (tFxn_fl32 *)fxn )( (const float32_t       *)X, N ); break;
  case FMT_REAL|FMT_FLOAT64: *(float64_t      *)Z = ( (tFxn_fl64 *)fxn )( (const float64_t       *)X, N ); break;
  case FMT_CPLX|FMT_FRACT16: *(complex_fract16*)Z = ( (tFxn_fr16c*)fxn )( (const complex_fract16 *)X, N ); break;
  case FMT_CPLX|FMT_FRACT32: *(complex_fract32*)Z = ( (tFxn_fr32c*)fxn )( (const complex_fract32 *)X, N ); break;
  case FMT_CPLX|FMT_FLOAT32: *(complex_float  *)Z = ( (tFxn_fl32c*)fxn )( (const complex_float   *)X, N ); break;
  case FMT_CPLX|FMT_FLOAT64: *(complex_double *)Z = ( (tFxn_fl64c*)fxn )( (const complex_double  *)X, N ); break;
  default: ASSERT( 0 );
  }

} /* te_processFxn_vXsZ() */


/* Apply the target function to the test case data set:
 * vector X (in), scalar int32 Z (out) */
void te_processFxn_vXsZ32( tTestEngContext * context )
{
  typedef int32_t tFxn( const void * X, int N );

  tFxn *fxn;
  void *X, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;

  *(int32_t*)Z = fxn( X, N );

} /* te_processFxn_vXsZ32() */

/* Apply the target function to the test case data set:
 * vector X (in), scalar Y (in), scalar int64 Z (out) */
void te_processFxn_vXsY32sZ64( tTestEngContext * context )
{
  typedef int64_t tFxn( const void * X, int y, int N );

  tFxn *fxn;
  void *X, *Z;
  int32_t y;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  y = *vecGetElem_i32( &context->dataSet.Y, 0 );

  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;

  *(int64_t*)Z = fxn( X, y, N );

} /* te_processFxn_vXsY32sZ64() */

/* scalar int64 Z (out), vector X (in), vector Y (in) */
void te_processFxn_sZ64vXvY( tTestEngContext * context )
{
  typedef int64_t tFxn( const void * X, const void * Y, int N );

  tFxn *fxn;
  void *X, *Y, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;

  *(int64_t*)Z = fxn( X, Y, N );

} /* te_processFxn_sZ64vXvY() */

/* scalar int64 Z (out), vector X (in), vector Y (in) */
void te_processFxn_sZ32vXvY( tTestEngContext * context )
{
  typedef int32_t tFxn( const void * X, const void * Y, int N );

  tFxn *fxn;
  void *X, *Y, *Z;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;

  *(int32_t*)Z = fxn( X, Y, N );

} /* te_processFxn_sZ32vXvY() */

/* Apply the target function to the test case data set:
 * vector X (in), vector Z (out), vector W (out) */
void te_processFxn_vXvZvW( tTestEngContext * context )
{
  typedef void tFxn( const void * X, void * Z, void * W, int N );

  tFxn *fxn;
  void *X, *Z, *W;
  int N;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );
  W = vecGetElem( &context->dataSet.W, 0 );

  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;

  fxn( X, Z, W, N );

} /* te_processFxn_vXvZvW() */

/* Apply the target function to the test case data set, streaming variant:
 * vector X (in), vector Y (in), vector Z (out) */
void te_processFxn_s_vXvYvZ( tTestEngContext * context )
{
  typedef void tFxn( const void * X, const void * Y, void * Z, int N, int L );

  tFxn *fxn;
  void *X, *Y, *Z;
  int N, L;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  L   = context->args.L;
  fxn = (tFxn*)context->target.fut;

  fxn( X, Y, Z, N, L );

} /* te_processFxn_s_vXvYvZ() */

/* Apply the target function to the test case data set, streaming variant:
 * vector X (in), vector Z (out) */
void te_processFxn_s_vXvZ( tTestEngContext * context )
{
  typedef void tFxn( const void * X, void * Z, int N, int L );

  tFxn *fxn;
  void *X, *Z;
  int N, L;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  L   = context->args.L;
  fxn = (tFxn*)context->target.fut;

  fxn( X, Z, N, L );

} /* te_processFxn_s_vXvZ() */


/* Test executive function. Performs the specified test on a brief or full version
 * of the designated SEQ-file. Return the test result (non-zero if passed). 

    first argument is a pointer to the table of type
    struct 
    {
    tTestEngTarget *    funcList[maxFuncNum];
    tTestEngDesc        testDesc;
    }
    the list of functions is ended with NULL and last entry in the arry of those elements 
    is a NULL as well
 */
int te_Exec( const void* pTestDefTbl, size_t tblSz, int maxFuncNum,
              tTestEngTarget   targetFxn, const char * seqName,
              int isFull, int isVerbose, int breakOnError )
{
    size_t m=0;
    int n;
    tTestEngTarget *  pFxns;
    const tTestEngDesc *pDesc;
    /*   * Search through the test definitions table.    */
    pFxns=NULL;
    while(m<tblSz)
    {
        pFxns=((tTestEngTarget *)pTestDefTbl);
        pDesc = (const tTestEngDesc *)(pFxns+maxFuncNum);

        
        if(pFxns[0]!=NULL) 
        {

          for(n=0; n<maxFuncNum; n++)
          {
              //if(pFxns[n]==NULL) break;
              if(pFxns[n]==targetFxn) 
              {   /* Invoke the Test Engine. */
                  return TestEngRun( targetFxn, pDesc,seqName, isFull, isVerbose, breakOnError ) ;
              }
          }
        }
        pTestDefTbl = (const void*)(pDesc+1);
        m++;
    }
    
    return 1;
}
