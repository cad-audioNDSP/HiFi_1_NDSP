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
/*
 * Real Matrix Inversion
 * Optimized code for HiFi1
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Common helper macros. */
#include "common.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, mtx_inv4x4f, (float32_t* x))

#else
/*-------------------------------------------------------------------------
  These functions implement in-place matrix inversion by Gauss elimination 
  with full pivoting

  Precision: 
  f   floating point (real and complex)

  Input:
  x[N*N]      input matrix
  Output:
  x[N*N]      result
  N is 2,3 or 4

  Restrictions:
  none
-------------------------------------------------------------------------*/
#define SZ_F32 (sizeof(float32_t))
#define SZ_2F32 (2*SZ_F32)

void mtx_inv4x4f(float32_t* x)
{
  xtfloatx2 *restrict pk;
  xtfloatx2 *restrict pmax;
  xtfloatx2 *restrict pA0;
  xtfloatx2 *restrict pA1;
  xtfloatx2 *restrict pA2;
  xtfloatx2 R, C0, C1, C2;
  xtfloatx2 A00, A01, A02, A03;
  xtfloatx2 A10, A11, A12, A13;
  xtfloatx2 A20, A21, A22, A23;
  xtfloatx2 Amax0, Amax1, Amax2, Amax3,
              Ak0,   Ak1,   Ak2,   Ak3;
  ae_valign vX;
  int n,k;
  float32_t ALIGN(8) A[32];
  /* Copy the matrix to buffer and
  * initialize identity matrix:
  
  * x00 x01 x02 x03 1.0 0.0 0.0 0.0
  * x10 x11 x12 x13 0.0 1.0 0.0 0.0
  * x20 x21 x22 x23 0.0 0.0 1.0 0.0
  * x30 x31 x32 x33 0.0 0.0 0.0 1.0
  */
  C0 = (xtfloatx2)0.0f;
  C1 = (xtfloatx2)1.0f;
  pA0 = (xtfloatx2 *)(x);
  pA1 = (xtfloatx2 *)(A);
  {
    xtfloatx2 C01, C10;
    C01 = XT_SEL32_HH_SX2(C0, C1);
    C10 = XT_SEL32_HH_SX2(C1, C0);
    /* Load input matrix */
    vX = XT_LASX2PP(pA0);
    XT_LASX2IP(A00, vX, pA0);
    XT_LASX2IP(A01, vX, pA0);
    XT_LASX2IP(A02, vX, pA0);
    XT_LASX2IP(A03, vX, pA0);
    XT_LASX2IP(A10, vX, pA0);
    XT_LASX2IP(A11, vX, pA0);
    XT_LASX2IP(A12, vX, pA0);
    XT_LASX2IP(A13, vX, pA0);
    /* Save input and identity matrix */
    /* 1-st row */
    XT_SSX2IP(A00, pA1, SZ_2F32);
    XT_SSX2IP(A01, pA1, SZ_2F32);
    XT_SSX2IP(C10, pA1, SZ_2F32);
    XT_SSX2IP(C0 , pA1, SZ_2F32);
    /* 2-nd row */
    XT_SSX2IP(A02, pA1, SZ_2F32);
    XT_SSX2IP(A03, pA1, SZ_2F32);
    XT_SSX2IP(C01, pA1, SZ_2F32);
    XT_SSX2IP(C0 , pA1, SZ_2F32);
    /* 3-rd row */
    XT_SSX2IP(A10, pA1, SZ_2F32);
    XT_SSX2IP(A11, pA1, SZ_2F32);
    XT_SSX2IP(C0 , pA1, SZ_2F32);
    XT_SSX2IP(C10, pA1, SZ_2F32);
    /* 4-th row */
    XT_SSX2IP(A12, pA1, SZ_2F32);
    XT_SSX2IP(A13, pA1, SZ_2F32);
    XT_SSX2IP(C0 , pA1, SZ_2F32);
    XT_SSX2IP(C01, pA1, SZ_2F32);
  }

  /* Set bounds of the buffer */
  WUR_AE_CBEGIN0((uintptr_t)(A));
  WUR_AE_CEND0  ((uintptr_t)(A+32));

  pk  = (xtfloatx2 *)(A);
  pA0 = (xtfloatx2 *)(A+8);
  pA1 = (xtfloatx2 *)(A+16);
  pA2 = (xtfloatx2 *)(A+24);
  /* Gauss elimination */
  for(k=0; k<4; k++)
  {
    xtfloat amax, max;
    unsigned int imax;
    /* pivoting */
    imax=k;
    amax=0.0f;
    max=0.0f;
    /* find absolute max value in the k-th column */
    for(n=k; n<4; n++)
    {
      xtbool cond;
      xtfloat a, t;

      t = A[n*8+k];
      a = XT_ABS_S(t);

      cond = xtbool2_extract_0(XT_OLT_S(amax, a));
      XT_MOVT_S(amax, a, cond);
      XT_MOVT_S(max,  t, cond);
      XT_MOVT  (imax, n, cond);
    }

    R = max;
    R = XT_RECIP_SX2(R);

    /* permutation of rows */
    pmax = (xtfloatx2 *)(A+imax*8);

    Ak0   = XT_LSX2I(pmax, 0*SZ_2F32);
    Ak1   = XT_LSX2I(pmax, 1*SZ_2F32);
    Ak2   = XT_LSX2I(pmax, 2*SZ_2F32);
    Ak3   = XT_LSX2I(pmax, 3*SZ_2F32);
    Amax0 = XT_LSX2I(  pk, 0*SZ_2F32);
    Amax1 = XT_LSX2I(  pk, 1*SZ_2F32);
    Amax2 = XT_LSX2I(  pk, 2*SZ_2F32);
    Amax3 = XT_LSX2I(  pk, 3*SZ_2F32);
    XT_SSX2IP(Amax0, pmax, SZ_2F32);
    XT_SSX2IP(Amax1, pmax, SZ_2F32);
    XT_SSX2IP(Amax2, pmax, SZ_2F32);
    XT_SSX2IP(Amax3, pmax, SZ_2F32);
    __Pragma("no_reorder")
    /* multiply k-th row by the reciprocal *
 *      * pivot element during swapping rows  */
    Ak0 = Ak0*R;
    Ak1 = Ak1*R;
    Ak2 = Ak2*R;
    Ak3 = Ak3*R;
    XT_SSX2IP(Ak0, pk, SZ_2F32);
    XT_SSX2IP(Ak1, pk, SZ_2F32);
    XT_SSX2IP(Ak2, pk, SZ_2F32);
    XT_SSX2IP(Ak3, pk, SZ_2F32);

    /* elimination */
    /* join forward and back substitution */

    C0 = XT_LSX((xtfloat *)pA0, k*SZ_F32);
    C1 = XT_LSX((xtfloat *)pA1, k*SZ_F32);
    C2 = XT_LSX((xtfloat *)pA2, k*SZ_F32);

    A00 = XT_LSX2I(pA0, 0*SZ_2F32);
    A01 = XT_LSX2I(pA0, 1*SZ_2F32);
    A02 = XT_LSX2I(pA0, 2*SZ_2F32);
    A03 = XT_LSX2I(pA0, 3*SZ_2F32);
    A10 = XT_LSX2I(pA1, 0*SZ_2F32);
    A11 = XT_LSX2I(pA1, 1*SZ_2F32);
    A12 = XT_LSX2I(pA1, 2*SZ_2F32);
    A13 = XT_LSX2I(pA1, 3*SZ_2F32);
    A20 = XT_LSX2I(pA2, 0*SZ_2F32);
    A21 = XT_LSX2I(pA2, 1*SZ_2F32);
    A22 = XT_LSX2I(pA2, 2*SZ_2F32);
    A23 = XT_LSX2I(pA2, 3*SZ_2F32);

    XT_MSUB_SX2(A00, Ak0, C0);
    XT_MSUB_SX2(A01, Ak1, C0);
    XT_MSUB_SX2(A02, Ak2, C0);
    XT_MSUB_SX2(A03, Ak3, C0);
    XT_MSUB_SX2(A10, Ak0, C1);
    XT_MSUB_SX2(A11, Ak1, C1);
    XT_MSUB_SX2(A12, Ak2, C1);
    XT_MSUB_SX2(A13, Ak3, C1);
    XT_MSUB_SX2(A20, Ak0, C2);
    XT_MSUB_SX2(A21, Ak1, C2);
    XT_MSUB_SX2(A22, Ak2, C2);
    XT_MSUB_SX2(A23, Ak3, C2);

    XT_SSX2XC(A00, pA0, SZ_2F32);
    XT_SSX2XC(A01, pA0, SZ_2F32);
    XT_SSX2XC(A02, pA0, SZ_2F32);
    XT_SSX2XC(A03, pA0, SZ_2F32);
    XT_SSX2XC(A10, pA1, SZ_2F32);
    XT_SSX2XC(A11, pA1, SZ_2F32);
    XT_SSX2XC(A12, pA1, SZ_2F32);
    XT_SSX2XC(A13, pA1, SZ_2F32);
    XT_SSX2XC(A20, pA2, SZ_2F32);
    XT_SSX2XC(A21, pA2, SZ_2F32);
    XT_SSX2XC(A22, pA2, SZ_2F32);
    XT_SSX2XC(A23, pA2, SZ_2F32);
  }
  /* copy inverse matrix to x */
  pA0 = (xtfloatx2 *)(x);
  pA1 = (xtfloatx2 *)(A+4);
  vX = AE_ZALIGN64();
  {
    XT_LSX2IP(A00, pA1,   SZ_2F32);
    XT_LSX2IP(A01, pA1, 3*SZ_2F32);
    XT_LSX2IP(A02, pA1,   SZ_2F32);
    XT_LSX2IP(A03, pA1, 3*SZ_2F32);
    XT_LSX2IP(A10, pA1,   SZ_2F32);
    XT_LSX2IP(A11, pA1, 3*SZ_2F32);
    XT_LSX2IP(A12, pA1,   SZ_2F32);
    XT_LSX2IP(A13, pA1,   0);

    XT_SASX2IP(A00, vX, pA0);
    XT_SASX2IP(A01, vX, pA0);
    XT_SASX2IP(A02, vX, pA0);
    XT_SASX2IP(A03, vX, pA0);
    XT_SASX2IP(A10, vX, pA0);
    XT_SASX2IP(A11, vX, pA0);
    XT_SASX2IP(A12, vX, pA0);
    XT_SASX2IP(A13, vX, pA0);
  }
  XT_SASX2POSFP(vX, pA0);
}/* mtx_inv4x4f() */

#endif /* #if HAVE_VFPU*/

