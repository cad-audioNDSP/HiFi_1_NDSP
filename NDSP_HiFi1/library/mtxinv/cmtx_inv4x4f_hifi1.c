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
 * Complex Matrix Inversion
 * Optimized code for HiFi1
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Common helper macros. */
#include "common.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, cmtx_inv4x4f, (complex_float* x))

#else
#define SZ_F32 (sizeof(float32_t))
#define SZ_CF32 (2*SZ_F32)

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
void cmtx_inv4x4f(complex_float* x)
{
    xtfloatx2 * restrict pX;
    xtfloatx2 * restrict pA;
    xtfloatx2 * restrict pA_orig;
    xtfloatx2 * pA0;
    xtfloatx2 * pA1;
    xtfloatx2 * pA2;
    xtfloatx2 * pmax;
          xtfloatx2 * pk_st;
    const xtfloatx2 * pk_ld;
    xtfloatx2 Ak,Ak1,Amax, A0, A1, A2, y0;
    xtfloatx2 X0, X1, X2, X3;
    xtfloatx2 R, C0, C1, C2, C3;
    xtfloatx2 _0, _1;
    int n,k;
    complex_float ALIGN(16) A[32];

    xtfloatx2 t0_R,amax, max;
    ae_int32x2 imax;
    int imax_0;
    
    vbool2 cond;
    xtfloatx2 t;
    xtfloatx2 a,temp_a,a_f;
    /* Copy the matrix to buffer and
     * initialize identity matrix:
     *
     * x00 x01 x02 x03 1.0 0.0 0.0 0.0
     * x10 x11 x12 x13 0.0 1.0 0.0 0.0
     * x20 x21 x22 x23 0.0 0.0 1.0 0.0
     * x30 x31 x32 x33 0.0 0.0 0.0 1.0
     */
    pX = (xtfloatx2 *)x;
    pA = (xtfloatx2 *)A;
    _0 = XT_CONST_S(0);
    _1 = XT_CONST_S(1);
    _1 = XT_SEL32_LL_SX2(_1, _0);
    C0 = _1; C1 = _0; C2 = _0; C3 = _0;
    /* save by 1 row per iteration */
    for (n=0; n<4; n++)
    {
        XT_LSX2IP(X0, pX, SZ_CF32);
        XT_LSX2IP(X1, pX, SZ_CF32);
        XT_LSX2IP(X2, pX, SZ_CF32);
        XT_LSX2IP(X3, pX, SZ_CF32);
        XT_SSX2IP(X0, pA, SZ_CF32);
        XT_SSX2IP(X1, pA, SZ_CF32);
        XT_SSX2IP(X2, pA, SZ_CF32);
        XT_SSX2IP(X3, pA, SZ_CF32);
        XT_SSX2I (C3, pA, 3*SZ_CF32);
        XT_SSX2I (C2, pA, 2*SZ_CF32);
        XT_SSX2I (C1, pA, 1*SZ_CF32);
        XT_SSX2IP(C0, pA, 4*SZ_CF32);
        R = C3;
        C3 = C2;
        C2 = C1;
        C1 = C0;
        C0 = R;
    }
    /* Set bounds of the buffer */
    WUR_AE_CBEGIN0((uintptr_t)(A));
    WUR_AE_CEND0  ((uintptr_t)(A+32));

    pk_ld = (xtfloatx2 *)(A);
    pk_st = (xtfloatx2 *)(A);
    pA0 = (xtfloatx2 *)(A+8);
    pA1 = (xtfloatx2 *)(A+16);
    pA2 = (xtfloatx2 *)(A+24);
    /* Gauss elimination */
    for(k=0; k<4; k++)
    {
        /* pivoting */
        //xtfloat  re, im;
        imax=AE_MOVDA32X2(k,k);
        amax=max=XT_CONST_S(0);
        /* find absolute max value in the k-th column */
        pA = (xtfloatx2 *)(A+k*9);
        pA_orig=pA;
        for(n=k; n<4; n++)
        {

        XT_LSX2XP(t, pA, 8*SZ_CF32);
        a=XT_MUL_SX2(t,t);
        temp_a=XT_SEL32_LH_SX2(a,a);
        a_f=XT_ADD_SX2(a,temp_a);

        cond = XT_OLT_SX2(amax, a_f);
        XT_MOVT_SX2(amax, a_f, cond);
        AE_MOVT32X2(imax, AE_MOVDA32X2(n,n), cond);
        }
        imax_0=AE_MOVAD32_H(imax);
        max=XT_LSX2X(pA_orig,8*SZ_CF32*(imax_0-k));
        /* find the reciprocal of the pivot value */
        t0_R=XT_RECIP_SX2(amax);
        R=XT_MULMUX_S(t0_R,max,1);

        /* permutation of rows */
        pmax = (xtfloatx2 *)(A+imax_0*8);
        for (n=0; n<8; n++)
        {
            Ak1 = XT_LSX2I(pmax, 0);
            XT_LSX2IP(Amax, pk_ld, SZ_CF32);
            XT_SSX2IP(Amax, pmax, SZ_CF32);
            /* multiply k-th row by the reciprocal *
             * pivot element during swapping rows  */
            y0=XT_MULC_S(Ak1,R);
            XT_SSX2IP(y0,pk_st,8);
        }

        /* elimination */
        /* join forward and back substitution */
        C0 = XT_LSX2X(pA0, k*SZ_CF32);
        C1 = XT_LSX2X(pA1, k*SZ_CF32);
        C2 = XT_LSX2X(pA2, k*SZ_CF32);
        pk_ld -= 8;   
        for (n=0; n<8; n++)
        {
            XT_LSX2IP(Ak, pk_ld, SZ_CF32);
            A0 = XT_LSX2I(pA0, 0);
            A1 = XT_LSX2I(pA1, 0);
            A2 = XT_LSX2I(pA2, 0);

            XT_MSUBC_S(A0,Ak,C0);
            XT_SSX2XC(A0,pA0,SZ_CF32);
            XT_MSUBC_S(A1,Ak,C1);
            XT_SSX2XC(A1,pA1,SZ_CF32);
            XT_MSUBC_S(A2,Ak,C2);
            XT_SSX2XC(A2,pA2,SZ_CF32);
        }
    }
    /* copy 5-8 columns to x */
    pX = (xtfloatx2 *)(x);
    pA = (xtfloatx2 *)(A+4);
    for(n=0; n<4; n++)
    {
        XT_LSX2IP(X0, pA, SZ_CF32);
        XT_LSX2IP(X1, pA, SZ_CF32);
        XT_LSX2IP(X2, pA, SZ_CF32);
        XT_LSX2IP(X3, pA, 5*SZ_CF32);
        XT_SSX2IP(X0, pX, SZ_CF32);
        XT_SSX2IP(X1, pX, SZ_CF32);
        XT_SSX2IP(X2, pX, SZ_CF32);
        XT_SSX2IP(X3, pX, SZ_CF32);
    }
}/* cmtx_inv4x4f() */


#endif  /* #if HAVE_VFPU*/
