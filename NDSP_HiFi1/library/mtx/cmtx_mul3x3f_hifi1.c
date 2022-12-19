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
DISCARD_FUN(void, cmtx_mul3x3f, (complex_float   * restrict z, const complex_float   *restrict x, const complex_float   *restrict y, int L))

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
/* complex matrix multiply */

void cmtx_mul3x3f(complex_float   * restrict z, const complex_float   *restrict x, const complex_float   *restrict y, int L)
{
  const xtfloatx2    *  restrict py0 = (const xtfloatx2    *)y;
  const xtfloatx2    *  restrict temp_py0;
        xtfloatx2    *  pz = (xtfloatx2      *)z;
  const xtfloatx2    *  restrict px = (const xtfloatx2      *)x;
  int l, m,n;
  xtfloatx2 vx0, vy0, vy1;
  xtfloatx2 vy2;
  xtfloatx2 A0, A1, A2;
  NASSERT_ALIGN(x, sizeof(*x));
  NASSERT_ALIGN(y, sizeof(*y));
  NASSERT_ALIGN(z, sizeof(*z));

  for (l = 0; l<L; l++)
  {
    temp_py0 = (const xtfloatx2    *)(y + 0 + 9 * l);
    for (n=0;n<3;n++)
    {
      py0=temp_py0;

      XT_LSX2IP(vx0, px, 8);
      XT_LSX2IP(vy0, py0, 8);
      XT_LSX2IP(vy1, py0, 8);
      XT_LSX2IP(vy2, py0, 8);
      
      A0=XT_MULC_S(vx0,vy0);
      A1=XT_MULC_S(vx0,vy1);
      A2=XT_MULC_S(vx0,vy2);

      for (m=1;m<3;m++)
      {
        XT_LSX2IP(vx0, px, 8);
        XT_LSX2IP(vy0, py0, 8);
        XT_LSX2IP(vy1, py0, 8);
        XT_LSX2IP(vy2, py0, 8);

        XT_MADDC_S(A0,vx0,vy0);
        XT_MADDC_S(A1,vx0,vy1);
        XT_MADDC_S(A2,vx0,vy2);
      }
        XT_SSX2IP(A0,pz,8);
        XT_SSX2IP(A1,pz,8);
        XT_SSX2IP(A2,pz,8);
        
    }
  }
} /* cmtx_mul3x3f() */

#endif  /* #if  HAVE_VFPU */


