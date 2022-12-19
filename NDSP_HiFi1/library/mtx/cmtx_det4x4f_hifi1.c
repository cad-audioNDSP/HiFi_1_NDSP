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
DISCARD_FUN(void, cmtx_det4x4f, (complex_float   * restrict d, const complex_float   *restrict x, int L))

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
#define sz_f    (int)sizeof(xtfloat)

void cmtx_det4x4f(complex_float   * restrict d, const complex_float   *restrict x, int L)
{
  xtfloatx2    *  pz = (xtfloatx2      *)d;
  const xtfloatx2    *  restrict px = (const xtfloatx2      *)(x + 4);
  const xtfloatx2    *          S_rd;
        xtfloatx2    *          S_wr;
  int l;
  /* Current block index; overall number of blocks; number of values in the current block */
  int blkIx, blkNum, blkLen;
  /* Block size, blkLen <= blkSize */
  const int blkSize = (MAX_ALLOCA_SZ) / (36 * 2 * sz_f);
  /* Allocate a fixed-size scratch area on the stack. */
  xtfloatx2 ALIGN(8) scr[36 * 2 * blkSize];
  xtfloatx2 x4, x5, x6, x7, x8;
  xtfloatx2 A0, A1, A2, A3, A4;
  
  NASSERT_ALIGN(x, sizeof(*x));
  NASSERT_ALIGN(d, sizeof(*d));
  if (L <= 0) return;

  /*
  * Data are processed in blocks of scratch area size.Further, the algorithm
  * implementation is splitted in order to feed the optimizing compiler with a
  * few loops of managable size.
  */

  blkNum = (L + blkSize - 1) / blkSize;
  px = (const xtfloatx2      *)(x);
  for (blkIx = 0; blkIx<blkNum; blkIx++)
  {
    xtfloatx2 x0, x1, x2, x3;
    xtfloatx2 y0, y1, y2, y3;
    blkLen = XT_MIN(L - blkIx*blkSize, blkSize);
    px = (const xtfloatx2      *)(x + 4);
    S_wr = (xtfloatx2*)scr;

    {
      xtfloatx2 x4, x5, x6, x7;
      xtfloatx2 x8, x9, x10, x11, x12, x13, x14, x15;
      XT_LSX2IP(x4, px, 8);
      XT_LSX2IP(x5, px, 8);
      XT_LSX2IP(x6, px, 8);
      XT_LSX2IP(x7, px, 8);
      XT_LSX2IP(x8, px, 8);
      XT_LSX2IP(x9, px, 8);
      XT_LSX2IP(x10, px, 8);
      XT_LSX2IP(x11, px, 8);
      XT_LSX2IP(x12, px, 8);
      XT_LSX2IP(x13, px, 8);
      XT_LSX2IP(x14, px, 8);
      XT_LSX2IP(x15, px, 5*8);

      XT_SSX2IP(x5, S_wr, 8);
      XT_SSX2IP(x6, S_wr, 8);
      XT_SSX2IP(x7, S_wr, 8);
      XT_SSX2IP(x9, S_wr, 8);
      XT_SSX2IP(x10, S_wr, 8);
      XT_SSX2IP(x11, S_wr, 8);
      XT_SSX2IP(x13, S_wr, 8);
      XT_SSX2IP(x14, S_wr, 8);
      XT_SSX2IP(x15, S_wr, 8);

      XT_SSX2IP(x4, S_wr, 8);
      XT_SSX2IP(x6, S_wr, 8);
      XT_SSX2IP(x7, S_wr, 8);
      XT_SSX2IP(x8, S_wr, 8);
      XT_SSX2IP(x10, S_wr, 8);
      XT_SSX2IP(x11, S_wr, 8);
      XT_SSX2IP(x12, S_wr, 8);
      XT_SSX2IP(x14, S_wr, 8);
      XT_SSX2IP(x15, S_wr, 8);

      XT_SSX2IP(x4, S_wr, 8);
      XT_SSX2IP(x5, S_wr, 8);
      XT_SSX2IP(x7, S_wr, 8);
      XT_SSX2IP(x8, S_wr, 8);
      XT_SSX2IP(x9, S_wr, 8);
      XT_SSX2IP(x11, S_wr, 8);
      XT_SSX2IP(x12, S_wr, 8);
      XT_SSX2IP(x13, S_wr, 8);
      XT_SSX2IP(x15, S_wr, 8);

      XT_SSX2IP(x4, S_wr, 8);
      XT_SSX2IP(x5, S_wr, 8);
      XT_SSX2IP(x6, S_wr, 8);
      XT_SSX2IP(x8, S_wr, 8);
      XT_SSX2IP(x9, S_wr, 8);
      XT_SSX2IP(x10, S_wr, 8);
      XT_SSX2IP(x12, S_wr, 8);
      XT_SSX2IP(x13, S_wr, 8);
      XT_SSX2IP(x14, S_wr, 8);
    }

    S_rd = (xtfloatx2*)(scr+3);
    S_wr = (xtfloatx2*)(scr+3);

    for (l = 0; l<4; l++)
    {
      
      XT_LSX2IP(x3, S_rd, 8);
      XT_LSX2IP(x4, S_rd, 8);
      XT_LSX2IP(x5, S_rd, 8 * 2);
      XT_LSX2IP(x7, S_rd, 8);
      XT_LSX2XP(x8, S_rd, -2 * 8);
      XT_LSX2IP(x6, S_rd, 6 * 8);
    
      A0=XT_MULC_S(x4,x8);
      XT_MSUBC_S(A0,x5,x7);
      
      A2=XT_MULC_S(x3,x8);
      XT_MSUBC_S(A2,x5,x6);

      A4=XT_MULC_S(x3,x7);
      XT_MSUBC_S(A4,x4,x6);

      XT_SSX2IP(A0,S_wr,8);
      XT_SSX2IP(A2,S_wr,8);
      XT_SSX2IP(A4,S_wr,7*8);
      
    }

    S_rd = (xtfloatx2*)(scr);
    S_wr = (xtfloatx2*)(scr);
    for (l = 0; l<4; l++)
    {
      xtfloatx2 C0, C1, C2;
      XT_LSX2IP(x0, S_rd, 8);
      XT_LSX2IP(x1, S_rd, 8);
      XT_LSX2IP(x2, S_rd, 8);

      XT_LSX2IP(C0, S_rd, 8);
      XT_LSX2IP(C1, S_rd, 8);
      XT_LSX2IP(C2, S_rd, 4*8);

      A3=XT_MULC_S(x0,C0);
      XT_MADDC_S(A3,x2,C2);
      XT_MSUBC_S(A3,x1,C1);

      XT_SSX2IP(A3,S_wr,8);
    }

    px = (const xtfloatx2      *)(x);
    S_rd = (xtfloatx2*)(scr);
    
    {
      XT_LSX2IP(x0, px, 8);
      XT_LSX2IP(x1, px, 8);
      XT_LSX2IP(x2, px, 8);
      XT_LSX2XP(x3, px, 13 * 8);
      XT_LSX2IP(y0, S_rd, 8);
      XT_LSX2IP(y1, S_rd, 8);
      XT_LSX2IP(y2, S_rd, 8);
      XT_LSX2IP(y3, S_rd, 8);

      A1=XT_MULC_S(x0,y0);
      XT_MADDC_S(A1,x2,y2);
      XT_MSUBC_S(A1,x1,y1);
      XT_MSUBC_S(A1,x3,y3);

      XT_SSX2IP(A1,pz,8);
      
    }
    x += 16 * blkLen;
  }
} /* cmtx_det4x4f() */

#endif  /* #if  HAVE_VFPU */

