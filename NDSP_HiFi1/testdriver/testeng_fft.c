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
/* Test engine extension for FFT. */
#include "testeng_fft.h"
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
typedef struct tagTestEngContext_fft_int
{
  tTestEngContext_fft ext; /* Externally visible part of FFT context. */

  struct {                 /* Twiddle factor tables.                  */
    int    baseSize;       /* First twiddle table FFT size.           */
    int    tblNum;         /* Number of twiddle tables.               */
    tVec * tblSet;         /* Twiddle tables, with FFT size doubled   */
  }                        /* in succession.                          */
  twd;

} tTestEngContext_fft_int;


/* unpack 24-bit stream to 32-bit data */
static void unpack24(uint8_t *x,int N)
{
    int n;
    ASSERT(N%4==0);
    for (n=N/4-1; n>=0; n--)
    {
        uint8_t b0,b1,b2,b3;
        b0=0;
        b1=x[3*n+0];
        b2=x[3*n+1];
        b3=x[3*n+2];
        x[4*n+0]=b0;
        x[4*n+1]=b1;
        x[4*n+2]=b2;
        x[4*n+3]=b3;
    }
}

/* pack 32-bit data to 24-bit stream */
static void pack24(uint8_t *x,int N)
{
    int n;
    ASSERT(N%4==0);
    for (n=0; n<N/4; n++)
    {
        uint8_t b0,b1,b2;
        b0=x[4*n+1];
        b1=x[4*n+2];
        b2=x[4*n+3];
        x[3*n+0]=b0;
        x[3*n+1]=b1;
        x[3*n+2]=b2;
    }
}

/* Create a target algorithm instance and set tTestEngContext::target fields.
 * Return zero if failed. */
int te_create_fft( tTestEngContext * context )
{
  tTestEngContext_fft_int * context_fft;

  tVec * tblSet = 0;
  int baseSize, tblNum, res;

  /*
   * Allocate and initialize the context structure.
   */

  context_fft = (tTestEngContext_fft_int * )malloc( sizeof(*context_fft) );

  if ( !( res = ( 0 != context_fft ) ) )
  {
    printf( "te_create_fft(): malloc() failed\n" );
  }
  else
  {
    memset( context_fft, 0, sizeof(*context_fft) );
  }

  /*
   * Load twiddle factor tables.
   */

  if ( res )
  {
    int tblIx;
    tVec tbl_fp64c;

    memset( &tbl_fp64c, 0, sizeof(tbl_fp64c) );

    res = 0;

    if ( 2 != seqFileScanf( context->seqFile, "%d %d", &baseSize, &tblNum ) )
    {
      printf( "te_create_fft(): bad SEQ-file format (1)\n" );
    }
    /* Allocate 64-bit vector of max FFT size. */
    else if ( !vecAlloc( &tbl_fp64c, baseSize<<(tblNum-1), 
                         TE_ALIGN_YES, FMT_FLOAT64|FMT_CPLX, 0 ) )
    {
      printf( "te_create_fft(): failed to allocate tbl_fp64c\n" );
    }
    /* Allocate vectors for all tables. */
    else if ( !( tblSet = (tVec*)calloc( tblNum, sizeof(tVec) ) ) )
    {
      printf( "te_create_fft(): failed to allocate the tables set\n" );
    }
    else res = 1;

    /* Read and convert twiddle tables. */
    for ( tblIx=0; tblIx<tblNum && res; tblIx++ )
    {
      int tblSize = ( baseSize << tblIx )*3/4;
      char fnameBuf[256];
      char * fname;
      FILE * f = 0;
      int fmtTwd;
      fmtTwd = context->desc->fmt & 0x7;
      if(context->desc->extraParam & TE_FFT_TWD16) fmtTwd = FMT_FRACT16;    /* load 16-bit twiddles instead of the same as input/output */

      NASSERT( strlen( context->seqDir ) + 64 < sizeof(fnameBuf) );
      strcpy( fnameBuf, context->seqDir );
      fname = fnameBuf + strlen( fnameBuf );

      res = 0;
      /* Read the twiddle factor table filename. */
      if ( !seqFileScanf( context->seqFile, "%63s", fname ) )
      {
        printf( "te_create_fft(): bad SEQ-file format (2), tblIx=%d\n", tblIx );
      }
      /* Open the table file. */
      else if ( !( f = fopen( fnameBuf, "rb" ) ) )
      {
        printf( "te_create_fft(): failed to open %s for reading\n", fnameBuf );
      }
      /* Read twiddle factors in 64-bit FP format. */
      else if ( tblSize != (int)fread( vecGetElem( &tbl_fp64c, 0 ), sz_fp64c, tblSize, f ) )
      {
        printf( "te_create_fft(): failed to read %s\n", fname );
      }
      /* Allocate the twiddle table vector. */
      else if ( !vecAlloc( &tblSet[tblIx], tblSize, 
                           0,//context->desc->isAligned, 
                           fmtTwd | FMT_CPLX, 0 ) )
      {
        printf( "te_create_fft(): failed to allocate twiddle table, tblIx=%d", tblIx );
      }
      else
      {
        res = 1;
        /* Convert twiddle factors to the target FFT format. */
        vecFromFp64( &tblSet[tblIx], (float64_t*)vecGetElem( &tbl_fp64c, 0 ) );
      }

      if ( f ) fclose( f );
    }

    if ( !res && tblSet )
    {
      for ( tblIx=0; tblIx<tblNum; tblIx++ )
      {
        if ( tblSet[tblIx].szBulk > 0 ) vecFree( &tblSet[tblIx] );
      }

      free( tblSet );
    }

    if ( tbl_fp64c.szBulk > 0 ) vecFree( &tbl_fp64c );
  }

  if ( res )
  {
    context_fft->twd.baseSize = baseSize;
    context_fft->twd.tblNum   = tblNum;
    context_fft->twd.tblSet   = tblSet;

    context->target.handle = &context_fft->ext;
  }

  {
    typedef void (*tFxn)( const void * x, void * y, const void * twdTbl, int twdStep, int N );
    tFxn fxn_fwd,fxn_inv ;
    fxn_fwd = (tFxn)((const tTestEngDesc_fft *)context->desc)->frwTransFxn;
    fxn_inv = (tFxn)((const tTestEngDesc_fft *)context->desc)->invTransFxn;
    if(!NatureDSP_Signal_isPresent(fxn_fwd) &&
       !NatureDSP_Signal_isPresent(fxn_inv))
    {
        // FUT is not defined
        return -1;
    }
  }
  return (res);

} /* te_create_fft() */

/* Destroy the target algorithm instance and free memory block(s) allocated
 * for the target object. Return zero whenever any violation of memory area
 * bounds is detected. */
int te_destroy_fft( tTestEngContext * context )
{
  tTestEngContext_fft_int * context_fft;

  ASSERT( context );

  if ( 0 != ( context_fft = (tTestEngContext_fft_int *)context->target.handle ) )
  {
    int tblIx;
    tVec * tblSet;

    if ( 0 != ( tblSet = context_fft->twd.tblSet ) )
    {
      for ( tblIx=0; tblIx<context_fft->twd.tblNum; tblIx++ )
      {
        if ( tblSet[tblIx].szBulk > 0 ) vecFree( &tblSet[tblIx] );
      }

      free( tblSet );
    }

    free( context_fft );
  }

  return (1);

} /* te_destroy_fft() */

/* Allocate in/out vectors for the next test case, and load the data set
 * from the SEQ-file. Return zero if failed. */
int te_load_fft( tTestEngContext * context )
{
  tTestEngContext_fft_int * context_fft = (tTestEngContext_fft_int *)context->target.handle;
  tVec BEXP, Z, Zlo, Zhi;
  int res = 0;
  int isFract = ( ( FMT_FRACT16 == ( context->desc->fmt & 15 ) ) ||
                  ( FMT_FRACT32 == ( context->desc->fmt & 15 ) ) );

  NASSERT( context_fft );

  memset( &context_fft->ext, 0, sizeof(context_fft->ext) );

  memset( &BEXP, 0, sizeof(BEXP) );
  memset( &Z   , 0, sizeof(Z   ) );
  memset( &Zlo , 0, sizeof(Zlo ) );
  memset( &Zhi , 0, sizeof(Zhi ) );

  /* If FFT supports the scaling option, read the scaling method from the SEQ-file. */
  if ( ( context->desc->extraParam & TE_FFT_OPT_SCALE_METH ) &&
       ( 1 != seqFileScanf( context->seqFile, "%d", &context_fft->ext.scale_method ) ) )
  {
    printf( "te_load_fft(): bad SEQ-file format (a)\n" );
  }
  /* For a fixed point blockwise FFT, allocate a vector for temporal storage of block exponent. */
  else if ( 0 != ( context->desc->extraParam & TE_FFT_BLOCKWISE ) && isFract &&
            !vecAlloc( &BEXP, context->args.L, TE_ALIGN_NO, FMT_INT16, 0 ) )
  {
    printf( "te_load_fft(): failed to allocate BEXP, L=%d\n", context->args.L );
  }
  /* Read input data filename. */
  else if ( 1 != seqFileScanf( context->seqFile, "%63s", &context_fft->ext.fInName ) )
  {
    printf( "te_load_fft(): bad SEQ-file format (b)\n" );
  }
  /* Read reference data filename. */
  else if ( 1 != seqFileScanf( context->seqFile, "%63s", &context_fft->ext.fRefName ) )
  {
    printf( "te_load_fft(): bad SEQ-file format (c)\n" );
  }
  /* Allocate vectors for SINAD verification. */
  else if ( 3 != vecsAlloc( TE_ALIGN_NO, FMT_FLOAT32, &Z, 1, &Zlo, 1, &Zhi, 1, 0 ) )
  {
    printf( "te_load_fft(): failed to allocate vectors Z/Zlo/Zhi\n" );
  }
  /* Read the minimum SINAD value from the SEQ-file. */
  else if ( 1 != seqFileScanf( context->seqFile, "%f", vecGetElem_fl32( &Zlo, 0 ) ) )
  {
    printf( "te_load_fft(): bad SEQ-file format (d)\n" );
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

} /* te_load_fft() */

/* Return a pointer to twiddle factor table. If step parameter
 * is non-zero, then the table is selected from the set of available
 * tables in dependence of the test frame counter, with appropriate
 * stride amount returned through step. If step is zero, return the
 * twiddle table such that stride is 1. Return zero if found no table
 * for the requested FFT size. */
void * te_get_twd_tbl( tTestEngContext * context, int fftSize, int * step )
{
  tTestEngContext_fft_int * context_fft;
  int baseSize, tblNum, cnt;
  int tblIx, stride = 0;

  ASSERT( context && context->target.handle );
  ASSERT( !( fftSize & (fftSize-1) ) );

  context_fft = (tTestEngContext_fft_int *)context->target.handle;
  baseSize = context_fft->twd.baseSize;
  tblNum   = context_fft->twd.tblNum;

  if ( step )
  {
    tblIx = S_exp0_l( baseSize ) - S_exp0_l( fftSize );

    if ( tblIx < tblNum )
    {
      /* Select the "randomizer" among the frame counter (plain FFT test) or
       * test case number (2D FFT test). */
      cnt = ( context_fft->ext.frameCnt > 0 ? context_fft->ext.frameCnt : context->args.caseNum );
      /* Choose a table for size >= fftSize */
      tblIx += ( stride = ( cnt % ( tblNum - tblIx ) ) );
    }
  }
  else
  {
    tblIx = S_exp0_l( baseSize ) - S_exp0_l( fftSize );
  }

  if ( 0 <= tblIx && tblIx < tblNum )
  {
    if ( step ) *step = ( 1 << stride );

    return ( vecGetElem( &context_fft->twd.tblSet[tblIx], 0 ) );
  }

  return (0);

} /* te_get_twd_tbl() */

/* Apply the FFT function to a single frame of test data, any FFT routine excepting feature
 * rich fixed point FFTs and blockwise FFTs. */
int te_frameProc_stnd_fft( tTestEngContext * context, 
                     const fract16         * in,
                           float64_t       * out,
                     tVec * xVec, tVec * yVec )
{
  typedef void (*tFxn)( const void * x, void * y, const void * twdTbl, int twdStep, int N );

  tFxn fxn = NULL;
  tTestEngContext_fft * context_fft;
  void *px, *py, *twdTbl;

  int bexp, shift;
  int N, logN, twdStep;
  int noReuse, doInplace, isRealForward;

  uint32_t crcSum = 0;

  NASSERT( context && context->desc && context->target.handle );
  NASSERT( in && out && xVec && yVec );
  NASSERT( 0 == ( context->args.N & ( context->args.N - 1 ) ) );
  NASSERT( 0 == ( context->desc->extraParam & TE_FFT_OPT_SCALE_METH ) );
  
  context_fft = (tTestEngContext_fft *)context->target.handle;
  N           = context->args.N;
  logN        = 30 - S_exp0_l( N );

  /* If the FFT routine supports inplace operation, try it every second frame. */
  doInplace = ( context->desc->extraParam & TE_FFT_OPT_INPLACE ) && ( context_fft->frameCnt & 1 );
  /* Also check if the target FFT is allowed to reuse the input buffer for intermediate data. */
  noReuse = !( context->desc->extraParam & TE_FFT_OPT_REUSE_INBUF ) && !doInplace;
  /* Real-valued forward FFT requires special block exponent on input. */
  isRealForward = ( context->desc->extraParam & TE_FFT_REAL ) &&
                  ( context->args.caseType == TE_FFT_TESTCASE_FORWARD );

  /* For all fixed point FFTs, block exponent of input data should be at least 1, but for
   * real-valued forward FFT zero is allowed. */
  bexp = ( isRealForward ? 0 : 1 );

  /* Convert 16-bit PCM input data to target FFT format. */
  shift = vecFromPcm16( xVec, (fract16*)in, bexp );

  /* Select in/out buffers for the FFT, and wipe the output buffer. */
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

  /* Select the target FFT routine (either forward or inverse). */
  if ( context->args.caseType == TE_FFT_TESTCASE_FORWARD )
  {
    fxn = (tFxn)((const tTestEngDesc_fft *)context->desc)->frwTransFxn;
    /* Compensate for scaling shift performed by fixed point FFTs. */
    shift -= logN;
  }
  else if ( context->args.caseType == TE_FFT_TESTCASE_INVERSE )
  {
    fxn = (tFxn)((const tTestEngDesc_fft *)context->desc)->invTransFxn;
    /* For fixed point inverse FFT, we have to divide output signal by FFT size.
     * Just don't compensate for the scaling shift performed by the FFT routine. */
  }
  else
  {
    NASSERT( !"Bad test case type!" );
  }

  /* If reuse is not allowed, make sure the buffer stays intact. */
  if ( noReuse ) crcSum = crc32( 0, (uint8_t*)px, vecGetSize( xVec ) );

  /* Select a twiddle factor table for FFT size >= N. */
  if ( !( twdTbl = te_get_twd_tbl( context, N, &twdStep ) ) )
  {
    printf( "te_frameProc_stnd_fft(): no twiddle factor table for N=%d\n", N );
    return (0);
  }

  /* Apply the target FFT routine. */
  fxn(py, px,  twdTbl, twdStep, N );

  if ( doInplace && vecGetSize( xVec ) >= vecGetSize( yVec ) )
  {
    memcpy( vecGetElem( yVec, 0 ), py, vecGetSize( yVec ) );
  }
  
  if ( noReuse && crcSum != crc32( 0, (uint8_t*)px, vecGetSize( xVec ) ) )
  {
    printf( "te_frameProc_stnd_fft(): target FFT has corrupted the input buffer\n" );
    return ( 0 );
  }

  /* Convert output data to complex 64-bit floating point and rescale them. */
  vecToFp64( (float64_t*)out, yVec, shift );

  return (1);

} /* te_frameProc_stnd_fft() */

/* Apply the FFT function to a single frame of test data, feature rich fixed point
 * FFT (with scaling method option). */
int te_frameProc_stnd_scl_fft( tTestEngContext * context, 
                         const fract16         * in,
                               float64_t       * out,
                         tVec * xVec, tVec * yVec )
{
  typedef int (*tFxn)(const void * x, void * y, .../* const void * twdTbl, int twdStep, int N, int scalingOpt*/);

  tFxn fxn = NULL;
  tTestEngContext_fft * context_fft;
  void *px, *py, *twdTbl;

  int bexp=0, shift;
  int N, logN, twdStep;
  int noReuse, doInplace, isRealForward;

  uint32_t crcSum = 0;

  NASSERT( context && context->desc && context->target.handle );
  NASSERT( in && out && xVec && yVec );
  NASSERT( 0 == ( context->args.N & ( context->args.N - 1 ) ) );
  NASSERT( 0 != ( context->desc->extraParam & TE_FFT_OPT_SCALE_METH ) );
  
  context_fft = (tTestEngContext_fft *)context->target.handle;
  N           = context->args.N;
  logN        = 30 - S_exp0_l( N );

  /* If the FFT routine supports inplace operation, try it every second frame. */
  doInplace = ( context->desc->extraParam & TE_FFT_OPT_INPLACE ) && ( context_fft->frameCnt & 1 );
  /* Also check if the target FFT is allowed to reuse the input buffer for intermediate data. */
  noReuse = !( context->desc->extraParam & TE_FFT_OPT_REUSE_INBUF ) && !doInplace;
  /* Real-valued forward FFT requires special block exponent on input. */
  isRealForward = ( context->desc->extraParam & TE_FFT_REAL ) &&
                  ( context->args.caseType == TE_FFT_TESTCASE_FORWARD );

  /* Select the target FFT routine. */
  if ( context->args.caseType == TE_FFT_TESTCASE_FORWARD )
  {
      fxn = (tFxn)((const tTestEngDesc_fft *)context->desc)->frwTransFxn;
  }
  else if ( context->args.caseType == TE_FFT_TESTCASE_INVERSE )
  {
      fxn = (tFxn)((const tTestEngDesc_fft *)context->desc)->invTransFxn;
  }
  else
  {
    NASSERT( !"Bad test case type!" );
  }

  /* Select the block exponent for fixed point FFT input data. Scale method 0 is
   * "no scaling", 3 - "static scaling". */
  if ( isRealForward )
  {
    bexp = ( context_fft->scale_method == 0 ? logN : 0 );
  }
  else
  {
    bexp = ( context_fft->scale_method == 0 ? logN+1 : 0 );
  }

  /* Convert 16-bit PCM input data to target FFT format. */
  shift = vecFromPcm16( xVec, (fract16*)in, bexp );

  /* Select in/out buffers for the FFT, and wipe the output buffer. */
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

  /* Select a twiddle factor table for FFT size >= N. */
  if ( !( twdTbl = te_get_twd_tbl( context, N, &twdStep ) ) )
  {
    printf( "te_frameProc_stnd_scl_fft(): no twiddle factor table for N=%d\n", N );
    return (0);
  }

  /* Apply the target FFT routine. */
  if (context->desc->extraParam & TE_FFT_PACKED24)
  {
      /* reallocate xVec, yVec to fit largest size */
      int res;
      tVec XVec,YVec;
      void* pX,*pY;
      size_t szX=vecGetSize( xVec ),szY=vecGetSize( yVec );
      size_t maxsz=MAX(szX,szY);
      res =vecAlloc(&XVec, maxsz/xVec->szElem, ((uintptr_t)px)%8==0, xVec->fmt, NULL);
      res&=vecAlloc(&YVec, maxsz/yVec->szElem, ((uintptr_t)py)%8==0, yVec->fmt, NULL);
      if(!res)
      {
        printf( "te_frameProc_stnd_scl_fft(): does not able to allocate memory\n");
        return (0);
      }
      pX=vecGetElem( &XVec, 0 );
      pY=vecGetElem( &YVec, 0 );
      memset(pX,0xcc,maxsz);
      memset(pY,0xcc,maxsz);
      memcpy( pX, px, szX );
      pack24((uint8_t*)pX, szX);
      bexp=fxn( pY, pX, twdTbl, twdStep, N, context_fft->scale_method );
      unpack24((uint8_t*)pY, szY);
      memcpy( py, pY, szY );
      vecsFree(&XVec,&YVec,NULL);
  }
  else if (context->desc->extraParam & TE_FFT_32X32)
  {
        fft_handle_t h_fft = NULL, h_ifft = NULL;
        if (context->desc->extraParam & TE_FFT_REAL)
        {
            switch (N)
            {
            case 16: ASSERT(0); break;
            case 32: h_fft = rfft32_32;   h_ifft = rifft32_32; break;
            case 64: h_fft = rfft32_64;   h_ifft = rifft32_64; break;
            case 128: h_fft = rfft32_128; h_ifft = rifft32_128; break;
            case 256: h_fft = rfft32_256; h_ifft = rifft32_256; break;
            case 512: h_fft = rfft32_512; h_ifft = rifft32_512; break;
            case 1024: h_fft = rfft32_1024; h_ifft = rifft32_1024; break;
            case 2048: h_fft = rfft32_2048; h_ifft = rifft32_2048; break;
            case 4096: h_fft = rfft32_4096; h_ifft = rifft32_4096; break;
            case 8192: h_fft = rfft32_8192; h_ifft = rifft32_8192; break;
            default:
                ASSERT(0);  // fft handles not found
            }
        }
        else
        {
            switch (N)
            {
            case 16: h_fft = cfft32_16;     h_ifft = cifft32_16;     break;
            case 32: h_fft = cfft32_32;     h_ifft = cifft32_32;     break;
            case 64: h_fft = cfft32_64;     h_ifft = cifft32_64;     break;
            case 128: h_fft = cfft32_128;   h_ifft = cifft32_128;    break;
            case 256: h_fft = cfft32_256;   h_ifft = cifft32_256;    break;
            case 512: h_fft = cfft32_512;   h_ifft = cifft32_512;    break;
            case 1024: h_fft = cfft32_1024; h_ifft = cifft32_1024;   break;
            case 2048: h_fft = cfft32_2048; h_ifft =  cifft32_2048;  break;
            case 4096: h_fft = cfft32_4096; h_ifft =  cifft32_4096;  break;
                default:
                    ASSERT(0);  // fft handles not found
            }
        }
        bexp = 0; 

        if (context->desc->extraParam & TE_INVFFT_32X32)
        {
            h_fft = h_ifft;
            if (context->desc->extraParam & TE_FFT_REAL)
            {
                // Additional shift need for ifft_real_32x32
                bexp = 1; 
            }
        }

        bexp += fxn(py, px, h_fft, context_fft->scale_method);
         
  }
  else
  {
      bexp=fxn( py, px, twdTbl, twdStep, N, context_fft->scale_method );
  }

  if ( doInplace && vecGetSize( xVec ) >= vecGetSize( yVec ) )
  {
    memcpy( vecGetElem( yVec, 0 ), py, vecGetSize( yVec ) );
  }
  
  if ( noReuse && crcSum != crc32( 0, (uint8_t*)px, vecGetSize( xVec ) ) )
  {
    printf( "te_frameProc_stnd_scl_fft(): target FFT has corrupted the input buffer\n" );
    return ( 0 );
  }

  /* Compensate for scaling shift performed by fixed point forward FFTs. */
  shift -= bexp;
  /* For inverse FFTs,  we also have to divide output signal by FFT size. */
  if ( context->args.caseType == TE_FFT_TESTCASE_INVERSE ) shift += logN;

  /* Convert output data to complex 64-bit floating point and rescale them. */
  vecToFp64( (float64_t*)out, yVec, shift );

  return (1);

} /* te_frameProc_stnd_scl_fft() */
