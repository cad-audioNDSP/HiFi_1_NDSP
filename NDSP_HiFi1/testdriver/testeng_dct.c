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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"
/* Fixed point arithmetics. */
#include "NatureDSP_Math.h"
/* Test engine API. */ 
#include "testeng.h"
/* Test engine extension for DCT */
#include "testeng_dct.h"
/* Test data vectors tools and SEQ-file reader. */
#include "vectools.h"
/* Test environment utils. */
#include "utils.h"

#define MAX(a,b)   ( (a)>(b) ? (a) : (b) )

/* Suppress Visual C warnings on +/-INFINITY macros. */
#ifdef COMPILER_MSVC
#pragma warning(disable:4056)
#pragma warning(disable:4756)
#endif

#define sz_fp64c    sizeof(complex_double)

/* FFT test context. */
typedef struct tagTestEngContext_dct_int
{
  tTestEngContext_dct ext; /* Externally visible part of FFT context. */

} tTestEngContext_dct_int;

/* Create a target algorithm instance and set tTestEngContext::target fields.
 * Return zero if failed. */
int te_create_dct( tTestEngContext * context )
{
  tTestEngContext_dct_int * context_dct;

  int res;

  /*
   * Allocate and initialize the context structure.
   */

  context_dct = (tTestEngContext_dct_int * )malloc( sizeof(*context_dct) );

  if ( !( res = ( 0 != context_dct ) ) )
  {
    printf( "te_create_dct(): malloc() failed\n" );
  }
  else
  {
    memset( context_dct, 0, sizeof(*context_dct) );
  }

  if ( res )
  {
    context->target.handle = &context_dct->ext;
  }

  {
    typedef int (*tFxn)(void * y, const void * x, int N );
    tFxn fxn;
    fxn = (tFxn)((const tTestEngDesc_dct *)context->desc)->frwTransFxn;
    if (!NatureDSP_Signal_isPresent(fxn)) return -1;    // FUT is not defined 
  }

  return (res);

} /* te_create_dct() */

/* Destroy the target algorithm instance and free memory block(s) allocated
 * for the target object. Return zero whenever any violation of memory area
 * bounds is detected. */
int te_destroy_dct( tTestEngContext * context )
{
  tTestEngContext_dct_int * context_dct;

  ASSERT( context );

  if ( 0 != ( context_dct = (tTestEngContext_dct_int *)context->target.handle ) )
  {
    free( context_dct );
  }

  return (1);

} /* te_destroy_dct() */

/* Allocate in/out vectors for the next test case, and load the data set
 * from the SEQ-file. Return zero if failed. */
int te_load_dct( tTestEngContext * context )
{
  tTestEngContext_dct_int * context_dct = (tTestEngContext_dct_int *)context->target.handle;
  tVec BEXP, Z, Zlo, Zhi;
  int res = 0;
  int isFract = ( ( FMT_FRACT16 == ( context->desc->fmt & 15 ) ) ||
                  ( FMT_FRACT32 == ( context->desc->fmt & 15 ) ) );

  NASSERT( context_dct );

  memset( &context_dct->ext, 0, sizeof(context_dct->ext) );

  memset( &BEXP, 0, sizeof(BEXP) );
  memset( &Z   , 0, sizeof(Z   ) );
  memset( &Zlo , 0, sizeof(Zlo ) );
  memset( &Zhi , 0, sizeof(Zhi ) );

  /* If DCT supports the scaling option, read the scaling method from the SEQ-file. */
  if ( ( context->desc->extraParam & TE_FFT_OPT_SCALE_METH ) &&
       ( 1 != seqFileScanf( context->seqFile, "%d", &context_dct->ext.scale_method ) ) )
  {
    printf( "te_load_dct(): bad SEQ-file format (a)\n" );
  }
  /* For a fixed point blockwise DCT, allocate a vector for temporal storage of block exponent. */
  else if ( 0 != ( context->desc->extraParam & TE_FFT_BLOCKWISE ) && isFract &&
            !vecAlloc( &BEXP, context->args.L, TE_ALIGN_NO, FMT_INT16, 0 ) )
  {
    printf( "te_load_dct(): failed to allocate BEXP, L=%d\n", context->args.L );
  }
  /* Read input data filename. */
  else if ( 1 != seqFileScanf( context->seqFile, "%63s", &context_dct->ext.fInName ) )
  {
    printf( "te_load_dct(): bad SEQ-file format (b)\n" );
  }
  /* Read reference data filename. */
  else if ( 1 != seqFileScanf( context->seqFile, "%63s", &context_dct->ext.fRefName ) )
  {
    printf( "te_load_dct(): bad SEQ-file format (c)\n" );
  }
  /* Allocate vectors for SINAD verification. */
  else if ( 3 != vecsAlloc( TE_ALIGN_NO, FMT_FLOAT32, &Z, 1, &Zlo, 1, &Zhi, 1, 0 ) )
  {
    printf( "te_load_dct(): failed to allocate vectors Z/Zlo/Zhi\n" );
  }
  /* Read the minimum SINAD value from the SEQ-file. */
  else if ( 1 != seqFileScanf( context->seqFile, "%f", vecGetElem_fl32( &Zlo, 0 ) ) )
  {
    printf( "te_load_dct(): bad SEQ-file format (d)\n" );
  }
  else
  {
    /* Set SINAD upper limit to infinity. */
    *vecGetElem_fl32( &Zhi, 0 ) = INFINITY;

    memset( &context->dataSet, 0, sizeof(context->dataSet) );

    context->dataSet.X   = BEXP;
    context->dataSet.Z   = Z;
    context->dataSet.Zlo = Zlo;
    context->dataSet.Zhi = Zhi;

    res = 1;
  }

  if ( !res )
  {
    if ( BEXP.szBulk ) vecFree( &BEXP );
    if ( Z   .szBulk ) vecFree( &Z    );
    if ( Zlo .szBulk ) vecFree( &Zlo  );
    if ( Zhi .szBulk ) vecFree( &Zhi  );
  }

  return (res);

} /* te_load_dct() */


/* Apply the DCT function to a single frame of test data, any DCT routine excepting feature
 * rich fixed point DCTs and blockwise DCTs. */
int te_frameProc_stnd_dct( tTestEngContext * context, 
                     const fract16         * in,
                           float64_t       * out,
                     tVec * xVec, tVec * yVec )
{
  typedef int (*tFxn)(void * y, const void * x, int N );

  tFxn fxn = NULL;
  tTestEngContext_dct * context_dct;
  void *px, *py;

  int bexp, shift;
  int N, logN;
  int noReuse, doInplace, isRealForward;

  uint32_t crcSum = 0;

  NASSERT( context && context->desc && context->target.handle );
  NASSERT( in && out && xVec && yVec );
  NASSERT( 0 == ( context->args.N & ( context->args.N - 1 ) ) );
  NASSERT( 0 == ( context->desc->extraParam & TE_FFT_OPT_SCALE_METH ) );
  
  context_dct = (tTestEngContext_dct *)context->target.handle;
  N           = context->args.N;
  logN        = 30 - S_exp0_l( N );

  /* If the DCT routine supports inplace operation, try it every second frame. */
  doInplace = ( context->desc->extraParam & TE_FFT_OPT_INPLACE ) && ( context_dct->frameCnt & 1 );
  /* Also check if the target DCT is allowed to reuse the input buffer for intermediate data. */
  noReuse = !( context->desc->extraParam & TE_FFT_OPT_REUSE_INBUF ) && !doInplace;
  /* Real-valued forward DCT requires special block exponent on input. */
  isRealForward = ( context->desc->extraParam & TE_DCT ) &&
                  ( context->args.caseType == TE_FFT_TESTCASE_FORWARD );

  /* For all fixed point DCTs, block exponent of input data should be at least 1, but for
   * real-valued forward DCT zero is allowed. */
  bexp = ( isRealForward ? 0 : 1 );

  /* Convert 16-bit PCM input data to target DCT format. */
  shift = vecFromPcm16( xVec, (fract16*)in, bexp );

  /* Select in/out buffers for the DCT, and wipe the output buffer. */
  if ( doInplace )
  {
    if ( vecGetSize( xVec ) < vecGetSize( yVec ) )
    {
      memcpy( vecGetElem( yVec, 0 ), vecGetElem( xVec, 0 ), vecGetSize( xVec ) );
      px = py = vecGetElem( yVec, 0 );
    }
    else
    {
      px = py = vecGetElem( xVec, 0 );
    }
  }
  else
  {
    memset( vecGetElem( yVec, 0 ), 0, vecGetSize( yVec ) );
    px = vecGetElem( xVec, 0 );
    py = vecGetElem( yVec, 0 );
  }

  /* Select the target DCT routine (either forward or inverse). */
  if ( context->args.caseType == TE_FFT_TESTCASE_FORWARD )
  {
    fxn = (tFxn)((const tTestEngDesc_dct *)context->desc)->frwTransFxn;
    /* Compensate for scaling shift performed by fixed point DCTs. */
    shift -= logN;
  }
  else if ( context->args.caseType == TE_FFT_TESTCASE_INVERSE )
  {
    fxn = (tFxn)((const tTestEngDesc_dct *)context->desc)->invTransFxn;
    /* For fixed point inverse DCT, we have to divide output signal by DCT size.
     * Just don't compensate for the scaling shift performed by the DCT routine. */
  }
  else
  {
    NASSERT( !"Bad test case type!" );
  }

  /* If reuse is not allowed, make sure the buffer stays intact. */
  if ( noReuse ) crcSum = crc32( 0, (uint8_t*)px, vecGetSize( xVec ) );

  /* Apply the target DCT routine. */
  fxn( py, px,  N );

  if ( doInplace && vecGetSize( xVec ) >= vecGetSize( yVec ) )
  {
    memcpy( vecGetElem( yVec, 0 ), py, vecGetSize( yVec ) );
  }
  
  if ( noReuse && crcSum != crc32( 0, (uint8_t*)px, vecGetSize( xVec ) ) )
  {
    printf( "te_frameProc_stnd_dct(): target DCT has corrupted the input buffer\n" );
    return ( 0 );
  }

  /* Convert output data to complex 64-bit floating point and rescale them. */
  vecToFp64( (float64_t*)out, yVec, shift );

  return (1);

} /* te_frameProc_stnd_dct() */

/* Apply the DCT function to a single frame of test data, feature rich fixed point
 * DCT (with scaling method option). */
int te_frameProc_stnd_scl_dct( tTestEngContext * context, 
                         const fract16         * in,
                               float64_t       * out,
                         tVec * xVec, tVec * yVec )
{
  typedef int (*tFxn)(void * y, const void * x, int N, int scalingOpt);

  tFxn fxn = NULL;
  tTestEngContext_dct * context_dct;
  void *px, *py;

  int bexp=0, shift;
  int N, logN;
  int noReuse, doInplace, isRealForward;

  uint32_t crcSum = 0;

  NASSERT( context && context->desc && context->target.handle );
  NASSERT( in && out && xVec && yVec );
  NASSERT( 0 == ( context->args.N & ( context->args.N - 1 ) ) );
  NASSERT( 0 != ( context->desc->extraParam & TE_FFT_OPT_SCALE_METH ) );
  NASSERT(context->args.caseType == TE_FFT_TESTCASE_FORWARD);
  
  context_dct = (tTestEngContext_dct *)context->target.handle;
  N           = context->args.N;
  logN        = 30 - S_exp0_l( N );

  /* If the DCT routine supports inplace operation, try it every second frame. */
  doInplace = ( context->desc->extraParam & TE_FFT_OPT_INPLACE ) && ( context_dct->frameCnt & 1 );
  /* Also check if the target DCT is allowed to reuse the input buffer for intermediate data. */
  noReuse = !( context->desc->extraParam & TE_FFT_OPT_REUSE_INBUF ) && !doInplace;
  /* Real-valued forward DCT requires special block exponent on input. */
  isRealForward = ( context->desc->extraParam & TE_DCT ) &&
                  ( context->args.caseType == TE_FFT_TESTCASE_FORWARD );

  /* Select the target DCT routine. */
  if ( context->args.caseType == TE_FFT_TESTCASE_FORWARD )
  {
      fxn = (tFxn)((const tTestEngDesc_dct *)context->desc)->frwTransFxn;
  }
  else
  {
    NASSERT( !"Bad test case type!" );
  }

  /* Select the block exponent for fixed point DCT input data. Scale method 0 is
   * "no scaling", 3 - "static scaling". */
  if ( isRealForward )
  {
    bexp = ( context_dct->scale_method == 0 ? logN : 0 );
  }
  else
  {
      NASSERT(!"wrong transform type");
  }

  /* Convert 16-bit PCM input data to target DCT format. */
  shift = vecFromPcm16( xVec, (fract16*)in, bexp );

  /* Select in/out buffers for the DCT, and wipe the output buffer. */
  if ( doInplace )
  {
    if ( vecGetSize( xVec ) < vecGetSize( yVec ) )
    {
      memcpy( vecGetElem( yVec, 0 ), vecGetElem( xVec, 0 ), vecGetSize( xVec ) );
      px = py = vecGetElem( yVec, 0 );
    }
    else
    {
      px = py = vecGetElem( xVec, 0 );
    }
  }
  else
  {
    memset( vecGetElem( yVec, 0 ), 0, vecGetSize( yVec ) );
    px = vecGetElem( xVec, 0 );
    py = vecGetElem( yVec, 0 );
  }

  /* If reuse is not allowed, make sure the buffer stays intact. */
  if ( noReuse ) crcSum = crc32( 0, (uint8_t*)px, vecGetSize( xVec ) );


  /* Apply the target DCT routine. */
  bexp=fxn( py,px,  N,context_dct->scale_method); 

  if ( doInplace && vecGetSize( xVec ) >= vecGetSize( yVec ) )
  {
    memcpy( vecGetElem( yVec, 0 ), py, vecGetSize( yVec ) );
  }
  
  if ( noReuse && crcSum != crc32( 0, (uint8_t*)px, vecGetSize( xVec ) ) )
  {
    printf( "te_frameProc_stnd_scl_dct(): target DCT has corrupted the input buffer\n" );
    return ( 0 );
  }

  /* Compensate for scaling shift performed by fixed point forward DCTs. */
  shift -= bexp;

  /* Convert output data to complex 64-bit floating point and rescale them. */
  vecToFp64( (float64_t*)out, yVec, shift );

  return (1);

} /* te_frameProc_stnd_scl_dct() */
