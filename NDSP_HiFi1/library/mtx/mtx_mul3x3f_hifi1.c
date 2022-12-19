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
DISCARD_FUN(void, mtx_mul3x3f, (float32_t * restrict z, const float32_t *restrict x, const float32_t *restrict y, int L))

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
/* real matrix multiply */
void mtx_mul3x3f(float32_t * restrict z, const float32_t *restrict x, const float32_t *restrict y, int L)
{
  const xtfloatx2    *  restrict py = (const xtfloatx2    *)y;
        xtfloatx2    *  restrict pz = (      xtfloatx2    *)z;
  const xtfloatx2    *  restrict px = (const xtfloatx2    *)x;

  xtfloat    *  restrict pz1 = (xtfloat    *)(z+8);
  int l;
  xtfloatx2 vx0, vy0, vx1, vy1, vy2, vy3, vy4,vx2,vx3,vx4;
  xtfloatx2 A0,A1,A2,A3,A4,A5,A1_f,A2_f,vx0_temp,vx1_temp,vx2_temp,vy1_temp,vy2_temp,vx3_temp;
  xtfloat A_scalar;
  ae_valign      y_align, x_align, z_align;
  NASSERT_ALIGN(x, sizeof(*x));
  NASSERT_ALIGN(y, sizeof(*y));
  NASSERT_ALIGN(z, sizeof(*z));

  x_align = AE_LA64_PP(px);
  y_align = AE_LA64_PP(py);
  z_align = AE_ZALIGN64();
 
  for (l = 0; l<L; l++)
  {
      py = (const xtfloatx2    *)(y+9*l);
      px = (const xtfloatx2    *)(x + 9 * l);
      pz = (xtfloatx2    *)(z + 9 *l );


      XT_LASX2IP(vy0, y_align, py);
      XT_LASX2IP(vy1, y_align, py);
      XT_LASX2IP(vy2, y_align, py);
      XT_LASX2IP(vy3, y_align, py);
      XT_LASX2IP(vy4, y_align, py);

  
      XT_LASX2IP(vx0, x_align, px);
      XT_LASX2IP(vx1, x_align, px);
      XT_LASX2IP(vx2, x_align, px);
      XT_LASX2IP(vx3, x_align, px);
      XT_LASX2IP(vx4, x_align, px);
     
      
      
      A0=XT_MULMUX_S(vx0,vy0,0);
      A1=XT_MULMUX_S(vx0,vy1,0);
      vx0_temp=XT_SEL32_LH_SX2(vx0,vx0);
      vy1_temp=XT_SEL32_LH_SX2(vy1,vy2);
      XT_MADDMUX_S(A0,vx0_temp,vy1_temp,0);
      vy2_temp=XT_SEL32_LH_SX2(vy2,vy2);
      XT_MADDMUX_S(A1,vx0_temp,vy2_temp,0);
      XT_MADDMUX_S(A0,vx1,vy3,0);
      XT_MADDMUX_S(A1,vx1,vy4,0);



      vx1_temp=XT_SEL32_LH_SX2(vx1,vx2);
      A2=XT_MULMUX_S(vx1_temp,vy0,0);
      A3=XT_MULMUX_S(vx1_temp,vy1,0);
      XT_MADDMUX_S(A2,vx2,vy1_temp,0);
      XT_MADDMUX_S(A3,vx2,vy2_temp,0);
      vx2_temp=XT_SEL32_LH_SX2(vx2,vx2);
      XT_MADDMUX_S(A2,vx2_temp,vy3,0);
      XT_MADDMUX_S(A3,vx2_temp,vy4,0);

      A1_f=XT_SEL32_HH_SX2(A1,A2);
      A2_f=XT_SEL32_LH_SX2(A2,A3);

      A4=XT_MULMUX_S(vx3,vy0,0);
      A5=XT_MULMUX_S(vx3,vy1,0);
      vx3_temp=XT_SEL32_LH_SX2(vx3,vx3);
      XT_MADDMUX_S(A4,vx3_temp,vy1_temp,0);
      XT_MADDMUX_S(A5,vx3_temp,vy2_temp,0);
      XT_MADDMUX_S(A4,vx4,vy3,0);
      XT_MADDMUX_S(A5,vx4,vy4,0);

      A_scalar=XT_HIGH_S(A5);
      XT_SASX2IP (A0,z_align,pz);
      XT_SASX2IP (A1_f,z_align,pz);
      XT_SASX2IP (A2_f,z_align,pz);
      XT_SASX2IP (A4,z_align,pz);
      
      XT_SSIP(A_scalar, pz1, 4*9);
      AE_SA64POS_FP(z_align, pz);

  }

  
 
} /* mtx_mul3x3f() */

#endif  /* #if  HAVE_VFPU */


