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
DISCARD_FUN(void, mtx_det3x3f, (float32_t * restrict d, const float32_t *restrict x, int L))

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
void mtx_det3x3f(float32_t * restrict d, const float32_t *restrict x, int L)
{
        xtfloatx2    *  pz = (xtfloatx2      *)d;
  const xtfloatx2    *  restrict px = (const xtfloatx2      *)(x);

  int l;
  xtfloatx2 x01, x23, x45, x67, x80, x12, x34, x56, x78;
  xtfloatx2 vA, vB, vC, vD_temp, vE_temp, vF_temp, vG_temp, vH_temp, vI_temp;
  xtfloatx2 v2_temp, v3_temp, v4_temp, v5_temp, v6_temp ;
  xtfloatx2 res, A0, A1, A2, A3, A1_tmp, A2_tmp, A3_tmp;
  ae_valign xa,za;

  NASSERT_ALIGN(x, sizeof(*x));
  NASSERT_ALIGN(d, sizeof(*d));


  if (L <= 0) return;
  xa = AE_LA64_PP(px);
  za = AE_ZALIGN64();

  for (l = 0; l<(L >> 1); l++)
  {
    XT_LASX2IP(x01, xa, px);
    XT_LASX2IP(x23, xa, px);
    XT_LASX2IP(x45, xa, px);
    XT_LASX2IP(x67, xa, px);
    XT_LASX2IP(x80, xa, px);
    XT_LASX2IP(x12, xa, px);
    XT_LASX2IP(x34, xa, px);
    XT_LASX2IP(x56, xa, px);
    XT_LASX2IP(x78, xa, px);

    vA = XT_SEL32_HL_SX2(x01,x80);     		// a
    vB = XT_SEL32_LH_SX2(x01,x12);		    // b
    vC = XT_SEL32_HL_SX2(x23,x12);		    // c
    vD_temp = XT_SEL32_LH_SX2(x23,x34);		// d
    vE_temp = XT_SEL32_HL_SX2(x45,x34);    	// e
    vF_temp = XT_SEL32_LH_SX2(x45,x56);    	// f
    vG_temp = XT_SEL32_HL_SX2(x67,x56);		// g
    vH_temp = XT_SEL32_LH_SX2(x67,x78);    	// h
    vI_temp = XT_SEL32_HL_SX2(x80,x78);    	// i

    A0 = XT_MUL_SX2(vE_temp,vI_temp);  	    // ei
    XT_MSUB_SX2(A0,vF_temp,vH_temp);      	// ei-fh
    res = XT_MUL_SX2(A0,vA);      			// a(ei-fh)

    A1 = XT_MUL_SX2(vG_temp,vF_temp);		// gf
    XT_MSUB_SX2(A1,vD_temp,vI_temp);		// di-gf -> gf-di
    XT_MADD_SX2(res,A1,vB); 				// -b(di-gf) -> b(gf-di)

    A2 = XT_MUL_SX2(vD_temp,vH_temp);		// dh
    XT_MSUB_SX2(A2,vG_temp,vE_temp);        // dh-ge
    XT_MADD_SX2(res,A2,vC); 				// c(di-gf)

    XT_SASX2IP (res,za,pz);
  }
  AE_SA64POS_FP(za, pz);

  if (L&1)
  {
    XT_LASX2IP(x01, xa, px);
    XT_LASX2IP(x23, xa, px);
    XT_LASX2IP(x45, xa, px);
    XT_LASX2IP(x67, xa, px);
    XT_LASX2IP(x80, xa, px);

    v2_temp = XT_SEL32_HL_SX2(x80,x67);		// i,h
    v3_temp = XT_SEL32_LH_SX2(x23,x67);		// d,g
    v4_temp = XT_SEL32_HL_SX2(x80,x45);		// i,f
    v5_temp = XT_SEL32_LH_SX2(x23,x67);		// d,g
    v6_temp = XT_SEL32_LH_SX2(x67,x45);		// h,e

    A1 = XT_MUL_SX2(x45,v2_temp);  	    	// ei, fh
    A2 = XT_MUL_SX2(v3_temp,v4_temp);  	    // di, gf
    A3 = XT_MUL_SX2(v5_temp,v6_temp);  	    // dh, ge

    A1_tmp = XT_SEL32_LH_SX2(A1,A1);		// fh, ei
    A1 = XT_SUB_SX2(A1,A1_tmp);				// (ei-fh), x
    res = XT_MULMUX_S(x01,A1,0);			// a(ei-fh), x

    A2_tmp = XT_SEL32_LH_SX2(A2,A2);		// gf, di
    A2 = XT_SUB_SX2(A2, A2_tmp);			// (gf-di), x
    XT_MADDMUX_S(res,x01,A2,5);				// res + b(gf-di), x

    A3_tmp = XT_SEL32_LH_SX2(A3,A3);		// ge, dh
    A3 = XT_SUB_SX2(A3,A3_tmp);				// (dh-ge), x
    XT_MADDMUX_S(res,x23,A3,0);				// res +  c(dh-ge), x

    XT_SSIP(XT_HIGH_S(res), castxcc(xtfloat, pz), 4);
  }

} /*mtx_det3x3f()*/

#endif  /* #if  HAVE_VFPU */

