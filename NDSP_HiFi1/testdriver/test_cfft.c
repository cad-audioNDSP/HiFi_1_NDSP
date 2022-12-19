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
 * Test procedures for complex FFT functions.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Fixed point arithmetics. */
#include "NatureDSP_Math.h"
/* Test engine API. */
#include "testeng.h"
/* Test engine extension for FFT. */
#include "testeng_fft.h"
/* Test data vectors tools and SEQ-file reader. */
#include "vectools.h"
/* Filters and transformations API. */
#include "NatureDSP_Signal.h"

#define MIN(a,b)   ( (a)<(b) ? (a) : (b) )
#define MAX(a,b)   ( (a)>(b) ? (a) : (b) )

/* Suppress Visual C warnings on +/-HUGE_VALF macros. */
#ifdef COMPILER_MSVC
#pragma warning(disable:4056)
#pragma warning(disable:4756)
#endif

#define sz_fr16c    sizeof(complex_fract16)
#define sz_fl64c    sizeof(complex_double)

/* Period, in frames, between reallocation of in/out buffers consumed by the
 * target FFT routine. */
#define XY_VEC_REALLOC_PERIOD     16

/* Test data file processing for a standalone test. Applies the target FFT function
 * to test data, compares the FFT output with reference data and estimates the SINAD. */
static void fileProc_stnd( tTestEngContext * context );

/* Initializer for a test description structure. */
#define TEST_DESC_STND( fmt, extraParam, align,fwdfft,invfft )  { { (fmt),(extraParam),TE_ARGNUM_1,(align), \
                                                     &te_create_fft, &te_destroy_fft, \
                                                     &te_load_fft, &fileProc_stnd },(tFrwTransFxn)fwdfft,(tInvTransFxn)invfft}

/* FFT attributes for feature rich and fast complex FFTs. */
#define ATTR_FEATURE_RICH       (TE_FFT_CPLX|TE_FFT_OPT_INPLACE)
#define ATTR_FEATURE_RICH_SCL   (TE_FFT_CPLX|TE_FFT_OPT_INPLACE|TE_FFT_OPT_SCALE_METH)
#define ATTR_FAST               (TE_FFT_CPLX|TE_FFT_OPT_REUSE_INBUF)

static const struct tagTestDef
{
  int                phaseNum;
  int                runAlways;    /* 1-always */
  int                testvecFlag;  /* 2-sanity; 1-brief; 0-full */
  const char *       seqFilename;
  tTestEngTarget     target;
  tTestEngDesc_fft   desc;

} testTbl[] = 
{
  /*  Stage 1 */
  { 1, 0, 0, "fft_cplx24x24_ie.seq"     , (tTestEngTarget)te_frameProc_stnd_scl_fft     , TEST_DESC_STND( FMT_FRACT32, TE_FFT_CPLX|TE_FFT_OPT_REUSE_INBUF|TE_FFT_OPT_SCALE_METH             , TE_ALIGN_YES, &fft_cplx24x24_ie  , NULL               ) },
  { 1, 0, 0, "ifft_cplx24x24_ie.seq"    , (tTestEngTarget)te_frameProc_stnd_scl_fft     , TEST_DESC_STND( FMT_FRACT32, TE_FFT_CPLX|TE_FFT_OPT_REUSE_INBUF|TE_FFT_OPT_SCALE_METH             , TE_ALIGN_YES, NULL               ,&ifft_cplx24x24_ie  ) },
  { 1, 0, 1, "fft_cplx32x16_ie.seq"     , (tTestEngTarget)te_frameProc_stnd_scl_fft     , TEST_DESC_STND( FMT_FRACT32, TE_FFT_CPLX|TE_FFT_OPT_REUSE_INBUF|TE_FFT_OPT_SCALE_METH|TE_FFT_TWD16, TE_ALIGN_YES, &fft_cplx32x16_ie  , NULL               ) },
  { 1, 0, 1, "ifft_cplx32x16_ie.seq"    , (tTestEngTarget)te_frameProc_stnd_scl_fft     , TEST_DESC_STND( FMT_FRACT32, TE_FFT_CPLX|TE_FFT_OPT_REUSE_INBUF|TE_FFT_OPT_SCALE_METH|TE_FFT_TWD16, TE_ALIGN_YES, NULL               ,&ifft_cplx32x16_ie  ) },
  { 1, 0, 2, "fft_cplx32x32.seq"        ,  (tTestEngTarget)te_frameProc_stnd_scl_fft    , TEST_DESC_STND( FMT_FRACT32, TE_FFT_CPLX|TE_FFT_OPT_REUSE_INBUF|TE_FFT_OPT_SCALE_METH|TE_FFT_TWD16|TE_FFT_32X32, TE_ALIGN_YES, &fft_cplx32x32, NULL) },
  { 1, 0, 2, "ifft_cplx32x32.seq"       , (tTestEngTarget)te_frameProc_stnd_scl_fft     , TEST_DESC_STND( FMT_FRACT32, TE_FFT_CPLX|TE_FFT_OPT_REUSE_INBUF|TE_FFT_OPT_SCALE_METH|TE_FFT_TWD16|TE_FFT_32X32|TE_INVFFT_32X32,TE_ALIGN_YES, NULL, &ifft_cplx32x32) },

  /*  Stage 2  */
  { 2, 0, 0, "fft_cplxf_ie.seq"     , (tTestEngTarget)te_frameProc_stnd_fft     , TEST_DESC_STND( FMT_FLOAT32, ATTR_FAST            , TE_ALIGN_YES, &fft_cplxf_ie     , NULL               ) },
  { 2, 0, 0, "ifft_cplxf_ie.seq"    , (tTestEngTarget)te_frameProc_stnd_fft     , TEST_DESC_STND( FMT_FLOAT32, ATTR_FAST            , TE_ALIGN_YES, NULL              , &ifft_cplxf_ie     ) },

  { 0 } /* End of table */
};

/* Perform all tests for cfft API functions. */
int main_cfft( int phaseNum, int isFull, int isVerbose, int breakOnError )
{
  int ix, res;

  printf( "\nComplex FFT with memory improved usage:\n" );

  for ( ix=0,res=1; testTbl[ix].seqFilename && ( res || !breakOnError ); ix++ )
  {
      if ( (phaseNum == 0 || phaseNum == testTbl[ix].phaseNum) && testTbl[ix].runAlways)
      {
          res &= ( 0 != TestEngRun( (tTestEngTarget)testTbl[ix].target,
                                &testTbl[ix].desc.desc,
                                testTbl[ix].seqFilename,
                                isFull, isVerbose, breakOnError ) );
      }
      else if ( isFull <= testTbl[ix].testvecFlag )
      {
          res &= ( 0 != TestEngRun( (tTestEngTarget)testTbl[ix].target,
	                                &testTbl[ix].desc.desc,
	                                testTbl[ix].seqFilename,
	                                isFull, isVerbose, breakOnError ) );
      }
  }

  return (res);

} /* main_cfft() */

/* Test data file processing for a standalone test. Applies the target FFT function
 * to test data, compares the FFT output with reference data and estimates the SINAD. */
void fileProc_stnd( tTestEngContext * context )
{
  tTestEngContext_fft           * context_fft;
  tTestEngFrameProcFxn_fft_stnd * frameProcFxn;

  char fInNameBuf[256], fRefNameBuf[256];

  tVec inVec, outVec, refVec; /* In/out/reference vectors related to test data files. */
  tVec xVec, yVec;            /* In/out vectors for the target FFT routine. */
  FILE *fIn = 0, *fRef = 0;
  int res = 0;
  int n, N;

  NASSERT( context && context->desc );
  NASSERT( context->target.fut && context->target.handle );

  context_fft = (tTestEngContext_fft*)context->target.handle;

  /* FFT size */
  N = context->args.N;

  /* Preset an invalid SINAD for the test result. */
  *vecGetElem_fl32( &context->dataSet.Z, 0 ) = -HUGE_VALF;

  if ( context->isVerbose )
  {
    if ( context->desc->extraParam & TE_FFT_OPT_SCALE_METH )
    {
      printf( "scale_mtd %d ", context_fft->scale_method );
    }

    printf( "%-40s  ", context_fft->fRefName );
  }

  NASSERT( strlen( context->seqDir ) + strlen( context_fft->fInName ) < sizeof(fInNameBuf) );
  sprintf( fInNameBuf, "%s%s", context->seqDir, context_fft->fInName );
  NASSERT( strlen( context->seqDir ) + strlen( context_fft->fRefName ) < sizeof(fRefNameBuf) );
  sprintf( fRefNameBuf, "%s%s", context->seqDir, context_fft->fRefName );

  memset( &inVec , 0, sizeof(inVec ) );
  memset( &outVec, 0, sizeof(outVec) );
  memset( &refVec, 0, sizeof(refVec) );
  memset( &xVec  , 0, sizeof(xVec  ) );
  memset( &yVec  , 0, sizeof(yVec  ) );

  /* Allocate vectors for in/out/reference data. */
  if ( !vecAlloc( &inVec , N, TE_ALIGN_YES, FMT_FRACT16|FMT_CPLX, 0 ) ||
       !vecAlloc( &outVec, N, TE_ALIGN_YES, FMT_FLOAT64|FMT_CPLX, 0 ) ||
       !vecAlloc( &refVec, N, TE_ALIGN_YES, FMT_FLOAT64|FMT_CPLX, 0 ) )
  {
    printf( "fileProc_stnd(): failed to allocate in/out/ref vectors, N=%d\n", N );
  }
  /* Open input data file. */
  else if ( !( fIn = fopen( fInNameBuf, "rb" ) ) )
  {
    printf( "fileProc_stnd(): failed to open %s for reading\n", fInNameBuf );
  }
  /* Open reference data file. */
  else if ( !( fRef = fopen( fRefNameBuf, "rb" ) ) )
  {
    printf( "fileProc_stnd(): failed to open %s for reading\n", fRefNameBuf );
  }
  else res = 1;

  /*
   * Process the input file frame-by-frame.
   */

  if ( res )
  {
    int fmt = context->desc->fmt | FMT_CPLX;
    int isAligned = context->desc->isAligned;

    complex_fract16 * pin  = vecGetElem_fr16c( &inVec , 0 );
    complex_double  * pout = vecGetElem_fl64c( &outVec, 0 );
    complex_double  * pref = vecGetElem_fl64c( &refVec, 0 );

    float32_t sinadAvg, sinadMin = HUGE_VALF;
    float64_t errSum = 0, refSum = 0;

    int efbMin = 32;

    context_fft->frameCnt = 0;

    frameProcFxn = ( (tTestEngFrameProcFxn_fft_stnd*)context->target.fut );

    /* Read N 16-bit complex samples from the input file. */
    while ( ( n = fread( pin, sz_fr16c, N, fIn ) ) > 0 )
    {
      /* Augment the (last) incomplete frame with zeros. */
      memset( (uint8_t*)pin + n*sz_fr16c, 0, (N-n)*sz_fr16c );
      /* Zero the output frame. */
      memset( pout, 0, N*sz_fl64c );

      /* Allocate in/out buffers for the target FFT routine. */
      if ( !xVec.szBulk && ( 2 != vecsAlloc( isAligned, fmt, &xVec, N, &yVec, N, 0 ) ) )
      {
        printf( "fileProc_stnd(): failed to allocate xVec/yVec, "
                "frameCnt=%d, fmt=0x%x, N=%d\n", context_fft->frameCnt, fmt, N );
        res = 0; break;
      }

      /* Use a proprietary frame processing function to perform the target FFT. */
      if ( !( res = frameProcFxn( context, (fract16*)pin, (float64_t*)pout, &xVec, &yVec ) ) ) break;

      /* When in unaligned mode, in/out vectors should be periodically reallocated to try various
       * address offsets. */
      if ( !( ++context_fft->frameCnt % XY_VEC_REALLOC_PERIOD ) && !isAligned )
      {
        if ( !( res = vecsFree( &xVec, &yVec, 0 ) ) )
        {
          printf( "fileProc_stnd(): vecsFree() failed for xVec/yVec, "
                  "frameCnt=%d, fmt=0x%x, N=%d\n", context_fft->frameCnt, fmt, N );
          break;
        }

        memset( &xVec, 0, sizeof(xVec) );
        memset( &yVec, 0, sizeof(yVec) );
      }

      /* Read N double precision complex reference samples. */
      if ( (int)fread( pref, sz_fl64c, N, fRef ) < N )
      {
        printf( "fileProc_stnd(): failed to read reference data, "
                "frameCnt=%d, fmt=0x%x, N=%d\n", context_fft->frameCnt, fmt, N );
        res = 0; break;
      }

      /* Estimate the SINAD ratio for the current frame, and update error/reference
       * power sums for the whole file SINAD. */
      {
        float64_t refMax = 0, errMax = 0;
        float64_t p, q;

        for ( p=q=0, n=0; n<N; n++ )
        {
          double err_re,err_im;
          err_re=creal(pout[n]) - creal(pref[n]);
          err_im=cimag(pout[n]) - cimag(pref[n]);

          /* |signal|^2 */
          p += creal(pref[n])*creal(pref[n]) + cimag(pref[n])*cimag(pref[n]);
          /* |noise+distortion|^2 */
          q += err_re*err_re + err_im*err_im;

          refMax = MAX( refMax, MAX( fabs(creal(pref[n])), fabs(cimag(pref[n])) ) );
          errMax = MAX( errMax, MAX( fabs(err_re        ), fabs(err_im        ) ) );
        }

        refSum += p;
        errSum += q;

        if ( p>0 )
        {
          sinadMin = MIN( sinadMin, (float32_t)(p/q) );
          efbMin   = MAX( 0, MIN( efbMin, L_sub_ll( (int)logb(refMax), (int)logb(errMax) ) ) );
        }
      }
    }

    /*
     * Finalize the min SINAD estimation and print a summary.
     */

    if ( res )
    {
      sinadMin = 10.f*STDLIB_MATH(log10f)( sinadMin );

      /* Set the test result for test verification. */
      *vecGetElem_fl32( &context->dataSet.Z, 0 ) = sinadMin;

      if ( context->isVerbose )
      {
        sinadAvg = ( refSum > 0 ? 10.f*STDLIB_MATH(log10f)( (float32_t)(refSum/errSum) ) : -HUGE_VALF );

        if ( ( fmt & 15 ) == FMT_FRACT16 || ( fmt & 15 ) == FMT_FRACT32 )
        {
          printf( "Error-Free Bits %2d  SINAD min %5.1f dB  avg %5.1f dB  ",
                  efbMin, sinadMin, sinadAvg );
        }
        else
        {
          printf( "SINAD min %5.1f dB  avg %5.1f dB  ", sinadMin, sinadAvg );
        }
      }
    }
  }

  /*
   * Close files and free vectors.
   */

  if ( fIn  ) fclose( fIn  );
  if ( fRef ) fclose( fRef );

  if ( inVec .szBulk > 0 ) vecFree( &inVec  );
  if ( outVec.szBulk > 0 ) vecFree( &outVec );
  if ( refVec.szBulk > 0 ) vecFree( &refVec );
  if ( xVec  .szBulk > 0 ) vecFree( &xVec   );
  if ( yVec  .szBulk > 0 ) vecFree( &yVec   );

} /* fileProc_stnd() */
