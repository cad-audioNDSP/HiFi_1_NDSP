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
 * Test data vectors tools and SEQ-file reader
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Memory allocator w/ alignment support. */
#include "malloc16.h"
/* Fixed point arithmetics. */
#include "NatureDSP_Math.h"
/* Test data vector API. */
#include "vectools.h"

#define MIN(a,b)    ( (a)<(b) ? (a) : (b) )
#define MAX(a,b)    ( (a)>(b) ? (a) : (b) )

#define MEM_ALIGN   16    /* Memory block alignment, in bytes. */

static const uint8_t guardPattern[2*MEM_ALIGN] =
{
  (uint8_t)0xd0,(uint8_t)0xe7,(uint8_t)0x20,(uint8_t)0xe9,
  (uint8_t)0xa1,(uint8_t)0x18,(uint8_t)0x47,(uint8_t)0x8c,
  (uint8_t)0xf5,(uint8_t)0xf7,(uint8_t)0x28,(uint8_t)0xf8,
  (uint8_t)0x5d,(uint8_t)0x7c,(uint8_t)0xcc,(uint8_t)0x24,
  (uint8_t)0x6b,(uint8_t)0xea,(uint8_t)0xca,(uint8_t)0xf5,
  (uint8_t)0xa7,(uint8_t)0x09,(uint8_t)0xd9,(uint8_t)0xef,
  (uint8_t)0xad,(uint8_t)0xc1,(uint8_t)0xbe,(uint8_t)0x64,
  (uint8_t)0xa7,(uint8_t)0x2b,(uint8_t)0xb4,(uint8_t)0x08
};

/* Read file and discard space and comments until some text is encountered. 
 * Comments start with a semicolon and span to the end of a line. */
static void skipComments( FILE * f );

typedef int tReadFxn( void * dst, FILE * f );

/* Skip spaces and read an int value from the file. Verify the value against
 * min/max bounds. Return zero if failed. */
static int read_int( int * dst, int min, int max, FILE * f );

/* Skip spaces and read a value from the file. Return zero if failed. */
static tReadFxn read_i16, read_i32, read_i64, read_fl32, read_fl64;

typedef int tCheckRangeFxn( const void * px, const void * pmin, const void * pmax );

/* Check if the value belongs to the closed range [min,max]. Return zero if
 * the tested value lies outside the range. */
static tCheckRangeFxn checkRange_i16, checkRange_i32, checkRange_i64, checkRange_fl32, checkRange_fl64;

/* Allocate memory for a single vector. Last argument, if not zero, is a pointer
 * to vector initialization data. Function returns zero if failed. */
int vecAlloc( tVec * vec, int nElem, int isAligned, int fmt, const void * initData )
{
  tAllocPtr ptr;
  size_t szElem, szBulk, szL, szR, unalignPad;
  int _nElem, offset, res = 0;

  ASSERT( vec );

  szElem = ( fmt == (FMT_REAL|FMT_INT16  ) ? sizeof( int16_t         ) :
             fmt == (FMT_REAL|FMT_INT32  ) ? sizeof( int32_t         ) :
             fmt == (FMT_REAL|FMT_INT64  ) ? sizeof( int64_t         ) :
             fmt == (FMT_REAL|FMT_FRACT16) ? sizeof( fract16         ) :
             fmt == (FMT_REAL|FMT_FRACT32) ? sizeof( fract32         ) :
             fmt == (FMT_REAL|FMT_FLOAT32) ? sizeof( float32_t       ) :
             fmt == (FMT_REAL|FMT_FLOAT64) ? sizeof( float64_t       ) :
             fmt == (FMT_CPLX|FMT_FRACT16) ? sizeof( complex_fract16 ) :
             fmt == (FMT_CPLX|FMT_FRACT32) ? sizeof( complex_fract32 ) :
             fmt == (FMT_CPLX|FMT_FLOAT32) ? sizeof( complex_float   ) :
             fmt == (FMT_CPLX|FMT_FLOAT64) ? sizeof( complex_double  ) : 0 );

  if ( szElem > 0 )
  {
    ASSERT( szElem & ~(szElem-1) );
    /* Negative vector length must be supported! */
    _nElem = ( nElem >= 0 ? nElem : 0 );
    /* Unaligned access is meaningless whenever the data item size is a
     * multiple of alignment size. */
    isAligned = ( isAligned || !( szElem & (MEM_ALIGN-1) ) );
    /* For unaligned buffers, use random offset for the buffer origin. */
    unalignPad = ( isAligned ? 0 : ( rand() & ( (MEM_ALIGN-1) & ~(szElem-1) ) ) );
    /* Left guard area size */
    szL = MEM_ALIGN + unalignPad;
    /* Right guard area size */
    szR = MEM_ALIGN + ( ( -( _nElem + (!isAligned) )*szElem ) & (MEM_ALIGN-1) );
    /* Total memory block size (guard areas + data) */
    szBulk = szL + _nElem*szElem + szR;
    /* Offset of the first data element */
    offset = szL/szElem;

    if ( !( res = ( 0 != mallocAlign( &ptr, szBulk, MEM_ALIGN ) ) ) )
    {
      printf( "vecAlloc(): mallocAlign failed to allocate %d bytes\n", (int)szBulk );
    }
  }
  else
  {
    ASSERT( !"Unknown data format!" );
  }

  if ( res )
  {
    ASSERT( MAX( szL, szR ) <= sizeof(guardPattern) );
    /* Initialize the left guard zone with special pattern. */
    memcpy( ptr.aligned, guardPattern, szL );
    /* Initialize the right guard zone. */
    memcpy( (uint8_t*)ptr.aligned + szL + _nElem*szElem, guardPattern, szR );
    /* Optionally initialize the vector data. */
    if ( initData )
    {
      memcpy( (uint8_t*)ptr.aligned + offset*szElem, initData, _nElem*szElem );
    }

    vec->ptr    = ptr;
    vec->nElem  = nElem;
    vec->fmt    = fmt;
    vec->offset = offset;
    vec->szElem = szElem;
    vec->szBulk = szBulk;
  }

  return (res);

} /* vecAlloc() */

/* Allocate memory for a few vectors of the same data format. Repeat last two
 * arguments for each new vector instance. The argument list must end up 
 * with zero pointer. Returns the number of successfully allocated vectors. */
int vecsAlloc( int isAligned, int fmt, tVec * vec, int nElem, ... )
{
  va_list args;
  int vecNum = 0;

  ASSERT( vec );

  va_start( args, nElem );

  do
  {
    if ( vecAlloc( vec, nElem, isAligned, fmt, 0 ) )
    {
      if ( ( vec = va_arg( args, tVec* ) ) ) nElem = va_arg( args, int );

      vecNum++;
    }
    else
    { 
      printf( "vecsAlloc(): vecAlloc failed for vector #%d\n", vecNum );
      break;
    }

  } while ( vec );

  va_end( args );

  return (vecNum);

} /* vecsAlloc() */

/* Free memory for a single vector. Return zero if guard memory areas check fails. */
int vecFree( tVec * vec )
{
  size_t szL, szR;
  int _nElem, res;

  ASSERT( vec && vec->szBulk > 0 );

  /* Negative vector length must be supported! */
  _nElem = ( vec->nElem >= 0 ? vec->nElem : 0 );
  /* Left guard area size. */
  szL = vec->offset * vec->szElem;
  /* Right guard area size. */
  szR = vec->szBulk - ( vec->offset + _nElem )*vec->szElem;
  /* Check integrity of left and right guard areas. */
  res = !memcmp( (uint8_t*)vec->ptr.aligned, guardPattern, szL ) &&
        !memcmp( (uint8_t*)vec->ptr.aligned + szL + _nElem*vec->szElem, guardPattern, szR );

  if ( !res )
  {
    printf( "vecFree(): guard area(s) appear damaged\n" );
  }

  /* Free the memory block. */
  freeAlign( &vec->ptr );
  memset( &vec->ptr, 0, sizeof( vec->ptr ) );
  vec->szBulk = 0;

  return (res);

} /* vecFree() */

/* Free memory for a list of vectors. The last argument must be zero pointer.
 * Return zero if guard memory areas check fails for any vector. */
int vecsFree( tVec * vec, ... )
{
  va_list args;
  int res = 1;

  va_start( args, vec );

  do
  {
    res &= ( vecFree( vec ) != 0 );
    vec = va_arg( args, tVec* );

  } while ( vec );

  va_end( args );

  return (res);

} /* vecsFree() */

/* Return the total size of vector data items, in bytes. */
size_t vecGetSize( const tVec * vec )
{
  ASSERT( vec && vec->szBulk>0 );

  return ( vec->nElem*vec->szElem );

} /* vecGetSize() */

/* Return a pointer to element vec[n]. */
void * vecGetElem( const tVec * vec, int n )
{
  ASSERT( vec && vec->szBulk>0 && n>=0 && ( n<vec->nElem || !vec->nElem ) );

  return ( (uint8_t*)vec->ptr.aligned + ( vec->offset + n )*vec->szElem );

} /* vecGetElem() */

/* Return a pointer to element vec[n] with appropriate type casting. */
int16_t * vecGetElem_i16( const tVec * vec, int n )
{
  ASSERT( vec && vec->fmt == ( FMT_INT16 | FMT_REAL ) );
  return (int16_t *)( vecGetElem( vec, n ) );
}
int32_t * vecGetElem_i32( const tVec * vec, int n )
{
  ASSERT( vec && vec->fmt == ( FMT_INT32 | FMT_REAL ) );
  return (int32_t *)( vecGetElem( vec, n ) );
}
int64_t * vecGetElem_i64( const tVec * vec, int n )
{
  ASSERT( vec && vec->fmt == ( FMT_INT64 | FMT_REAL ) );
  return (int64_t *)( vecGetElem( vec, n ) );
}
fract16 * vecGetElem_fr16( const tVec * vec, int n )
{
  ASSERT( vec && vec->fmt == ( FMT_FRACT16 | FMT_REAL ) );
  return (fract16 *)( vecGetElem( vec, n ) );
}
fract32 * vecGetElem_fr32( const tVec * vec, int n )
{
  ASSERT( vec && vec->fmt == ( FMT_FRACT32 | FMT_REAL ) );
  return (fract32 *)( vecGetElem( vec, n ) );
}
float32_t * vecGetElem_fl32( const tVec * vec, int n )
{
  ASSERT( vec && vec->fmt == ( FMT_FLOAT32 | FMT_REAL ) );
  return (float32_t *)( vecGetElem( vec, n ) );
}
float64_t * vecGetElem_fl64( const tVec * vec, int n )
{
  ASSERT( vec && vec->fmt == ( FMT_FLOAT64 | FMT_REAL ) );
  return (float64_t *)( vecGetElem( vec, n ) );
}
complex_float * vecGetElem_fl32c( const tVec * vec, int n )
{
  ASSERT( vec && vec->fmt == ( FMT_FLOAT32 | FMT_CPLX ) );
  return (complex_float *)( vecGetElem( vec, n ) );
}
complex_double * vecGetElem_fl64c( const tVec * vec, int n )
{
  ASSERT( vec && vec->fmt == ( FMT_FLOAT64 | FMT_CPLX ) );
  return (complex_double *)( vecGetElem( vec, n ) );
}
complex_fract16 * vecGetElem_fr16c( const tVec * vec, int n )
{
  ASSERT( vec && vec->fmt == ( FMT_FRACT16 | FMT_CPLX ) );
  return (complex_fract16 *)( vecGetElem( vec, n ) );
}
complex_fract32 * vecGetElem_fr32c( const tVec * vec, int n )
{
  ASSERT( vec && vec->fmt == ( FMT_FRACT32 | FMT_CPLX ) );
  return (complex_fract32 * )( vecGetElem( vec, n ) );
}

static void printFloat32(char* str, const float32_t* x)
{
    const uint32_t* y = (const uint32_t*)x;
    if ( isnan(x[0]) )
    {
        int is_snan = ( 0 == ( y[0] & (1<<22) ) );
        sprintf(str,"%s[0x%08x]", ( is_snan ? "sNaN" : "qNaN" ), y[0] );
    }
    else
    {
        sprintf(str,"%g[0x%08x]", (double)x[0], y[0]);
    }
}

static void printFloat64(char* str, const float64_t * x)
{
    const uint32_t* y = (const uint32_t*)x;
    if ( isnan(x[0]) )
    {
        int is_snan = ( 0 == ( y[1] & (1<<(51-32)) ) );
        sprintf(str,"%s[0x%08x%08x]", ( is_snan ? "sNaN" : "qNaN" ), y[1], y[0] );
    }
    else
    {
        sprintf(str,"%g[0x%08x%08x]", x[0], y[1], y[0]);
    }
}

/* Print the velue of the specified element to a string buffer. */
void vecPrintElem( char * buf, const tVec * vec, int n )
{
  const void * p;

  ASSERT( vec && buf );

  p = vecGetElem( vec, n );

  switch ( vec->fmt )
  {
  case FMT_REAL|FMT_INT16  :
  case FMT_REAL|FMT_FRACT16: sprintf( buf, "%d", (int)*(int16_t*)p ); break;
  case FMT_REAL|FMT_INT32  :
  case FMT_REAL|FMT_FRACT32: sprintf( buf, "%d", (int)*(int32_t*)p ); break;
  case FMT_REAL|FMT_INT64  : sprintf( buf, "%lld", (long long)*(int64_t*)p ); break;
  case FMT_REAL|FMT_FLOAT32: printFloat32(buf,(float32_t*)p); break;
  case FMT_REAL|FMT_FLOAT64: printFloat64(buf,(float64_t*)p); break;
  case FMT_CPLX|FMT_FRACT16: sprintf( buf, "(%d,%d)", (int)*((int16_t*)p+0), (int)*((int16_t*)p+1) ); break;
  case FMT_CPLX|FMT_FRACT32: sprintf( buf, "(%d,%d)", (int)*((int32_t*)p+0), (int)*((int32_t*)p+1) ); break;
  case FMT_CPLX|FMT_FLOAT32:
  {
      char str1[40],str2[40];
      printFloat32(str1,((float32_t*)p+0)); 
      printFloat32(str2,((float32_t*)p+1));
      sprintf( buf, "(%s,%s)", str1,str2 ); break;
  }
  case FMT_CPLX|FMT_FLOAT64: 
  {
      char str1[40],str2[40];
      printFloat64(str1,((float64_t*)p+0)); 
      printFloat64(str2,((float64_t*)p+1));
      sprintf( buf, "(%s,%s)", str1,str2 ); break;
  }
  default: ASSERT( 0 );
  }

} /* vecPrintElem() */

/* Convert 16-bit PCM samples to vector data format. When the target format is
 * fractional (either Q15 or Q31), data are normalized and shifted to the right
 * by bexp bit positions so that block exponent of output data equals bexp.
 * Return the overall shift amount, with positive values corresponding to the
 * left shift. */
int vecFromPcm16( tVec * vec, const fract16 * x, int bexp )
{
  int n, N;
  int shift = 0;

  ASSERT( vec && x );

  N = ( ( vec->fmt & FMT_CPLX ) ? 2*vec->nElem : vec->nElem );

  switch ( vec->fmt & 15 )
  {
  case FMT_INT16:
    {
      memcpy( vecGetElem( vec, 0 ), x, N*sizeof(int16_t) );
      break;
    }
  case FMT_INT32:
    {
      int32_t * z = (int32_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ ) z[n] = x[n];
      break;
    }
  case FMT_INT64:
    {
      int64_t * z = (int64_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ ) z[n] = x[n];
      break;
    }
  case FMT_FRACT16:
    {
      fract16 * z = (fract16 *)vecGetElem( vec, 0 );
      int nsa;
      for ( nsa=15, n=0; n<N; n++ ) nsa = MIN( nsa, S_exp0_s(x[n]) );
      shift = nsa - bexp;
      for ( n=0; n<N; n++ ) z[n] = S_round_l( L_shl_l( x[n], 16+shift ) );
      break;
    }
  case FMT_FRACT32:
    {
      fract32 * z = (fract32 *)vecGetElem( vec, 0 );
      int nsa;
      for ( nsa=15, n=0; n<N; n++ ) nsa = MIN( nsa, S_exp0_s(x[n]) );
      shift = nsa - bexp;
      for ( n=0; n<N; n++ ) z[n] = L_shl_l( x[n], 16+shift );
      break;
    }
  case FMT_FLOAT32:
    {
      float32_t * z = (float32_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ ) z[n] = STDLIB_MATH(ldexpf)( x[n], -15 );
      break;
    }
  case FMT_FLOAT64:
    {
      float64_t * z = (float64_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ ) z[n] = STDLIB_MATH(ldexp)( x[n], -15 );
      break;
    }
  default:
    ASSERT(0);
  }

  return (shift);

} /* vecFromPcm16() */

/* Convert 16-bit PCM samples to vector data format, blockwise variant of 
 * vecFromPcm16() (see above). */
void bvecFromPcm16( tVec * vec, const fract16 * x, int N, int L, int bexp, int16_t shift[] )
{
  int n, l;

  ASSERT( vec && x );
  ASSERT( vec->nElem >= N*L );

  if ( vec->fmt & FMT_CPLX ) N <<= 1;

  if ( shift ) memset( shift, 0, sizeof(shift[0])*L );

  switch ( vec->fmt & 15 )
  {
  case FMT_INT16:
    {
      memcpy( vecGetElem( vec, 0 ), x, N*L*sizeof(int16_t) );
      break;
    }
  case FMT_INT32:
    {
      int32_t * z = (int32_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N*L; n++ ) z[n] = x[n];
      break;
    }
  case FMT_INT64:
    {
      int64_t * z = (int64_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N*L; n++ ) z[n] = x[n];
      break;
    }
  case FMT_FRACT16:
    {
      fract16 * z = (fract16 *)vecGetElem( vec, 0 );
      int nsa;
      ASSERT( shift );
      for ( l=0; l<L; l++, x+=N, z+=N, shift++ )
      {
        for ( nsa=15, n=0; n<N; n++ ) nsa = MIN( nsa, S_exp0_s(x[n]) );
        for ( n=0; n<N; n++ ) z[n] = S_round_l( L_shl_l( x[n], 16+nsa-bexp ) );
        *shift = (int16_t)( nsa - bexp );
      }
      break;
    }
  case FMT_FRACT32:
    {
      fract32 * z = (fract32 *)vecGetElem( vec, 0 );
      int nsa;
      ASSERT( shift );
      for ( l=0; l<L; l++, x+=N, z+=N, shift++ )
      {
        for ( nsa=15, n=0; n<N; n++ ) nsa = MIN( nsa, S_exp0_s(x[n]) );
        for ( n=0; n<N; n++ ) z[n] = L_shl_l( x[n], 16+nsa-bexp );
        *shift = (int16_t)( nsa - bexp );
      }
      break;
    }
  case FMT_FLOAT32:
    {
      float32_t * z = (float32_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N*L; n++ ) z[n] = STDLIB_MATH(ldexpf)( x[n], -15 );
      break;
    }
  case FMT_FLOAT64:
    {
      float64_t * z = (float64_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N*L; n++ ) z[n] = STDLIB_MATH(ldexp)( x[n], -15 );
      break;
    }
  default:
    ASSERT(0);
  }

} /* bvecFromPcm16() */

/* Convert 64-bit floating point values to vector data format. When the target
 * format is fractional, we assume either Q15 or Q31. */
void vecFromFp64( tVec * vec, const float64_t * x )
{
  int n, N;

  ASSERT( vec && x );

  N = ( ( vec->fmt & FMT_CPLX ) ? 2*vec->nElem : vec->nElem );

  switch ( vec->fmt & 15 )
  {
  case FMT_INT16:
    {
      int16_t * z = (int16_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ )
      {
        float64_t t = MAX( MIN_INT16, MIN( MAX_INT16, x[n]+0.5 ) );
        z[n] = (int16_t)STDLIB_MATH(floor)( t );
      }
      break;
    }
  case FMT_INT32:
    {
      int32_t * z = (int32_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ )
      {
        float64_t t = MAX( MIN_INT32, MIN( MAX_INT32, x[n]+0.5 ) );
        z[n] = (int32_t)STDLIB_MATH(floor)( t );
      }
      break;
    }
  case FMT_INT64:
    ASSERT( !"not implemented" );
    break;
  case FMT_FRACT16:
    {
      fract16 * z = (fract16 *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ )
      {
        float64_t t = STDLIB_MATH(ldexp)( x[n], 15 );
        t = MAX( MIN_INT16, MIN( MAX_INT16, round(t) ) );
        z[n] = (fract16)t;
      }
      break;
    }
  case FMT_FRACT32:
    {
      fract32 * z = (fract32 *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ )
      {
        float64_t t = STDLIB_MATH(ldexp)( x[n], 31 );
        t = MAX( MIN_INT32, MIN( MAX_INT32, round(t) ) );
        z[n] = (fract32)t;
      }
      break;
    }
  case FMT_FLOAT32:
    {
      float32_t * z = (float32_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ ) z[n] = (float32_t)x[n];
      break;
    }
  case FMT_FLOAT64:
    {
      memcpy( vecGetElem( vec, 0 ), x, N*sizeof(float64_t) );
      break;
    }
  default:
    ASSERT(0);
  }

} /* vecFromFp64() */

/* Convert vector data to 64-bit floating point numbers. If the source vector
 * is of a fractional format (we assume Q15 or Q31), then data are additionally
 * scaled by 2^-shift. */
void vecToFp64( float64_t * z, const tVec * vec, int shift )
{
  int n, N;

  ASSERT( z && vec );

  N = ( ( vec->fmt & FMT_CPLX ) ? 2*vec->nElem : vec->nElem );

  switch ( vec->fmt & 15 )
  {
  case FMT_INT16:
    {
      int16_t * x = (int16_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ ) z[n] = x[n];
      break;
    }
  case FMT_INT32:
    {
      int32_t * x = (int32_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ ) z[n] = x[n];
      break;
    }
  case FMT_INT64:
    ASSERT( !"not implemented" );
    break;
  case FMT_FRACT16:
    {
      fract16 * x = (fract16 *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ ) z[n] = STDLIB_MATH(ldexp)( x[n], -shift-15 );
      break;
    }
  case FMT_FRACT32:
    {
      fract32 * x =(fract32 *) vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ ) z[n] = STDLIB_MATH(ldexp)( x[n], -shift-31 );
      break;
    }
  case FMT_FLOAT32:
    {
      float32_t * x = (float32_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N; n++ ) z[n] = x[n];
      break;
    }
  case FMT_FLOAT64:
    {
      memcpy( z, vecGetElem( vec, 0 ), N*sizeof(float64_t) );
      break;
    }
  default:
    ASSERT(0);
  }

} /* vecToFp64() */

/* Convert vector data to 64-bit floating point numbers, blockwise variant of 
 * vecToFp64() (see above). */
void bvecToFp64( float64_t * z, const tVec * vec, int N, int L, int16_t shift[] )
{
  int n, l;

  ASSERT( z && vec );
  ASSERT( vec->nElem >= N*L );

  if ( vec->fmt & FMT_CPLX ) N <<= 1;

  switch ( vec->fmt & 15 )
  {
  case FMT_INT16:
    {
      int16_t * x = (int16_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N*L; n++ ) z[n] = x[n];
      break;
    }
  case FMT_INT32:
    {
      int32_t * x = (int32_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N*L; n++ ) z[n] = x[n];
      break;
    }
  case FMT_INT64:
    ASSERT( !"not implemented" );
    break;
  case FMT_FRACT16:
    {
      fract16 * x = (fract16 *)vecGetElem( vec, 0 );
      ASSERT( shift );
      for ( l=0; l<L; l++, x+=N, z+=N, shift++ )
      {
        for ( n=0; n<N; n++ ) z[n] = STDLIB_MATH(ldexp)( x[n], -(*shift)-15 );
      }
      break;
    }
  case FMT_FRACT32:
    {
      fract32 * x = (fract32 *)vecGetElem( vec, 0 );
      ASSERT( shift );
      for ( l=0; l<L; l++, x+=N, z+=N, shift++ )
      {
        for ( n=0; n<N; n++ ) z[n] = STDLIB_MATH(ldexp)( x[n], -(*shift)-31 );
      }
      break;
    }
  case FMT_FLOAT32:
    {
      float32_t * x = (float32_t *)vecGetElem( vec, 0 );
      for ( n=0; n<N*L; n++ ) z[n] = x[n];
      break;
    }
  case FMT_FLOAT64:
    {
      memcpy( z, vecGetElem( vec, 0 ), N*L*sizeof(float64_t) );
      break;
    }
  default:
    ASSERT(0);
  }

} /* bvecToFp64() */

/* Open a SEQ-file and return either the file descriptor or zero if failed. */
tSeqFile seqFileOpen( const char * fname )
{
  FILE * f;

  ASSERT( fname );

  if ( !( f = fopen( fname, "rb" ) ) )
  {
    printf( "seqFileOpen(): failed to open %s for reading\n", fname );
  }

  return ( (tSeqFile)f );

} /* seqFileOpen() */

/* Close a SEQ-file. */
void seqFileClose( tSeqFile seqFile )
{
  ASSERT( seqFile );

  fclose( (FILE*)seqFile );

} /* seqFileClose() */

/* fscanf-equivalent reading function for a SEQ-file. Returns the number of data
 * fields that are successfully assigned. */
int seqFileScanf( tSeqFile seqFile, const char * fmt, ... )
{
  FILE * f = (FILE*)seqFile;
  va_list args;
  int res;

  ASSERT( seqFile );
  skipComments( f );

  va_start( args, fmt );
  res = vfscanf( f, fmt, args );
  va_end( args );

  return ( res );

} /* seqFileScanf() */

/* Read a data vector. Argument vec specifies vector length and data format. It
 * must also have a memory block allocated to accomodate the loaded data. The
 * function returns zero if failed to read vector data. */
int seqFileReadVec( tSeqFile seqFile, const tVec * vec )
{
  static tReadFxn * const fxnTbl[8] =
    { &read_i16, &read_i32, &read_i64, &read_i16, &read_i32, &read_fl32, &read_fl64, NULL };

  tReadFxn * readFxn;

  FILE * f = (FILE*)seqFile;

  uint8_t * p;
  int n, nElem, res;
  size_t szElem;

  ASSERT( seqFile && vec && vec->szBulk > 0 );

  readFxn = fxnTbl[vec->fmt & 7];
  ASSERT( readFxn );

  if ( !( vec->fmt & FMT_CPLX ) )
  {
    nElem  = vec->nElem;
    szElem = vec->szElem;
  }
  else
  {
    nElem  = 2*vec->nElem;
    szElem = vec->szElem/2;
  }

  p = (uint8_t*)vecGetElem( vec, 0 );

  skipComments( f );

  for ( n=0, res=1; n<nElem && res; n++, p+=szElem ) res = readFxn( p, f );

  if ( !res )
  {
    printf( "seqFileReadVec(): failed to read vector, fmt = 0x%02x, nElem = %d\n",
            (unsigned)vec->fmt, vec->nElem );
  }

  return (res);

} /* seqFileReadVec() */

/* Read a few data vectors. Repeat the last argument for each new vector. The
 * arguments list must end up with zero pointer. The function returns the number
 * of successfully loaded vectors. */
int seqFileReadVecs( tSeqFile seqFile, const tVec * vec, ... )

{
  va_list args;
  int res = 1;

  va_start( args, vec );

  do
  {
    res = seqFileReadVec( seqFile, vec );

    vec = va_arg( args, const tVec* );

  } while ( res && vec );

  va_end( args );

  return (res);

} /* seqFileReadVecs() */

/* Read file and discard space and comments until some text is encountered. 
 * Comments start with a semicolon and span to the end of a line. */
void skipComments( FILE * f )
{
  char buf[256];
  const char * pstr;
  long pos;

  ASSERT( f );

  do
  {
    pos = ftell( f );

    if ( ( pstr = fgets( buf, sizeof(buf), f ) ) )
    {
      /* Skip spaces, tabs and new line. */
      pstr += strspn( pstr, " \t\r\n" );
    }

  } while ( pstr && ( !*pstr || *pstr == ';' ) );

  if ( pstr )
  {
    fseek( f, pos + ( pstr - buf ), SEEK_SET );
  }
  
} /* skipComments() */

/* Skip spaces and read an int value from the file. Verify the value against
 * min/max bounds. Return zero if failed. */
static int read_int( int * dst, int min, int max, FILE * f )
{
  int c, sgn = 1;
  int w = 0;

  /* Skip spaces */
  do
  {
    if ( ( c = fgetc( f ) ) == EOF ) return (0);

  } while ( isspace( c ) );

  /* Read the sign */
  if ( c == '-' || c == '+' )
  {
    sgn = ( c == '-' ? -1 : +1 );
    c = fgetc( f );
  }

  /* Read decimal digits */
  if ( isdigit( c ) )
  {
    do
    {
      w = 10*w + ( c - '0' );

    } while ( isdigit( c = fgetc( f ) ) );

    w = ( sgn > 0 ? w : -w );

    if ( min <= w && w <= max )
    {
      *dst = w;
      return (1);
    }
  }

  return (0);

} /* read_int() */

/* Skip spaces and read a int16 value from the file. Return zero if failed. */
int read_i16( void * dst, FILE * f )
{
  int w;

  if ( !read_int( &w, MIN_INT16, MAX_INT16, f ) ) return (0);

  *(int16_t*)dst = (int16_t)w;

  return (1);

} /* read_i16() */

/* Skip spaces and read a int32 value from the file. Return zero if failed. */
int read_i32( void * dst, FILE * f )
{
  int w;

  if ( !read_int( &w, MIN_INT32, MAX_INT32, f ) ) return (0);

  *(int32_t*)dst = (int32_t)w;

  return (1);

} /* read_i32() */

/* Skip spaces and read a int64 value from the file. Return zero if failed. */
int read_i64( void * dst, FILE * f )
{
  long long w;

  if ( 1 != fscanf( f, "%lld", &w ) ) return (0);

  *(int64_t*)dst = (int64_t)w;

  return (1);

} /* read_i64() */

/* Skip spaces and read a float32 value from the file. Return zero if failed. */
int read_fl32( void * dst, FILE * f )
{
  int c, n = 0;
  uint32_t w32 = 0;

  /* Skip spaces */
  do
  {
    if ( ( c = fgetc( f ) ) == EOF ) return (0);

  } while ( !isxdigit( c ) );

  /* Read hex digits */
  do
  {
    w32 = ( w32 << 4 ) | ( isdigit( c ) ? ( c - '0' ) : ( toupper( c ) - 'A' + 10 ) );

  } while ( ++n < 8 && isxdigit( c = fgetc( f ) ) );

  if ( n>=8 )
  {
        union ufloat32uint32 t;
        t.u=w32;
        *((float32_t*)dst) = t.f;
  }

  return (n>=8);

} /* read_fl32() */

/* Skip spaces and read a float64 value from the file. Return zero if failed. */
int read_fl64( void * dst, FILE * f )
{
  int c, n = 0;
  uint64_t w64 = 0;

  /* Skip spaces */
  do
  {
    if ( ( c = fgetc( f ) ) == EOF ) return (0);

  } while ( !isxdigit( c ) );

  /* Read hex digits */
  do
  {
    w64 = ( w64 << 4 ) | ( isdigit( c ) ? ( c - '0' ) : ( toupper( c ) - 'A' + 10 ) );

  } while ( n++ < 16 && isxdigit( c = fgetc( f ) ) );

  if ( n>=16 )
  {
        union ufloat64uint64 t;
        t.u=w64;
        *((float64_t*)dst) = t.f;
  }

  return (n>=8);

} /* read_fl64() */

/* Verify that every vector element belongs to the closed range defined by the lower
 * and upper bound vectors. Return zero if any vector element lies outside the range,
 * in which case *failIx may optionally receive the index of the first failed item.
 * Notes:
 *   1. All three vector arguments must have identical data format. The number of
 *      elements in vec must not exceed the length of lowerVec and upperVec.
 *   2. For complex data types the real and imaginary components are verified 
 *      independently against real and imaginary parts of lower and upper range
 *      bounds.
 *   3. For floating-point data formats: if either of lower and upper range bounds
 *      is NaN, then the test data item must be also NaN to pass the check. */
int vecCheckRange( const tVec * vec, const tVec * lowerVec, const tVec * upperVec, int * failIx )
{
  static tCheckRangeFxn * const fxnTbl[8] =
    { &checkRange_i16 , &checkRange_i32 ,
      &checkRange_i64 , &checkRange_i16 ,
      &checkRange_i32 , &checkRange_fl32,
      &checkRange_fl64, NULL };

  tCheckRangeFxn * checkRangeFxn;
  const uint8_t *px, *pmin, *pmax;
  size_t szElem;
  int n, nElem;

  ASSERT( vec && lowerVec && upperVec );
  ASSERT( vec->fmt == lowerVec->fmt && vec->fmt == upperVec->fmt );
  ASSERT( lowerVec->nElem == upperVec->nElem );

  checkRangeFxn = fxnTbl[vec->fmt & 7];
  ASSERT( checkRangeFxn );

  if ( !( vec->fmt & FMT_CPLX ) )
  {
    nElem  = MIN( vec->nElem, lowerVec->nElem );
    szElem = vec->szElem;
  }
  else
  {
    nElem  = 2*MIN( vec->nElem, lowerVec->nElem );
    szElem = vec->szElem/2;
  }

  px   = (uint8_t*)vecGetElem( vec     , 0 );
  pmin = (uint8_t*)vecGetElem( lowerVec, 0 );
  pmax = (uint8_t*)vecGetElem( upperVec, 0 );

  for ( n=0; n<nElem; n++, px+=szElem, pmin+=szElem, pmax+=szElem )
  {
    if ( !checkRangeFxn( px, pmin, pmax ) ) break;
  }

  if ( n < nElem && failIx ) *failIx = ( !( vec->fmt & FMT_CPLX ) ? n : n/2 );

  return ( n >= nElem );

} /* vecCheckRange() */

int checkRange_i16( const void * px, const void * pmin, const void * pmax )
{
  int16_t x, min, max;

  ASSERT( px && pmin && pmax );

  x   = *(int16_t*)px; 
  min = *(int16_t*)pmin;
  max = *(int16_t*)pmax;

  return ( min <= x && x <= max );

} /* checkRange_i16() */

int checkRange_i32( const void * px, const void * pmin, const void * pmax )
{
  int32_t x, min, max;

  ASSERT( px && pmin && pmax );

  x   = *(int32_t*)px; 
  min = *(int32_t*)pmin;
  max = *(int32_t*)pmax;

  return ( min <= x && x <= max );

} /* checkRange_i32() */

int checkRange_i64( const void * px, const void * pmin, const void * pmax )
{
  int64_t x, min, max;

  ASSERT( px && pmin && pmax );

  x   = *(int64_t*)px; 
  min = *(int64_t*)pmin;
  max = *(int64_t*)pmax;

  return ( min <= x && x <= max );

} /* checkRange_i64() */

/* Check if the value belongs to the closed range [min,max]. Return zero if
 * the tested value lies outside the range. */
int checkRange_fl32( const void * px, const void * pmin, const void * pmax )
{
    union 
    {
        float32_t f;
        uint32_t  u;
    }
    x, min, max;
    ASSERT( px && pmin && pmax );

    x.u   = *(uint32_t*)px; 
    min.u = *(uint32_t*)pmin;
    max.u = *(uint32_t*)pmax;

    if (((min.u|max.u)&0x7fffffff)==0)
    {   /* special case: both bounds are zeroes 
           if min and max are -0 and 0, just make floating point comparison with zero 
           otherwize result should be exacly equal to zero with right sign
        */
        return (min.u!=max.u) ? x.f==0.f: x.u==min.u;
    }
    if ( isnan(min.f) || isnan(max.f) )
    {
        /* Special case: NaN is expected. 
         * There are two kinds of NaN: quiet and signaling. Normally, a signaling NaN
         * may not appear as a result of floating-point operation, but we still allow a
         * signaling NaN in the value under test, iff min or max range bound is a signaling
         * NaN. This is done to address validation of low-level manipulation functions like
         * copysign(), which may treat NaNs the same way as other floating-point values. 
         */
        int min_sNaN = isnan(min.f) && ( 0 == ( min.u & (1<<22) ) );
        int max_sNaN = isnan(max.f) && ( 0 == ( max.u & (1<<22) ) );
        int x_sig = ( 0 == ( x.u & (1<<22) ) );

        return ( isnan(x.f) && ( !x_sig || min_sNaN || max_sNaN ) );
    }
    else
    {
        return ( min.f <= x.f && x.f <= max.f );
    }

} /* checkRange_fl32() */

int checkRange_fl64( const void * px, const void * pmin, const void * pmax )
{
    union 
    {
        float64_t f;
        uint64_t  u;
    } x, min, max;

    ASSERT( px && pmin && pmax );

    x.u   = *(uint64_t*)px; 
    min.u = *(uint64_t*)pmin;
    max.u = *(uint64_t*)pmax;

    if (((min.u|max.u)&0x7fffffffffffffffULL)==0)
    {   /* special case: both bounds are zeroes 
           if min and max are -0 and 0, just make floating point comparison with zero 
           otherwize result should be exacly equal to zero with right sign
        */
        return (min.u!=max.u) ? x.f==0.f: x.u==min.u;
    }
    if ( isnan(min.f) || isnan(max.f) )
    {
        /* Special case: NaN is expected. 
         * There are two kinds of NaN: quiet and signaling. Normally, a signaling NaN
         * may not appear as a result of floating-point operation, but we still allow a
         * signaling NaN in the value under test, iff min or max range bound is a signaling
         * NaN. This is done to address validation of low-level manipulation functions like
         * copysign(), which may treat NaNs the same way as other floating-point values. 
         */
        int min_sNaN = isnan(min.f) && ( 0 == ( min.u & (1ULL<<51) ) );
        int max_sNaN = isnan(max.f) && ( 0 == ( max.u & (1ULL<<51) ) );
        int x_sig = ( 0 == ( x.u & (1ULL<<51) ) );

        return ( isnan(x.f) && ( !x_sig || min_sNaN || max_sNaN ) );
    }
    else
    {
        return ( min.f <= x.f && x.f <= max.f );
    }

} /* checkRange_fl64() */
