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

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
/* Common helper macros. */
#include "common.h" 

#if !(HAVE_VFPU)
DISCARD_FUN(void, mtx_det4x4f, (float32_t * restrict d, const float32_t *restrict x, int L))

#else
/*-------------------------------------------------------------------------
  Operations with Small Matrices
  These functions implement basic operations under the sequence of small 
  square matrices. Fixed point data are interpreted as Q15 or Q31 and 
  results might be saturated.
  NOTE: 
  Determinant is computed recursively via minors of submatrices. So, in 
  the fixed-point routines, intermediate results might be saturated 
  although final result is in range. To avoid this saturation, right shift 
  might be applied at the first stage of computations. It means that final 
  result would be represented in Q(15-rsh) or Q(31-rsh) respectively. 
  Ad-hoc formula for rsh is rsh>=N-2 for real matrices and rsh>=N-1 for 
  complex matrices.

  Precision: 
  16x16  16-bit input, 16-bit output (real and complex)
  32x32  32-bit input, 32-bit output (real and complex)
  f      floating point (real and complex)

  Matrix dimensions are 2x2, 3x3, 4x4

  Input:
  x[L][N*N]      L input matrices
  y[L][N*N]      L input matrices (for addition, subtraction, multiply 
                 functions)
  rsh            right shift for fixed-point multiply and determinant 
                 function
  L              number of matrices
  Output:
  z[L][N*N]      L output matrices (for addition, subtraction, multiply, 
                 transpose functions)
  d[L]           determinants for L matrices (for determinant functions)

  Restrictions:
  rsh should be in range 0..15
  x,y,z should not overlap
-------------------------------------------------------------------------*/
/* real matrix determinant */
void mtx_det4x4f     (float32_t * restrict d, const float32_t *restrict x,          int L)
{
          xtfloat    *  pz = (xtfloat      *)d;
  const xtfloatx2    *  restrict px = (const xtfloatx2      *)(x);
  int l;
  xtfloatx2 x01, x23, x45, x67;
  xtfloatx2 B2A1, xef_R, B1A2, A0V,A0_R,xab_R,C2V,C2_R,xcd_R,B1A1,A1A2,A2B2,B2B1,x67_R;
  xtfloatx2 M10,M23,M01,A01,A10;

  xtfloatx2 x89, xab, xcd, xef;
  ae_valign xa;
  NASSERT_ALIGN(x, sizeof(*x));
  NASSERT_ALIGN(d, sizeof(*d));
  if (L <= 0) return;
  xa = AE_LA64_PP(px);

  for (l = 0; l<L; l++)
  {

    XT_LASX2IP(x89, xa, px);
    XT_LASX2IP(xab, xa, px);
    XT_LASX2IP(xcd, xa, px);
    XT_LASX2IP(xef, xa, px);

    B2A1=XT_MUL_SX2(x89,xef);
    xef_R=XT_SEL32_LH_SX2(xef,xef);
    B1A2=XT_MUL_SX2(x89,xef_R);
    A0V=XT_MUL_SX2(xab,xef_R);

    A0_R=XT_SEL32_LH_SX2(A0V,A0V);
    A0V=XT_SUB_SX2(A0V,A0_R);
    XT_MSUB_SX2(B2A1,xcd,xab);
    xab_R=XT_SEL32_LH_SX2(xab,xab);
    XT_MSUB_SX2(B1A2,xcd,xab_R);
    xcd_R=XT_SEL32_LH_SX2(xcd,xcd);
    C2V=XT_MUL_SX2(x89,xcd_R);
    C2_R=XT_SEL32_LH_SX2(C2V,C2V);
    C2V=XT_SUB_SX2(C2V,C2_R);

    XT_LASX2IP(x01, xa, px);
    XT_LASX2IP(x23, xa, px);
    XT_LASX2IP(x45, xa, px);
    XT_LASX2IP(x67, xa, px);

    B1A1=XT_SEL32_HL_SX2(B1A2,B2A1);
    A1A2=XT_SEL32_LL_SX2(B2A1,B1A2);
    A2B2=XT_SEL32_LH_SX2(B1A2,B2A1);
    B2B1=XT_SEL32_HH_SX2(B2A1,B1A2);
    x67_R=XT_SEL32_LH_SX2(x67,x67);

    M10=XT_MULMUX_S(A0V,x45,0);
    XT_MADDMUX_S(M10,x67,B1A1,2);
    XT_MADDMUX_S(M10,x67,A2B2,5);

    M23=XT_MULMUX_S(x45,A1A2,0);
    XT_MADDMUX_S(M23,x45,B2B1,7);
    XT_MADDMUX_S(M23,C2V,x67_R,0);

    M01=XT_SEL32_LH_SX2(M10,M10);
    A01=XT_MUL_SX2(x01,M01);
    XT_MADD_SX2(A01,x23,M23);
    A10=XT_SEL32_LH_SX2(A01,A01);
    A01=XT_SUB_SX2(A01,A10);

    XT_SSIP(XT_HIGH_S(A01), pz, 4);

  }

} /* mtx_det4x4f() */

#endif  /* #if  HAVE_VFPU */


