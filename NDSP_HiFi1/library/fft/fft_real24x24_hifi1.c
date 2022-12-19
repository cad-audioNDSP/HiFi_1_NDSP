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
	NatureDSP Signal Processing Library. FFT part
    C code optimized for HiFi1
*/

#include "common.h"
#include "NatureDSP_Signal.h"
#include "fft_real_twiddles_24x24.h"
#include "fft_cplx_twiddles.h"
//#include "baseop.h"

/*
	in-place split part of FFT:
	x[N+2]	input/output
	N		size of FFT
*/

static inline ae_f32x2 AE_MULFC24RA_HIFI1(ae_f32x2 x, ae_f32x2 y)
{
  ae_f64 re, im;
  ae_f32x2 r;

  re = AE_MULF32S_HH(x, y);
  im = AE_MULF32S_LH(x, y);
  AE_MULSF32S_LL(re, x, y);
  AE_MULAF32S_LH(im, y, x);

  r = AE_ROUND24X2F48SASYM(re, im);
  return r;
}

static void splitPart_x2_24x24(f24 * x, int N)
{
  int i, step;
  const int step_back = -8;

  ae_f32x2 * restrict p_x0, * restrict p_x1;
  const ae_f32x2 * restrict p_twd;

  ae_int32x2  vA0, vA1, vB0, vB1, vC0, vC1, vR;
  ae_int32x2  vI;
  ae_f32x2    vT;
  ae_f32x2    vF2;
  ae_f32x2    vF0, vF1;

  NASSERT_ALIGN8(x);

  // split part: remainder
  {
    ae_q56s tmp = AE_CVTQ48A32S(N);
    step = 1<<(AE_NSAQ56S(tmp)-(38-MAX_RFFT_PWR));
    step *= sizeof(int32_t);
    step <<= 1;
  }

  p_twd = (const ae_f32x2 *)twiddleSplit24x24;
  p_x0 = (ae_f32x2 *)x;
  p_x1 = (ae_f32x2 *)(x+N);

  // load data and prepare pointers for pre-increment
  // first and last samples
  vF0 = AE_L32X2_I(p_x0, 0);
  vA0 = AE_SRAI32(vF0, 8);
  //vA0 = (vF0);
  vA1 = AE_SEL32_LH(vA0, vA0);
  vB0 = AE_ADD32S(vA0, vA1);
  vB1 = AE_SUB32S(vA0, vA1);
  vI  = 0;
  vA1 = 0;
  vB0 = AE_SEL32_HH(vB0, vA1);
  vB1 = AE_SEL32_HH(vB1, vA1);
  vF0 = (vB0);
  vF1 = (vB1);
  AE_S32X2_IP(AE_SLAI32(vF0, 8), p_x0, 8);
  AE_S32X2_XP(AE_SLAI32(vF1, 8), p_x1, step_back);

  vF0 = AE_L32X2_I(p_x0, 0);
  vF1 = AE_L32X2_I(p_x1, 0);
  vA0 = AE_SRAI32(vF0, 8);
  vA1 = AE_SRAI32(vF1, 8);

  vI = 1;
  vR = 1;

  AE_L32X2_XP(vT, p_twd, step);
  vT = AE_SRAI32(vT, 8);

  for (i = 1; i < N>>2; i++) 
  {
    // load twiddle
    AE_L32X2_XP(vT, p_twd, step);
	vT = AE_SRAI32(vT, 8);
    //vB1 = (vT);
    //vB1 = AE_SELP24_LH(vB1, vB1);
    //vT =  (vB1);

    // ADD/SUBB
    vB0 = AE_ADD32S(vA0, vA1);
    vB1 = AE_SUB32S(vA0, vA1);

    vA0 = AE_SEL32_HL(vB1, vB0);
    vB1 = AE_SEL32_HL(vB0, vB1);

    // do rotation
    vF1 =  (vA0);
    vF2 = AE_MULFC24RA_HIFI1((vF1),(vT));
    vB0 = (vF2);

    // load next data
    vF0 = AE_L32X2_I(p_x0, 8);
    vA0 = AE_SRAI32(vF0, 8);
    vF1 = AE_L32X2_X(p_x1, step_back);
    vA1 = AE_SRAI32(vF1, 8);

    vB0 = AE_ADD32S(vB0, vR);
    vB1 = AE_ADD32S(vB1, vR);
    vB0 = AE_SRAI32(vB0, 1);
    vB1 = AE_SRAI32(vB1, 1);

    // SUM/DIFF
    vC0 = AE_SUB32S(vB1, vB0);
    vC1 = AE_ADD32S(vB1, vB0);
    vB1 = AE_NEG32S(vC1);
    vC1 = AE_SEL32_HL(vC1, vB1);

    vF0 =  (vC0);
    AE_S32X2_IP(AE_SLAI32(vF0, 8), p_x0, 8);
    vF1 =  (vC1);
    AE_S32X2_XP(AE_SLAI32(vF1, 8), p_x1, step_back);
  }

  // middle sample
  vB0 = AE_NEG32S(vA0);
  vC0 = AE_SEL32_HL(vA0, vB0);
  vF0 =  (vC0);
  AE_S32X2_I(AE_SLAI32(vF0, 8), p_x0, 0);
}

/*-------------------------------------------------------------------------
  FFT on real data forming half of spectrum

  Precision: 
  32x32  32-bit input/outputs, 32-bit twiddles
  24x24  24-bit input/outputs, 24-bit twiddles
  32x16  32-bit input/outputs, 16-bit twiddles
  16x16  16-bit input/outputs, 16-bit twiddles

  NOTES:
  1. Bit-reversal reordering is done here. 
  2. FFT runs in-place so INPUT DATA WILL APPEAR DAMAGED after the call.
  3. Real data FFT function calls fft_cplx() to apply complex FFT of size
     N/2 to input data and then transforms the resulting spectrum.

  Input:
  x[N]         - input signal
  N            - FFT size
  scalingOpt   - scaling option:
                0 - no scaling
                1 - 24-bit scaling
                2 - 32-bit scaling on the first stage and 24-bit scaling later
                3 - fixed scaling
  Output:
  y[(N/2+1)*2] - output spectrum (positive side)

  Restrictions:
  Arrays should not overlap
  x,y - aligned on a 8-bytes boundary
  N   - 2^m: 32...8192

-------------------------------------------------------------------------*/
int fft_real24x24( 
              f24* y,
              int32_t* x,
              fft_handle_t h,
              int scalingOpt)
{
  int scale;
  int N;

  ae_f32x2 *    restrict  px_24;

  ae_f32x2    vA0, vA1, vB0, vB1;

  NASSERT_ALIGN8(x);
  NASSERT_ALIGN8(y);
  NASSERT(scalingOpt>=0 && scalingOpt<=3);

  N=(((const tFftDescr*)h)->N)<<1;

  scale=fft_cplx24x24(y, x, h, scalingOpt);    /* fft of half-size              */
  
  if (scalingOpt)
  {
    int exp, k;

    exp=3-vec_bexp24(y,N);
    scale += exp;

    px_24 = (ae_f32x2 *)y;

    for (k=0; k<N/4; k++)
    {
      AE_L32X2_IP(vA0, px_24, 8);
      AE_L32X2_IP(vA1, px_24, 8);
      vB0 = AE_SRAA32(vA0, exp);
      vB1 = AE_SRAA32(vA1, exp);
      AE_S32X2_I(vB0, px_24, -16);
      AE_S32X2_I(vB1, px_24, -8);
    }
  }

  splitPart_x2_24x24(y, N);	    /* convert results to normal FFT */

  return scale;
}
