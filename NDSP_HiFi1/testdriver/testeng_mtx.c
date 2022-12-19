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

#include <string.h>

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Matrix functions API. */
#include "NatureDSP_Signal.h"
/* Test engine API. */
#include "testeng_mtx.h"
#include <stdlib.h>

#define MAX(a,b)   ( (a)>(b) ? (a) : (b) )


/* Allocate vectors and load the data set for [c]matmmlt:
 * X[M][N], Y[N][P], Z[M][P] */
int te_loadFxn_mmlt( tTestEngContext * context )
{
    tVec X, Y, Z, Zlo, Zhi;
    int M, N, P;
    int nElemX, nElemY, nElemZ, res = 0;

    ASSERT( context && context->seqFile );

    memset( &X  , 0, sizeof(X  ) );
    memset( &Y  , 0, sizeof(Y  ) );
    memset( &Z  , 0, sizeof(Z  ) );
    memset( &Zlo, 0, sizeof(Zlo) );
    memset( &Zhi, 0, sizeof(Zhi) );

    M = MAX( 0, context->args.M );
    N = MAX( 0, context->args.N );
    P = MAX( 0, context->args.P );

    nElemX = M*N;
    nElemY = N*P;
    nElemZ = M*P;

    /* Allocate data vectors memory. */
    res=(3 == vecsAlloc( context->desc->isAligned,
                            context->desc->fmt,
                            &X, nElemX,
                            &Y, nElemY,
                            &Z, nElemZ, 0 ));
    res&=(2 == vecsAlloc( TE_ALIGN_NO,
                            context->desc->fmt,
                            &Zlo, nElemZ,
                            &Zhi, nElemZ, 0 ));
    if (!res)
    {
    printf( "loadFxn_mmlt(): failed to allocate vectors; "
            "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemY, nElemZ );
    }
    /* Load vectors data from the SEQ-file. */
    else if ( !seqFileReadVecs( context->seqFile, &X, &Y, &Zlo, &Zhi, 0 ) )
    {
    printf( "loadFxn_mmlt(): failed to read vectors data; "
            "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemY, nElemZ );
    }
    else
    {
        memset( &context->dataSet, 0, sizeof( context->dataSet ) );

        context->dataSet.X   = X;
        context->dataSet.Y   = Y;
        context->dataSet.Z   = Z;
        context->dataSet.Zlo = Zlo;
        context->dataSet.Zhi = Zhi;
        res = 1;
    }

  if ( !res ) freeVectors(context); /* Free vectors data if failed. */
  return (res);

} /* loadFxn_mmlt() */
/*function similar to te_loadFxn_mmlt but, lower 8 bits from each element of input buffers are zeroed*/ 
int te_loadFxn_mmlt_24( tTestEngContext * context )
{
    tVec X, Y, Z, Zlo, Zhi;
    int M, N, P, i;
    int nElemX, nElemY, nElemZ, res = 0;

    ASSERT( context && context->seqFile );

    memset( &X  , 0, sizeof(X  ) );
    memset( &Y  , 0, sizeof(Y  ) );
    memset( &Z  , 0, sizeof(Z  ) );
    memset( &Zlo, 0, sizeof(Zlo) );
    memset( &Zhi, 0, sizeof(Zhi) );

    M = MAX( 0, context->args.M );
    N = MAX( 0, context->args.N );
    P = MAX( 0, context->args.P );

    nElemX = M*N;
    nElemY = N*P;
    nElemZ = M*P;

    /* Allocate data vectors memory. */
    res=(3 == vecsAlloc( context->desc->isAligned,
                            context->desc->fmt,
                            &X, nElemX,
                            &Y, nElemY,
                            &Z, nElemZ, 0 ));
    res&=(2 == vecsAlloc( TE_ALIGN_NO,
                            context->desc->fmt,
                            &Zlo, nElemZ,
                            &Zhi, nElemZ, 0 ));
    if (!res)
    {
    printf( "loadFxn_mmlt(): failed to allocate vectors; "
            "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemY, nElemZ );
    }
    /* Load vectors data from the SEQ-file. */
    else if ( !seqFileReadVecs( context->seqFile, &X, &Y, &Zlo, &Zhi, 0 ) )
    {
    printf( "loadFxn_mmlt(): failed to read vectors data; "
            "fmt = 0x%02x, nElemX = %d, nElemY = %d, nElemZ = %d\n",
            (unsigned)context->desc->fmt, nElemX, nElemY, nElemZ );
    }
    else
    {
        memset( &context->dataSet, 0, sizeof( context->dataSet ) );
		for(i=0;i<nElemX;i++) ((int32_t *)((uint8_t*)X.ptr.aligned + ( X.offset )*X.szElem))[i]&=0xFFFFFF00;
		for(i=0;i<nElemY;i++) ((int32_t *)((uint8_t*)Y.ptr.aligned + ( Y.offset )*Y.szElem))[i]&=0xFFFFFF00;
		context->dataSet.X   = X;
        context->dataSet.Y   = Y;
        context->dataSet.Z   = Z;
		context->dataSet.Zlo = Zlo;
        context->dataSet.Zhi = Zhi;
        res = 1;
    }

  if ( !res ) freeVectors(context); /* Free vectors data if failed. */
  return (res);
} 

/* Apply the function under test to test case data set: [c]matmmlt */
void te_processFxn_mmlt( tTestEngContext * context )
{
  typedef void tFxn      (            void * z,   const void * x, const void * y, int M, int N, int P        );
  typedef void tFxnlsh   (            void** z,   const void * x, const void** y, int M, int N, int P,  int lsh);
  typedef void tFxnlshscr(void *pScr, void** z,   const void * x, const void** y, int M, int N, int P,  int lsh);
  void *X, *Y, *Z;
  int M, N, P, lsh;
  size_t scrSz;
  tAllocPtr scratch;
  void *pScr;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  M   = context->args.M;
  N   = context->args.N;
  P   = context->args.P;
  lsh   = context->args.L;

  scrSz=0;
  if(context->desc->extraParam & MTX_SCR16X16) scrSz=SCRATCH_MTX_MPY16X16(MAX(M,0),MAX(N,0),MAX(P,0));
  if(context->desc->extraParam & MTX_SCR24X24) scrSz=SCRATCH_MTX_MPY24X24(MAX(M,0),MAX(N,0),MAX(P,0));
  pScr=scrSz==0 ? NULL: mallocAlign(&scratch,scrSz,8);

  switch(context->desc->extraParam & 1)
  {
  case MTX_PLAIN:     ((tFxn*   )context->target.fut)(Z, X, Y, M, N, P      ); break;
  case MTX_PLAIN_LSH:
    {   /* adaptation to crazy API */
        int n,m;
        void** y;
        void** z;
        tVec *zarr,*yarr;
       
        z=(void**)(calloc(MAX(M,1),sizeof(void*)));
        y=(void**)(calloc(MAX(N,1),sizeof(void*)));
        zarr=(tVec*)(calloc(MAX(M,1),sizeof(tVec)));
        yarr=(tVec*)(calloc(MAX(N,1),sizeof(tVec)));
        /* allocate arrays and copy original Y */
        for (m=0; m<M; m++) 
        {
            vecAlloc(zarr+m,MAX(0,P),context->desc->isAligned,context->desc->fmt,NULL);
            z[m]=vecGetElem( zarr+m, 0 );
        }
        for (n=0; n<N; n++) 
        {
            vecAlloc(yarr+n,MAX(0,P),context->desc->isAligned,context->desc->fmt, NULL);
            y[n]=vecGetElem( yarr+n, 0 );
        }
        if(N>0 && P>0)
        {
            for (n=0; n<N; n++) 
            {
                memcpy(y[n],vecGetElem( &context->dataSet.Y, n*P ),vecGetSize(yarr+n));
            }
        }
        if (context->desc->extraParam & ~1)        ((tFxnlshscr*)context->target.fut)(pScr, z, X, (const void**)y, M, N, P, lsh ); 
        else                                       ((tFxnlsh   *)context->target.fut)(      z, X, (const void**)y, M, N, P, lsh ); 
        /* copy results from z to Z */
        if(M>0 && P>0)
        {
            for (m=0; m<M; m++) 
            {
                memcpy(vecGetElem( &context->dataSet.Z, m*P ),z[m],vecGetSize(zarr+m));
            }
        }
        /* free resources */
        free(z); free(y);
        for (m=0; m<M; m++) vecFree(zarr+m);
        for (n=0; n<N; n++) vecFree(yarr+n);
        free(yarr); free(zarr);
    }
    if(pScr) freeAlign(&scratch);
  }

} /* processFxn_mmlt() */
/* Apply the function under test to test case data set: vecmpy */
void te_processFxn_vecmmlt( tTestEngContext * context )
{
  typedef void tFxn   (void * z,   const void * x, const void * y, int M, int N         );
  typedef void tFxnlsh(void * z,   const void * x, const void * y, int M, int N, int lsh);
  void *X, *Y, *Z;
  int M, N, P, lsh;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  M   = context->args.M;
  N   = context->args.N;
  P   = context->args.P; (void)P;
  lsh   = context->args.L;
  ASSERT(P==1);
  switch(context->desc->extraParam)
  {
  case MTX_PLAIN:     ((tFxn*   )context->target.fut)(Z, X, Y, M, N     ); break;
  case MTX_PLAIN_LSH: ((tFxnlsh*)context->target.fut)(Z, X, Y, M, N, lsh); break;
  }

} /* te_processFxn_vecmmlt() */


/* Allocate vectors and load the data set for matrix operations:
 * X[L][N][N], Y[L][N][N], Z[L][N][N] */
int te_loadFxn_mtx_op0( tTestEngContext * context )
{
    tVec rsh;
    int N,L;
    int nElemX, nElemY, nElemZ, res = 0;

    ASSERT( context && context->seqFile );

    memset( &context->dataSet.X  , 0, sizeof(context->dataSet.X  ) );
    memset( &context->dataSet.Y  , 0, sizeof(context->dataSet.Y  ) );
    memset( &context->dataSet.Z  , 0, sizeof(context->dataSet.Z  ) );
    memset( &context->dataSet.Zlo, 0, sizeof(context->dataSet.Zlo) );
    memset( &context->dataSet.Zhi, 0, sizeof(context->dataSet.Zhi) );

    N = MAX( 0, context->args.N );
    L = MAX( 0, context->args.L );

    nElemX = 
    nElemY = 
    nElemZ = MAX(0,N*N*L);

    /* Allocate data vectors memory. */
    vecAlloc(&rsh,1,1,FMT_INT32,0);
    res=(5 == vecsAlloc( context->desc->isAligned,
                            context->desc->fmt,
                            &context->dataSet.X, nElemX,
                            &context->dataSet.Y, nElemY,
                            &context->dataSet.Z, nElemZ,
                            &context->dataSet.Zlo, nElemZ,
                            &context->dataSet.Zhi, nElemZ, 0 ));
    if (!res)
    {
        printf( "te_loadFxn_mtx_op0(): failed to allocate vectors\n");
    }
    else 
    {
        /* Load vectors data from the SEQ-file. */
        switch(context->desc->extraParam & 1)
        {
        case MTX_PLAIN:     res=seqFileReadVecs( context->seqFile, 
                                             &context->dataSet.X, 
                                             &context->dataSet.Y, 
                                             &context->dataSet.Zlo, 
                                             &context->dataSet.Zhi, 0 );
                            break;
        case MTX_PLAIN_LSH: res=seqFileReadVecs( context->seqFile, 
                                             &context->dataSet.X, 
                                             &context->dataSet.Y, 
                                             &rsh, 
                                             &context->dataSet.Zlo, 
                                             &context->dataSet.Zhi, 0 );
                            context->args.P=vecGetElem_i32(&rsh,0)[0];
                            break;
        }
        if (!res)
        {
            printf( "te_loadFxn_mtx_op0(): failed to read vectors data; \n");
        }
    }
    if ( !res ) freeVectors(context); /* Free vectors data if failed. */
    vecFree(&rsh);
    return (res);
} /* te_loadFxn_mtx_op0() */

/* Allocate vectors and load the data set for matrix operations:
 * X[L][N][N], Z[L][N][N] */
int te_loadFxn_mtx_op1( tTestEngContext * context )
{
    int N,L;
    int nElemX, nElemY, nElemZ, res = 0;

    ASSERT( context && context->seqFile );

    memset( &context->dataSet.X  , 0, sizeof(context->dataSet.X  ) );
    memset( &context->dataSet.Y  , 0, sizeof(context->dataSet.Y  ) );
    memset( &context->dataSet.Z  , 0, sizeof(context->dataSet.Z  ) );
    memset( &context->dataSet.Zlo, 0, sizeof(context->dataSet.Zlo) );
    memset( &context->dataSet.Zhi, 0, sizeof(context->dataSet.Zhi) );

    N = MAX( 0, context->args.N );
    L = MAX( 0, context->args.L );

    nElemX = 
    nElemY = 
    nElemZ = MAX(0,N*N*L);

    /* Allocate data vectors memory. */
    res=(4 == vecsAlloc( context->desc->isAligned,
                            context->desc->fmt,
                            &context->dataSet.X, nElemX,
                            &context->dataSet.Z, nElemZ,
                            &context->dataSet.Zlo, nElemZ,
                            &context->dataSet.Zhi, nElemZ, 0 ));
    if (!res)
    {
        printf( "te_loadFxn_mtx_op1(): failed to allocate vectors\n");
    }
    else 
    {
        /* Load vectors data from the SEQ-file. */
        if ( !seqFileReadVecs( context->seqFile, &context->dataSet.X, &context->dataSet.Zlo, &context->dataSet.Zhi, 0 ))
        {
            printf( "te_loadFxn_mtx_op1(): failed to read vectors data; \n");
            res=0;
        }
    }
    if ( !res ) freeVectors(context); /* Free vectors data if failed. */
    return (res);
} /* te_loadFxn_mtx_op1() */

/* Allocate vectors and load the data set for matrix operations:
 * X[L][N][N], Z[L] */
int te_loadFxn_mtx_op2( tTestEngContext * context )
{
    tVec rsh;
    int N,L;
    int nElemX, nElemZ, res = 0;

    ASSERT( context && context->seqFile );

    memset( &context->dataSet.X  , 0, sizeof(context->dataSet.X  ) );
    memset( &context->dataSet.Y  , 0, sizeof(context->dataSet.Y  ) );
    memset( &context->dataSet.Z  , 0, sizeof(context->dataSet.Z  ) );
    memset( &context->dataSet.Zlo, 0, sizeof(context->dataSet.Zlo) );
    memset( &context->dataSet.Zhi, 0, sizeof(context->dataSet.Zhi) );
    vecAlloc(&rsh,1,1,FMT_INT32,0);

    N = MAX( 0, context->args.N );
    L = MAX( 0, context->args.L );

    nElemX = MAX(0,N*N*L); 
    nElemZ = MAX(0,L);

    /* Allocate data vectors memory. */
    res=(4 == vecsAlloc( context->desc->isAligned,
                            context->desc->fmt,
                            &context->dataSet.X, nElemX,
                            &context->dataSet.Z, nElemZ,
                            &context->dataSet.Zlo, nElemZ,
                            &context->dataSet.Zhi, nElemZ, 0 ));
    if (!res)
    {
        printf( "te_loadFxn_mtx_op2(): failed to allocate vectors\n");
    }
    else 
    {
        /* Load vectors data from the SEQ-file. */
                switch(context->desc->extraParam & 1)
        {
        case MTX_PLAIN:
            res=seqFileReadVecs( context->seqFile, &context->dataSet.X, &context->dataSet.Zlo, &context->dataSet.Zhi, 0 );
            break;
        case MTX_PLAIN_LSH:
            res=seqFileReadVecs( context->seqFile, &context->dataSet.X, &rsh, &context->dataSet.Zlo, &context->dataSet.Zhi, 0 );
            context->args.P=vecGetElem_i32(&rsh,0)[0];
        }
        if ( !res)
        {
            printf( "te_loadFxn_mtx_op2(): failed to read vectors data; \n");
        }
    }
    if ( !res ) freeVectors(context); /* Free vectors data if failed. */
    vecFree(&rsh);
    return (res);
} /* te_loadFxn_mtx_op2() */

/* X[N][4], Z[N][9] */
int te_loadFxn_q2rot( tTestEngContext * context )
{
    int N;
    int nElemX, nElemZ, res = 0;

    ASSERT( context && context->seqFile );

    memset( &context->dataSet.X  , 0, sizeof(context->dataSet.X  ) );
    memset( &context->dataSet.Z  , 0, sizeof(context->dataSet.Z  ) );
    memset( &context->dataSet.Zlo, 0, sizeof(context->dataSet.Zlo) );
    memset( &context->dataSet.Zhi, 0, sizeof(context->dataSet.Zhi) );

    N = MAX( 0, context->args.N );

    nElemX = MAX(0,4*N); 
    nElemZ = MAX(0,9*N);

    /* Allocate data vectors memory. */
    res=(4 == vecsAlloc( context->desc->isAligned,
                            context->desc->fmt,
                            &context->dataSet.X, nElemX,
                            &context->dataSet.Z, nElemZ,
                            &context->dataSet.Zlo, nElemZ,
                            &context->dataSet.Zhi, nElemZ, 0 ));
    if (!res)
    {
        printf( "te_loadFxn_q2rot(): failed to allocate vectors\n");
    }
    else 
    {
        /* Load vectors data from the SEQ-file. */
        res=seqFileReadVecs( context->seqFile, &context->dataSet.X, &context->dataSet.Zlo, &context->dataSet.Zhi, 0 );
        if ( !res)
        {
            printf( "te_loadFxn_q2rot(): failed to read vectors data; \n");
        }
    }
    if ( !res ) freeVectors(context); /* Free vectors data if failed. */
    return (res);
}

/* Apply the function under test to test case data set: 
X[L][N][N], Y[L][N][N], Z[L][N][N]  */
void te_processFxn_mtx_op0( tTestEngContext * context )
{
  typedef void tFxn      (            void* z,   const void * x, const void* y, int L);
  typedef void tFxnrsh   (            void* z,   const void * x, const void* y, int, int L);
  void *X, *Y, *Z;
  int rsh, N, L;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N; (void)N;
  L   = context->args.L;
  rsh = context->args.P;

  switch(context->desc->extraParam & 1)
  {
  case MTX_PLAIN:     ((tFxn*   )context->target.fut)(Z, X, Y, L );     break;
  case MTX_PLAIN_LSH: ((tFxnrsh*)context->target.fut)(Z, X, Y, rsh, L ); break;
  default: ASSERT(0);
  }

} /* te_processFxn_mtx_op0() */

/* Apply the function under test to test case data set: 
X[L][N][N], Z[L][N][N]  */
void te_processFxn_mtx_op1( tTestEngContext * context )
{
  typedef void tFxn      (            void* z,   const void * x, int L);
  void *X, *Z;
  int N, L;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;  (void)N;
  L   = context->args.L;

  switch(context->desc->extraParam & 1)
  {
  case MTX_PLAIN:     ((tFxn*   )context->target.fut)(Z, X, L ); break;
  default: ASSERT(0);
  }

} /* te_processFxn_mtx_op1() */

/* Apply the function under test to test case data set: 
X[L][N][N], Z[L]  */
void te_processFxn_mtx_op2( tTestEngContext * context )
{
  typedef void tFxn      (            void* z,   const void * x,  int L);
  typedef void tFxnrsh   (            void* z,   const void * x,  int, int L);
  void *X, *Z;
  int rsh, N, L;

  ASSERT( context && context->target.fut );

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  N   = context->args.N;
  L   = context->args.L;
  rsh = context->args.P;

  switch(context->desc->extraParam & 1)
  {
  case MTX_PLAIN:     ((tFxn*   )context->target.fut)(Z, X, L );     break;
  case MTX_PLAIN_LSH: ((tFxnrsh*)context->target.fut)(Z, X, rsh, L ); break;
  default: ASSERT(0);
  }

} /* te_processFxn_mtx_op0() */
