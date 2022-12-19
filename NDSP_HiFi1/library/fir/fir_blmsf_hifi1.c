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
 * Least Mean Squares
 * C code optimized for HiFi1
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Common helper macros. */
#include "common.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"

/*-------------------------------------------------------------------------
  Blockwise Adaptive LMS Algorithm for Real Data
  Blockwise LMS algorithm performs filtering of input samples x[N+M-1],
  computation of error e[N] over a block of reference data r[N] and makes
  blockwise update of IR to minimize the error output.
  Algorithm includes FIR filtering, calculation of correlation between the 
  error output e[N] and reference signal x[N+M-1] and IR taps update based
  on that correlation.
NOTES: 
  1. The algorithm must be provided with the normalization factor, which is
     the power of the input signal times N - the number of samples in a
     data block. This can be calculated using the vec_power24x24() or 
     vec_power16x16() function. In order to avoid the saturation of the 
     normalization factor, it may be biased, i.e. shifted to the right.
     If it's the case, then the adaptation coefficient must be also shifted
     to the right by the same number of bit positions.
  2. this algorithm consumes less CPU cycles per block than single 
     sample algorithm at similar convergence rate.
  3. Right selection of N depends on the change rate of impulse response:
     on static or slow varying channels convergence rate depends on
     selected mu and M, but not on N.
  4. 16x16 routine may converge slower on small errors due to roundoff 
     errors. In that cases, 16x32 rountine will give better results although 
     convergence rate on bigger errors is the same

  Precision: 
  16x16   16-bit coefficients, 16-bit data, 16-bit output
  24x24   24-bit coefficients, 24-bit data, 32-bit output
  16x32   32-bit coefficients, 16-bit data, 16-bit output
  32x32   32-bit coefficients, 32-bit data, 32-bit output
  f       floating point
  Input:
  h[M]     impulse response, Q15, Q31 or floating point
  r[N]     reference data vector (near end). First in time value is in 
           r[0], 24-bit or 16-bit, Qx or floating point
  x[N+M-1] input vector (far end). First in time value is in x[0],  
           24-bit or 16-bit, Qx or floating point
  norm     normalization factor: power of signal multiplied by N, Q15, 
           Q31 or floating point
           Fixed-point format for the 24x24-bit variant: Q(2*x-31-bias)
           Fixed-point format for the 32x16-bit variant: Q(2*x+1-bias)
  mu       adaptation coefficient (LMS step), Q(31-bias) or Q(15-bias)
  N        length of data block
  M        length of h
  Output:
  e[N]     estimated error, Q31,Q15 or floating point
  h[M]     updated impulse response, Q15, Q31 or floating point

  Restriction:
  x,r,h,e - should not overlap
  x,r,h,e - aligned on a 8-bytes boundary
  N,M     - multiples of 8 and >0
-------------------------------------------------------------------------*/
#if !(HAVE_VFPU)
DISCARD_FUN(void, fir_blmsf, ( float32_t * e, float32_t * h, const float32_t * r,
            const float32_t * x, float32_t norm, float32_t mu, int N, int M))

#else
#if 0
void fir_blmsf(float32_t * e, float32_t * h, const float32_t * r,
    const float32_t * x, float32_t norm, float32_t mu, int N, int M)
{
    float32_t b;
    int m, n, _N;

    const xtfloatx2*  restrict pX = (const xtfloatx2*)x;
    const xtfloatx2*  restrict pR = (const xtfloatx2*)r;
    xtfloatx2*  restrict pE = (xtfloatx2*)e;
    const xtfloat*  restrict pe = (xtfloat*)e;
    const xtfloat*    restrict ph = (const xtfloat*)h;
    xtfloatx2*    restrict pH = (xtfloatx2*)h;
    xtfloatx2*    restrict pH_wr = (xtfloatx2*)h;
    ae_valign h_align, hwr_align;
    NASSERT(e);
    NASSERT(h);
    NASSERT(r);
    NASSERT(x);
    NASSERT_ALIGN(e, 8);
    NASSERT_ALIGN(h, 8);
    NASSERT_ALIGN(r, 8);
    NASSERT_ALIGN(x, 8);
    NASSERT(N > 0 && M > 0);
    NASSERT(M % 8 == 0 && N % 8 == 0);

#define P1 8    
#define P2 8    

    _N = N & (~(P1 - 1));
    /* estimate error */
    for (n = 0; n < _N; n += P1)
    {
        xtfloatx2 r0, r1, r2, r3;
        xtfloatx2 s0, s1, s2, s3;
        xtfloatx2 x0, x1, x2, x3, x4;
        s0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(0);
        s1 = XT_AE_MOVXTFLOATX2_FROMINT32X2(0);
        s2 = XT_AE_MOVXTFLOATX2_FROMINT32X2(0);
        s3 = XT_AE_MOVXTFLOATX2_FROMINT32X2(0);
        pX = (xtfloatx2*)((uintptr_t)(x + n));
        XT_LSX2IP(x0, pX, 8);
        XT_LSX2IP(x1, pX, 8);
        XT_LSX2IP(x2, pX, 8);
        XT_LSX2IP(x3, pX, 8);
        for (m = 0; m < M; m += 4)
        {
            xtfloatx2 hm;
            xtfloatx2 t0, t1, t2, t3;
            hm = ph[M - 1 - m];
            XT_LSX2IP(x4, pX, 8);
            XT_MADD_SX2(s0, hm, x0);
            XT_MADD_SX2(s1, hm, x1);
            XT_MADD_SX2(s2, hm, x2);
            XT_MADD_SX2(s3, hm, x3);
            t0 = XT_SEL32_LH_SX2(x0, x1);
            t1 = XT_SEL32_LH_SX2(x1, x2);
            t2 = XT_SEL32_LH_SX2(x2, x3);
            t3 = XT_SEL32_LH_SX2(x3, x4);
            hm = ph[M - 2 - m];
            XT_MADD_SX2(s0, hm, t0);
            XT_MADD_SX2(s1, hm, t1);
            XT_MADD_SX2(s2, hm, t2);
            XT_MADD_SX2(s3, hm, t3);
            x0 = x1; x1 = x2; x2 = x3; x3 = x4;

            hm = ph[M - 3 - m];
            XT_LSX2IP(x4, pX, 8);
            XT_MADD_SX2(s0, hm, x0);
            XT_MADD_SX2(s1, hm, x1);
            XT_MADD_SX2(s2, hm, x2);
            XT_MADD_SX2(s3, hm, x3);
            t0 = XT_SEL32_LH_SX2(x0, x1);
            t1 = XT_SEL32_LH_SX2(x1, x2);
            t2 = XT_SEL32_LH_SX2(x2, x3);
            t3 = XT_SEL32_LH_SX2(x3, x4);
            hm = ph[M - 4 - m];
            XT_MADD_SX2(s0, hm, t0);
            XT_MADD_SX2(s1, hm, t1);
            XT_MADD_SX2(s2, hm, t2);
            XT_MADD_SX2(s3, hm, t3);
            x0 = x1; x1 = x2; x2 = x3; x3 = x4;
        }
        XT_LSX2IP(r0, pR, 8);
        XT_LSX2IP(r1, pR, 8);
        XT_LSX2IP(r2, pR, 8);
        XT_LSX2IP(r3, pR, 8);
        s0 = XT_SUB_SX2(r0, s0);
        s1 = XT_SUB_SX2(r1, s1);
        s2 = XT_SUB_SX2(r2, s2);
        s3 = XT_SUB_SX2(r3, s3);
        XT_SSX2XP(s0, pE, 8);
        XT_SSX2XP(s1, pE, 8);
        XT_SSX2XP(s2, pE, 8);
        XT_SSX2XP(s3, pE, 8);
    }
    /* update impluse response */
    b = mu / norm;
    for (m = 0; m < M; m += P2)
    {
        xtfloatx2 h0, h1, h2, h3;
        xtfloatx2 s0, s1, s2, s3;
        xtfloatx2 x0, x1, x2, x3, x4;
        s0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(0);
        s1 = XT_AE_MOVXTFLOATX2_FROMINT32X2(0);
        s2 = XT_AE_MOVXTFLOATX2_FROMINT32X2(0);
        s3 = XT_AE_MOVXTFLOATX2_FROMINT32X2(0);
        pX = (xtfloatx2*)((uintptr_t)(x + m));
        XT_LSX2IP(x0, pX, 8);
        XT_LSX2IP(x1, pX, 8);
        XT_LSX2IP(x2, pX, 8);
        XT_LSX2IP(x3, pX, 8);

        for (n = 0; n < N; n += 4)
        {
            xtfloatx2 en;
            xtfloatx2 t0, t1, t2, t3;
            en = pe[n];
            XT_LSX2IP(x4, pX, 8);
            XT_MADD_SX2(s0, en, x0);
            XT_MADD_SX2(s1, en, x1);
            XT_MADD_SX2(s2, en, x2);
            XT_MADD_SX2(s3, en, x3);
            t0 = XT_SEL32_LH_SX2(x0, x1);
            t1 = XT_SEL32_LH_SX2(x1, x2);
            t2 = XT_SEL32_LH_SX2(x2, x3);
            t3 = XT_SEL32_LH_SX2(x3, x4);
            en = pe[n + 1];
            XT_MADD_SX2(s0, en, t0);
            XT_MADD_SX2(s1, en, t1);
            XT_MADD_SX2(s2, en, t2);
            XT_MADD_SX2(s3, en, t3);
            x0 = x1; x1 = x2; x2 = x3; x3 = x4;


            en = pe[n+2];
            XT_LSX2IP(x4, pX, 8);
            XT_MADD_SX2(s0, en, x0);
            XT_MADD_SX2(s1, en, x1);
            XT_MADD_SX2(s2, en, x2);
            XT_MADD_SX2(s3, en, x3);
            t0 = XT_SEL32_LH_SX2(x0, x1);
            t1 = XT_SEL32_LH_SX2(x1, x2);
            t2 = XT_SEL32_LH_SX2(x2, x3);
            t3 = XT_SEL32_LH_SX2(x3, x4);
            en = pe[n + 3];
            XT_MADD_SX2(s0, en, t0);
            XT_MADD_SX2(s1, en, t1);
            XT_MADD_SX2(s2, en, t2);
            XT_MADD_SX2(s3, en, t3);
            x0 = x1; x1 = x2; x2 = x3; x3 = x4;
        }
        pH = (xtfloatx2*)((uintptr_t)(h + M - 1 - m));
        h_align = AE_LA64_PP(pH);
        XT_LASX2RIP(h0, h_align, pH);
        XT_LASX2RIP(h1, h_align, pH);
        XT_LASX2RIP(h2, h_align, pH);
        XT_LASX2RIP(h3, h_align, pH);
        XT_MADD_SX2(h0, (xtfloatx2)b, s0);
        XT_MADD_SX2(h1, (xtfloatx2)b, s1);
        XT_MADD_SX2(h2, (xtfloatx2)b, s2);
        XT_MADD_SX2(h3, (xtfloatx2)b, s3);
        pH_wr = (xtfloatx2*)((uintptr_t)(h + M - 1 - m));
        hwr_align = AE_ZALIGN64();
        XT_SASX2RIP(h0, hwr_align, pH_wr);
        XT_SASX2RIP(h1, hwr_align, pH_wr);
        XT_SASX2RIP(h2, hwr_align, pH_wr);
        XT_SASX2RIP(h3, hwr_align, pH_wr);
        AE_SA64POS_FP(hwr_align, pH_wr);
  }
} /* fir_blmsf() */
#else
void fir_blmsf(float32_t * e, float32_t * h, const float32_t * r,
    const float32_t * x, float32_t norm, float32_t mu, int N, int M)
{
    float32_t b;
    int m, n;

          xtfloatx2* restrict pE = (      xtfloatx2*)e;
          xtfloatx2* restrict ph = (      xtfloatx2*)h;
    const xtfloatx2* restrict pX = (const xtfloatx2*)x;
    const xtfloatx2* restrict pR = (const xtfloatx2*)r;
          xtfloatx2* restrict pH = (xtfloatx2*)h;
          xtfloatx2* restrict pH_wr = (xtfloatx2*)h;
    const xtfloatx2* restrict pX1;
    ae_valign h_align, hwr_align;
    NASSERT(e);
    NASSERT(h);
    NASSERT(r);
    NASSERT(x);
    NASSERT_ALIGN(e, 8);
    NASSERT_ALIGN(h, 8);
    NASSERT_ALIGN(r, 8);
    NASSERT_ALIGN(x, 8);
    NASSERT(N > 0 && M > 0);
    NASSERT(M % 8 == 0 && N % 8 == 0);

    /* estimate error */
    for ( n=0; n<N; n+=4 )
    {
        xtfloatx2 A0, A1, A2, A3, A01h, A23h, A01l, A23l;
        xtfloatx2 X23,X34,H01,H23;
        xtfloatx2 R01,R23,E01,E23;
        xtfloatx2 x0,x1;
        ae_valign ax, ah;

        ph = (xtfloatx2*)((uintptr_t)(h + M - 1));
        ah = AE_LA64_PP(ph);
        pX = (const xtfloatx2 *)(x+n);
        pX1= (const xtfloatx2 *)(x+n+1);
        ax = XT_LASX2PP(pX1);

        A2 = A3 = (xtfloatx2)0.f;
        AE_LASX2RIP(H01,ah,ph);
        XT_LSX2IP (X23,pX,8);
        XT_LASX2IP(X34,ax,pX1);
        A0=XT_MUL_SX2(X23,H01);
        A1=XT_MUL_SX2(X34,H01);
        for ( m=0; m<M-2; m+=2 )
        {
            AE_LASX2RIP(H23,ah,ph);
            XT_LSX2IP (X23,pX,8);
            XT_LASX2IP(X34,ax,pX1);
            XT_MADD_SX2(A0,X23,H23);
            XT_MADD_SX2(A1,X34,H23);
            XT_MADD_SX2(A2,X23,H01);
            XT_MADD_SX2(A3,X34,H01);
            H01=H23;
        }
        XT_LSX2IP (X23,pX,8);
        XT_LASX2IP(X34,ax,pX1);
        XT_MADD_SX2(A2,X23,H01);
        XT_MADD_SX2(A3,X34,H01);

        A01h = XT_SEL32_HH_SX2(A0, A1);
        A01l = XT_SEL32_LL_SX2(A0, A1);
        x0   = A01h + A01l;
        A23h = XT_SEL32_HH_SX2(A2, A3);
        A23l = XT_SEL32_LL_SX2(A2, A3);
        x1   = A23h + A23l;

        XT_LSX2IP (R01,pR,2*sizeof(float32_t));
        XT_LSX2IP (R23,pR,2*sizeof(float32_t));
        E01=XT_SUB_SX2(R01,x0);
        E23=XT_SUB_SX2(R23,x1);

        XT_SSX2IP (E01,pE,2*sizeof(float32_t));
        XT_SSX2IP (E23,pE,2*sizeof(float32_t));
    }
    /* update impluse response */
    b = mu / norm;
    for ( m=0; m<M; m+=4 )
    {
        int n;
        ae_valign ax;
        xtfloatx2 E01,E23,X01,X12,X23,X34;
        xtfloatx2 A0, A1, A2, A3, A01h, A23h, A01l, A23l,s0,s1,h0,h1;
        pE=(      xtfloatx2*)e;
        pX=(const xtfloatx2*)(x+m);
        pX1=(const xtfloatx2*)(x+m+1);
        ax=AE_LA64_PP(pX1);
        XT_LSX2IP(E23,pE,2*sizeof(int32_t));
        XT_LSX2IP (X01,pX,2*sizeof(int32_t));
        XT_LASX2IP(X12,ax,pX1);
        A2=A3=(xtfloatx2)0.f;
        A0=XT_MUL_SX2(X01,E23);
        A1=XT_MUL_SX2(X12,E23);
        for ( n=0; n<N-2; n+=2 )
        {
            E01=E23;
            XT_LSX2IP(E23,pE,2*sizeof(int32_t));
            XT_LSX2IP (X23,pX,2*sizeof(int32_t));
            XT_LASX2IP(X34,ax,pX1);
            XT_MADD_SX2(A0,X23,E23);
            XT_MADD_SX2(A1,X34,E23);
            XT_MADD_SX2(A2,X23,E01);
            XT_MADD_SX2(A3,X34,E01);
        }
        E01=E23;
        XT_LSX2IP (X23,pX,2*sizeof(int32_t));
        XT_LASX2IP(X34,ax,pX1);
        XT_MADD_SX2(A2,X23,E01);
        XT_MADD_SX2(A3,X34,E01);

        A01h = XT_SEL32_HH_SX2(A0, A1);
        A01l = XT_SEL32_LL_SX2(A0, A1);
        s0   = A01h + A01l;
        A23h = XT_SEL32_HH_SX2(A2, A3);
        A23l = XT_SEL32_LL_SX2(A2, A3);
        s1   = A23h + A23l;

        pH = (xtfloatx2*)((uintptr_t)(h + M - 1 - m));
        h_align = AE_LA64_PP(pH);
        XT_LASX2RIP(h0, h_align, pH);
        XT_LASX2RIP(h1, h_align, pH);
        XT_MADD_SX2(h0, (xtfloatx2)b, s0);
        XT_MADD_SX2(h1, (xtfloatx2)b, s1);
        pH_wr = (xtfloatx2*)((uintptr_t)(h + M - 1 - m));
        hwr_align = AE_ZALIGN64();
        XT_SASX2RIP(h0, hwr_align, pH_wr);
        XT_SASX2RIP(h1, hwr_align, pH_wr);
        AE_SA64POS_FP(hwr_align, pH_wr);
        }
    }
#endif
#endif /*HAVE_VFPU*/

