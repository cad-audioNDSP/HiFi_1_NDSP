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
DISCARD_FUN(void, cmtx_det3x3f, (complex_float   * restrict d, const complex_float   *restrict x, int L))

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
/* complex matrix determinant */
 
#define cmtx_2x2(A,X0,X1,X2,X3)    A=XT_MULC_S(X0, X3);\
                                   XT_MSUBC_S(A,X1,X2);
void cmtx_det3x3f(complex_float   * restrict d, const complex_float   *restrict x, int L)
{
  xtfloatx2 * pz = (xtfloatx2    *)d;
  const xtfloatx2    *  restrict px = (const xtfloatx2      *)x;
  int l;
   /* Allocate a fixed-size scratch area on the stack. */
  xtfloatx2 x0, x1, x2, x3, x4, x5, x6, x7, x8;
  xtfloatx2 A0, A1, A2,A3,A4,A5,A6,A7;

  NASSERT_ALIGN(x, sizeof(*x));
  NASSERT_ALIGN(d, sizeof(*d));
  if (L <= 0) return;
  /*
  * Data are processed in blocks of scratch area size.Further, the algorithm
  * implementation is splitted in order to feed the optimizing compiler with a
  * few loops of managable size.
  */
  for (l = 0; l<(L); l++)
  {
    XT_LSX2IP(x0, px, 8);
    XT_LSX2IP(x1, px, 8);
    XT_LSX2IP(x2, px, 8);
    XT_LSX2IP(x3, px, 8);
    XT_LSX2IP(x4, px, 8);
    XT_LSX2IP(x5, px, 8);
    XT_LSX2IP(x6, px, 8);
    XT_LSX2IP(x7, px, 8);
    XT_LSX2IP(x8, px, 8);

    cmtx_2x2(A0,x4,x5,x7,x8);
    cmtx_2x2(A1,x3,x5,x6,x8);
    cmtx_2x2(A2,x3,x4,x6,x7);


    A3=XT_MULC_S(x0, A0);
    A4=XT_MULC_S(x1, A1);
    A5=XT_MULC_S(x2, A2);

    A6=XT_SUB_SX2(A3,A4);
    A7=XT_ADD_SX2(A6,A5);

    XT_SSX2IP(A7,pz,8);

  }
    
} /* cmtx_det3x3f()*/


#endif  /* #if  HAVE_VFPU */



