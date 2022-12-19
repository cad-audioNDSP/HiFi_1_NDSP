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
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, vec_poly4f, (float32_t * restrict z, const float32_t * restrict x, const float32_t * restrict c, int N))

#else
/*-------------------------------------------------------------------------
  Polynomial approximation
  Functions calculate polynomial approximation for all values from given 
  vector. Fixed point functions take polynomial coefficients in Q15 or 
  Q31 precision. 
  NOTE:
  approximation is calculated like Taylor series that is why overflow may 
  potentially occur if cumulative sum of coefficients given from the last 
  to the first coefficient is bigger than 1. To avoid this negative effect,
  all the coefficients may be scaled down and result will be shifted left 
  after all intermediate computations.

  Precision: 
  16x16  16-bit inputs, 16-bit coefficients, 16-bit output. 
  24x24  24-bit inputs, 24-bit coefficients, 24-bit output. 
  32x32  32-bit inputs, 32-bit coefficients, 32-bit output.
  f      floating point

  Input:
  x[N]    input data, Q15, Q31 or floating point
  N       length of vector
  lsh     additional left shift for result
  c[M+1]  coefficients (5 coefficients for vec_poly4_xxx 
          and 9 coefficients for vec_poly8_xxx), Q15, Q31 
          or floating point
  Output:
  z[N]    result, Q15, Q31 or floating point

  Restriction:
  x,c,z should not overlap
  lsh   should be in range 0...31
-------------------------------------------------------------------------*/
void vec_poly4f      (float32_t * restrict z, const float32_t * restrict x, const float32_t * restrict c, int N )
{
  int n;

  xtfloatx2 vxf;
  xtfloatx2 vc0f, vc1f, vc2f, vc3f, vc4f;
  xtfloat c0f, c1f, c2f, c3f, c4f;
  xtfloat xf, zf;

  const xtfloatx2* restrict px = (const xtfloatx2 *)x;
  const xtfloat *  restrict pc = (const xtfloat *)c;
        xtfloatx2* restrict pz = (xtfloatx2 *)z;
  ae_valign x_align, z_align;
  NASSERT(x);
  NASSERT(c);
  NASSERT(z);
  if (N <= 0) return;

  x_align = AE_LA64_PP(px);
  z_align = AE_ZALIGN64();

  XT_LSIP(c0f, pc, sizeof(*pc));
  XT_LSIP(c1f, pc, sizeof(*pc));
  XT_LSIP(c2f, pc, sizeof(*pc));
  XT_LSIP(c3f, pc, sizeof(*pc));
  XT_LSIP(c4f, pc, sizeof(*pc));

  vc0f = (c0f);
  vc1f = (c1f);
  vc2f = (c2f);
  vc3f = (c3f);
  vc4f = (c4f);

  XT_LASX2IP(vxf, x_align, px);
  for (n = 0; n < N>>1; n ++)
  {
      XT_MADD_SX2(vc3f, vxf, vc4f);
      XT_MADD_SX2(vc2f, vxf, vc3f);
      XT_MADD_SX2(vc1f, vxf, vc2f);
      XT_MADD_SX2(vc0f, vxf, vc1f);
      XT_LASX2IP(vxf, x_align, px);
      XT_SASX2IP(vc0f, z_align, pz);
      vc3f = c3f;
      vc2f = c2f;
      vc1f = c1f;
      vc0f = c0f;
  }
  AE_SA64POS_FP(z_align, pz);
  if (N & 1)
  {
    xf = XT_LSI((const xtfloat *)px, -8);
    XT_MADD_S(c3f, xf, c4f);
    XT_MADD_S(c2f, xf, c3f);
    XT_MADD_S(c1f, xf, c2f);
    XT_MADD_S(c0f, xf, c1f);
    zf = c0f;
    XT_SSI(zf, (xtfloat *)pz, 0);
  }
} /* vec_poly4f() */

#endif /* #if HAVE_VFPU */

