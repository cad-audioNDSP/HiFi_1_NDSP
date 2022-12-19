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
 * C code optimized for HiFi1
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Common helper macros. */
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(int, fft_cplxf_ie, ( complex_float * y, complex_float * x, const complex_float* twd, int twdstep, int N))

#else
/*-------------------------------------------------------------------------
  These functions make FFT on complex data with optimized memory usage.
  Scaling  : Fixed data scaling at each stage
  NOTES:
  1. Bit-reversing reordering is done here.
  2. FFT runs in-place algorithm so INPUT DATA WILL APPEAR DAMAGED after 
     the call
  3. Forward/inverse FFT of size N may be supplied with constant data
     (twiddle factors) of a larger-sized FFT = N*twdstep.

  Precision: 
  24x24_ie      24-bit input/outputs, 24-bit twiddles
  32x16_ie      32-bit input/outputs, 16-bit twiddles
  f             floating point
 
  Input:
  x[N]                  complex input signal. Real and imaginary data are interleaved 
                        and real data goes first
  twd[N*twdstep*3/4]    twiddle factor table of a complex-valued FFT of size N*twdstep
  N                     FFT size
  twdstep               twiddle step 
  scalingOpt            scaling option = 3 (fixed scaling)
  Output:
  y[N]                  output spectrum. Real and imaginary data are interleaved and 
                        real data goes first

  Returned value: total number of right shifts occurred during scaling 
                  procedure

  Restrictions:
  x,y - should not overlap
  x,y - aligned on 8-bytes boundary
  N   - 256, 512, 1024 for fixed-point routines, power of 2 and >=8 for 
        floating point

-------------------------------------------------------------------------*/

#include "NatureDSP_Signal.h"
#define SZ_CF32 (sizeof(complex_float))
#define LOG2_SZ_CF32 3/* log2(sizeof(complex_float)) */


int fft_cplxf_ie    (complex_float * y, complex_float * x, const complex_float* twd, int twdstep, int N )
{
  const xtfloatx2 *restrict p_twd;
  const xtfloatx2 *restrict p0_ld;
  const xtfloatx2 *restrict p1_ld;
  const xtfloatx2 *restrict p2_ld;
  const xtfloatx2 *restrict p3_ld;
        xtfloatx2 *restrict p0_st;
        xtfloatx2 *restrict p1_st;
        xtfloatx2 *restrict p2_st;
        xtfloatx2 *restrict p3_st;
  xtfloatx2 tw1, tw2, tw3;
  xtfloatx2 a0, a1, a2, a3;
  xtfloatx2 b0, b1, b2, b3;
  xtfloatx2 c1;
  int N4, logN, stride;
  int m, n;
  unsigned int idx, bitrevstride;

  NASSERT( x );
  NASSERT( y );
  NASSERT( twd );

  NASSERT( x != y );

  NASSERT_ALIGN( x, 8 );
  NASSERT_ALIGN( y, 8 );
  NASSERT_ALIGN( twd, 8 );

  NASSERT( twdstep >= 1 );
  NASSERT( N>=8 && 0 == (N&(N-1)) );

  N4 = N>>2;
  logN = 32 - XT_NSA(N4);
  /* Helping constant, used for multiplication by j */
  c1 = (xtfloatx2)1.0f;

  /* Set the pointer to the twiddle table            *
 *    * and set bounds of the table for circular loads. */
  p_twd = (const xtfloatx2 *)(twd);
  WUR_AE_CBEGIN0((uintptr_t)(twd));
  WUR_AE_CEND0  ((uintptr_t)(twd+3*twdstep*(N4)));
  /*----------------------------------------------------------------------------*
 *    * Perform the first stage. We use DIF, all permutations are deferred until   *
 *       * the last stage.                                                            */
  {
    stride = N4;
    
    p0_st = (xtfloatx2 *)(x);
    p1_st = (xtfloatx2 *)((uintptr_t)p0_st + 8 * stride);
    p2_st = (xtfloatx2 *)((uintptr_t)p1_st + 8 * stride);
    p3_st = (xtfloatx2 *)((uintptr_t)p2_st + 8 * stride);
    p0_ld = p0_st;
    p1_ld = p1_st;
    p2_ld = p2_st;
    p3_ld = p3_st;
    __Pragma("loop_count min=2")
    for ( n=0; n<stride; n++ )
    {
      tw2 = XT_LSX2I(p_twd,   SZ_CF32);
      XT_LSX2IP(tw1, p_twd, 2*SZ_CF32);
      XT_LSX2XC(tw3, p_twd, (twdstep*3-2)*SZ_CF32);

      XT_LSX2IP(a0, p0_ld, SZ_CF32);
      XT_LSX2IP(a1, p1_ld, SZ_CF32);
      XT_LSX2IP(a2, p2_ld, SZ_CF32);
      XT_LSX2IP(a3, p3_ld, SZ_CF32);

      b0 = a0 + a2;
      b1 = a1 + a3;
      b2 = a0 - a2;
      b3 = a1 - a3;

      a0 = b0 + b1;
      a2 = b0 - b1;
      /* a1 <- b2-j*b3 */
      /* a3 <- b2+j*b3 */
      a1 = a3 = b2;
      XT_MADDMUX_S(a1, c1, b3, 3);
      XT_MADDMUX_S(a3, c1, b3, 1);

      b0 = a0;
      b2 = XT_MULC_S(a2, tw2);
      b1 = XT_MULC_S(a1, tw1);
      b3 = XT_MULC_S(a3, tw3);

      /* Two middle quartiles are swapped on all but the last stage to use the bit reversal
 *        * permutation instead of the digit reverse. */
      XT_SSX2IP(b0, p0_st, SZ_CF32);
      XT_SSX2IP(b2, p1_st, SZ_CF32);
      XT_SSX2IP(b1, p2_st, SZ_CF32);
      XT_SSX2IP(b3, p3_st, SZ_CF32);
    }
  }
  /*----------------------------------------------------------------------------
 *    Perform second through the next to last stages.*/

  for ( stride>>=2; stride>1; stride>>=2 )
  {
    twdstep <<= 2;

    p0_st = (xtfloatx2 *)(x);

    for ( m=0; m<N4; m+=stride )
    {
      p1_st = (xtfloatx2 *)((uintptr_t)p0_st + 8 * stride);
      p2_st = (xtfloatx2 *)((uintptr_t)p1_st + 8 * stride);
      p3_st = (xtfloatx2 *)((uintptr_t)p2_st + 8 * stride);
      p0_ld = p0_st;
      p1_ld = p1_st;
      p2_ld = p2_st;
      p3_ld = p3_st;
      
      __Pragma("loop_count min=2")
      for ( n=0; n<stride; n++ )
      {
        tw2 = XT_LSX2I(p_twd,   SZ_CF32);
        XT_LSX2IP(tw1, p_twd, 2*SZ_CF32);
        XT_LSX2XC(tw3, p_twd, (twdstep*3-2)*SZ_CF32);
        
        XT_LSX2IP(a0, p0_ld, SZ_CF32);
        XT_LSX2IP(a1, p1_ld, SZ_CF32);
        XT_LSX2IP(a2, p2_ld, SZ_CF32);
        XT_LSX2IP(a3, p3_ld, SZ_CF32);

        b0 = a0 + a2;
        b1 = a1 + a3;
        b2 = a0 - a2;
        b3 = a1 - a3;
        
        a0 = b0 + b1;
        a2 = b0 - b1;
        /* a1 <- b2-j*b3 */
        /* a3 <- b2+j*b3 */
        a1 = a3 = b2;
        XT_MADDMUX_S(a1, c1, b3, 3);
        XT_MADDMUX_S(a3, c1, b3, 1);
        
        b0 = a0;
        b2 = XT_MULC_S(a2, tw2);
        b1 = XT_MULC_S(a1, tw1);
        b3 = XT_MULC_S(a3, tw3);

        /* Two middle quartiles are swapped on all but the last stage to use the bit reversal
 *          * permutation instead of the digit reverse. */
        XT_SSX2IP(b0, p0_st, SZ_CF32);
        XT_SSX2IP(b2, p1_st, SZ_CF32);
        XT_SSX2IP(b1, p2_st, SZ_CF32);
        XT_SSX2IP(b3, p3_st, SZ_CF32);
      }
      p0_st = p3_st;
    }
  }

  /*----------------------------------------------------------------------------
 *    Last stage (radix-4 or radix-2 for odd powers of two) with bit reversal
 *       permutation.*/
  idx = 0;
  bitrevstride = 0x80000000U >> (logN-3+LOG2_SZ_CF32);
  if ( stride != 0 )
  { 
    p0_ld = (const xtfloatx2 *)(x);
    p1_ld = p0_ld+1;
    p2_ld = p1_ld+1;
    p3_ld = p2_ld+1;
    p0_st = (xtfloatx2 *)(y);
    p1_st = p0_st+N4;
    p2_st = p1_st+N4;
    p3_st = p2_st+N4;
    
    __Pragma("loop_count min=2")
    for ( n=0; n<N4; n++ )
    {
      XT_LSX2IP(a0, p0_ld, 4*SZ_CF32);
      XT_LSX2IP(a1, p1_ld, 4*SZ_CF32);
      XT_LSX2IP(a2, p2_ld, 4*SZ_CF32);
      XT_LSX2IP(a3, p3_ld, 4*SZ_CF32);

      b0 = a0 + a2;
      b1 = a1 + a3;
      b2 = a0 - a2;
      b3 = a1 - a3;
      
      a0 = b0 + b1;
      a2 = b0 - b1;
      /* a1 <- b2-j*b3 */
      /* a3 <- b2+j*b3 */
      a1 = a3 = b2;
      XT_MADDMUX_S(a1, c1, b3, 3);
      XT_MADDMUX_S(a3, c1, b3, 1);
      
      XT_SSX2X(a0, p0_st, idx);
      XT_SSX2X(a1, p1_st, idx);
      XT_SSX2X(a2, p2_st, idx);
      XT_SSX2X(a3, p3_st, idx);

      idx = AE_ADDBRBA32(idx, bitrevstride);
    }
  }
  else
  {
    bitrevstride >>= 1;

    p0_ld = (const xtfloatx2 *)(x);
    p1_ld = p0_ld+1;
    p0_st = (xtfloatx2 *)(y);
    p1_st = (xtfloatx2 *)((uintptr_t)p0_st + 8 * (N4 << 1));

    __Pragma("loop_count min=1")
    for ( n=0; n<(N4<<1); n++ )
    {
      XT_LSX2IP(a0, p0_ld, 2*SZ_CF32);
      XT_LSX2IP(a1, p1_ld, 2*SZ_CF32);

      b0 = a0 + a1;
      b1 = a0 - a1;
      
      XT_SSX2X(b0, p0_st, idx);
      XT_SSX2X(b1, p1_st, idx);

      idx = AE_ADDBRBA32(idx, bitrevstride);
    }
  }
  return 0;
} /* fft_cplxf_ie() */

#endif
