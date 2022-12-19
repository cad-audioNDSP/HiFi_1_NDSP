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
DISCARD_FUN(int, fft_realf_ie, (complex_float* y, float32_t* x, const complex_float* twd, int twdstep, int N))

#else

/*-------------------------------------------------------------------------
  These functions make FFT on real data with optimized memory usage.
  Scaling  : 
      24x24_ie        - Fixed data scaling at each stage
      32x16_ie        - Fixed data scaling at each stage
      32x16_ie_24p    - Fixed data scaling at each stage
      24x24_ie_24p    - 24-bit scaling
    
  NOTES:
  1. Bit-reversing reordering is done here.
  2. INPUT DATA MAY APPEAR DAMAGED after the call
  3. FFT functions may use input and output buffers for temporal
  storage of intermediate 32-bit data, so FFT functions
  with 24-bit packed I/O (Nx3-byte data) require that the
  buffers are large enough to keep Nx4-byte data
  4. Forward/inverse FFT of size N may be supplied with constant data
  (twiddle factors) of a larger-sized FFT = N*twdstep.

  Precision:
  24x24_ie      24-bit input/outputs, 24-bit data, 24-bit twiddles
  32x16_ie      32-bit input/outputs, 32-bit data, 16-bit twiddles
  24x24_ie_24p  24-bit packed input/outputs, 24-bit data, 24-bit twiddles
  32x16_ie_24p  24-bit packed input/outputs, 32-bit data, 16-bit twiddles
  f             floating point

  Input:
  x - real input signal. Real and imaginary data are interleaved
  and real data goes first:
  --------------+----------+-----------------+-----------
  Function      |   Size   |  Allocated Size |  type    |
  --------------+----------+-----------------+-----------
  24x24_ie      |     N    |      N          |   f24    |
  32x16_ie      |     N    |      N          |  int32_t |
  24x24_ie_24p  |     3*N  |      4*N+8      |  uint8_t |
  32x16_ie_24p  |     3*N  |      4*N+8      |  uint8_t |
  --------------+----------+-----------------+-----------

  twd[N*twdstep*3/4]    twiddle factor table of a complex-valued 
                        FFT of size N*twdstep
  N                     FFT size
  twdstep               twiddle step
  scalingOpt            24x24_ie        - 3 (Fixed scaling)
                        32x16_ie        - 3 (Fixed scaling)
                        32x16_ie_24p    - 3 (Fixed scaling)
                        24x24_ie_24p    - 1 (24-bit scaling)

  Output:
  y - output spectrum. Real and imaginary data are interleaved and
  real data goes first:
  --------------+----------+-----------------+----------------
  Function      |   Size   |  Allocated Size |  type         |
  --------------+----------+-----------------+----------------
  24x24_ie      |   N/2+1  |      N/2+1      |complex_fract32|
  32x16_ie      |   N/2+1  |      N/2+1      |complex_fract32|
  24x24_ie_24p  |  3*(N+2) |      4*N+8      |  uint8_t      |
  32x16_ie_24p  |  3*(N+2) |      4*N+8      |  uint8_t      |
  f_ie          |   N/2+1  |      N/2+1      |complex_float  |
  --------------+----------+-----------------+----------------

  Returned value: total number of right shifts occurred during scaling
  procedure

  Restrictions:
  x,y - should not overlap
  x,y - aligned on 8-bytes boundary
  N   - 256, 512, 1024 for fixed point functions, power of two and >=8 for 
        floating point

-------------------------------------------------------------------------*/

#include "NatureDSP_Signal.h"
#define SZ_CF32 (sizeof(complex_float))

int fft_realf_ie        (complex_float* y,float32_t* x, const complex_float* twd, int twdstep, int N)
{
  const xtfloatx2 *restrict p_twd;
  const xtfloatx2 *restrict p0_ld;
  const xtfloatx2 *restrict p1_ld;
        xtfloatx2 *restrict p0_st;
        xtfloatx2 *restrict p1_st;
  xtfloatx2 a0, a1, a2, a3;
  xtfloatx2 b0, b1, b2, b3;

  int n;
  
  NASSERT( x );
  NASSERT( y );
  NASSERT( twd );

  NASSERT( (void*)x != y );

  NASSERT_ALIGN( x, 8 );
  NASSERT_ALIGN( y, 8 );
  NASSERT_ALIGN( twd, 8 );

  NASSERT( twdstep >= 1 );
  NASSERT( N>=8 && 0 == (N&(N-1)) );
  
  /*----------------------------------------------------------------------------
 *    Perform the half-sized complex-valued forward FFT
 *      
 *         MATLAB code:
 *             y(1:N/2) = fft(x(1:2:N)+1j*x(2:2:N));
 *               */
  p0_ld = (const xtfloatx2 *)(x);
  p0_st = (      xtfloatx2 *)(y);

  if ( N == 8 )
  {
    xtfloatx2 c1;
    /* Helping constant, used for multiplication by j */
    c1 = (xtfloatx2)1.0f;

    XT_LSX2IP(a0, p0_ld, SZ_CF32);
    XT_LSX2IP(a1, p0_ld, SZ_CF32);
    XT_LSX2IP(a2, p0_ld, SZ_CF32);
    XT_LSX2IP(a3, p0_ld, SZ_CF32);

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
    
    XT_SSX2IP(a0, p0_st, SZ_CF32);
    XT_SSX2IP(a1, p0_st, SZ_CF32);
    XT_SSX2IP(a2, p0_st, SZ_CF32);
    XT_SSX2IP(a3, p0_st, SZ_CF32);
  }
  else
  {
    fft_cplxf_ie( y, (complex_float*)x, twd, twdstep<<1, N>>1 );
  }

  /*----------------------------------------------------------------------------
 *    Apply the in-place real-to-complex spectrum conversion
 *      
 *         MATLAB code:
 *             twd = exp(-2*pi*1j*(0:N/4-1)/N);
 *                 a0 = y(1:N/4);
 *                     a1 = [y(1),wrev(y(N/4+2:N/2))];
 *                         b0 = 1/2*(a0+conj(a1));
 *                             b1 = 1/2*(a0-conj(a1))*-1j.*twd;
 *                                 a0 = b0+b1;
 *                                     a1 = b0-b1;
 *                                         y(1:N) = [a0,conj(y(N/4+1)),wrev(conj(a1))];
 *                                           */
  {
    xtfloatx2 c05, tw, temp;

    p_twd = (const xtfloatx2 *)(twd+3*twdstep);
    p0_st = (xtfloatx2 *)(y);
    p1_st = p0_st+(N>>1);
    p0_ld = p0_st;
    p1_ld = p1_st-1;

    c05 = (xtfloatx2)0.5f;
    
    /* a0.re = y[0].re+y[0].im; a0.im = 0 */
    /* a1.re = y[0].re-y[0].im; a1.im = 0 */
    XT_LSX2IP(a0, p0_ld, SZ_CF32);
    temp = XT_SEL32_LL_SX2(a0, a0);
    a1 = a0 - temp;
#if 0
    temp = XT_SEL32_HL_SX2(temp, -temp);
#else
    temp = XT_CONJC_S(temp);
#endif
    a0 = a0 + temp;
   
    XT_SSX2IP(a0, p0_st,       SZ_CF32);
    XT_SSX2XP(a1, p1_st, -(int)SZ_CF32);
    
    __Pragma("loop_count min=1")
    for ( n=1; n<(N>>2); n++ )
    {
      XT_LSX2IP(a0, p0_ld,       SZ_CF32);
      XT_LSX2XP(a1, p1_ld, -(int)SZ_CF32);
      
      /* b0 <- 1/2*(a0+conj(a1)) */
      /* b1 <- 1/2*(a0-conj(a1)) */
      b0 = b1 = c05*a0;
      XT_MADDMUX_S(b0, c05, a1, 4);
      XT_MADDMUX_S(b1, c05, a1, 6);
      
      /* a0 <- b0-j*b1*twd */
      /* a1 <- b0+j*b1*twd */
      XT_LSX2XP(tw, p_twd, 3*twdstep*SZ_CF32);
      tw = XT_SEL32_LH_SX2(tw, tw);
      a0 = a1 = b0;
      XT_MADDMUX_S(a0, b1, tw, 4);
      XT_MADDMUX_S(a0, b1, tw, 5);
      XT_MADDMUX_S(a1, b1, tw, 6);
      XT_MADDMUX_S(a1, b1, tw, 7);
      
#if 0
      a1 = XT_SEL32_HL_SX2(a1, -a1);
#else
      a1 = XT_CONJC_S(a1);
#endif
      XT_SSX2IP(a0, p0_st, SZ_CF32);
      XT_SSX2XP(a1, p1_st, -(int)SZ_CF32);
    }
    a0 = XT_LSX2I(p0_ld, 0);
#if 0
    a0 = XT_SEL32_HL_SX2(a0, -a0);
#else
    a0 = XT_CONJC_S(a0);
#endif
    XT_SSX2I(a0, p0_st, 0);
  }
  return 0;
} /* fft_realf_ie() */

#endif
