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
  NatureDSP Signal Processing Library. FIR part
    Real block FIR filter, 24x24-bit, packed delay line and coefficient storage
    C code optimized for HiFi1
*/

/*-------------------------------------------------------------------------
  Real FIR filter.
  Computes a real FIR filter (direct-form) using IR stored in vector h. The 
  real data input is stored in vector x. The filter output result is stored 
  in vector y. The filter calculates N output samples using M coefficients 
  and requires last M-1 samples in the delay line.
  NOTE: 
   user application is not responsible for management of delay lines


  Precision: 
  16x16  16-bit data, 16-bit coefficients, 16-bit outputs
  24x24  24-bit data, 24-bit coefficients, 24-bit outputs
  24x24p use 24-bit data packing for internal delay line buffer
         and internal coefficients storage
  32x16    32-bit data, 16-bit coefficients, 32-bit outputs
  32x32    32-bit data, 32-bit coefficients, 32-bit outputs
  f        floating point

  Input:
  x[N]      - input samples, Q31, Q15, floating point
  h[M]      - filter coefficients in normal order, Q31, Q15, floating point
  N         - length of sample block, should be a multiple of 4
  M         - length of filter, should be a multiple of 4
  Output:
  y[N]      - input samples, Q31, Q15, floating point

  Restrictions:
  x,y should not be overlapping
  x,h - aligned on a 8-bytes boundary
  N,M - multiples of 4 
-------------------------------------------------------------------------*/


/* Portable data types. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
/* Common utility and macros declarations. */
#include "common.h"

/* Instance pointer validation number. */
#define MAGIC     0x68c08c65

#if 0
/*Reference code*/
#include "baseop.h"

/* Reserve memory for alignment. */
#define ALIGNED_SIZE( size, align ) \
      ( (size_t)(size) + (align) - 1 )

/* Align address on a specified boundary. */
#define ALIGNED_ADDR( addr, align ) \
      (void*)( ( (uintptr_t)(addr) + ( (align) - 1 ) ) & ~( (align) - 1 ) )

#define MIN(a,b)  ( (a) < (b) ? (a) : (b) )

/*
 * Helper data types and utilities for handling packed 24-bit data.
 */

typedef struct tag_f24p
{
  uint8_t b0; // Bits 7..0
  uint8_t b1; // Bits 15..8
  int8_t  b2; // Bits 23..16

} f24p;

#define sz_f24p   sizeof(f24p)

static int32_t load_f24p( f24p x )
{
  return ( ( ( int32_t)x.b2 << 24 ) |
           ( (uint32_t)x.b1 << 16 ) |
           ( (uint32_t)x.b0 <<  8 ) );
}

static f24p store_f24p( int32_t x )
{
  f24p p;

  p.b2 = ( int8_t)( x >> 24 );
  p.b1 = (uint8_t)( x >> 16 );
  p.b0 = (uint8_t)( x >>  8 );

  return (p);
}

/* Filter instance structure. */
typedef struct tag_bkfir24x24p_t
{
  uint32_t     magic;     // Instance pointer validation number
  int          M;         // Number of filter coefficients
  const f24p * coef;      // M filter coefficients, reverted
  f24p *       delayLine; // Delay line for samples
  int          delayLen;  // Delay line length, in samples
  int          wrIx;      // Index of the oldest sample

} bkfir24x24p_t, *bkfir24x24p_ptr_t;

/* Calculate the memory block size for an FIR filter with given attributes. */
size_t bkfir24x24p_alloc( int M )
{
  NASSERT( M > 0 );

  ASSERT( M%4==0 );

  return ( ALIGNED_SIZE( sizeof( bkfir24x24p_t ), 4 )
           + // Delay line
           ALIGNED_SIZE( ( M+8-1 )*sz_f24p, 1 )
           + // Filter coefficients
           ALIGNED_SIZE( M*sz_f24p, 1 ) );

} /* bkfir24x24p_alloc() */

/* Initialize the filter structure. The delay line is zeroed. */
bkfir24x24p_handle_t bkfir24x24p_init( void *         objmem,
                                       int            M,
                                 const f24 * restrict h )
{
  bkfir24x24p_ptr_t bkfir;
  void *            ptr;
  f24p *            coef;
  f24p *            delLine;

  int m;

  ASSERT( objmem && M > 0 && h );

  ASSERT( M%4==0 && IS_ALIGN( h ) );

  //
  // Partition the memory block
  //

  ptr     = objmem;
  bkfir   = (bkfir24x24p_ptr_t)ALIGNED_ADDR( ptr, 4 );
  ptr     = bkfir + 1;
  delLine = (f24p*)ALIGNED_ADDR( ptr, 1 );
  ptr     = delLine + M+8-1;
  coef    = (f24p*)ALIGNED_ADDR( ptr, 1 );
  ptr     = coef + M;

  NASSERT( (int8_t*)ptr - (int8_t*)objmem <= (int)bkfir24x24p_alloc( M ) );

  //
  // Copy the filter coefficients in reverted order and zero the delay line.
  //

  for ( m=0; m<M; m++ )
  {
    coef[m] = store_f24p( h[M-1-m] );
  }

  for ( m=0; m<M+8-1; m++ )
  {
    delLine[m] = store_f24p( 0 );
  }

  //
  // Initialize the filter instance.
  //

  bkfir->magic     = MAGIC;
  bkfir->M         = M;
  bkfir->coef      = coef;
  bkfir->delayLine = delLine;
  bkfir->delayLen  = M+8-1;
  bkfir->wrIx      = 0;

  return (bkfir);

} /* bkfir24x24p_init() */

/* Put a chunk of input signal into the delay line and compute the filter
 * response. */
void bkfir24x24p_process( bkfir24x24p_handle_t _bkfir,
                          f24 * restrict      y,
                    const f24 * restrict      x, int N )
{
  bkfir24x24p_ptr_t bkfir = (bkfir24x24p_ptr_t)_bkfir;

  int64_t acc;
  int     rdIx, wrIx;
  int     len;
  int     M, K;
  int     m, n, k;

  ASSERT( bkfir && bkfir->magic == MAGIC && y && x );

  ASSERT( IS_ALIGN( x ) );

  M    = bkfir->M;
  len  = bkfir->delayLen;
  NASSERT(N%4==0);
  NASSERT(M%4==0);
  if(N<=0) return;

  //
  // Break the input signal into 8-samples blocks. For each block, store 8
  // samples to the delay line and compute the filter response.
  //

  wrIx = bkfir->wrIx;

  for ( n=0; n<((N+4)>>3); n++ )
  {
    K = MIN( N - (n<<3), 8 );

    for ( k=0; k<K; k++ )
    {
      bkfir->delayLine[wrIx] = store_f24p( x[8*n+k] );

      if ( ++wrIx >= len ) wrIx = 0;
    }

    for ( k=0; k<K; k++ )
    {
      if ( ( rdIx = wrIx + 8-K + k ) >= len ) rdIx -= len;

      for ( acc=0, m=0; m<M; m++ )
      {
        // Q16.47 <- Q16.47 + ( Q(23+8)*Q(23+8) - 16 + 1 )
        acc += mpy_f24f24( load_f24p( bkfir->delayLine[rdIx] ),
                           load_f24p( bkfir->coef[m] ) );

        if ( ++rdIx >= len ) rdIx = 0;
      }

      // Q(23+8) <- [ Q16.47 - 24 w/ rounding and saturation ] + 8
      y[8*n+k] = store_f24( satQ47_f24( acc + (1LL<<23) ) );
    }
  }

  bkfir->wrIx = wrIx;

} /* bkfir24x24p_process() */
#endif/*Reference code*/

/* Filter instance structure. */
typedef struct tag_bkfir24x24_t
{
  int32_t magic;
  int             M; /* Filter length                   */
  const int32_t * h; /* Filter coefficients             */
  int32_t * d; /* Delay line of length M          */
  int32_t * p; /* Pointer into the delay line     */
} bkfir24x24p_t, *bkfir24x24p_ptr_t;

/* Calculate the memory block size for an FIR filter with given attributes. */
size_t bkfir24x24p_alloc( int M )
{
  NASSERT( M > 0 );
  NASSERT(M%4==0);
  M=(M+7)&~7;
  return 2*M*sizeof(int32_t) + sizeof(bkfir24x24p_t) + 7;
} // bkfir24x24p_alloc()

/* Initialize the filter structure. The delay line is zeroed. */
bkfirf_handle_t bkfir24x24p_init( void * objmem, int M, const f24 * h )
{
  bkfir24x24p_t* bkfir;
  void *           ptr;
  int32_t * restrict pd;
  int32_t * restrict ph;
  int m;
  NASSERT( objmem && M > 0 && h );
  NASSERT(M%4==0);
  NASSERT_ALIGN(h,8);
  /* Partition the memory block */
  ptr     = objmem;
  ph = (int32_t*)((((uintptr_t)ptr)+7)&~7);
  pd = ph+M;
  bkfir=(bkfir24x24p_t*)(pd+M);
  bkfir->magic=MAGIC;
  bkfir->M   = M;
  bkfir->h = ph;
  bkfir->d = bkfir->p = pd;
  /* copy coefficients and clean upd delay line */
  for (m=0; m<M; m++) ph[m]=h[m];
  for (m=0; m<M; m++) pd[m]=0;
  return bkfir;
} // bkfir24x24p_init()

/* process block of samples */
void bkfir24x24p_process( bkfir24x24p_handle_t _bkfir, 
                         f24 * restrict  y,
                   const f24 * restrict  x, int N )
{ 
    const ae_f32x2* restrict pH;
    const ae_f32x2* restrict pX;
          ae_f32x2* restrict pY;
    const ae_f32x2* pD01;
    const ae_f32x2* pD12;
    ae_f32x2* p;
    ae_valign  ay;
    int n,m,M;
    const int32_t* h;
    bkfir24x24p_t* bkfir;
    NASSERT(_bkfir);
    bkfir=(bkfir24x24p_t*)_bkfir;
    NASSERT(bkfir->magic==MAGIC);
    NASSERT(bkfir->h);
    NASSERT(bkfir->d);
    NASSERT(bkfir->p);
    NASSERT_ALIGN(bkfir->h,8);
    NASSERT_ALIGN(bkfir->d,8);
    NASSERT_ALIGN(bkfir->p,8);
    NASSERT(N%4==0);
    NASSERT_ALIGN(x,8);
    NASSERT((bkfir->M%4)==0);
    NASSERT(x);
    NASSERT(y);
    if(N<=0) return;
    M=bkfir->M;
    NASSERT(N>0);
    NASSERT(M>0);
    h=bkfir->h;
    pX=(const ae_f32x2*)x;
    WUR_AE_CBEGIN0( (uintptr_t)( bkfir->d            ) );
    WUR_AE_CEND0  ( (uintptr_t)( bkfir->d + bkfir->M ) );
    ay=AE_ZALIGN64();
    pY=(ae_f32x2*)y;
    p=(ae_f32x2*)(bkfir->p);
    
    for (n=0; n<N; n+=4)
    {   
        ae_f64     q0, q1, q2, q3;
        ae_f32x2   t,X01,X12,X23,X34,XN0,XN1,H01,H23;
        ae_valign  ad01,ad12;
       	ae_f32x2   d0;
 
        pH=(ae_f32x2* )h;
        pD01=(const ae_f32x2*)(p);
        pD12=(const ae_f32x2*)(((int32_t*)p)+1);
        AE_LA32X2NEG_PC(ad01,pD01);
        AE_LA32X2NEG_PC(ad12,pD12);
        AE_LA32X2_RIC(X12,ad12,pD12);
        AE_LA32X2_RIC(t,ad01,pD01);
        XN0=AE_L32X2_I(pX,0*sizeof(int32_t));
        XN1=AE_L32X2_I(pX,2*sizeof(int32_t));
        X12=AE_INTSWAP(XN0); 
        X34=AE_INTSWAP(XN1);
	
        X01=AE_SEL32_HL(XN0,t);
        X23=AE_SEL32_HL(XN1,XN0);        
            
        AE_L32X2_IP(H01,pH,2*sizeof(int32_t));
        q0=q1=AE_ZERO();
          
        q2=AE_MULF32R_HH(H01,X23);
        AE_MULAF32R_LL(q2,H01,X23);

        q3=AE_MULF32R_HH(H01,X34);
        AE_MULAF32R_LL(q3,H01,X34);
        
        __Pragma("loop_count min=1")
        for (m=0; m<M-2; m+=2)
        {
            AE_L32X2_IP(H23,pH,2*sizeof(int32_t));

            AE_MULAF32R_HH(q0,H01,X01);
            AE_MULAF32R_LL(q0,H01,X01);

            AE_MULAF32R_HH(q1,H01,X12);
            AE_MULAF32R_LL(q1,H01,X12);

            AE_MULAF32R_HH(q2,H23,X01);
            AE_MULAF32R_LL(q2,H23,X01);

            AE_MULAF32R_HH(q3,H23,X12);
            AE_MULAF32R_LL(q3,H23,X12);

            H01=H23; 

            AE_LA32X2_RIC(X01,ad01,pD01);
            AE_LA32X2_RIC(X12,ad12,pD12);
        }
	    	AE_MULAF32R_HH(q0,H01,X01);
        AE_MULAF32R_LL(q0,H01,X01);
        AE_MULAF32R_HH(q1,H01,X12);
        AE_MULAF32R_LL(q1,H01,X12);
  
        AE_L32X2_IP(XN0,pX,2*sizeof(int32_t));
        AE_L32X2_IP(XN1,pX,2*sizeof(int32_t));
        AE_S32X2_XC(XN0,p,8);
        AE_S32X2_XC(XN1,p,8);
        d0 = AE_ROUND24X2F48SASYM( q0, q1 );
        AE_SA32X2_IP( AE_SLAI32(d0, 8), ay, pY );
        d0 = AE_ROUND24X2F48SASYM( q2, q3 );
        AE_SA32X2_IP( AE_SLAI32(d0, 8), ay, pY );
    }
    AE_SA64POS_FP(ay,pY);
    bkfir->p=(int32_t*)p;
}
