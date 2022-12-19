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
 * Test procedures for vector mathematics
 */

#include <math.h>

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* DSP Library API: arithmetic and logic functions on data vectors. */
#include "NatureDSP_Signal.h"
/* Test engine API. */
#include "testeng.h"

/* Test executive function. Performs the specified test on a brief or full version
 * of the designated SEQ-file. Return the test result (non-zero if passed). */
static int testExec( tTestEngTarget targetFxn, const char * seqName,
                     int errhExtendedTest, int isFull, int isVerbose, int breakOnError );

/* Apply a function to the test case data set:
 * scalar functions with single argument, e.g. cos() */
void processFxn_scl_vXvZ( tTestEngContext * context )
{
  typedef float32_t tFxn_fl32( float32_t );
  typedef float64_t tFxn_fl64( float64_t );
  typedef fract16   tFxn_fr16( fract16   );
  typedef fract32   tFxn_fr32( fract32   );

  #define CALL_FXN( typeFxn, Fxn, typeXZ, X, Z, n )           \
      { te_errh_resetStates( context );                       \
        *(typeXZ*)(Z) = ( (typeFxn*)(Fxn) )( *(typeXZ*)(X) ); \
        te_errh_verifyStates( context, n );                   \
        (X) = (typeXZ*)(X) + 1; (Z) = (typeXZ*)(Z) + 1; }

  tTestEngTarget  fxn;
  void *X, *Z;
  int n, N;

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  fxn = context->target.fut;
  N   = context->args.N;

  switch ( context->desc->fmt & ~FMT_CPLX )
  {
  case FMT_FLOAT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl32, fxn, float32_t, X, Z, n ); break;
  case FMT_FLOAT64: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl64, fxn, float64_t, X, Z, n ); break;
  case FMT_FRACT16: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr16, fxn, fract16  , X, Z, n ); break;
  case FMT_FRACT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr32, fxn, fract32  , X, Z, n ); break;
  default: ASSERT( 0 );
  }

  #undef CALL_FXN

} /* processFxn_scl_vXvZ() */

void processFxn_scl_vXvZ32( tTestEngContext * context )
{
  typedef int32_t tFxn_fl32( float32_t );
  typedef int32_t tFxn_fl64( float64_t );
  typedef int32_t tFxn_fr16( fract16   );
  typedef int32_t tFxn_fr32( fract32   );

  #define CALL_FXN( typeFxn, Fxn, typeXZ, X, Z ) \
      { *(int32_t*)(Z) = ( (typeFxn*)(Fxn) )( *(typeXZ*)(X) ); \
        (X) = (typeXZ*)(X) + 1; (Z) = (typeXZ*)(Z) + 1; }

  tTestEngTarget  fxn;
  void *X, *Z;
  int n, N;

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  fxn = context->target.fut;
  N   = context->args.N;

  switch ( context->desc->fmt & ~FMT_CPLX )
  {
  case FMT_FLOAT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl32, fxn, float32_t, X, Z ); break;
  case FMT_FLOAT64: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl64, fxn, float64_t, X, Z ); break;
  case FMT_FRACT16: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr16, fxn, fract16  , X, Z ); break;
  case FMT_FRACT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr32, fxn, fract32  , X, Z ); break;
  default: ASSERT( 0 );
  }

  #undef CALL_FXN

} /* processFxn_scl_vXvZ32() */
void processFxn_scl_vX32sY32vZ( tTestEngContext * context )
{
  typedef float32_t tFxn_fl32( int32_t,int32_t );
  typedef float64_t tFxn_fl64( int32_t,int32_t );
  typedef fract16   tFxn_fr16( int32_t,int32_t );
  typedef fract32   tFxn_fr32( int32_t,int32_t );

  #define CALL_FXN( typeFxn, Fxn, typeX,typeY,typeZ, X,Y,Z ) \
      { *(typeZ*)(Z) = ( (typeFxn*)(Fxn) )( *(typeX*)(X),*(typeY*)(Y) ); \
        (X) = (typeX*)(X) + 1; (Y) = (typeY*)(Y) + 0; (Z) = (typeZ*)(Z) + 1; }

  tTestEngTarget  fxn;
  void *X, *Y, *Z;
  int n, N;

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  fxn = context->target.fut;
  N   = context->args.N;

  switch ( context->desc->fmt & ~FMT_CPLX )
  {
  case FMT_FLOAT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl32, fxn, int32_t,int32_t,float32_t, X,Y,Z ); break;
  case FMT_FLOAT64: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl64, fxn, int32_t,int32_t,float64_t, X,Y,Z ); break;
  case FMT_FRACT16: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr16, fxn, int32_t,int32_t,fract16  , X,Y,Z ); break;
  case FMT_FRACT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr32, fxn, int32_t,int32_t,fract32  , X,Y,Z ); break;
  default: ASSERT( 0 );
  }

  #undef CALL_FXN

} /* processFxn_scl_vX32sY32vZ() */

void processFxn_scl_vXsY32vZ32( tTestEngContext * context )
{
  typedef int32_t tFxn_fl32( float32_t,int32_t );
  typedef int32_t tFxn_fl64( float64_t,int32_t );
  typedef int32_t tFxn_fr16( fract16  ,int32_t );
  typedef int32_t tFxn_fr32( fract32  ,int32_t );

  #define CALL_FXN( typeFxn, Fxn, typeX,typeY,typeZ, X,Y,Z ) \
      { *(typeZ*)(Z) = ( (typeFxn*)(Fxn) )( *(typeX*)(X),*(typeY*)(Y) ); \
        (X) = (typeX*)(X) + 1; (Y) = (typeY*)(Y) + 0; (Z) = (typeZ*)(Z) + 1; }

  tTestEngTarget  fxn;
  void *X, *Y, *Z;
  int n, N;

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  fxn = context->target.fut;
  N   = context->args.N;

  switch ( context->desc->fmt & ~FMT_CPLX )
  {
  case FMT_FLOAT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl32, fxn, float32_t,int32_t,int32_t, X,Y,Z ); break;
  case FMT_FLOAT64: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl64, fxn, float64_t,int32_t,int32_t, X,Y,Z ); break;
  case FMT_FRACT16: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr16, fxn, fract16  ,int32_t,int32_t, X,Y,Z ); break;
  case FMT_FRACT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr32, fxn, fract32  ,int32_t,int32_t, X,Y,Z ); break;
  default: ASSERT( 0 );
  }

  #undef CALL_FXN

} /* processFxn_scl_vXsY32vZ32() */

/* Apply a function to the test case data set:
 * scalar functions with single argument, e.g. cos(). Input X is complex, output is real*/
void processFxn_scl_vXcvZ( tTestEngContext * context )
{
  typedef float32_t tFxn_fl32( complex_float  );
  typedef float64_t tFxn_fl64( complex_double );
  typedef fract16   tFxn_fr16( complex_fract16);
  typedef fract32   tFxn_fr32( complex_fract32);

  #define CALL_FXN( typeFxn, Fxn, typeX,typeZ, X, Z ) \
      { *(typeZ*)(Z) = ( (typeFxn*)(Fxn) )( *(typeX*)(X) ); \
        (X) = (typeX*)(X) + 1; (Z) = (typeZ*)(Z) + 1; }

  tTestEngTarget  fxn;
  void *X, *Z;
  int n, N;

  X = vecGetElem( &context->dataSet.X, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  fxn = context->target.fut;
  N   = context->args.N;

  switch ( context->desc->fmt & ~FMT_CPLX )
  {
  case FMT_FLOAT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl32, fxn, complex_float  ,float32_t, X, Z ); break;
  case FMT_FLOAT64: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl64, fxn, complex_double ,float64_t, X, Z ); break;
  case FMT_FRACT16: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr16, fxn, complex_fract16,fract16  , X, Z ); break;
  case FMT_FRACT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr32, fxn, complex_fract32,fract32  , X, Z ); break;
  default: ASSERT( 0 );
  }

  #undef CALL_FXN

} /* processFxn_scl_vXcvZ() */

/* Apply a function to the test case data set:
 * scalar atan2(y,x) */
void processFxn_scl_atan2( tTestEngContext * context )
{
  typedef float32_t tFxn_fl32( float32_t, float32_t );
  typedef float64_t tFxn_fl64( float64_t, float64_t );
  typedef fract16   tFxn_fr16( fract16  , fract16   );
  typedef fract32   tFxn_fr32( fract32  , fract32   );

  #define CALL_FXN( typeFxn, Fxn, typeXYZ, Y, X, Z, n )                         \
      { te_errh_resetStates( context );                                         \
        *(typeXYZ*)(Z) = ( (typeFxn*)(Fxn) )( *(typeXYZ*)(Y), *(typeXYZ*)(X) ); \
        te_errh_verifyStates( context, n );                                     \
        (Y) = (typeXYZ*)(Y) + 1; (X) = (typeXYZ*)(X) + 1; (Z) = (typeXYZ*)(Z) + 1; }

  tTestEngTarget  fxn;
  void *X, *Y, *Z;
  int n, N;

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  fxn = context->target.fut;
  N   = context->args.N;

  switch ( context->desc->fmt & ~FMT_CPLX )
  {
  case FMT_FLOAT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl32, fxn, float32_t, Y, X, Z, n ); break;
  case FMT_FLOAT64: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl64, fxn, float64_t, Y, X, Z, n ); break;
  case FMT_FRACT16: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr16, fxn, fract16  , Y, X, Z, n ); break;
  case FMT_FRACT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fr32, fxn, fract32  , Y, X, Z, n ); break;
  default: ASSERT( 0 );
  }

  #undef CALL_FXN

} /* processFxn_scl_atan2() */

void processFxn_scl_dividef( tTestEngContext * context )
{
  typedef float32_t tFxn_fl32( float32_t, float32_t );

  #define CALL_FXN( typeFxn, Fxn, typeXYZ, Y, X, Z, n )                         \
      { te_errh_resetStates( context );                                         \
        *(typeXYZ*)(Z) = ( (typeFxn*)(Fxn) )( *(typeXYZ*)(X), *(typeXYZ*)(Y) ); \
        te_errh_verifyStates( context, n );                                     \
        (Y) = (typeXYZ*)(Y) + 1; (X) = (typeXYZ*)(X) + 1; (Z) = (typeXYZ*)(Z) + 1; }

  tTestEngTarget  fxn;
  void *X, *Y, *Z;
  int n, N;

  X = vecGetElem( &context->dataSet.X, 0 );
  Y = vecGetElem( &context->dataSet.Y, 0 );
  Z = vecGetElem( &context->dataSet.Z, 0 );

  fxn = context->target.fut;
  N   = context->args.N;

  switch ( context->desc->fmt & ~FMT_CPLX )
  {
  case FMT_FLOAT32: for ( n=0; n<N; n++ ) CALL_FXN( tFxn_fl32, fxn, float32_t, Y, X, Z, n ); break;
  default: ASSERT( 0 );
  }

  #undef CALL_FXN

} /* processFxn_scl_dividef() */

/* processing function for vec_recip16x16 */
void te_processFxn_recip16x16( tTestEngContext * context )
{
  typedef void tFxn( int16_t * restrict frac, 
                  int16_t *exp, 
                  const int16_t * restrict x, 
                  int N);
  tVec frac,exponent;
  tFxn *fxn;
  const int16_t *X; 
  float32_t *Z;
  int16_t* pFrac,*pExp;
  int n,N;
  ASSERT( context && context->target.fut );
  X = vecGetElem_fr16( &context->dataSet.X, 0 );
  Z = vecGetElem_fl32( &context->dataSet.Z, 0 );
  N   = context->args.N;
  vecsAlloc(context->desc->isAligned,FMT_FRACT16,&frac,N<0?0:N,&exponent,N<0?0:N,NULL);
  pFrac=vecGetElem_fr16(&frac,0);
  pExp =vecGetElem_fr16(&exponent,0);
  fxn = (tFxn*)context->target.fut;
  fxn( pFrac, pExp, X, N );
  /* convert results to single precision floating point */
  for(n=0; n<N; n++)
  {
      Z[n]=STDLIB_MATH(ldexpf)(pFrac[n],pExp[n]-15);
  }
  vecsFree(&frac,&exponent,NULL);
} /* te_processFxn_recip16x16() */

/* processing function for vec_divide16x16 */
void te_processFxn_divide16x16( tTestEngContext * context )
{
  typedef void tFxn( int16_t * restrict frac, 
                  int16_t *exp, 
                  const int16_t * restrict x, 
                  const int16_t * restrict y, 
                  int N);
  tVec frac,exponent;
  tFxn *fxn;
  const int16_t *X; 
  const int16_t *Y; 
  float32_t *Z;
  int16_t* pFrac,*pExp;
  int n,N;
  ASSERT( context && context->target.fut );
  X = vecGetElem_fr16( &context->dataSet.X, 0 );
  Y = vecGetElem_fr16( &context->dataSet.Y, 0 );
  Z = vecGetElem_fl32( &context->dataSet.Z, 0 );
  N   = context->args.N;
  vecsAlloc(context->desc->isAligned,FMT_FRACT16,&frac,N<0?0:N,&exponent,N<0?0:N,NULL);
  pFrac=vecGetElem_fr16(&frac,0);
  pExp =vecGetElem_fr16(&exponent,0);
  fxn = (tFxn*)context->target.fut;
  fxn( pFrac, pExp, X,Y, N );
  /* convert results to single precision floating point */
  for(n=0; n<N; n++)
  {
      Z[n]=STDLIB_MATH(ldexpf)(pFrac[n],pExp[n]-15);
  }
  vecsFree(&frac,&exponent,NULL);
} /* te_processFxn_divide16x16() */

/* processing function for vec_divide32x32 */
void te_processFxn_divide32x32( tTestEngContext * context )
{
  typedef void tFxn( int32_t * restrict frac, 
                  int16_t *exp, 
                  const int32_t * restrict x, 
                  const int32_t * restrict y, 
                  int N);
  tVec frac,exponent;
  tFxn *fxn;
  const int32_t *X; 
  const int32_t *Y; 
  float64_t *Z;
  int32_t* pFrac;
  int16_t *pExp;
  int n,N;
  ASSERT( context && context->target.fut );
  X = vecGetElem_fr32( &context->dataSet.X, 0 );
  Y = vecGetElem_fr32( &context->dataSet.Y, 0 );
  Z = vecGetElem_fl64( &context->dataSet.Z, 0 );
  N   = context->args.N;
  vecsAlloc(context->desc->isAligned,FMT_FRACT32,&frac,N<0?0:N,NULL);
  vecsAlloc(context->desc->isAligned,FMT_FRACT16,&exponent,N<0?0:N,NULL);
  pFrac=vecGetElem_fr32(&frac,0);
  pExp =vecGetElem_fr16(&exponent,0);
  fxn = (tFxn*)context->target.fut;
  fxn( pFrac, pExp, X,Y, N );
  /* convert results to single precision floating point */
  for(n=0; n<N; n++)
  {
      Z[n]=STDLIB_MATH(ldexp)(pFrac[n],pExp[n]-31);
  }
  vecsFree(&frac,&exponent,NULL);
} /* te_processFxn_divide32x32() */

/* processing function for vec_recip32x32 vec_recip24x24 */
void te_processFxn_recip32x32( tTestEngContext * context )
{
  typedef void tFxn( int32_t * restrict frac, 
                  int16_t *exp, 
                  const int32_t * restrict x, 
                  int N);
  tVec frac,exponent;
  tFxn *fxn;
  int32_t *X; 
  float64_t *Z;
  int32_t* pFrac;
  int16_t *pExp;
  int n,N;
  ASSERT( context && context->target.fut );
  X = vecGetElem_fr32( &context->dataSet.X, 0 );
  Z = vecGetElem_fl64( &context->dataSet.Z, 0 );
  N   = context->args.N;
  vecsAlloc(context->desc->isAligned,FMT_FRACT32,&frac,N<0?0:N,NULL);
  vecsAlloc(context->desc->isAligned,FMT_FRACT16,&exponent,N<0?0:N,NULL);
  pFrac=vecGetElem_fr32(&frac,0);
  pExp =vecGetElem_fr16(&exponent,0);
  fxn = (tFxn*)context->target.fut;
  fxn( pFrac, pExp, X, N );
  /* convert results to single precision floating point */
  for(n=0; n<N; n++)
  {
      Z[n]=STDLIB_MATH(ldexp)(pFrac[n],pExp[n]-31);
  }
  vecsFree(&frac,&exponent,NULL);
} /* te_processFxn_recip32x32() */
/* processing function for scl_recip16x16 */
void te_processFxn_scl_recip16x16( tTestEngContext * context )
{
  typedef uint32_t tFxn( int16_t);
  tFxn *fxn;
  int16_t *X; 
  float32_t *Z;
  int n,N;
  ASSERT( context && context->target.fut );
  X = vecGetElem_fr16( &context->dataSet.X, 0 );
  Z = vecGetElem_fl32( &context->dataSet.Z, 0 );
  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;
  /* convert results to single precision floating point */
  for(n=0; n<N; n++)
  {
      uint32_t z;
      int16_t  frac,exponent;
      z=fxn(X[n]);
      frac=(int16_t)z;
      exponent=(int16_t)((int32_t)z>>16);
      Z[n]=STDLIB_MATH(ldexpf)(frac,exponent-15);
  }
} /* te_processFxn_scl_recip16x16() */
/* processing function for scl_recip32x32 scl_recip24x24 */
void te_processFxn_scl_recip32x32( tTestEngContext * context )
{
  typedef uint32_t tFxn( int32_t);
  tFxn *fxn;
  int32_t *X; 
  float64_t *Z;
  int n,N;
  ASSERT( context && context->target.fut );
  X = vecGetElem_fr32( &context->dataSet.X, 0 );
  Z = vecGetElem_fl64( &context->dataSet.Z, 0 );
  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;
  /* convert results to single precision floating point */
  for(n=0; n<N; n++)
  {
      uint32_t z;
      int32_t  frac;
      int16_t exponent;
      z=fxn(X[n]);
      frac=(int32_t)(z<<8);
      exponent=(int16_t)((int32_t)z>>24);
      Z[n]=STDLIB_MATH(ldexp)(frac,exponent-31);
  }
} /* te_processFxn_scl_recip32x32() */

/* processing function for scl_divide32x32 scl_divide24x24 */
void te_processFxn_scl_divide32x32( tTestEngContext * context )
{
  typedef uint32_t tFxn( int32_t, int32_t);
  tFxn *fxn;
  int32_t *X; 
  int32_t *Y; 
  float64_t *Z;
  int n,N;
  ASSERT( context && context->target.fut );
  X = vecGetElem_fr32( &context->dataSet.X, 0 );
  Y = vecGetElem_fr32( &context->dataSet.Y, 0 );
  Z = vecGetElem_fl64( &context->dataSet.Z, 0 );
  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;
  /* convert results to single precision floating point */
  for(n=0; n<N; n++)
  {
      uint32_t z;
      int32_t  frac;
      int16_t exponent;
      z=fxn(X[n],Y[n]);
      frac=(int32_t)(z<<8);
      exponent=(int16_t)((int32_t)z>>24);
      Z[n]=STDLIB_MATH(ldexp)(frac,exponent-31);
  }
} /* te_processFxn_scl_divide32x32() */

/* processing function for scl_divide16x16 */
void te_processFxn_scl_divide16x16( tTestEngContext * context )
{
  typedef uint32_t tFxn( int16_t, int16_t);
  tFxn *fxn;
  int16_t *X; 
  int16_t *Y; 
  float32_t *Z;
  int n,N;
  ASSERT( context && context->target.fut );
  X = vecGetElem_fr16( &context->dataSet.X, 0 );
  Y = vecGetElem_fr16( &context->dataSet.Y, 0 );
  Z = vecGetElem_fl32( &context->dataSet.Z, 0 );
  N   = context->args.N;
  fxn = (tFxn*)context->target.fut;
  /* convert results to single precision floating point */
  for(n=0; n<N; n++)
  {
      uint32_t z;
      int16_t frac;
      int16_t exponent;
      z=fxn(X[n],Y[n]);
      frac=(int16_t)(z);
      exponent=(int16_t)((int32_t)z>>16);
      Z[n]=STDLIB_MATH(ldexpf)(frac,exponent-15);
  }
} /* te_processFxn_scl_divide16x16() */

/* Perform all tests for math API functions. */
int main_vec_old( int phaseNum, int isFull, int isVerbose, int breakOnError );
int main_math( int phaseNum, int isFull, int isVerbose, int breakOnError )
{
  int res = 1;

  printf( "\nVector mathematics:\n" );

  #define DO_TEST( fxn, seqFile, extraFlags )                                                         \
    if ( res || !breakOnError ) res &= ( 0 != testExec( (tTestEngTarget)(fxn), (seqFile),             \
                                                        (extraFlags) | TE_ERRH_EXTENDED_TEST_DISABLE, \
                                                        isFull, isVerbose, breakOnError ) )
  /* Extended variant runs each SEQ-file twice, with extended Error Handling check in the second
   * invocation. */
  #define DO_TEST_EXT( fxn, seqFile, extraFlags )                                                     \
      { if ( res || !breakOnError ) res &= ( 0 != testExec(                                           \
                                                        (tTestEngTarget)(fxn),                        \
                                                        (seqFile),                                    \
                                                        (extraFlags) | TE_ERRH_EXTENDED_TEST_DISABLE, \
                                                        isFull, isVerbose, breakOnError ) );          \
        if ( res || !breakOnError ) res &= ( 0 != testExec(                                           \
                                                        (tTestEngTarget)(fxn),                        \
                                                        (seqFile),                                    \
                                                        (extraFlags) | TE_ERRH_EXTENDED_TEST_ENABLE,  \
                                                        isFull, isVerbose, breakOnError ) ); }

  /*
   * Stage 1
   */

  if ( phaseNum == 0 || phaseNum == 1 )
  {
        DO_TEST( &scl_recip16x16         , "scl_recip16x16.seq"       , 0 );
        DO_TEST( &scl_recip32x32         , "scl_recip32x32.seq"       , 0 );
        DO_TEST( &scl_recip24x24         , "scl_recip24x24.seq"       , 0 );
        DO_TEST( &scl_divide16x16        , "scl_divide16x16.seq"      , 0 );
        DO_TEST( &scl_divide32x32        , "scl_divide32x32.seq"      , 0 );
        DO_TEST( &scl_divide24x24        , "scl_divide24x24.seq"      , 0 );
        DO_TEST( &scl_log2_32x32         , "scl_log2_32x32.seq"       , 0 );
        DO_TEST( &scl_logn_32x32         , "scl_logn_32x32.seq"       , 0 );
        DO_TEST( &scl_log10_32x32        , "scl_log10_32x32.seq"      , 0 );
        DO_TEST( &scl_log2_24x24         , "scl_log2_24x24.seq"       , 0 );
        DO_TEST( &scl_logn_24x24         , "scl_logn_24x24.seq"       , 0 );
        DO_TEST( &scl_log10_24x24        , "scl_log10_24x24.seq"      , 0 );
        DO_TEST( &scl_antilog2_32x32     , "scl_antilog2_32x32.seq"   , 0 );
        DO_TEST( &scl_antilogn_32x32     , "scl_antilogn_32x32.seq"   , 0 );
        DO_TEST( &scl_antilog10_32x32    , "scl_antilog10_32x32.seq"  , 0 );
        DO_TEST( &scl_antilog2_24x24     , "scl_antilog2_24x24.seq"   , 0 );
        DO_TEST( &scl_antilogn_24x24     , "scl_antilogn_24x24.seq"   , 0 );
        DO_TEST( &scl_antilog10_24x24    , "scl_antilog10_24x24.seq"  , 0 );
        DO_TEST( &scl_sqrt32x32          , "scl_sqrt32x32.seq"        , 0 );
        DO_TEST( &scl_sqrt24x24          , "scl_sqrt24x24.seq"        , 0 );
        DO_TEST( &scl_sine32x32          , "scl_sine32x32.seq"        , 0 );
        DO_TEST( &scl_cosine32x32        , "scl_cosine32x32.seq"      , 0 );
        DO_TEST( &scl_sine24x24          , "scl_sine24x24.seq"        , 0 );
        DO_TEST( &scl_cosine24x24        , "scl_cosine24x24.seq"      , 0 );
        DO_TEST( &scl_atan32x32          , "scl_atan32x32.seq"        , 0 );
        DO_TEST( &scl_atan24x24          , "scl_atan24x24.seq"        , 0 );
        DO_TEST( &scl_tan32x32           , "scl_tan32x32.seq"         , 0 );
        DO_TEST( &scl_tan24x24           , "scl_tan24x24.seq"         , 0 );
        DO_TEST( &scl_bexp32             , "scl_bexp32.seq"           , 0 );
        DO_TEST( &scl_bexp24             , "scl_bexp24.seq"           , 0 );
        DO_TEST( &scl_bexp16             , "scl_bexp16.seq"           , 0 );

        DO_TEST( &vec_divide16x16        , "vec_divide16x16.seq"      , 0 );
        DO_TEST( &vec_divide32x32        , "vec_divide32x32.seq"      , 0 );
        DO_TEST( &vec_divide24x24        , "vec_divide24x24.seq"      , 0 );
        DO_TEST( &vec_divide16x16_fast   , "vec_divide16x16_fast.seq" , 0 );
        DO_TEST( &vec_divide32x32_fast   , "vec_divide32x32_fast.seq" , 0 );
        DO_TEST( &vec_divide24x24_fast   , "vec_divide24x24_fast.seq" , 0 );
        DO_TEST( &vec_recip16x16         , "vec_recip16x16.seq"       , 0 );
        DO_TEST( &vec_recip32x32         , "vec_recip32x32.seq"       , 0 );
        DO_TEST( &vec_recip24x24         , "vec_recip24x24.seq"       , 0 );
        DO_TEST( &vec_log2_32x32         , "vec_log2_32x32.seq"       , 0 );
        DO_TEST( &vec_logn_32x32         , "vec_logn_32x32.seq"       , 0 );
        DO_TEST( &vec_log10_32x32        , "vec_log10_32x32.seq"      , 0 );
        DO_TEST( &vec_log2_24x24         , "vec_log2_24x24.seq"       , 0 );
        DO_TEST( &vec_logn_24x24         , "vec_logn_24x24.seq"       , 0 );
        DO_TEST( &vec_log10_24x24        , "vec_log10_24x24.seq"      , 0 );
        DO_TEST( &vec_antilog2_32x32     , "vec_antilog2_32x32.seq"   , 0 );
        DO_TEST( &vec_antilogn_32x32     , "vec_antilogn_32x32.seq"   , 0 );
        DO_TEST( &vec_antilog10_32x32    , "vec_antilog10_32x32.seq"  , 0 );
        DO_TEST( &vec_antilog10_24x24    , "vec_antilog10_24x24.seq"  , 0 );
        DO_TEST( &vec_antilog2_24x24     , "vec_antilog2_24x24.seq"   , 0 );
        DO_TEST( &vec_antilogn_24x24     , "vec_antilogn_24x24.seq"   , 0 );
        DO_TEST( &vec_sqrt32x32_fast     , "vec_sqrt32x32_fast.seq"   , 0 );
        DO_TEST( &vec_sqrt24x24_fast     , "vec_sqrt24x24_fast.seq"   , 0 );
        DO_TEST( &vec_sqrt32x32          , "vec_sqrt32x32.seq"        , 0 );
        DO_TEST( &vec_sqrt24x24          , "vec_sqrt24x24.seq"        , 0 );
        DO_TEST( &vec_sine32x32          , "vec_sine32x32.seq"        , 0 );
        DO_TEST( &vec_cosine32x32        , "vec_cosine32x32.seq"      , 0 );
        DO_TEST( &vec_sine24x24          , "vec_sine24x24.seq"        , 0 );
        DO_TEST( &vec_cosine24x24        , "vec_cosine24x24.seq"      , 0 );
        DO_TEST( &vec_sine32x32_fast     , "vec_sine32x32_fast.seq"   , 0 );
        DO_TEST( &vec_cosine32x32_fast   , "vec_cosine32x32_fast.seq" , 0 );
        DO_TEST( &vec_sine24x24_fast     , "vec_sine24x24_fast.seq"   , 0 );
        DO_TEST( &vec_cosine24x24_fast   , "vec_cosine24x24_fast.seq" , 0 );
        DO_TEST( &vec_atan32x32          , "vec_atan32x32.seq"        , 0 );
        DO_TEST( &vec_atan24x24          , "vec_atan24x24.seq"        , 0 );
        DO_TEST( &vec_tan32x32           , "vec_tan32x32.seq"         , 0 );
        DO_TEST( &vec_tan24x24           , "vec_tan24x24.seq"         , 0 );
        DO_TEST( &vec_bexp32             , "vec_bexp32.seq"           , 0 );
        DO_TEST( &vec_bexp24             , "vec_bexp24.seq"           , 0 );
        DO_TEST( &vec_bexp16             , "vec_bexp16.seq"           , 0 );
        DO_TEST( &vec_bexp32_fast        , "vec_bexp32_fast.seq"      , 0 );
        DO_TEST( &vec_bexp24_fast        , "vec_bexp24_fast.seq"      , 0 );
        DO_TEST( &vec_bexp16_fast        , "vec_bexp16_fast.seq"      , 0 );
  }

  /*
   * Stage 2
   */

  if ( phaseNum == 0 || phaseNum == 2 )
  {
        DO_TEST    ( &scl_bexpf         , "scl_bexpf.seq"           , 0 );
        DO_TEST    ( &scl_int2float     , "scl_int2float.seq"       , 0 );
        DO_TEST    ( &scl_float2int     , "scl_float2int.seq"       , 0 );
        DO_TEST    ( &scl_complex2mag   , "scl_complex2mag.seq"     , 0 );
        DO_TEST    ( &scl_complex2invmag, "scl_complex2invmag.seq"  , 0 );
        DO_TEST_EXT( &scl_sinef         , "scl_sinef.seq"           , 0 );
        DO_TEST_EXT( &scl_cosinef       , "scl_cosinef.seq"         , 0 );
        DO_TEST_EXT( &scl_tanf          , "scl_tanf.seq"            , 0 );
        DO_TEST_EXT( &scl_log2f         , "scl_log2f.seq"           , 0 );
        DO_TEST_EXT( &scl_lognf         , "scl_lognf.seq"           , 0 );
        DO_TEST_EXT( &scl_log10f        , "scl_log10f.seq"          , 0 );
        DO_TEST_EXT( &scl_antilog2f     , "scl_antilog2f.seq"       , 0 );
        DO_TEST_EXT( &scl_antilognf     , "scl_antilognf.seq"       , 0 );
        DO_TEST_EXT( &scl_antilog10f    , "scl_antilog10f.seq"      , 0 );
        DO_TEST_EXT( &scl_atanf         , "scl_atanf.seq"           , 0 );
        DO_TEST_EXT( &scl_atan2f        , "scl_atan2f.seq"          , 0 );
        DO_TEST    ( &vec_bexpf         , "vec_bexpf.seq"           , 0 );
        DO_TEST    ( &vec_int2float     , "vec_int2float.seq"       , 0 );
        DO_TEST    ( &vec_float2int     , "vec_float2int.seq"       , 0 );
        DO_TEST    ( &vec_complex2mag   , "vec_complex2mag.seq"     , 0 );
        DO_TEST    ( &vec_complex2invmag, "vec_complex2invmag.seq"  , 0 );
        DO_TEST    ( &vec_sinef         , "vec_sinef.seq"           , 0 );
        DO_TEST    ( &vec_cosinef       , "vec_cosinef.seq"         , 0 );
        DO_TEST    ( &vec_tanf          , "vec_tanf.seq"            , 0 );
        DO_TEST    ( &vec_log2f         , "vec_log2f.seq"           , 0 );
        DO_TEST    ( &vec_lognf         , "vec_lognf.seq"           , 0 );
        DO_TEST    ( &vec_log10f        , "vec_log10f.seq"          , 0 );
        DO_TEST    ( &vec_antilog2f     , "vec_antilog2f.seq"       , 0 );
        DO_TEST    ( &vec_antilognf     , "vec_antilognf.seq"       , 0 );
        DO_TEST    ( &vec_antilog10f    , "vec_antilog10f.seq"      , 0 );
        DO_TEST    ( &vec_atanf         , "vec_atanf.seq"           , 0 );
        DO_TEST    ( &vec_atan2f        , "vec_atan2f.seq"          , 0 );
  }
  /* Phase 3 */
  if ( phaseNum == 0 || phaseNum == 3 )
  {
        DO_TEST    ( &scl_atan16x16      , "scl_atan16x16.seq"      , 0 );
        DO_TEST    ( &scl_atan2_16x16    , "scl_atan2_16x16.seq"    , 0 );
        DO_TEST    ( &scl_log2_16x16     , "scl_log2_16x16.seq"     , 0 );
        DO_TEST    ( &scl_logn_16x16     , "scl_logn_16x16.seq"     , 0 );
        DO_TEST    ( &scl_log10_16x16    , "scl_log10_16x16.seq"    , 0 );
        DO_TEST    ( &scl_antilog2_16x16 , "scl_antilog2_16x16.seq" , 0 );
        DO_TEST    ( &scl_antilogn_16x16 , "scl_antilogn_16x16.seq" , 0 );
        DO_TEST    ( &scl_antilog10_16x16, "scl_antilog10_16x16.seq", 0 );
        DO_TEST    ( &scl_sine16x16      , "scl_sine16x16.seq"      , 0 );
        DO_TEST    ( &scl_cosine16x16    , "scl_cosine16x16.seq"    , 0 );
        DO_TEST    ( &scl_tan16x16       , "scl_tan16x16.seq"       , 0 );
        DO_TEST    ( &scl_sqrt16x16      , "scl_sqrt16x16.seq"      , 0 );
        DO_TEST    ( &scl_dividef        , "scl_dividef.seq"        , 0 );
        DO_TEST    ( &scl_recipf         , "scl_recipf.seq"         , 0 );
        DO_TEST_EXT( &scl_asinf          , "scl_asinf.seq"          , 0 );
        DO_TEST_EXT( &scl_acosf          , "scl_acosf.seq"          , 0 );
        DO_TEST_EXT( &scl_sqrtf          , "scl_sqrtf.seq"          , TE_ERRH_IGNORE_FE_INEXACT );
        DO_TEST_EXT( &scl_rsqrtf         , "scl_rsqrtf.seq"         , TE_ERRH_IGNORE_FE_INEXACT );
#if 0//XCHAL_HAVE_HIFI1_VFPU /*disabled: sNaN qNaN conflict for DEBUG mode [TENX-59250]*/
        DO_TEST    ( &scl_float2floor    , "scl_float2floor.seq"    , 0 );
        DO_TEST    ( &scl_float2ceil     , "scl_float2ceil.seq"     , 0 );
#else
        DO_TEST    ( &scl_float2floor    , "scl_float2floor_nospl.seq"    , 0 );
        DO_TEST    ( &scl_float2ceil     , "scl_float2ceil_nospl.seq"     , 0 );
#endif

        DO_TEST( &vec_atan16x16          , "vec_atan16x16.seq"      , 0 );
        DO_TEST( &vec_atan2_16x16        , "vec_atan2_16x16.seq"    , 0 );
        DO_TEST( &vec_log2_16x16         , "vec_log2_16x16.seq"     , 0 );
        DO_TEST( &vec_logn_16x16         , "vec_logn_16x16.seq"     , 0 );
        DO_TEST( &vec_log10_16x16        , "vec_log10_16x16.seq"    , 0 );
        DO_TEST( &vec_antilog2_16x16     , "vec_antilog2_16x16.seq" , 0 );
        DO_TEST( &vec_antilogn_16x16     , "vec_antilogn_16x16.seq" , 0 );
        DO_TEST( &vec_antilog10_16x16    , "vec_antilog10_16x16.seq", 0 );
        DO_TEST( &vec_sine16x16          , "vec_sine16x16.seq"      , 0 );
        DO_TEST( &vec_cosine16x16        , "vec_cosine16x16.seq"    , 0 );
        DO_TEST( &vec_tan16x16           , "vec_tan16x16.seq"       , 0 );
        DO_TEST( &vec_sqrt16x16          , "vec_sqrt16x16.seq"      , 0 );
        DO_TEST( &vec_dividef            , "vec_dividef.seq"        , 0 );
        DO_TEST( &vec_recipf             , "vec_recipf.seq"         , 0 );
        DO_TEST( &vec_asinf              , "vec_asinf.seq"          , 0 );
        DO_TEST( &vec_acosf              , "vec_acosf.seq"          , 0 );
        DO_TEST( &vec_sqrtf              , "vec_sqrtf.seq"          , 0 );
        DO_TEST( &vec_rsqrtf             , "vec_rsqrtf.seq"         , 0 );
#if 0//XCHAL_HAVE_HIFI1_VFPU /*disabled: sNaN qNaN conflict for DEBUG mode [TENX-59250]*/
        DO_TEST( &vec_float2floor        , "vec_float2floor.seq"    , 0 );
        DO_TEST( &vec_float2ceil         , "vec_float2ceil.seq"     , 0 );
#else
        DO_TEST( &vec_float2floor        , "vec_float2floor_nospl.seq"    , 0 );
        DO_TEST( &vec_float2ceil         , "vec_float2ceil_nospl.seq"     , 0 );
#endif
  }

  return (res);

} /* main_vec() */

/* Test executive function. Performs the specified test on a brief or full version
 * of the designated SEQ-file. Return the test result (non-zero if passed). */
static int testExec( tTestEngTarget   targetFxn, const char * seqName, 
              int errhExtendedTest, int isFull, int isVerbose, int breakOnError )
{
  #define MAX_FUNC_NUM   16
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
      FUNC_LIST( (tTestEngTarget)&vec_sine32x32,     (tTestEngTarget)&vec_cosine32x32,   (tTestEngTarget)&vec_tan32x32,
                 (tTestEngTarget)&vec_log2_32x32,    (tTestEngTarget)&vec_logn_32x32,    (tTestEngTarget)&vec_log10_32x32,
                 (tTestEngTarget)&vec_antilog2_32x32,(tTestEngTarget)&vec_antilogn_32x32,(tTestEngTarget)&vec_antilog10_32x32),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_atan32x32,     (tTestEngTarget)&vec_atan24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_sine32x32_fast,(tTestEngTarget)&vec_cosine32x32_fast,   
                 (tTestEngTarget)&vec_sine24x24_fast,(tTestEngTarget)&vec_cosine24x24_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_YES, &te_loadFxn_vXvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_sqrt32x32,     (tTestEngTarget)&vec_sqrt24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_sqrt32x32_fast,(tTestEngTarget)&vec_sqrt24x24_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_YES, &te_loadFxn_vXvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_sine24x24,     (tTestEngTarget)&vec_cosine24x24,   (tTestEngTarget)&vec_tan24x24,
                 (tTestEngTarget)&vec_log2_24x24,    (tTestEngTarget)&vec_logn_24x24,    (tTestEngTarget)&vec_log10_24x24,
                 (tTestEngTarget)&vec_antilog2_24x24,(tTestEngTarget)&vec_antilogn_24x24,(tTestEngTarget)&vec_antilog10_24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_sine32x32,     (tTestEngTarget)&scl_cosine32x32,   (tTestEngTarget)&scl_tan32x32,
                 (tTestEngTarget)&scl_log2_32x32,    (tTestEngTarget)&scl_logn_32x32,    (tTestEngTarget)&scl_log10_32x32,
                 (tTestEngTarget)&scl_antilog2_32x32,(tTestEngTarget)&scl_antilogn_32x32,(tTestEngTarget)&scl_antilog10_32x32),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &processFxn_scl_vXvZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_atan32x32,     (tTestEngTarget)&scl_atan24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &processFxn_scl_vXvZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_sqrt32x32, (tTestEngTarget)&scl_sqrt24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &processFxn_scl_vXvZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_sine24x24,     (tTestEngTarget)&scl_cosine24x24,   (tTestEngTarget)&scl_tan24x24,
                 (tTestEngTarget)&scl_log2_24x24,    (tTestEngTarget)&scl_logn_24x24,    (tTestEngTarget)&scl_log10_24x24,
                 (tTestEngTarget)&scl_antilog2_24x24,(tTestEngTarget)&scl_antilogn_24x24,(tTestEngTarget)&scl_antilog10_24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &processFxn_scl_vXvZ ) },

    { 
      FUNC_LIST( (tTestEngTarget)&vec_bexp32  ,(tTestEngTarget)&vec_bexp24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsZ  , &te_processFxn_vXsZ ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_bexp32_fast  ,(tTestEngTarget)&vec_bexp24_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXsZ  , &te_processFxn_vXsZ ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_bexp16),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsZ  , &te_processFxn_vXsZ ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_bexp16_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_YES, &te_loadFxn_vXsZ  , &te_processFxn_vXsZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_bexp32, (tTestEngTarget)&scl_bexp24 ),
      TEST_DESC( FMT_FRACT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsZ, &processFxn_scl_vXvZ32 ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_bexp16),
      TEST_DESC( FMT_FRACT16, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsZ32, &processFxn_scl_vXvZ32 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_recip16x16),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZf, &te_processFxn_recip16x16 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_recip32x32, (tTestEngTarget)&vec_recip24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZd, &te_processFxn_recip32x32 ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_recip16x16),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZf, &te_processFxn_scl_recip16x16 ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_recip32x32,(tTestEngTarget)&scl_recip24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZd, &te_processFxn_scl_recip32x32 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_divide16x16),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvYvZf, &te_processFxn_divide16x16 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_divide16x16_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_YES, &te_loadFxn_vXvYvZf, &te_processFxn_divide16x16 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_divide32x32, (tTestEngTarget)&vec_divide24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvYvZd, &te_processFxn_divide32x32 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_divide32x32_fast, (tTestEngTarget)&vec_divide24x24_fast),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_YES, &te_loadFxn_vXvYvZd, &te_processFxn_divide32x32 ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_divide16x16),
      TEST_DESC( FMT_REAL|FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvYvZf, &te_processFxn_scl_divide16x16 ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_divide32x32,(tTestEngTarget)&scl_divide24x24),
      TEST_DESC( FMT_REAL|FMT_FRACT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvYvZd, &te_processFxn_scl_divide32x32 ) },

    /*
     * Stage 2
     */
#if 1
    {
      FUNC_LIST( (tTestEngTarget)&scl_bexpf),
      TEST_DESC( FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsZ32, &processFxn_scl_vXvZ32 ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_int2float),
      TEST_DESC( FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vX32sY32vZ, &processFxn_scl_vX32sY32vZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_float2int),
      TEST_DESC( FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsY32vZ32, &processFxn_scl_vXsY32vZ32 ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_bexpf),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsZ32  , &te_processFxn_vXsZ32 ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_int2float ),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vX32sY32vZ, &te_processFxn_vZvXsY32 ) },
    { 
      FUNC_LIST( (tTestEngTarget)&vec_float2int ),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_2, TE_ALIGN_NO, &te_loadFxn_vXsY32vZ32, &te_processFxn_vZvXsY32 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_complex2mag,(tTestEngTarget)&vec_complex2invmag ),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXcvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_complex2mag,(tTestEngTarget)&scl_complex2invmag ),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXcvZ, &processFxn_scl_vXcvZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_sinef,(tTestEngTarget)&vec_cosinef,(tTestEngTarget)&vec_tanf,
                 (tTestEngTarget)&vec_log2f,(tTestEngTarget)&vec_lognf,(tTestEngTarget)&vec_log10f,
                 (tTestEngTarget)&vec_atanf,(tTestEngTarget)&vec_antilog2f,(tTestEngTarget)&vec_antilognf,
                 (tTestEngTarget)&vec_antilog10f),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_sinef,(tTestEngTarget)&scl_cosinef,(tTestEngTarget)&scl_tanf,
                 (tTestEngTarget)&scl_log2f,(tTestEngTarget)&scl_lognf,(tTestEngTarget)&scl_log10f,
                 (tTestEngTarget)&scl_atanf,(tTestEngTarget)&scl_antilog2f,(tTestEngTarget)&scl_antilognf,
                 (tTestEngTarget)&scl_antilog10f),
      TEST_DESC( FMT_REAL|FMT_FLOAT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &processFxn_scl_vXvZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_atan2f ),
      TEST_DESC( FMT_FLOAT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvYvZ, &processFxn_scl_atan2 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_atan2f ),
      TEST_DESC( FMT_FLOAT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvYvZ, &te_processFxn_vZvYvX ) },

#endif
      /* Phase 3 */
    {
      FUNC_LIST( (tTestEngTarget)&vec_sqrt16x16,
                 (tTestEngTarget)&vec_sine16x16, (tTestEngTarget)&vec_cosine16x16,
                 (tTestEngTarget)&vec_antilog2_16x16, (tTestEngTarget)&vec_antilogn_16x16, (tTestEngTarget)&vec_antilog10_16x16,
                 (tTestEngTarget)&vec_log2_16x16, (tTestEngTarget)&vec_logn_16x16, (tTestEngTarget)&vec_log10_16x16),
      TEST_DESC( FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_tan16x16),
      TEST_DESC( FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_atan16x16),
      TEST_DESC( FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_sqrtf, (tTestEngTarget)&vec_rsqrtf, 
                 (tTestEngTarget)&vec_recipf, 
                 (tTestEngTarget)&vec_asinf, (tTestEngTarget)&vec_acosf,
                 (tTestEngTarget)&vec_float2floor, (tTestEngTarget)&vec_float2ceil),
      TEST_DESC( FMT_FLOAT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &te_processFxn_vZvX ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_dividef),
      TEST_DESC( FMT_FLOAT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvYvZ, &te_processFxn_vZvXvY ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_sqrt16x16,
                 (tTestEngTarget)&scl_sine16x16, (tTestEngTarget)&scl_cosine16x16,
                 (tTestEngTarget)&scl_antilog2_16x16, (tTestEngTarget)&scl_antilogn_16x16, (tTestEngTarget)&scl_antilog10_16x16,
                 (tTestEngTarget)&scl_log2_16x16, (tTestEngTarget)&scl_logn_16x16, (tTestEngTarget)&scl_log10_16x16),
      TEST_DESC( FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &processFxn_scl_vXvZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_tan16x16),
      TEST_DESC( FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &processFxn_scl_vXvZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_atan16x16),
      TEST_DESC( FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &processFxn_scl_vXvZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_dividef),
      TEST_DESC( FMT_FLOAT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvYvZ, &processFxn_scl_dividef ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_sqrtf, (tTestEngTarget)&scl_rsqrtf,
                 (tTestEngTarget)&scl_recipf,
                 (tTestEngTarget)&scl_asinf, (tTestEngTarget)&scl_acosf, 
                 (tTestEngTarget)&scl_float2floor, (tTestEngTarget)&scl_float2ceil),
      TEST_DESC( FMT_FLOAT32, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvZ, &processFxn_scl_vXvZ ) },
    {
      FUNC_LIST( (tTestEngTarget)&scl_atan2_16x16 ),
      TEST_DESC( FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvYvZ, &processFxn_scl_atan2 ) },
    {
      FUNC_LIST( (tTestEngTarget)&vec_atan2_16x16 ),
      TEST_DESC( FMT_FRACT16, TE_ARGNUM_1, TE_ALIGN_NO, &te_loadFxn_vXvYvZ, &te_processFxn_vZvYvX ) },

    { 
      FUNC_LIST( NULL ), TEST_DESC(  0, 0, 0, NULL, NULL ) } /* End of table */
  };

  {
    int tblIx, funcIx;

    for ( tblIx=0; tblIx<(int)sizeof(testDefTbl)/sizeof(testDefTbl[0]); tblIx++ )
    {
      for ( funcIx=0; funcIx<MAX_FUNC_NUM; funcIx++ )
      {
        if ( targetFxn == testDefTbl[tblIx].funcList[funcIx] )
        {
          tTestEngDesc testDesc = testDefTbl[tblIx].testDesc;
          testDesc.extraParam = (uint32_t)errhExtendedTest;

          return ( TestEngRun( targetFxn, &testDesc, 
                               seqName, isFull, 
                               isVerbose, breakOnError ) );
        }
      }
    }

    ASSERT( !"Test not defined" );
    return (0);
  }
  return te_Exec(testDefTbl, sizeof(testDefTbl) / sizeof(testDefTbl[0]), MAX_FUNC_NUM, targetFxn, seqName, isFull, isVerbose, breakOnError);


} /* testExec() */
