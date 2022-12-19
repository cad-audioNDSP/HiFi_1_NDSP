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
#include "NatureDSP_Signal.h"
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(float32_t, scl_complex2invmag, (complex_float x))

#else
#include "inff_tbl.h"

/*===========================================================================
  Scalar matematics:
  scl_complex2invmag             Complex magnitude (reciprocal) 
===========================================================================*/
/*-------------------------------------------------------------------------
  Complex magnitude
  routines compute complex magnitude or its reciprocal

  Precision: 
  f     single precision floating point

  Output:
  y[N]  output data
  Input:
  x[N]  input complex data
  N     length of vector

  Restriction:
  none
-------------------------------------------------------------------------*/

float32_t  scl_complex2invmag (complex_float x)
{
  /*
  float32_t x_re,x_im;
  float32_t mnt_re, mnt_im, mnt_abs;
  int exp_re, exp_im, exp_abs;
  const int minexp = FLT_MIN_EXP - FLT_MANT_DIG;

  x_re = fabsf( crealf(x) );
  x_im = fabsf( cimagf(x) );

  exp_re = ( x_re != 0 ? (int)STDLIB_MATH(ceilf)( log2f(x_re) ) : minexp );
  exp_im = ( x_im != 0 ? (int)STDLIB_MATH(ceilf)( log2f(x_im) ) : minexp );

  exp_abs = ( exp_re > exp_im ? exp_re : exp_im );

  mnt_re = STDLIB_MATH(ldexpf)( x_re, -exp_abs );
  mnt_im = STDLIB_MATH(ldexpf)( x_im, -exp_abs );

  mnt_abs = 1.f/sqrtf( mnt_re*mnt_re + mnt_im*mnt_im );

  return STDLIB_MATH(ldexpf)( mnt_abs, -exp_abs );

  */
  complex_float x_tmp;
  xtfloat xf;
  {
    xtfloatx2 x0, x1, y0, z0, xre, xim, MAXEXP, _0;
    ae_int32x2 u0, u1, I0, QNAN;
    xtbool2 bnan, binf, bzero;
    MAXEXP = XT_AE_MOVXTFLOATX2_FROMINT32X2(0x7f000000);/*254 << 23*/
    QNAN = 0xffc00000;/* Unordered */
    I0 = 0x7f800000;
    _0 = (xtfloatx2)0.0f;

    x_tmp = x;
    x0 = XT_LSX2I((xtfloatx2 *)&x_tmp, 0);

    x0 = XT_ABS_SX2(x0); 
    xre = XT_SEL32_HH_SX2(x0, x0);
    xim = XT_SEL32_LL_SX2(x0, x0);
    u0 = XT_AE_MOVINT32X2_FROMXTFLOATX2(xre);
    u1 = XT_AE_MOVINT32X2_FROMXTFLOATX2(xim);
    bnan = XT_UN_SX2(xre, xim); /* is NaN */

    u0 = AE_AND32(u0, I0);
    u1 = AE_AND32(u1, I0);
    u0 = AE_MAX32(u0, u1);
    binf = AE_EQ32(u0, I0); /* is Inf */

    u0 = AE_SUB32(I0, u0);
    y0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(u0);
    y0 = XT_MIN_SX2(y0, MAXEXP);
    xre = XT_MUL_SX2(xre, y0);
    xim = XT_MUL_SX2(xim, y0);

    x0 = XT_MUL_SX2(xre, xre);
    x1 = XT_MUL_SX2(xim, xim);
    x0 = XT_ADD_SX2(x0, x1);

    z0 = XT_RSQRT_SX2(x0);

    bzero = XT_OEQ_SX2(x0, _0); /* is zero */
    u0 = XT_AE_MOVINT32X2_FROMXTFLOATX2(z0);
    AE_MOVT32X2(u0, I0, bzero);
    AE_MOVT32X2(u0, AE_ZERO32(), binf);
    AE_MOVT32X2(u0, QNAN, bnan);
    z0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(u0);
    y0 = XT_MUL_SX2(y0, z0);
    xf = XT_HIGH_S(y0);
  }
  return xf;
}

#endif /* #if HAVE_VFPU */

