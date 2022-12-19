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
DISCARD_FUN(void, vec_complex2invmag, (float32_t  * restrict y, const complex_float  * restrict x, int N))

#else
#include "inff_tbl.h"

/*===========================================================================
  Vector matematics:
  vec_complex2invmag     complex magnitude (reciprocal)
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
void       vec_complex2invmag (float32_t  * restrict y, const complex_float  * restrict x, int N)
{
  /*
  union ufloat32uint32 R, I, T, X, Z, TMP, U0, U1;
  float32_t mnt_re, mnt_im;
  T.u = 0x7f800000;
  TMP.u = 0x7f000000;

  R.f = fabsf( crealf(x) );
  I.f = fabsf( cimagf(x) );

  if (isnan(R.f) || isnan(I.f)) return NAN;
  if (isinf(R.f) || isinf(I.f)) return 0;


  U0.u = R.u&T.u;
  U1.u = I.u&T.u;
  X.u = MAX(U0.u, U1.u);
  T.u = (T.u - X.u );
  T.f = fminf(T.f, TMP.f); 
  mnt_re = R.f*T.f;
  mnt_im = I.f*T.f;
  Z.f = 1.f / sqrtf(mnt_re*mnt_re + mnt_im*mnt_im);
  Z.f = Z.f*T.f;
  return Z.f;
  */
  const xtfloatx2 * restrict X = (const xtfloatx2 *)x;
        xtfloatx2 * restrict Y = (xtfloatx2 *)y;
  int n;
  xtfloatx2 x0, x1, y0, z0, xre, xim, MAXEXP, _0;
  xtfloat l;
  ae_int32x2 u0, u1, I0, QNAN;
  xtbool2 bnan, binf, bzero;
  ae_valign Y_va;
  if (N <= 0) return;

  Y_va = AE_ZALIGN64();
  MAXEXP = XT_AE_MOVXTFLOATX2_FROMINT32X2(0x7f000000);/*254 << 23*/
  QNAN = 0xffc00000;/* Unordored */
  I0 = 0x7f800000;
  _0 = (xtfloatx2)0.0f;
  for (n = 0; n<(N >> 2); n++) 
  {
    XT_LSX2IP(x0, X, sizeof(complex_float));
    XT_LSX2IP(x1, X, sizeof(complex_float));

    MAXEXP = XT_AE_MOVXTFLOATX2_FROMINT32X2(0x7f000000);/*254 << 23*/
    QNAN = 0xffc00000;/* Unordored */
    I0 = 0x7f800000;
    _0 = (xtfloatx2)0.0f;

    x0 = XT_ABS_SX2(x0); 
    x1 = XT_ABS_SX2(x1);
    xre = XT_SEL32_HH_SX2(x0, x1);
    xim = XT_SEL32_LL_SX2(x0, x1);
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
    XT_SASX2IP(y0, Y_va, Y);


    XT_LSX2IP(x0, X, sizeof(complex_float));
    XT_LSX2IP(x1, X, sizeof(complex_float));

  
    x0 = XT_ABS_SX2(x0); 
    x1 = XT_ABS_SX2(x1);
    xre = XT_SEL32_HH_SX2(x0, x1);
    xim = XT_SEL32_LL_SX2(x0, x1);
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
    XT_SASX2IP(y0, Y_va, Y);
  }
  if (N&2)
  {
    XT_LSX2IP(x0, X, sizeof(complex_float));
    XT_LSX2IP(x1, X, sizeof(complex_float));

    MAXEXP = XT_AE_MOVXTFLOATX2_FROMINT32X2(0x7f000000);/*254 << 23*/
    QNAN = 0xffc00000;/* Unordored */
    I0 = 0x7f800000;
    _0 = (xtfloatx2)0.0f;

    x0 = XT_ABS_SX2(x0); 
    x1 = XT_ABS_SX2(x1);
    xre = XT_SEL32_HH_SX2(x0, x1);
    xim = XT_SEL32_LL_SX2(x0, x1);
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
    XT_SASX2IP(y0, Y_va, Y);

  }
  XT_SASX2POSFP(Y_va, Y);

  if (N&1)
  {
    MAXEXP = XT_AE_MOVXTFLOATX2_FROMINT32X2(0x7f000000);/*254 << 23*/
    QNAN = 0xffc00000;/* Unordored */
    I0 = 0x7f800000;
    _0 = (xtfloatx2)0.0f;
    XT_LSX2IP(x0, X, sizeof(complex_float));

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
    l = XT_LOW_S(y0);
    XT_SSX(l, (xtfloat *)Y, 0);
  }
}

#endif /* #if HAVE_VFPU */

