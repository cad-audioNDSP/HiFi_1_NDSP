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
 * Test procedures for FIR
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"
/* Test data vectors tools and SEQ-file reader. */
#include "vectools.h"
#include "testeng_fir.h"
#include "testeng_fir_old.h"
#include <stdlib.h>
#include <string.h>

static int testExec( tTestEngTarget   targetFxn, const char * seqName,int isFull, int isVerbose, int breakOnError );

#define MIN(a,b)   ( (a)<(b) ? (a) : (b) )
#define MAX(a,b)   ( (a)>(b) ? (a) : (b) )

/* parameters in extraParam (bit 9:8) for load_crosscorr/process_crosscorr */
#define CROSSCORR_API (0<<8)
#define CONVOLVE_API  (1<<8)
#define LCROSSCORR_API (2<<8)   // linear correlation
#define LCONVOLVE_API  (3<<8)   // linear convolution

/* Allocate vectors and load the data set for autocorrelation functions:
 * vector X (in), vector Z (out) */
static int te_loadFxn_autocorr( tTestEngContext * context )
{
  int  N;
  int res;
  ASSERT( context && context->seqFile );
  N = MAX( 0, context->args.N );
  if(N<0) { N=0; }
  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate data vectors memory. */
  res = ( 4 == vecsAlloc( context->desc->isAligned, context->desc->fmt,
                         &context->dataSet.X  , N,
                         &context->dataSet.Z  , N,
                         &context->dataSet.Zlo, N,
                         &context->dataSet.Zhi, N, 0 ) );
  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &context->dataSet.X,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 0 ) ) )
    {
      printf( "te_loadFxn_autocorr(): failed to read vectors data\n");
    }
  }
  else
  {
    printf( "te_loadFxn_autocorr(): failed to allocate vectors\n");
  }

  /* Free vectors data if failed. */
  if ( !res )    freeVectors(context);
  return (res);

} /* te_loadFxn_autocorr() */
/*Function similar to te_loadFxn_autocorr() but lower 8 bits in elements of input buffer are zeroed */
static int te_loadFxn_autocorr_24( tTestEngContext * context )
{
  int  N, i;
  int res;
  ASSERT( context && context->seqFile );
  N = MAX( 0, context->args.N );
  if(N<0) { N=0; }
  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  /* Allocate data vectors memory. */
  res = ( 4 == vecsAlloc( context->desc->isAligned, context->desc->fmt,
                         &context->dataSet.X  , N,
                         &context->dataSet.Z  , N,
                         &context->dataSet.Zlo, N,
                         &context->dataSet.Zhi, N, 0 ) );
  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &context->dataSet.X,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 0 ) ) )
    {
      printf( "te_loadFxn_autocorr(): failed to read vectors data\n");
    }
	else
	{
		for(i=0;i<(context->dataSet.X.nElem);i++) ((int32_t *)((uint8_t*)context->dataSet.X.ptr.aligned + (context->dataSet.X.offset )*context->dataSet.X.szElem))[i]&=0xFFFFFF00;
	}
  }
  else
  {
    printf( "te_loadFxn_autocorr(): failed to allocate vectors\n");
  }

  /* Free vectors data if failed. */
  if ( !res )    freeVectors(context);
  return (res);

}
/* Allocate vectors and load the data set for crosscorrelation functions:
 * vector X (in), vector Z (out) */
static int te_loadFxn_crosscorr( tTestEngContext * context )
{
  int M, N;
  int lenX=0,lenY=0,lenZ=0;
  int res,fmtX,fmtY,fmtZ;
  ASSERT( context && context->seqFile );
  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  M = MAX( 0, context->args.N );
  N = MAX( 0, context->args.L );
  switch(context->desc->extraParam & (3<<8))
  {
      case CROSSCORR_API:
            lenX=N;
            lenY=M;
            lenZ=N;
            break;
      case LCROSSCORR_API:
            lenX=N;
            lenY=M;
            lenZ=N+M-1;
            break;
      case CONVOLVE_API:
            lenX=N;
            lenY=M;
            lenZ=N;
            break;
      case LCONVOLVE_API:
            lenX=N;
            lenY=M;
            lenZ=N+M-1;
            break;
      default: /* should not happen */
            ASSERT(0);
            break;
  }
  fmtX=fmtZ=context->desc->extraParam & 0x1f;
  fmtY=context->desc->fmt;

  /* Allocate data vectors memory. */
  res = ( 1 == vecsAlloc( context->desc->isAligned, fmtX,
                         &context->dataSet.X  , lenX, 0 ) );
  res&= ( 1 == vecsAlloc( context->desc->isAligned, fmtY,
                         &context->dataSet.Y  , lenY, 0 ) );
  res&= ( 3 == vecsAlloc( context->desc->isAligned, fmtZ,
                         &context->dataSet.Z  , lenZ,
                         &context->dataSet.Zlo, lenZ,
                         &context->dataSet.Zhi, lenZ, 0 ) );
  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &context->dataSet.X,
                                  &context->dataSet.Y,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 0 ) ) )
    {
      printf( "te_loadFxn_crosscorr(): failed to read vectors data\n");
    }
  }
  else
  {
    printf( "te_loadFxn_crosscorr(): failed to allocate vectors\n");
  }

  /* Free vectors data if failed. */
  if ( !res )    freeVectors(context);
  return (res);

} /* te_loadFxn_crosscorr() */ 
/*Function similar to te_loadFxn_crosscorr() but with lower 8 bits in each elements of input buffers zeroed*/
static int te_loadFxn_crosscorr_24( tTestEngContext * context )
{
  int M, N, i;
  int lenX=0,lenY=0,lenZ=0;
  int res,fmtX,fmtY,fmtZ;
  ASSERT( context && context->seqFile );
  memset( &context->dataSet, 0, sizeof(context->dataSet) );

  M = MAX( 0, context->args.N );
  N = MAX( 0, context->args.L );
  switch(context->desc->extraParam & (3<<8))
  {
      case CROSSCORR_API:
            lenX=N;
            lenY=M;
            lenZ=N;
            break;
      case LCROSSCORR_API:
            lenX=N;
            lenY=M;
            lenZ=N+M-1;
            break;
      case CONVOLVE_API:
            lenX=N;
            lenY=M;
            lenZ=N;
            break;
      case LCONVOLVE_API:
            lenX=N;
            lenY=M;
            lenZ=N+M-1;
            break;
      default: /* should not happen */
            ASSERT(0);
            break;
  }
  fmtX=fmtZ=context->desc->extraParam & 0x1f;
  fmtY=context->desc->fmt;

  /* Allocate data vectors memory. */
  res = ( 1 == vecsAlloc( context->desc->isAligned, fmtX,
                         &context->dataSet.X  , lenX, 0 ) );
  res&= ( 1 == vecsAlloc( context->desc->isAligned, fmtY,
                         &context->dataSet.Y  , lenY, 0 ) );
  res&= ( 3 == vecsAlloc( context->desc->isAligned, fmtZ,
                         &context->dataSet.Z  , lenZ,
                         &context->dataSet.Zlo, lenZ,
                         &context->dataSet.Zhi, lenZ, 0 ) );
  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &context->dataSet.X,
                                  &context->dataSet.Y,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 0 ) ) )
    {
      printf( "te_loadFxn_crosscorr(): failed to read vectors data\n");
    }
	else
	{
		for(i=0;i<(context->dataSet.X.nElem);i++) ((int32_t *)((uint8_t*)context->dataSet.X.ptr.aligned + (context->dataSet.X.offset )*context->dataSet.X.szElem))[i]&=0xFFFFFF00;
		for(i=0;i<(context->dataSet.Y.nElem);i++) ((int32_t *)((uint8_t*)context->dataSet.Y.ptr.aligned + (context->dataSet.Y.offset )*context->dataSet.Y.szElem))[i]&=0xFFFFFF00;
	}

  }
  else
  {
    printf( "te_loadFxn_crosscorr(): failed to allocate vectors\n");
  }

  /* Free vectors data if failed. */
  if ( !res )    freeVectors(context);
  return (res);

} 

/* Apply the target function to the test case data set (crosscorrelation):
 * vector X (in), vector Z (out) */
static void te_processFxn_autocorr( tTestEngContext * context )
{
  typedef void tFxnautocorr_scratch( void* S, void * X, const void * Z,  int N );
  typedef void tFxnautocorr( void * X, const void * Z,  int N );
  void *X, *Z;
  int N;
  ASSERT( context && context->target.fut );
  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );
  N=context->args.N;
  if(context->desc->isAligned)
  {
      ((tFxnautocorr*)context->target.fut)( Z , X, N );
  }
  else
  {
      /* allocate scratch and use anothe API */
      tAllocPtr scratch;
      void *S;
      size_t sz=0;
      switch(context->desc->fmt)
      {
      case FMT_FRACT16:      sz=FIR_ACORRA16X16_SCRATCH_SIZE(N); break;
      case FMT_FRACT32:      sz=FIR_ACORRA24X24_SCRATCH_SIZE(N); break;
      case FMT_FLOAT32:      sz=FIR_ACORRAF_SCRATCH_SIZE(N); break;
      }
      ASSERT(sz);
      S=mallocAlign(&scratch,sz,8);
      memset(S,0xcc,sz);
      ((tFxnautocorr_scratch*)context->target.fut)( S, Z , X, N );
      freeAlign(&scratch);
  }

} /* te_processFxn_autocorr() */ 


/* Apply the target function to the test case data set (crosscorrelation):
 * vector X (in), vector Z (out) */
static void te_processFxn_crosscorr( tTestEngContext * context )
{
  typedef void tFxncrosscorr( void * Z, const void * X,  const void * Y,  int N , int M );
  typedef void tFxncrosscorr_scratch(void* S, void * Z, const void * X,  const void * Y,  int N , int M );
  void *X, *Y, *Z;
  int M,N;
  ASSERT( context && context->target.fut );
  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );
  M = MAX( 0, context->args.N );
  N = MAX( 0, context->args.L );
  if(!context->desc->isAligned)
  {
        /* select right scratch size */
        tAllocPtr scratch;
        size_t sz;
        void * S;
        sz=0;
        switch(context->desc->extraParam & (3<<8))
        {
            case CROSSCORR_API:
                switch(context->desc->fmt | ((context->desc->extraParam & 0x1f)<<8))
                {
                    case (FMT_REAL|FMT_FLOAT32)|((FMT_REAL|FMT_FLOAT32)<<8): sz=FIR_XCORRAF_SCRATCH_SIZE(N,M);       break;
                    case (FMT_REAL|FMT_FRACT32)|((FMT_REAL|FMT_FRACT32)<<8): sz=FIR_XCORRA24X24_SCRATCH_SIZE(N,M);   break;
                    case (FMT_REAL|FMT_FRACT16)|((FMT_REAL|FMT_FRACT32)<<8): sz=FIR_XCORRA32X16_SCRATCH_SIZE(N,M);   break;
                    case (FMT_CPLX|FMT_FLOAT32)|((FMT_CPLX|FMT_FLOAT32)<<8): sz=CXFIR_XCORRAF_SCRATCH_SIZE(N,M);     break;
                    case (FMT_REAL|FMT_FRACT16)|((FMT_REAL|FMT_FRACT16)<<8): sz=FIR_XCORRA16X16_SCRATCH_SIZE(N,M);   break;
                    default: ASSERT(0);
                }
                break;
            case CONVOLVE_API:
                switch(context->desc->fmt | ((context->desc->extraParam & 0x1f)<<8))
                {
                    case (FMT_REAL|FMT_FLOAT32)|((FMT_REAL|FMT_FLOAT32)<<8): sz=FIR_CONVOLAF_SCRATCH_SIZE(N,M);      break;
                    case (FMT_REAL|FMT_FRACT32)|((FMT_REAL|FMT_FRACT32)<<8): sz=FIR_CONVOLA24X24_SCRATCH_SIZE(N,M);  break;
                    case (FMT_REAL|FMT_FRACT16)|((FMT_REAL|FMT_FRACT32)<<8): sz=FIR_CONVOLA32X16_SCRATCH_SIZE(N,M);  break;
                    case (FMT_CPLX|FMT_FRACT16)|((FMT_CPLX|FMT_FRACT32)<<8): sz=CXFIR_CONVOLA32X16_SCRATCH_SIZE(N,M);break;
                    case (FMT_REAL|FMT_FRACT16)|((FMT_REAL|FMT_FRACT16)<<8): sz=FIR_CONVOLA16X16_SCRATCH_SIZE(N,M);  break;
                    default: ASSERT(0);
                }
                break;
            case LCROSSCORR_API:
                switch(context->desc->fmt | ((context->desc->extraParam & 0x1f)<<8))
                {
                    case (FMT_REAL|FMT_FLOAT32)|((FMT_REAL|FMT_FLOAT32)<<8): sz=FIR_LXCORRAF_SCRATCH_SIZE(N,M);      break;
                    case (FMT_REAL|FMT_FRACT16)|((FMT_REAL|FMT_FRACT16)<<8): sz=FIR_LXCORRA16X16_SCRATCH_SIZE(N,M);  break;
                    case (FMT_REAL|FMT_FRACT32)|((FMT_REAL|FMT_FRACT32)<<8): sz=FIR_LXCORRA32X32_SCRATCH_SIZE(N,M);  break;
                    case (FMT_REAL|FMT_FRACT16)|((FMT_REAL|FMT_FRACT32)<<8): sz=FIR_LXCORRA32X16_SCRATCH_SIZE(N,M);  break;
                    default: ASSERT(0);
                }
                break;
            case LCONVOLVE_API:
                switch(context->desc->fmt | ((context->desc->extraParam & 0x1f)<<8))
                {
                    case (FMT_REAL|FMT_FLOAT32)|((FMT_REAL|FMT_FLOAT32)<<8): sz=FIR_LCONVOLAF_SCRATCH_SIZE(N,M);      break;
                    case (FMT_REAL|FMT_FRACT32)|((FMT_REAL|FMT_FRACT32)<<8): sz=FIR_LCONVOLA32X32_SCRATCH_SIZE(N,M);  break;
                    case (FMT_REAL|FMT_FRACT16)|((FMT_REAL|FMT_FRACT32)<<8): sz=FIR_LCONVOLA32X16_SCRATCH_SIZE(N,M);  break;
                    case (FMT_REAL|FMT_FRACT16)|((FMT_REAL|FMT_FRACT16)<<8): sz=FIR_LCONVOLA16X16_SCRATCH_SIZE(N,M);  break;
                    default: ASSERT(0);
                }
                break;
            default:                ASSERT(0);
        }
        S=mallocAlign(&scratch,sz,8);
        memset(S,0xcc,sz);
        ((tFxncrosscorr_scratch*)context->target.fut)( S, Z, X, Y, N,M );
        freeAlign(&scratch);
  }
  else
  {
    ((tFxncrosscorr*)context->target.fut)( Z, X, Y, N,M );
  }
} /* te_processFxn_crosscorr() */ 

typedef struct
{
    tVec h;          /* impulse response */
    tVec normmu;     /* delay line */
}
tLmsContext;


/* function reads impulse response from file and creates FIR structure. returns 0 if failed */
static int te_create_lms(tTestEngContext * context)
{
    tLmsContext *lmsContext;
    lmsContext = (tLmsContext *)malloc(sizeof(*lmsContext));
    context->target.handle = (void*)lmsContext;
    if (lmsContext==NULL) return 0;
    memset(lmsContext, 0, sizeof(*lmsContext));
    return 1;
}

static void lms_vecsFree(tTestEngContext * context)
{
    tLmsContext *lmsContext;
    lmsContext = (tLmsContext *)context->target.handle;
    if (lmsContext)
    {
        if(lmsContext->h.szBulk) vecFree(&lmsContext->h);
        if(lmsContext->normmu.szBulk) vecFree(&lmsContext->normmu);
    }
}
/* function destroys FIR structure, returns 0 if failed */
static int te_destroy_lms(tTestEngContext * context)
{
    tLmsContext *lmsContext;
    lmsContext = (tLmsContext *)context->target.handle;
    if (lmsContext)
    {
        lms_vecsFree(context);
        free(lmsContext);
    }
    return 1;
}

/* Allocate vectors and load the data set for lms functions: */
static int te_loadFxn_lms( tTestEngContext * context )
{
    int irfmt,errfmt,normmufmt;
    tLmsContext *lmsContext = (tLmsContext *)context->target.handle;
    int M, N, P;
    int res;

    ASSERT( context && context->seqFile );

    M = context->args.N ;
    N = context->args.L ;
    P = MAX( 0, M+N-1);
    M = MAX( 0, M );
    N = MAX( 0, N );
    memset( &context->dataSet, 0, sizeof(context->dataSet) );
    lms_vecsFree(context);
    /* use 32-bit IR data for 16-bit functions */
    irfmt    =context->desc->extraParam & 0x1f;
    errfmt   =context->desc->extraParam & 0x1f;
    normmufmt=context->desc->extraParam & 0x1f;

  /* Allocate data vectors memory. */
  res = ( 2 == vecsAlloc( context->desc->isAligned, context->desc->fmt,
                         &context->dataSet.X  , P,
                         &context->dataSet.Y  , N, /*reference (near end) data vector (r) */
                         0 )) ;
  res&= ( 3 == vecsAlloc( context->desc->isAligned, errfmt,
                         &context->dataSet.Z  , N, /*error vector                         */
                         &context->dataSet.Zlo, N, /*                                     */
                         &context->dataSet.Zhi, N, /*                                     */
                         0 )) ;
  res&= ( 1 == vecsAlloc( context->desc->isAligned, normmufmt,
                         &lmsContext->normmu,   2,
                         0 )) ;
  res &= (4 == vecsAlloc( context->desc->isAligned, irfmt,
                         &context->dataSet.W  , M, /* impulse response (h)                */
                         &context->dataSet.Wlo, M,
                         &context->dataSet.Whi, M,
                         &lmsContext->h,        M,
                         0 )) ;

  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &lmsContext->h,
                                  &context->dataSet.Y,
                                  &context->dataSet.X,
                                  &lmsContext->normmu,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 
                                  &context->dataSet.Wlo,
                                  &context->dataSet.Whi, 
                                  0 ) ) )
    {
      printf( "te_loadFxn_lms(): failed to read vectors data\n");
    }
  }
  else
  {
    printf( "te_loadFxn_lms(): failed to allocate vectors\n");
  }

  /* Free vectors data if failed. */
  if ( !res )    
  {
    freeVectors(context);
    lms_vecsFree(context);
  }
  return (res);

} /* te_loadFxn_lms() */
/*Function similar to te_loadFxn_lms() but lower 8 bits in elements of input buffer are zeroed */
static int te_loadFxn_lms_24( tTestEngContext * context )
{
    int irfmt,errfmt,normmufmt;
    tLmsContext *lmsContext = (tLmsContext *)context->target.handle;
    int M, N, P, i;
    int res;

    ASSERT( context && context->seqFile );

    M = context->args.N ;
    N = context->args.L ;
    P = MAX( 0, M+N-1);
    M = MAX( 0, M );
    N = MAX( 0, N );
    memset( &context->dataSet, 0, sizeof(context->dataSet) );
    lms_vecsFree(context);
    /* use 32-bit IR data for 16-bit functions */
    irfmt    =context->desc->extraParam & 0x1f;
    errfmt   =context->desc->extraParam & 0x1f;
    normmufmt=context->desc->extraParam & 0x1f;

  /* Allocate data vectors memory. */
  res = ( 2 == vecsAlloc( context->desc->isAligned, context->desc->fmt,
                         &context->dataSet.X  , P,
                         &context->dataSet.Y  , N, /*reference (near end) data vector (r) */
                         0 )) ;
  res&= ( 3 == vecsAlloc( context->desc->isAligned, errfmt,
                         &context->dataSet.Z  , N, /*error vector                         */
                         &context->dataSet.Zlo, N, /*                                     */
                         &context->dataSet.Zhi, N, /*                                     */
                         0 )) ;
  res&= ( 1 == vecsAlloc( context->desc->isAligned, normmufmt,
                         &lmsContext->normmu,   2,
                         0 )) ;
  res &= (4 == vecsAlloc( context->desc->isAligned, irfmt,
                         &context->dataSet.W  , M, /* impulse response (h)                */
                         &context->dataSet.Wlo, M,
                         &context->dataSet.Whi, M,
                         &lmsContext->h,        M,
                         0 )) ;

  if ( res )
  {
    /* Load vectors data from the SEQ-file. */
    if ( !( res = seqFileReadVecs( context->seqFile,
                                  &lmsContext->h,
                                  &context->dataSet.Y,
                                  &context->dataSet.X,
                                  &lmsContext->normmu,
                                  &context->dataSet.Zlo,
                                  &context->dataSet.Zhi, 
                                  &context->dataSet.Wlo,
                                  &context->dataSet.Whi, 
                                  0 ) ) )
    {
      printf( "te_loadFxn_lms(): failed to read vectors data\n");
    }
	else
	{
		for(i=0;i<(context->dataSet.X.nElem);i++) ((int32_t *)((uint8_t*)context->dataSet.X.ptr.aligned + (context->dataSet.X.offset )*context->dataSet.X.szElem))[i]&=0xFFFFFF00;
		for(i=0;i<(context->dataSet.Y.nElem);i++) ((int32_t *)((uint8_t*)context->dataSet.Y.ptr.aligned + (context->dataSet.Y.offset )*context->dataSet.Y.szElem))[i]&=0xFFFFFF00;
		for(i=0;i<(lmsContext->h.nElem);i++) ((int32_t *)((uint8_t*)lmsContext->h.ptr.aligned + (lmsContext->h.offset )*lmsContext->h.szElem))[i]&=0xFFFFFF00;
	}

  }
  else
  {
    printf( "te_loadFxn_lms(): failed to allocate vectors\n");
  }

  /* Free vectors data if failed. */
  if ( !res )    
  {
    freeVectors(context);
    lms_vecsFree(context);
  }
  return (res);

}

/* Apply the target function to the test case data set (LMS):
 * vector X (in), vector Z (out) */
static void te_processFxn_lms( tTestEngContext * context )
{
    tLmsContext *lmsContext;
    typedef void tFxn_float32x32(float32_t *e, float32_t *h, const float32_t *r,const float32_t *x, float32_t norm, float32_t mu,int N, int M);
    typedef void tFxn_fract32x32(fract32   *e, fract32   *h, const fract32   *r,const fract32   *x, fract32   norm, fract32   mu,int N, int M);
    typedef void tFxn_fract16x32(fract32   *e, fract32   *h, const fract16   *r,const fract16   *x, fract32   norm, fract16   mu,int N, int M);
    typedef void tFxn_fract16x16(int16_t   *e, int16_t   *h, const int16_t   *r,const int16_t   *x, int16_t   norm, int16_t   mu,int N, int M);
    int M,N;
    void *E, *Hout,*Hin,*R,*X;

    ASSERT( context && context->target.fut );
    lmsContext = (tLmsContext *)context->target.handle;
    N=context->args.L; M=context->args.N;
    E    = vecGetElem( &context->dataSet.Z, 0 );
    Hout = vecGetElem( &context->dataSet.W, 0 );
    Hin  = vecGetElem( &lmsContext->h     , 0 );
    R    = vecGetElem( &context->dataSet.Y, 0 );
    X    = vecGetElem( &context->dataSet.X, 0 );
    memcpy(Hout,Hin,context->dataSet.W.nElem*context->dataSet.W.szElem);  /* copy input IR to the output */
    switch(context->desc->fmt & 7)
    {
    case FMT_FLOAT32:
    {
        const float32_t* _normmu=vecGetElem_fl32(&lmsContext->normmu,0);
        ((tFxn_float32x32*)context->target.fut)((float32_t *)E,(float32_t *)Hout,(const float32_t *)R,(const float32_t *)X,_normmu[0],_normmu[1],N,M);
        break;
    }
    case FMT_FRACT16:
    {
        switch(context->desc->extraParam & 0x1f)
        {
        case FMT_FRACT32:
        {
            const fract32  * _normmu=vecGetElem_fr32(&lmsContext->normmu,0);
            ((tFxn_fract16x32*)context->target.fut)((fract32   *)E,(fract32   *)Hout,(const fract16   *)R,(const fract16   *)X,_normmu[0],(int16_t)_normmu[1],N,M);
            break;
        }
        case FMT_FRACT16:
        {
            const fract16  * _normmu=vecGetElem_fr16(&lmsContext->normmu,0);
            ((tFxn_fract16x16*)context->target.fut)((int16_t   *)E,(int16_t   *)Hout,(const int16_t   *)R,(const int16_t   *)X,_normmu[0],(int16_t)_normmu[1],N,M);
            break;
        }
        default:
            ASSERT(0);
        }
        break;
    }
    case FMT_FRACT32:
    {
        const fract32  * _normmu=vecGetElem_fr32(&lmsContext->normmu,0);
        ((tFxn_fract32x32*)context->target.fut)((fract32   *)E,(fract32   *)Hout,(const fract32   *)R,(const fract32   *)X,_normmu[0],_normmu[1],N,M);
        break;
    }
    default: 
        ASSERT(0); /* not supported yet */
        break;
    }
} /* te_processFxn_lms() */

/* phase 1*/
static tFirOldDescr api_bkfir32x16    ={(tFirOldFxnAlloc*)bkfir32x16_alloc,    (tFirOldFxnInit*)bkfir32x16_init,    (tFirOldFxnProcess*)bkfir32x16_process    };
static tFirOldDescr api_bkfir32x32    ={(tFirOldFxnAlloc*)bkfir32x32_alloc,    (tFirOldFxnInit*)bkfir32x32_init,    (tFirOldFxnProcess*)bkfir32x32_process    };
static tFirOldDescr api_bkfir24x24    ={(tFirOldFxnAlloc*)bkfir24x24_alloc,    (tFirOldFxnInit*)bkfir24x24_init,    (tFirOldFxnProcess*)bkfir24x24_process    };
static tFirOldDescr api_bkfir24x24p   ={(tFirOldFxnAlloc*)bkfir24x24p_alloc,   (tFirOldFxnInit*)bkfir24x24p_init,   (tFirOldFxnProcess*)bkfir24x24p_process   };
static tFirOldDescr api_cxfir32x16    ={(tFirOldFxnAlloc*)cxfir32x16_alloc,    (tFirOldFxnInit*)cxfir32x16_init,    (tFirOldFxnProcess*)cxfir32x16_process    };
static tFirOldDescr api_cxfir24x24    ={(tFirOldFxnAlloc*)cxfir24x24_alloc,    (tFirOldFxnInit*)cxfir24x24_init,    (tFirOldFxnProcess*)cxfir24x24_process    };
static tFirOldDescr api_bkfira32x16   ={(tFirOldFxnAlloc*)bkfira32x16_alloc,   (tFirOldFxnInit*)bkfira32x16_init,   (tFirOldFxnProcess*)bkfira32x16_process   };
static tFirOldDescr api_bkfira24x24   ={(tFirOldFxnAlloc*)bkfira24x24_alloc,   (tFirOldFxnInit*)bkfira24x24_init,   (tFirOldFxnProcess*)bkfira24x24_process   };
static tFirOldDescr api_firdec32x16   ={(tFirOldFxnAlloc*)firdec32x16_alloc,   (tFirOldFxnInit*)firdec32x16_init,   (tFirOldFxnProcess*)firdec32x16_process   };
static tFirOldDescr api_firdec24x24   ={(tFirOldFxnAlloc*)firdec24x24_alloc,   (tFirOldFxnInit*)firdec24x24_init,   (tFirOldFxnProcess*)firdec24x24_process   };
static tFirOldDescr api_firinterp32x16={(tFirOldFxnAlloc*)firinterp32x16_alloc,(tFirOldFxnInit*)firinterp32x16_init,(tFirOldFxnProcess*)firinterp32x16_process};
static tFirOldDescr api_firinterp24x24={(tFirOldFxnAlloc*)firinterp24x24_alloc,(tFirOldFxnInit*)firinterp24x24_init,(tFirOldFxnProcess*)firinterp24x24_process};
static const tTestEngDesc descr_bkfir32x16      = { FMT_REAL |FMT_FRACT32 , TE_FIR_FIR|FMT_REAL |FMT_FRACT16 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_bkfir32x32      = { FMT_REAL |FMT_FRACT32 , TE_FIR_FIR|FMT_REAL |FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_bkfir24x24      = { FMT_REAL |FMT_FRACT32 , TE_FIR_FIR|FMT_REAL |FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old_24, te_destroy_fir_old, &te_loadFxn_fir_old_24, &te_processFxn_fir_old };
static const tTestEngDesc descr_cxfir32x16      = { FMT_CPLX |FMT_FRACT32 , TE_FIR_FIR|FMT_CPLX |FMT_FRACT16 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_cxfir24x24      = { FMT_CPLX |FMT_FRACT32 , TE_FIR_FIR|FMT_CPLX |FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_bkfira32x16     = { FMT_REAL |FMT_FRACT32 , TE_FIR_FIR|FMT_REAL |FMT_FRACT16 ,TE_ARGNUM_1, TE_ALIGN_NO , te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_bkfira24x24     = { FMT_REAL |FMT_FRACT32 , TE_FIR_FIR|FMT_REAL |FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_NO , te_create_fir_old_24, te_destroy_fir_old, &te_loadFxn_fir_old_24, &te_processFxn_fir_old };
static const tTestEngDesc descr_firdec32x16     = { FMT_REAL |FMT_FRACT32 , TE_FIR_DN |FMT_REAL |FMT_FRACT16 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_firdec24x24     = { FMT_REAL |FMT_FRACT32 , TE_FIR_DN |FMT_REAL |FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old_24, te_destroy_fir_old, &te_loadFxn_fir_old_24, &te_processFxn_fir_old };
static const tTestEngDesc descr_firinterp32x16  = { FMT_REAL |FMT_FRACT32 , TE_FIR_UP |FMT_REAL |FMT_FRACT16 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_firinterp24x24  = { FMT_REAL |FMT_FRACT32 , TE_FIR_UP |FMT_REAL |FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old_24, te_destroy_fir_old, &te_loadFxn_fir_old_24, &te_processFxn_fir_old };
/* phase 2*/
static tFirOldDescr api_bkfirf        ={(tFirOldFxnAlloc*)bkfirf_alloc,        (tFirOldFxnInit*)bkfirf_init,        (tFirOldFxnProcess*)bkfirf_process        };
static tFirOldDescr api_bkfiraf       ={(tFirOldFxnAlloc*)bkfiraf_alloc,       (tFirOldFxnInit*)bkfiraf_init,       (tFirOldFxnProcess*)bkfiraf_process       };
static tFirOldDescr api_cxfirf        ={(tFirOldFxnAlloc*)cxfirf_alloc,        (tFirOldFxnInit*)cxfirf_init,        (tFirOldFxnProcess*)cxfirf_process        };
static tFirOldDescr api_firdecf       ={(tFirOldFxnAlloc*)firdecf_alloc,       (tFirOldFxnInit*)firdecf_init,       (tFirOldFxnProcess*)firdecf_process       };
static tFirOldDescr api_firinterpf    ={(tFirOldFxnAlloc*)firinterpf_alloc,    (tFirOldFxnInit*)firinterpf_init,    (tFirOldFxnProcess*)firinterpf_process    };
static const tTestEngDesc descr_bkfirf          = { FMT_REAL | FMT_FLOAT32, TE_FIR_FIR|FMT_REAL | FMT_FLOAT32,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_bkfiraf         = { FMT_REAL | FMT_FLOAT32, TE_FIR_FIR|FMT_REAL | FMT_FLOAT32,TE_ARGNUM_1, TE_ALIGN_NO , te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_cxfirf          = { FMT_CPLX | FMT_FLOAT32, TE_FIR_FIR|FMT_CPLX | FMT_FLOAT32,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_firdecf         = { FMT_REAL | FMT_FLOAT32, TE_FIR_DN |FMT_REAL | FMT_FLOAT32,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_firinterpf      = { FMT_REAL | FMT_FLOAT32, TE_FIR_UP |FMT_REAL | FMT_FLOAT32,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };

/* phase 3*/
static tFirOldDescr api_bkfir16x16    ={(tFirOldFxnAlloc*)bkfir16x16_alloc,    (tFirOldFxnInit*)bkfir16x16_init,    (tFirOldFxnProcess*)bkfir16x16_process    };
static tFirOldDescr api_cxfir16x16    ={(tFirOldFxnAlloc*)cxfir16x16_alloc,    (tFirOldFxnInit*)cxfir16x16_init,    (tFirOldFxnProcess*)cxfir16x16_process    };
static tFirOldDescr api_bkfira16x16   ={(tFirOldFxnAlloc*)bkfira16x16_alloc,   (tFirOldFxnInit*)bkfira16x16_init,   (tFirOldFxnProcess*)bkfira16x16_process   };
static tFirOldDescr api_firdec16x16   ={(tFirOldFxnAlloc*)firdec16x16_alloc,   (tFirOldFxnInit*)firdec16x16_init,   (tFirOldFxnProcess*)firdec16x16_process   };
static tFirOldDescr api_firinterp16x16={(tFirOldFxnAlloc*)firinterp16x16_alloc,(tFirOldFxnInit*)firinterp16x16_init,(tFirOldFxnProcess*)firinterp16x16_process};
static tFirOldDescr api_cxfirinterp16x16={(tFirOldFxnAlloc*)cxfirinterp16x16_alloc,(tFirOldFxnInit*)cxfirinterp16x16_init,(tFirOldFxnProcess*)cxfirinterp16x16_process};
static tFirOldDescr api_bkfira32x32   ={(tFirOldFxnAlloc*)bkfira32x32_alloc,   (tFirOldFxnInit*)bkfira32x32_init,   (tFirOldFxnProcess*)bkfira32x32_process   };
static tFirOldDescr api_cxfir32x32    ={(tFirOldFxnAlloc*)cxfir32x32_alloc,    (tFirOldFxnInit*)cxfir32x32_init,    (tFirOldFxnProcess*)cxfir32x32_process    };
static tFirOldDescr api_firdec32x32   ={(tFirOldFxnAlloc*)firdec32x32_alloc,   (tFirOldFxnInit*)firdec32x32_init,   (tFirOldFxnProcess*)firdec32x32_process   };
static tFirOldDescr api_firinterp32x32={(tFirOldFxnAlloc*)firinterp32x32_alloc,(tFirOldFxnInit*)firinterp32x32_init,(tFirOldFxnProcess*)firinterp32x32_process};

static const tTestEngDesc descr_bkfir16x16      = { FMT_REAL | FMT_FRACT16, TE_FIR_FIR|FMT_REAL | FMT_FRACT16,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_cxfir16x16      = { FMT_CPLX | FMT_FRACT16, TE_FIR_FIR|FMT_CPLX | FMT_FRACT16,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_bkfira16x16     = { FMT_REAL | FMT_FRACT16, TE_FIR_FIR|FMT_REAL | FMT_FRACT16,TE_ARGNUM_1, TE_ALIGN_NO , te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_firdec16x16     = { FMT_REAL | FMT_FRACT16, TE_FIR_DN |FMT_REAL | FMT_FRACT16,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_firinterp16x16  = { FMT_REAL | FMT_FRACT16, TE_FIR_UP |FMT_REAL | FMT_FRACT16,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_cxfirinterp16x16= { FMT_CPLX | FMT_FRACT16, TE_FIR_UP |FMT_REAL | FMT_FRACT16,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_cxfir32x32      = { FMT_CPLX |FMT_FRACT32 , TE_FIR_FIR|FMT_CPLX |FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_bkfira32x32     = { FMT_REAL |FMT_FRACT32 , TE_FIR_FIR|FMT_REAL |FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_NO , te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_firdec32x32     = { FMT_REAL |FMT_FRACT32 , TE_FIR_DN |FMT_REAL |FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };
static const tTestEngDesc descr_firinterp32x32  = { FMT_REAL |FMT_FRACT32 , TE_FIR_UP |FMT_REAL |FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_YES, te_create_fir_old, te_destroy_fir_old, &te_loadFxn_fir_old, &te_processFxn_fir_old };

typedef struct
{
  int                 phaseNum;
  const tTestEngDesc *pFirDescr;
  tTestEngTarget      fxns;
  int                 runAlways;   	/* 1-always */
  int                 testvecFlag;  /* 2-sanity; 1-brief; 0-full */
  const char*         seqFile;
}
tTbl;

static const tTbl tests[] =
{
  { 1, &descr_bkfir32x16, (tTestEngTarget)&api_bkfir32x16, 1,2,"bkfir32x16_lpf1.seq" },
  { 1, &descr_bkfir32x16, (tTestEngTarget)&api_bkfir32x16, 1,2,"bkfir32x16_bpf1.seq" },
  { 1, &descr_bkfir32x16, (tTestEngTarget)&api_bkfir32x16, 1,2,"bkfir32x16_4taps.seq"},
  { 1, &descr_bkfir32x16, (tTestEngTarget)&api_bkfir32x16, 0,0,"bkfir32x16_lpf2.seq" },
  { 1, &descr_bkfir32x16, (tTestEngTarget)&api_bkfir32x16, 0,0,"bkfir32x16_hpf1.seq" },
  { 1, &descr_bkfir32x16, (tTestEngTarget)&api_bkfir32x16, 0,0,"bkfir32x16_bpf2.seq" },

  { 1, &descr_bkfir32x32, (tTestEngTarget)&api_bkfir32x32, 1,2,"bkfir32x32_lpf1.seq" },
  { 1, &descr_bkfir32x32, (tTestEngTarget)&api_bkfir32x32, 1,2,"bkfir32x32_bpf1.seq" },
  { 1, &descr_bkfir32x32, (tTestEngTarget)&api_bkfir32x32, 1,2,"bkfir32x32_4taps.seq"},
  { 1, &descr_bkfir32x32, (tTestEngTarget)&api_bkfir32x32, 0,0,"bkfir32x32_lpf2.seq" },
  { 1, &descr_bkfir32x32, (tTestEngTarget)&api_bkfir32x32, 0,0,"bkfir32x32_hpf1.seq" },
  { 1, &descr_bkfir32x32, (tTestEngTarget)&api_bkfir32x32, 0,0,"bkfir32x32_bpf2.seq" },

  { 1, &descr_bkfira24x24, (tTestEngTarget)&api_bkfira24x24, 1,2,"bkfira24x24_lpf1.seq"  },
  { 1, &descr_bkfira24x24, (tTestEngTarget)&api_bkfira24x24, 1,2,"bkfira24x24_bpf1.seq"  },
  { 1, &descr_bkfira24x24, (tTestEngTarget)&api_bkfira24x24, 1,2,"bkfira24x24_3taps.seq" },
  { 1, &descr_bkfira24x24, (tTestEngTarget)&api_bkfira24x24, 0,0,"bkfira24x24_lpf2.seq"  },
  { 1, &descr_bkfira24x24, (tTestEngTarget)&api_bkfira24x24, 0,0,"bkfira24x24_hpf1.seq"  },
  { 1, &descr_bkfira24x24, (tTestEngTarget)&api_bkfira24x24, 0,0,"bkfira24x24_bpf2.seq"  },
  { 1, &descr_bkfira24x24, (tTestEngTarget)&api_bkfira24x24, 0,0,"bkfira24x24_4taps.seq" },
  { 1, &descr_bkfira24x24, (tTestEngTarget)&api_bkfira24x24, 0,0,"bkfira24x24_2taps.seq" },
  { 1, &descr_bkfira24x24, (tTestEngTarget)&api_bkfira24x24, 0,0,"bkfira24x24_1tap.seq"  },

  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24, 1,2,"bkfir24x24_lpf1.seq" },
  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24, 1,2,"bkfir24x24_bpf1.seq" },
  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24, 1,2,"bkfir24x24_4taps.seq"},
  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24, 0,0,"bkfir24x24_lpf2.seq" },
  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24, 0,0,"bkfir24x24_hpf1.seq" },
  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24, 0,0,"bkfir24x24_bpf2.seq" },

  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24p, 0,2,"bkfir24x24p_lpf1.seq" },
  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24p, 0,2,"bkfir24x24p_bpf1.seq" },
  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24p, 0,2,"bkfir24x24p_4taps.seq"},
  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24p, 0,0,"bkfir24x24p_lpf2.seq" },
  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24p, 0,0,"bkfir24x24p_hpf1.seq" },
  { 1, &descr_bkfir24x24, (tTestEngTarget)&api_bkfir24x24p, 0,0,"bkfir24x24p_bpf2.seq" },

  { 1, &descr_cxfir32x16, (tTestEngTarget)&api_cxfir32x16, 1,2,"cxfir32x16_lpf1.seq" },
  { 1, &descr_cxfir32x16, (tTestEngTarget)&api_cxfir32x16, 1,1,"cxfir32x16_bpf1.seq" },
  { 1, &descr_cxfir32x16, (tTestEngTarget)&api_cxfir32x16, 1,1,"cxfir32x16_4taps.seq"},
  { 1, &descr_cxfir32x16, (tTestEngTarget)&api_cxfir32x16, 0,0,"cxfir32x16_lpf2.seq" },
  { 1, &descr_cxfir32x16, (tTestEngTarget)&api_cxfir32x16, 0,0,"cxfir32x16_hpf1.seq" },
  { 1, &descr_cxfir32x16, (tTestEngTarget)&api_cxfir32x16, 0,0,"cxfir32x16_bpf2.seq" },

  { 1, &descr_cxfir24x24, (tTestEngTarget)&api_cxfir24x24, 1,2,"cxfir24x24_lpf1.seq" },
  { 1, &descr_cxfir24x24, (tTestEngTarget)&api_cxfir24x24, 1,1,"cxfir24x24_bpf1.seq" },
  { 1, &descr_cxfir24x24, (tTestEngTarget)&api_cxfir24x24, 1,1,"cxfir24x24_4taps.seq"},
  { 1, &descr_cxfir24x24, (tTestEngTarget)&api_cxfir24x24, 0,0,"cxfir24x24_lpf2.seq" },
  { 1, &descr_cxfir24x24, (tTestEngTarget)&api_cxfir24x24, 0,0,"cxfir24x24_hpf1.seq" },
  { 1, &descr_cxfir24x24, (tTestEngTarget)&api_cxfir24x24, 0,0,"cxfir24x24_bpf2.seq" },

  { 1, &descr_bkfira32x16, (tTestEngTarget)&api_bkfira32x16, 1,2,"bkfira32x16_lpf1.seq"  },
  { 1, &descr_bkfira32x16, (tTestEngTarget)&api_bkfira32x16, 1,2,"bkfira32x16_bpf1.seq"  },
  { 1, &descr_bkfira32x16, (tTestEngTarget)&api_bkfira32x16, 1,2,"bkfira32x16_3taps.seq" },
  { 1, &descr_bkfira32x16, (tTestEngTarget)&api_bkfira32x16, 0,0,"bkfira32x16_lpf2.seq"  },
  { 1, &descr_bkfira32x16, (tTestEngTarget)&api_bkfira32x16, 0,0,"bkfira32x16_hpf1.seq"  },
  { 1, &descr_bkfira32x16, (tTestEngTarget)&api_bkfira32x16, 0,0,"bkfira32x16_bpf2.seq"  },
  { 1, &descr_bkfira32x16, (tTestEngTarget)&api_bkfira32x16, 0,0,"bkfira32x16_4taps.seq" },
  { 1, &descr_bkfira32x16, (tTestEngTarget)&api_bkfira32x16, 0,0,"bkfira32x16_2taps.seq" },
  { 1, &descr_bkfira32x16, (tTestEngTarget)&api_bkfira32x16, 0,0,"bkfira32x16_1tap.seq"  },

  { 1, &descr_firdec32x16, (tTestEngTarget)&api_firdec32x16,1,2,"firdec32x16_dn2x.seq" },
  { 1, &descr_firdec32x16, (tTestEngTarget)&api_firdec32x16,1,1,"firdec32x16_dn3x.seq" },
  { 1, &descr_firdec32x16, (tTestEngTarget)&api_firdec32x16,1,1,"firdec32x16_dn11x.seq"},
  { 1, &descr_firdec32x16, (tTestEngTarget)&api_firdec32x16,1,0,"firdec32x16_dn6x.seq" },
  { 1, &descr_firdec32x16, (tTestEngTarget)&api_firdec32x16,0,0,"firdec32x16_dn4x.seq" },
  { 1, &descr_firdec32x16, (tTestEngTarget)&api_firdec32x16,0,0,"firdec32x16_dn5x.seq" },
  { 1, &descr_firdec32x16, (tTestEngTarget)&api_firdec32x16,0,0,"firdec32x16_dn23x.seq"},

  { 1, &descr_firdec24x24, (tTestEngTarget)&api_firdec24x24,1,2,"firdec24x24_dn2x.seq" },
  { 1, &descr_firdec24x24, (tTestEngTarget)&api_firdec24x24,1,1,"firdec24x24_dn3x.seq" },
  { 1, &descr_firdec24x24, (tTestEngTarget)&api_firdec24x24,1,1,"firdec24x24_dn11x.seq"},
  { 1, &descr_firdec24x24, (tTestEngTarget)&api_firdec24x24,1,1,"firdec24x24_dn6x.seq" },
  { 1, &descr_firdec24x24, (tTestEngTarget)&api_firdec24x24,0,0,"firdec24x24_dn4x.seq" },
  { 1, &descr_firdec24x24, (tTestEngTarget)&api_firdec24x24,0,0,"firdec24x24_dn5x.seq" },
  { 1, &descr_firdec24x24, (tTestEngTarget)&api_firdec24x24,0,0,"firdec24x24_dn23x.seq"},

  { 1, &descr_firinterp32x16, (tTestEngTarget)&api_firinterp32x16,1,2,"firinterp32x16_up2x.seq" },
  { 1, &descr_firinterp32x16, (tTestEngTarget)&api_firinterp32x16,1,1,"firinterp32x16_up3x.seq" },
  { 1, &descr_firinterp32x16, (tTestEngTarget)&api_firinterp32x16,1,1,"firinterp32x16_up6x.seq"},
  { 1, &descr_firinterp32x16, (tTestEngTarget)&api_firinterp32x16,0,0,"firinterp32x16_up4x.seq" },
  { 1, &descr_firinterp32x16, (tTestEngTarget)&api_firinterp32x16,0,0,"firinterp32x16_up5x.seq" },

  { 1, &descr_firinterp24x24, (tTestEngTarget)&api_firinterp24x24,1,2,"firinterp24x24_up2x.seq" },
  { 1, &descr_firinterp24x24, (tTestEngTarget)&api_firinterp24x24,1,1,"firinterp24x24_up3x.seq" },
  { 1, &descr_firinterp24x24, (tTestEngTarget)&api_firinterp24x24,1,1,"firinterp24x24_up6x.seq"},
  { 1, &descr_firinterp24x24, (tTestEngTarget)&api_firinterp24x24,0,0,"firinterp24x24_up4x.seq" },
  { 1, &descr_firinterp24x24, (tTestEngTarget)&api_firinterp24x24,0,0,"firinterp24x24_up5x.seq" },

/*
 * Stage 2
 */

  { 2, &descr_bkfirf, (tTestEngTarget)&api_bkfirf, 1,2,"bkfirf_lpf1.seq"},
  { 2, &descr_bkfirf, (tTestEngTarget)&api_bkfirf, 1,2,"bkfirf_bpf1.seq"},
  { 2, &descr_bkfirf, (tTestEngTarget)&api_bkfirf, 1,2,"bkfirf_8taps.seq"},
  { 2, &descr_bkfirf, (tTestEngTarget)&api_bkfirf, 0,0,"bkfirf_lpf2.seq"},
  { 2, &descr_bkfirf, (tTestEngTarget)&api_bkfirf, 0,0,"bkfirf_hpf1.seq"},
  { 2, &descr_bkfirf, (tTestEngTarget)&api_bkfirf, 0,0,"bkfirf_bpf2.seq"},
  { 2, &descr_bkfirf, (tTestEngTarget)&api_bkfirf, 0,0,"bkfirf_4taps.seq"},
 
  { 2, &descr_bkfiraf, (tTestEngTarget)&api_bkfiraf, 1,2,"bkfiraf_lpf1.seq"},
  { 2, &descr_bkfiraf, (tTestEngTarget)&api_bkfiraf, 1,2,"bkfiraf_bpf1.seq"},
  { 2, &descr_bkfiraf, (tTestEngTarget)&api_bkfiraf, 1,2,"bkfiraf_3taps.seq"},
  { 2, &descr_bkfiraf, (tTestEngTarget)&api_bkfiraf, 0,0,"bkfiraf_lpf2.seq"},
  { 2, &descr_bkfiraf, (tTestEngTarget)&api_bkfiraf, 0,0,"bkfiraf_hpf1.seq"},
  { 2, &descr_bkfiraf, (tTestEngTarget)&api_bkfiraf, 0,0,"bkfiraf_bpf2.seq"},
  { 2, &descr_bkfiraf, (tTestEngTarget)&api_bkfiraf, 0,0,"bkfiraf_4taps.seq"},
  { 2, &descr_bkfiraf, (tTestEngTarget)&api_bkfiraf, 0,0,"bkfiraf_2taps.seq"},
  { 2, &descr_bkfiraf, (tTestEngTarget)&api_bkfiraf, 0,0,"bkfiraf_1tap.seq"},
 
  { 2, &descr_cxfirf, (tTestEngTarget)&api_cxfirf,1,2,"cxfirf_lpf1.seq"},
  { 2, &descr_cxfirf, (tTestEngTarget)&api_cxfirf,1,1,"cxfirf_bpf1.seq"},
  { 2, &descr_cxfirf, (tTestEngTarget)&api_cxfirf,1,1,"cxfirf_lpf2.seq"},
  { 2, &descr_cxfirf, (tTestEngTarget)&api_cxfirf,0,0,"cxfirf_hpf1.seq"},
  { 2, &descr_cxfirf, (tTestEngTarget)&api_cxfirf,0,0,"cxfirf_bpf2.seq"},
  { 2, &descr_cxfirf, (tTestEngTarget)&api_cxfirf,0,0,"cxfirf_4taps.seq"},
 
  { 2, &descr_firdecf, (tTestEngTarget)&api_firdecf,1,2,"firdecf_dn2x.seq"},
  { 2, &descr_firdecf, (tTestEngTarget)&api_firdecf,1,2,"firdecf_dn3x.seq"},
  { 2, &descr_firdecf, (tTestEngTarget)&api_firdecf,1,2,"firdecf_dn11x.seq"},
  { 2, &descr_firdecf, (tTestEngTarget)&api_firdecf,1,2,"firdecf_dn6x.seq"},
  { 2, &descr_firdecf, (tTestEngTarget)&api_firdecf,0,0,"firdecf_dn4x.seq"},
  { 2, &descr_firdecf, (tTestEngTarget)&api_firdecf,0,0,"firdecf_dn5x.seq"},
  { 2, &descr_firdecf, (tTestEngTarget)&api_firdecf,0,0,"firdecf_dn23x.seq"},
 
  { 2, &descr_firinterpf, (tTestEngTarget)&api_firinterpf,1,2,"firinterpf_up2x.seq"},
  { 2, &descr_firinterpf, (tTestEngTarget)&api_firinterpf,1,2,"firinterpf_up3x.seq"},
  { 2, &descr_firinterpf, (tTestEngTarget)&api_firinterpf,1,2,"firinterpf_up6x.seq"},
  { 2, &descr_firinterpf, (tTestEngTarget)&api_firinterpf,0,0,"firinterpf_up4x.seq"},
  { 2, &descr_firinterpf, (tTestEngTarget)&api_firinterpf,0,0,"firinterpf_up5x.seq"},

  /* Phase 3 */

  { 3, &descr_bkfir16x16, (tTestEngTarget)&api_bkfir16x16, 1,2,"bkfir16x16_lpf1.seq" },
  { 3, &descr_bkfir16x16, (tTestEngTarget)&api_bkfir16x16, 1,2,"bkfir16x16_bpf1.seq" },
  { 3, &descr_bkfir16x16, (tTestEngTarget)&api_bkfir16x16, 1,2,"bkfir16x16_4taps.seq"},
  { 3, &descr_bkfir16x16, (tTestEngTarget)&api_bkfir16x16, 0,0,"bkfir16x16_lpf2.seq" },
  { 3, &descr_bkfir16x16, (tTestEngTarget)&api_bkfir16x16, 0,0,"bkfir16x16_hpf1.seq" },
  { 3, &descr_bkfir16x16, (tTestEngTarget)&api_bkfir16x16, 0,0,"bkfir16x16_bpf2.seq" },

  { 3, &descr_bkfira16x16, (tTestEngTarget)&api_bkfira16x16, 1,2,"bkfira16x16_lpf1.seq"  },
  { 3, &descr_bkfira16x16, (tTestEngTarget)&api_bkfira16x16, 1,2,"bkfira16x16_bpf1.seq"  },
  { 3, &descr_bkfira16x16, (tTestEngTarget)&api_bkfira16x16, 1,2,"bkfira16x16_3taps.seq" },
  { 3, &descr_bkfira16x16, (tTestEngTarget)&api_bkfira16x16, 0,0,"bkfira16x16_lpf2.seq"  },
  { 3, &descr_bkfira16x16, (tTestEngTarget)&api_bkfira16x16, 0,0,"bkfira16x16_hpf1.seq"  },
  { 3, &descr_bkfira16x16, (tTestEngTarget)&api_bkfira16x16, 0,0,"bkfira16x16_bpf2.seq"  },
  { 3, &descr_bkfira16x16, (tTestEngTarget)&api_bkfira16x16, 0,0,"bkfira16x16_4taps.seq" },
  { 3, &descr_bkfira16x16, (tTestEngTarget)&api_bkfira16x16, 0,0,"bkfira16x16_2taps.seq" },
  { 3, &descr_bkfira16x16, (tTestEngTarget)&api_bkfira16x16, 0,0,"bkfira16x16_1tap.seq"  },

  { 3, &descr_cxfir16x16, (tTestEngTarget)&api_cxfir16x16, 1,2,"cxfir16x16_lpf1.seq" },
  { 3, &descr_cxfir16x16, (tTestEngTarget)&api_cxfir16x16, 1,1,"cxfir16x16_bpf1.seq" },
  { 3, &descr_cxfir16x16, (tTestEngTarget)&api_cxfir16x16, 1,1,"cxfir16x16_4taps.seq"},
  { 3, &descr_cxfir16x16, (tTestEngTarget)&api_cxfir16x16, 0,0,"cxfir16x16_lpf2.seq" },
  { 3, &descr_cxfir16x16, (tTestEngTarget)&api_cxfir16x16, 0,0,"cxfir16x16_hpf1.seq" },
  { 3, &descr_cxfir16x16, (tTestEngTarget)&api_cxfir16x16, 0,0,"cxfir16x16_bpf2.seq" },

  { 3, &descr_firdec16x16, (tTestEngTarget)&api_firdec16x16,1,2,"firdec16x16_dn2x.seq" },
  { 3, &descr_firdec16x16, (tTestEngTarget)&api_firdec16x16,1,1,"firdec16x16_dn3x.seq" },
  { 3, &descr_firdec16x16, (tTestEngTarget)&api_firdec16x16,1,1,"firdec16x16_dn11x.seq"},
  { 3, &descr_firdec16x16, (tTestEngTarget)&api_firdec16x16,1,1,"firdec16x16_dn6x.seq" },
  { 3, &descr_firdec16x16, (tTestEngTarget)&api_firdec16x16,0,0,"firdec16x16_dn4x.seq" },
  { 3, &descr_firdec16x16, (tTestEngTarget)&api_firdec16x16,0,0,"firdec16x16_dn5x.seq" },
  { 3, &descr_firdec16x16, (tTestEngTarget)&api_firdec16x16,0,0,"firdec16x16_dn23x.seq"},

  { 3, &descr_firinterp16x16, (tTestEngTarget)&api_firinterp16x16,1,2,"firinterp16x16_up2x.seq" },
  { 3, &descr_firinterp16x16, (tTestEngTarget)&api_firinterp16x16,1,2,"firinterp16x16_up3x.seq" },
  { 3, &descr_firinterp16x16, (tTestEngTarget)&api_firinterp16x16,1,2,"firinterp16x16_up6x.seq" },
  { 3, &descr_firinterp16x16, (tTestEngTarget)&api_firinterp16x16,0,0,"firinterp16x16_up4x.seq" },
  { 3, &descr_firinterp16x16, (tTestEngTarget)&api_firinterp16x16,0,0,"firinterp16x16_up5x.seq" },

  { 3, &descr_cxfirinterp16x16, (tTestEngTarget)&api_cxfirinterp16x16,0,1,"cxfirinterp16x16_up2x.seq" },
  { 3, &descr_cxfirinterp16x16, (tTestEngTarget)&api_cxfirinterp16x16,0,1,"cxfirinterp16x16_up3x.seq" },
  { 3, &descr_cxfirinterp16x16, (tTestEngTarget)&api_cxfirinterp16x16,0,1,"cxfirinterp16x16_up6x.seq" },
  { 3, &descr_cxfirinterp16x16, (tTestEngTarget)&api_cxfirinterp16x16,0,0,"cxfirinterp16x16_up4x.seq" },
  { 3, &descr_cxfirinterp16x16, (tTestEngTarget)&api_cxfirinterp16x16,0,0,"cxfirinterp16x16_up5x.seq" },

  { 3, &descr_bkfira32x32, (tTestEngTarget)&api_bkfira32x32, 1,2,"bkfira32x32_lpf1.seq"  },
  { 3, &descr_bkfira32x32, (tTestEngTarget)&api_bkfira32x32, 1,2,"bkfira32x32_bpf1.seq"  },
  { 3, &descr_bkfira32x32, (tTestEngTarget)&api_bkfira32x32, 1,2,"bkfira32x32_3taps.seq" },
  { 3, &descr_bkfira32x32, (tTestEngTarget)&api_bkfira32x32, 0,0,"bkfira32x32_lpf2.seq"  },
  { 3, &descr_bkfira32x32, (tTestEngTarget)&api_bkfira32x32, 0,0,"bkfira32x32_hpf1.seq"  },
  { 3, &descr_bkfira32x32, (tTestEngTarget)&api_bkfira32x32, 0,0,"bkfira32x32_bpf2.seq"  },
  { 3, &descr_bkfira32x32, (tTestEngTarget)&api_bkfira32x32, 0,0,"bkfira32x32_4taps.seq" },
  { 3, &descr_bkfira32x32, (tTestEngTarget)&api_bkfira32x32, 0,0,"bkfira32x32_2taps.seq" },
  { 3, &descr_bkfira32x32, (tTestEngTarget)&api_bkfira32x32, 0,0,"bkfira32x32_1tap.seq"  },

  { 3, &descr_cxfir32x32, (tTestEngTarget)&api_cxfir32x32, 1,2,"cxfir32x32_lpf1.seq" },
  { 3, &descr_cxfir32x32, (tTestEngTarget)&api_cxfir32x32, 1,1,"cxfir32x32_bpf1.seq" },
  { 3, &descr_cxfir32x32, (tTestEngTarget)&api_cxfir32x32, 1,1,"cxfir32x32_4taps.seq"},
  { 3, &descr_cxfir32x32, (tTestEngTarget)&api_cxfir32x32, 0,0,"cxfir32x32_lpf2.seq" },
  { 3, &descr_cxfir32x32, (tTestEngTarget)&api_cxfir32x32, 0,0,"cxfir32x32_hpf1.seq" },
  { 3, &descr_cxfir32x32, (tTestEngTarget)&api_cxfir32x32, 0,0,"cxfir32x32_bpf2.seq" },

  { 3, &descr_firdec32x32, (tTestEngTarget)&api_firdec32x32,1,2,"firdec32x32_dn2x.seq" },
  { 3, &descr_firdec32x32, (tTestEngTarget)&api_firdec32x32,1,1,"firdec32x32_dn3x.seq" },
  { 3, &descr_firdec32x32, (tTestEngTarget)&api_firdec32x32,1,1,"firdec32x32_dn11x.seq"},
  { 3, &descr_firdec32x32, (tTestEngTarget)&api_firdec32x32,1,1,"firdec32x32_dn6x.seq" },
  { 3, &descr_firdec32x32, (tTestEngTarget)&api_firdec32x32,0,0,"firdec32x32_dn4x.seq" },
  { 3, &descr_firdec32x32, (tTestEngTarget)&api_firdec32x32,0,0,"firdec32x32_dn5x.seq" },
  { 3, &descr_firdec32x32, (tTestEngTarget)&api_firdec32x32,0,0,"firdec32x32_dn23x.seq"},

  { 3, &descr_firinterp32x32, (tTestEngTarget)&api_firinterp32x32,1,2,"firinterp32x32_up2x.seq" },
  { 3, &descr_firinterp32x32, (tTestEngTarget)&api_firinterp32x32,1,2,"firinterp32x32_up3x.seq" },
  { 3, &descr_firinterp32x32, (tTestEngTarget)&api_firinterp32x32,1,2,"firinterp32x32_up6x.seq" },
  { 3, &descr_firinterp32x32, (tTestEngTarget)&api_firinterp32x32,0,0,"firinterp32x32_up4x.seq" },
  { 3, &descr_firinterp32x32, (tTestEngTarget)&api_firinterp32x32,0,0,"firinterp32x32_up5x.seq" },
};

#define DO_TEST(fxn, seqFile)  if ( res || !breakOnError ) res &= ( 0 != testExec( (tTestEngTarget)(fxn), (seqFile),isFull, isVerbose, breakOnError ) )
int main_fir_old( void );
/* Perform all tests for FIR API functions. */
int main_fir( int phaseNum, int isFull, int isVerbose, int breakOnError )
{
    int n;
    int res = 1;
    printf( "\nFIR:\n" );
    /* first, test different FIR filters */
#if 1
    for (n=0; n<(int)(sizeof(tests)/sizeof(tests[0])); n++)
    {
        if ( (phaseNum == 0 || phaseNum == tests[n].phaseNum) && tests[n].runAlways )
        {
            res = TestEngRun(tests[n].fxns, tests[n].pFirDescr, tests[n].seqFile, isFull, isVerbose, breakOnError);
            if (res == 0 && breakOnError) break;
        }
        else if ( isFull <= tests[n].testvecFlag)
        {
  		  res = TestEngRun(tests[n].fxns, tests[n].pFirDescr, tests[n].seqFile, isFull, isVerbose, breakOnError);
  		  if (res == 0 && breakOnError) break;
        }
    }
#endif
    /* next, test other FIR related functions */
    if ( phaseNum == 0 || phaseNum == 1 )
    {
        DO_TEST( &fir_acorr24x24     , "fir_acorr24x24.seq"      );
        DO_TEST( &fir_xcorr32x16     , "fir_xcorr32x16.seq"      );
        DO_TEST( &fir_xcorr24x24     , "fir_xcorr24x24.seq"      );
        DO_TEST( &fir_convol32x16    , "fir_convol32x16.seq"     );
        DO_TEST( &fir_convol24x24    , "fir_convol24x24.seq"     );
        DO_TEST( &cxfir_convol32x16  , "cxfir_convol32x16.seq"   );
        DO_TEST( &fir_acorra24x24    , "fir_acorra24x24.seq"     );
        DO_TEST( &fir_xcorra32x16    , "fir_xcorra32x16.seq"     );
        DO_TEST( &fir_xcorra24x24    , "fir_xcorra24x24.seq"     );
        DO_TEST( &fir_convola32x16   , "fir_convola32x16.seq"    );
        DO_TEST( &fir_convola24x24   , "fir_convola24x24.seq"    );
        DO_TEST( &cxfir_convola32x16 , "cxfir_convola32x16.seq"  );
        DO_TEST( &fir_blms24x24      , "fir_blms24x24.seq"       );
        DO_TEST( &fir_blms16x32      , "fir_blms16x32.seq"       );
    }
    if ( phaseNum == 0 || phaseNum == 2)
    {
        DO_TEST( &fir_acorrf       , "fir_acorrf.seq"       );
        DO_TEST( &fir_xcorrf       , "fir_xcorrf.seq"       );
        DO_TEST( &cxfir_xcorrf     , "cxfir_xcorrf.seq"     );
        DO_TEST( &fir_convolf      , "fir_convolf.seq"      );
        DO_TEST( &fir_acorraf      , "fir_acorraf.seq"      );
        DO_TEST( &fir_xcorraf      , "fir_xcorraf.seq"      );
        DO_TEST( &cxfir_xcorraf    , "cxfir_xcorraf.seq"    );
        DO_TEST( &fir_convolaf     , "fir_convolaf.seq"     );
        DO_TEST( &fir_blmsf        , "fir_blmsf.seq"        );
    }
    if ( phaseNum == 0 || phaseNum ==3 )
    {

        DO_TEST( &fir_acorr16x16     , "fir_acorr16x16.seq"      );
        DO_TEST( &fir_xcorr16x16     , "fir_xcorr16x16.seq"      );
        DO_TEST( &fir_convol16x16    , "fir_convol16x16.seq"     );
        DO_TEST( &fir_acorra16x16    , "fir_acorra16x16.seq"     );
        DO_TEST( &fir_xcorra16x16    , "fir_xcorra16x16.seq"     );
        DO_TEST( &fir_convola16x16   , "fir_convola16x16.seq"    );
        DO_TEST( &fir_lacorra16x16   , "fir_lacorra16x16.seq"    );
        DO_TEST( &fir_lxcorra16x16   , "fir_lxcorra16x16.seq"    );
        DO_TEST( &fir_lconvola16x16  , "fir_lconvola16x16.seq"   );

        DO_TEST( &fir_lxcorra32x16   , "fir_lxcorra32x16.seq"    );
        DO_TEST( &fir_lconvola32x16  , "fir_lconvola32x16.seq"   );

        DO_TEST( &fir_acorr32x32     , "fir_acorr32x32.seq"      );
        DO_TEST( &fir_xcorr32x32     , "fir_xcorr32x32.seq"      );
        DO_TEST( &fir_convol32x32    , "fir_convol32x32.seq"     );
        DO_TEST( &fir_acorra32x32    , "fir_acorra32x32.seq"     );
        DO_TEST( &fir_xcorra32x32    , "fir_xcorra32x32.seq"     );
        DO_TEST( &fir_convola32x32   , "fir_convola32x32.seq"    );
        DO_TEST( &fir_lacorra32x32   , "fir_lacorra32x32.seq"    );
        DO_TEST( &fir_lxcorra32x32   , "fir_lxcorra32x32.seq"    );
        DO_TEST( &fir_lconvola32x32  , "fir_lconvola32x32.seq"   );

        DO_TEST( &fir_lacorraf       , "fir_lacorraf.seq"        );
        DO_TEST( &fir_lxcorraf       , "fir_lxcorraf.seq"        );
        DO_TEST( &fir_lconvolaf      , "fir_lconvolaf.seq"       );

        DO_TEST( &fir_blms16x16      , "fir_blms16x16.seq"       );
        DO_TEST( &fir_blms32x32      , "fir_blms32x32.seq"       );
    }

    return (res);
} /* main_fir() */

/* Test executive function. Performs the specified test on a brief or full version
 * of the designated SEQ-file. Return the test result (non-zero if passed). */
static int testExec( tTestEngTarget   targetFxn, const char * seqName,int isFull, int isVerbose, int breakOnError )
{
  #define MAX_FUNC_NUM  4
  /* Initializer for a function pointer array, appends NULL to a sequence of pointers. */
  #define FUNC_LIST(...) { __VA_ARGS__, NULL }
  /* Initializer for a test description structure. */
  #define TEST_DESC( fmt, extraParam, argNum, align, loadFxn, procFxn ) { (fmt),extraParam,(argNum),(align),NULL,NULL,(loadFxn),(procFxn) }

  /* vec API test definitions. */
  static const struct 
  {
    tTestEngTarget   funcList[MAX_FUNC_NUM];
    tTestEngDesc     testDesc;
  }
  testDefTbl[] =
  {
      /* Stage 1 */
    {  FUNC_LIST( (tTestEngTarget)&fir_acorra24x24),    TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | 0            , TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_autocorr_24, &te_processFxn_autocorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_xcorra24x24),    TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | CROSSCORR_API, TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr_24, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_convola24x24),   TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | CONVOLVE_API , TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr_24, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_acorr24x24),     TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | 0            , TE_ARGNUM_1, TE_ALIGN_YES, &te_loadFxn_autocorr_24, &te_processFxn_autocorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_xcorr24x24),     TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | CROSSCORR_API, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr_24, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_convol24x24),    TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | CONVOLVE_API , TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr_24, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_xcorr32x16),     TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT32 | CROSSCORR_API, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_convol32x16),    TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT32 | CONVOLVE_API , TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_xcorra32x16),    TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT32 | CROSSCORR_API, TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_convola32x16),   TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT32 | CONVOLVE_API , TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&cxfir_convol32x16),  TEST_DESC( FMT_CPLX|FMT_FRACT16, FMT_CPLX|FMT_FRACT32 | CONVOLVE_API , TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&cxfir_convola32x16), TEST_DESC( FMT_CPLX|FMT_FRACT16, FMT_CPLX|FMT_FRACT32 | CONVOLVE_API , TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_blms16x32),      {          FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_create_lms,&te_destroy_lms, &te_loadFxn_lms, &te_processFxn_lms } },
    {  FUNC_LIST( (tTestEngTarget)&fir_blms24x24),      {          FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_create_lms,&te_destroy_lms, &te_loadFxn_lms_24, &te_processFxn_lms } },

      /* Stage 2 */
    {  FUNC_LIST( (tTestEngTarget)&fir_acorrf),         TEST_DESC( FMT_REAL|FMT_FLOAT32, FMT_REAL|FMT_FLOAT32 | 0            , TE_ARGNUM_1, TE_ALIGN_YES, &te_loadFxn_autocorr, &te_processFxn_autocorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_xcorrf),         TEST_DESC( FMT_REAL|FMT_FLOAT32, FMT_REAL|FMT_FLOAT32 | CROSSCORR_API, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_convolf),        TEST_DESC( FMT_REAL|FMT_FLOAT32, FMT_REAL|FMT_FLOAT32 | CONVOLVE_API,  TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_acorraf),        TEST_DESC( FMT_REAL|FMT_FLOAT32, FMT_REAL|FMT_FLOAT32 | 0           ,  TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_autocorr, &te_processFxn_autocorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_xcorraf),        TEST_DESC( FMT_REAL|FMT_FLOAT32, FMT_REAL|FMT_FLOAT32 | CROSSCORR_API, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_convolaf),       TEST_DESC( FMT_REAL|FMT_FLOAT32, FMT_REAL|FMT_FLOAT32 | CONVOLVE_API , TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&cxfir_xcorrf),       TEST_DESC( FMT_CPLX|FMT_FLOAT32, FMT_CPLX|FMT_FLOAT32 | CROSSCORR_API, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&cxfir_xcorraf),      TEST_DESC( FMT_CPLX|FMT_FLOAT32, FMT_CPLX|FMT_FLOAT32 | CROSSCORR_API, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    //{  FUNC_LIST( (tTestEngTarget)&fir_blmsf),        { FMT_REAL|FMT_FLOAT32, 0,TE_ARGNUM_2, TE_ALIGN_YES, &te_create_lms,&te_destroy_lms, &te_loadFxn_lms, &te_processFxn_lms } },
    { FUNC_LIST((tTestEngTarget)&fir_blmsf), { FMT_REAL | FMT_FLOAT32, FMT_REAL | FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_create_lms, &te_destroy_lms, &te_loadFxn_lms, &te_processFxn_lms } },

      /* Stage 3 */
    {  FUNC_LIST( (tTestEngTarget)&fir_acorra16x16),    TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT16 | 0            ,TE_ARGNUM_1, TE_ALIGN_NO , &te_loadFxn_autocorr, &te_processFxn_autocorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_xcorra16x16),    TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT16 | CROSSCORR_API,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_convola16x16),   TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT16 | CONVOLVE_API ,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_acorr16x16),     TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT16 | 0            ,TE_ARGNUM_1, TE_ALIGN_YES, &te_loadFxn_autocorr, &te_processFxn_autocorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_xcorr16x16),     TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT16 | CROSSCORR_API,TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_convol16x16),    TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT16 | CONVOLVE_API ,TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_acorra32x32),    TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | 0            ,TE_ARGNUM_1, TE_ALIGN_NO , &te_loadFxn_autocorr, &te_processFxn_autocorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_xcorra32x32),    TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | CROSSCORR_API,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_convola32x32),   TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | CONVOLVE_API ,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_acorr32x32),     TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | 0            ,TE_ARGNUM_1, TE_ALIGN_YES, &te_loadFxn_autocorr, &te_processFxn_autocorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_xcorr32x32),     TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | CROSSCORR_API,TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_convol32x32),    TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | CONVOLVE_API ,TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },

    {  FUNC_LIST( (tTestEngTarget)&fir_lconvola16x16),  TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT16 |LCONVOLVE_API ,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_lconvola32x16),  TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT32 |LCONVOLVE_API ,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_lconvola32x32),  TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 |LCONVOLVE_API ,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_lconvolaf    ),  TEST_DESC( FMT_REAL|FMT_FLOAT32, FMT_REAL|FMT_FLOAT32 |LCONVOLVE_API ,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },

    {  FUNC_LIST( (tTestEngTarget)&fir_lxcorra16x16),   TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT16 |LCROSSCORR_API,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_lxcorra32x16),   TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT32 |LCROSSCORR_API,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_lxcorra32x32),   TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 |LCROSSCORR_API,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_lxcorraf    ),   TEST_DESC( FMT_REAL|FMT_FLOAT32, FMT_REAL|FMT_FLOAT32 |LCROSSCORR_API,TE_ARGNUM_2, TE_ALIGN_NO , &te_loadFxn_crosscorr, &te_processFxn_crosscorr ) },

    {  FUNC_LIST( (tTestEngTarget)&fir_lacorra16x16),   TEST_DESC( FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT16 | 0            ,TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_autocorr, &te_processFxn_autocorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_lacorra32x32),   TEST_DESC( FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32 | 0            ,TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_autocorr, &te_processFxn_autocorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_lacorraf    ),   TEST_DESC( FMT_REAL|FMT_FLOAT32, FMT_REAL|FMT_FLOAT32 | 0            ,TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_autocorr, &te_processFxn_autocorr ) },
    {  FUNC_LIST( (tTestEngTarget)&fir_blms16x16),      {          FMT_REAL|FMT_FRACT16, FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_YES, &te_create_lms,&te_destroy_lms, &te_loadFxn_lms, &te_processFxn_lms } },
    {  FUNC_LIST( (tTestEngTarget)&fir_blms32x32),      {          FMT_REAL|FMT_FRACT32, FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_create_lms,&te_destroy_lms, &te_loadFxn_lms, &te_processFxn_lms } },

    {  FUNC_LIST( (tTestEngTarget)NULL ),              TEST_DESC(  0, 0, 0, 0, NULL, NULL ) } /* End of table */
  };

  int defIx = 0, funcIx = 0;

  /*
   * Search through the test definitions table. 
   */

  while ( defIx<(int)(sizeof(testDefTbl)/sizeof(testDefTbl[0])))
  {
    if ( targetFxn == testDefTbl[defIx].funcList[funcIx] ) 
    {
        return ( TestEngRun( testDefTbl[defIx].funcList[funcIx], 
                            &testDefTbl[defIx].testDesc,
                            seqName, isFull, isVerbose, breakOnError ) );
    }
    if (testDefTbl[defIx].funcList[++funcIx]==NULL)
    {
      defIx++; funcIx = 0;
    }
  }
    ASSERT( !"Test not defined" );
    return (0);
//  }

} /* testExec() */
