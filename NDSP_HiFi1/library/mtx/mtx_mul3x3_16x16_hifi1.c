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

void mtx_mul3x3_16x16(int16_t   * restrict z, const int16_t   *restrict x, const int16_t   *restrict y, int rsh, int L)
{
  const ae_int16x4    *  restrict py = (const ae_int16x4    *)y;
  ae_int16x4    *  pz = (ae_int16x4      *)z;

  const ae_int16x4    *  restrict px = (const ae_int16x4      *)x;
  int l;

  ae_int16x4 vx0, vy0, vy2, vx1, vx2, vy1;
  ae_int32x2 t0, t1, y0, y1, y2, y3, y4;
  ae_int16x4 vz0;
  ae_int64 A0, A1, A2, A3, A4, A5;
  ae_valign      y_align, x_align, z_align;
  NASSERT_ALIGN(x, sizeof(*x));
  NASSERT_ALIGN(y, sizeof(*y));
  NASSERT_ALIGN(z, sizeof(*z));
  NASSERT(rsh >= 0 && rsh <= 15);
  x_align = AE_LA64_PP(px);
  y_align = AE_LA64_PP(py);
  z_align = AE_ZALIGN64();
  if (L<=0) return;
  pz = (ae_int16x4      *)z;
  z_align = AE_ZALIGN64();
  for (l = 0; l<(L-1); l++,x+=9, y+=9, z+=9)
  {
    px = (const ae_int16x4      *)x;
    x_align = AE_LA64_PP(px);
    py = (const ae_int16x4      *)y;
    y_align = AE_LA64_PP(py);
    
    AE_LA16X4_IP(vx0, x_align, px);
    AE_LA16X4_IP(vx1, x_align, px);
    AE_LA16X4_IP(vx2, x_align, px);
    AE_LA16X4_IP(vy0, y_align, py);
    AE_LA16X4_IP(vy1, y_align, py);
    AE_LA16X4_IP(vy2, y_align, py);

    y0 = AE_SEXT32X2D16_32(vy0);
    y1 = AE_SEXT32X2D16_10(vy0);
    y2 = AE_SEXT32X2D16_32(vy1);
    y3 = AE_SEXT32X2D16_10(vy1);
    y4 = AE_SEXT32X2D16_32(vy2);

    A0 = AE_MUL32X16_H3(y0, vx0);
    AE_MULA32X16_L2(A0, y1, vx0);
    AE_MULA32X16_H1(A0, y3, vx0);

    A1 = AE_MUL32X16_L3(y0, vx0);
    AE_MULA32X16_H2(A1, y2, vx0);
    AE_MULA32X16_L1(A1, y3, vx0);

    A2 = AE_MUL32X16_H3(y1, vx0);
    AE_MULA32X16_L2(A2, y2, vx0);
    AE_MULA32X16_H1(A2, y4, vx0);

    A3 = AE_MUL32X16_H0(y0, vx0);
    AE_MULA32X16_L3(A3, y1, vx1);
    AE_MULA32X16_H2(A3, y3, vx1);

    A4 = AE_MUL32X16_L0(y0, vx0);
    AE_MULA32X16_H3(A4, y2, vx1);
    AE_MULA32X16_L2(A4, y3, vx1);

    A5 = AE_MUL32X16_H0(y1, vx0);
    AE_MULA32X16_L3(A5, y2, vx1);
    AE_MULA32X16_H2(A5, y4, vx1);

    A0 = AE_SLAA64(A0, 17 - rsh);
    A1 = AE_SLAA64(A1, 17 - rsh);
    A2 = AE_SLAA64(A2, 17 - rsh);
    A3 = AE_SLAA64(A3, 17 - rsh);
    t0 = AE_ROUND32X2F48SASYM(A0, A1);
    t1 = AE_ROUND32X2F48SASYM(A2, A3);
    vz0 = AE_ROUND16X4F32SASYM(t0, t1);
    AE_SA16X4_IP(vz0, z_align, pz);

    A0 = AE_MUL32X16_H1(y0, vx1);
    AE_MULA32X16_L0(A0, y1, vx1);
    AE_MULA32X16_H3(A0, y3, vx2);

    A1 = AE_MUL32X16_L1(y0, vx1);
    AE_MULA32X16_H0(A1, y2, vx1);
    AE_MULA32X16_L3(A1, y3, vx2);

    A2 = AE_MUL32X16_H1(y1, vx1);
    AE_MULA32X16_L0(A2, y2, vx1);
    AE_MULA32X16_H3(A2, y4, vx2);

    A4 = AE_SLAA64(A4, 17 - rsh);
    A5 = AE_SLAA64(A5, 17 - rsh);
    A0 = AE_SLAA64(A0, 17 - rsh);
    A1 = AE_SLAA64(A1, 17 - rsh);
    t0 = AE_ROUND32X2F48SASYM(A4, A5);
    t1 = AE_ROUND32X2F48SASYM(A0, A1);
    vz0 = AE_ROUND16X4F32SASYM(t0, t1);
    AE_SA16X4_IP(vz0, z_align, pz);
    AE_SA64POS_FP(z_align, pz);
    t0 = AE_TRUNCA32X2F64S(A2, A2, 33 - rsh);
    vz0 = AE_ROUND16X4F32SASYM(t0, t0);
    AE_S16_0_IP(vz0, (ae_int16 *)pz, 2);
  }
  px = (const ae_int16x4      *)x;
  x_align = AE_LA64_PP(px);
  py = (const ae_int16x4      *)y;
  y_align = AE_LA64_PP(py);
  pz = (ae_int16x4      *)z;
  z_align = AE_ZALIGN64();
  AE_LA16X4_IP(vx0, x_align, px);
  AE_LA16X4_IP(vx1, x_align, px);
  AE_LA16X4_IP(vx2, x_align, px);
  AE_LA16X4_IP(vy0, y_align, py);
  AE_LA16X4_IP(vy1, y_align, py);
  AE_LA16X4_IP(vy2, y_align, py);

  y0 = AE_SEXT32X2D16_32(vy0);
  y1 = AE_SEXT32X2D16_10(vy0);
  y2 = AE_SEXT32X2D16_32(vy1);
  y3 = AE_SEXT32X2D16_10(vy1);
  y4 = AE_SEXT32X2D16_32(vy2);


  A0 = AE_MUL32X16_H3(y0, vx0);
  AE_MULA32X16_L2(A0, y1, vx0);
  AE_MULA32X16_H1(A0, y3, vx0);

  A1 = AE_MUL32X16_L3(y0, vx0);
  AE_MULA32X16_H2(A1, y2, vx0);
  AE_MULA32X16_L1(A1, y3, vx0);

  A2 = AE_MUL32X16_H3(y1, vx0);
  AE_MULA32X16_L2(A2, y2, vx0);
  AE_MULA32X16_H1(A2, y4, vx0);

  A3 = AE_MUL32X16_H0(y0, vx0);
  AE_MULA32X16_L3(A3, y1, vx1);
  AE_MULA32X16_H2(A3, y3, vx1);

  A4 = AE_MUL32X16_L0(y0, vx0);
  AE_MULA32X16_H3(A4, y2, vx1);
  AE_MULA32X16_L2(A4, y3, vx1);

  A5 = AE_MUL32X16_H0(y1, vx0);
  AE_MULA32X16_L3(A5, y2, vx1);
  AE_MULA32X16_H2(A5, y4, vx1);

  A0 = AE_SLAA64(A0, 17 - rsh);
  A1 = AE_SLAA64(A1, 17 - rsh);
  A2 = AE_SLAA64(A2, 17 - rsh);
  A3 = AE_SLAA64(A3, 17 - rsh);
  t0 = AE_ROUND32X2F48SASYM(A0, A1);
  t1 = AE_ROUND32X2F48SASYM(A2, A3);
  vz0 = AE_ROUND16X4F32SASYM(t0, t1);
  AE_SA16X4_IP(vz0, z_align, pz);

  A0 = AE_MUL32X16_H1(y0, vx1);
  AE_MULA32X16_L0(A0, y1, vx1);
  AE_MULA32X16_H3(A0, y3, vx2);

  A1 = AE_MUL32X16_L1(y0, vx1);
  AE_MULA32X16_H0(A1, y2, vx1);
  AE_MULA32X16_L3(A1, y3, vx2);

  A2 = AE_MUL32X16_H1(y1, vx1);
  AE_MULA32X16_L0(A2, y2, vx1);
  AE_MULA32X16_H3(A2, y4, vx2);

  A4 = AE_SLAA64(A4, 17 - rsh);
  A5 = AE_SLAA64(A5, 17 - rsh);
  A0 = AE_SLAA64(A0, 17 - rsh);
  A1 = AE_SLAA64(A1, 17 - rsh);
  t0 = AE_ROUND32X2F48SASYM(A4, A5);
  t1 = AE_ROUND32X2F48SASYM(A0, A1);
  vz0 = AE_ROUND16X4F32SASYM(t0, t1);
  AE_SA16X4_IP(vz0, z_align, pz);
  AE_SA64POS_FP(z_align, pz);
  t0 = AE_TRUNCA32X2F64S(A2, A2, 33 - rsh);
  vz0 = AE_ROUND16X4F32SASYM(t0, t0);
  AE_S16_0_IP(vz0, (ae_int16 *)pz, 2);
 
}

