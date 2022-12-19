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
    Real FIR Filter with decimation (4x)
    C code optimized for HiFi1
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Common helper macros. */
#include "common.h"
/* Filters and transformations */
#include "NatureDSP_Signal.h"

#if !(HAVE_VFPU)
DISCARD_FUN(float32_t *, fir_decimaf_4x, (float32_t * restrict z, float32_t * restrict delay, 
            float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int N))
            
#else
#include "fir_decimaf_4x.h"

/*
    4x decimator:
    Input/output:
    delay[M] - circular delay line
    Input:
    p        - pointer to delay line
    x[N*4]   - input signal
    h[M]     - impulse response
    N        - number of output samples
    Output:
    z[N]     - output samples
    Restrictions:
    N>0, M>0
    M multiple of 2
    N multiple of 8
    delay should be aligned on 8 byte boundary

    Returns:
    updated pointer to delay line
*/
#if 0
float32_t * fir_decimaf_4x(float32_t * restrict z, float32_t * restrict delay, float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int N)
{
    xtfloat   *restrict pz;
    const xtfloatx2 *restrict px;
    const xtfloatx2 *restrict ph;
    xtfloatx2 *pp;
    const xtfloatx2 *pd;
    ae_valign ax, ah, ad;
    int n, m;
    NASSERT(x);
    NASSERT(z);
    NASSERT(h);
    NASSERT(delay);
    NASSERT(p);
    NASSERT_ALIGN(delay, 8);
    NASSERT(N > 0);
    NASSERT(M > 0);
    NASSERT(M % 2 == 0);
    NASSERT(N % 8 == 0);
    /* set circular buffer boundaries */
    WUR_AE_CBEGIN0((uintptr_t)(delay + 0));
    WUR_AE_CEND0((uintptr_t)(delay + M));

    pz = (xtfloat*)z;
    pp = (xtfloatx2*)p;
    for (n = 0; n < (N >> 1); n++, x += 8)
    {
        xtfloatx2 A0, B0, A1, B1, X0, X1, X2, X3, H0, H1;
        xtfloatx2 Y0, Y1, Y2, Y3, Y4;
        A0 = B0 = A1 = B1 = (xtfloatx2)0.0f;
 
        ph = (const xtfloatx2*)h;
        ah = AE_LA64_PP(ph);

        px = (xtfloatx2*)(x);
        ax = AE_LA64_PP(px);
        XT_LASX2IP(Y0, ax, px);
        XT_LASX2IP(Y1, ax, px);
        XT_LASX2IP(Y2, ax, px);
        pd = (const xtfloatx2*)(((const float32_t*)pp));
        XT_LASX2NEGPC(ad, pd);
        XT_LASX2RIC(Y3, ad, pd);
        XT_LASX2RIC(Y4, ad, pd);
        X3 = XT_SEL32_HL_SX2(Y1, Y0);
        X2 = XT_SEL32_HL_SX2(Y2, Y1);
        X0 = XT_SEL32_HL_SX2(Y0, Y3);
        X1 = Y4;

        for (m = 0; m < (M >> 2); m++)
        {
            XT_LASX2IP(H0, ah, ph);
            XT_LASX2IP(H1, ah, ph);
            XT_MADD_SX2(A0, H0, X0);
            XT_MADD_SX2(A1, H0, X2);
            XT_MADD_SX2(B0, H1, X1);
            XT_MADD_SX2(B1, H1, X3);
            X2 = X0;
            X3 = X1;
            XT_LASX2RIC(X0, ad, pd);
            XT_LASX2RIC(X1, ad, pd);
        }
        if (M & 2)
        {
            XT_LASX2IP(H0, ah, ph);
            XT_MADD_SX2(A0, H0, X0);
            XT_MADD_SX2(A1, H0, X2);
        }
        A0 = A0 + B0;
        A1 = A1 + B1;
        A0 = A0 + XT_SEL32_LH_SX2(A0, A0);
        A1 = A1 + XT_SEL32_LH_SX2(A1, A1);

        px = (xtfloatx2*)(x);
        ax = AE_LA64_PP(px);
        XT_LASX2IP(Y0, ax, px);
        XT_LASX2IP(Y1, ax, px);
        XT_LASX2IP(Y2, ax, px);
        XT_LASX2IP(Y3, ax, px);
        XT_SSX2XC(Y0, pp, sizeof(Y0));
        XT_SSX2XC(Y1, pp, sizeof(Y1));
        XT_SSX2XC(Y2, pp, sizeof(Y2));
        XT_SSX2XC(Y3, pp, sizeof(Y3));
        XT_SSIP(A0, pz, sizeof(float32_t));
        XT_SSIP(A1, pz, sizeof(float32_t));
    }
    return (float32_t*)pp;
} /* fir_decimaf_4x() */
#else
/*The following code is implemented to improve MAC throughput*/
#if XCHAL_HAVE_HIFI1_LOW_LATENCY_MAC_FMA
float32_t* firdecf_D4(
              const float32_t* h, int M,
              float32_t* d, float32_t* p, int L,
              float32_t * restrict y, const float32_t * x, int N, int D )
{
    const xtfloatx2 *restrict pX;
    xtfloatx2 *restrict pY;
    const xtfloatx2* pD;
    const xtfloatx2* pH;
    xtfloatx2 * pWr;
    ae_valign ay,aD;
    xtfloatx2 X01,X23,X89,Xab,H01,H23;
    int m,n;
    NASSERT(h);
    NASSERT(d);
    NASSERT(p);
    NASSERT(x);
    NASSERT(y);
    NASSERT_ALIGN(x,8);
    NASSERT_ALIGN(d,8);
    NASSERT_ALIGN(p,8);
    NASSERT(N>0 && N%8==0);
    NASSERT(M>0 && M%2==0);
    NASSERT(D==4);

    pX=(const xtfloatx2 *)x;
    WUR_AE_CBEGIN0( (uintptr_t)( d    ) );
    WUR_AE_CEND0  ( (uintptr_t)( d + L) );
    ay=AE_ZALIGN64();
    pY=(xtfloatx2*)y;
    pWr=(xtfloatx2*)p;
    for (n=0; n<N; n+=2)
    {
        xtfloatx2 t;
        xtfloat   t1;
        xtfloatx2 q0,q1,q01h,q01l,q0r;
        q0=q1=(xtfloatx2)0.f;
        pD=(xtfloatx2*)pWr;
        XT_LSXC(t1, castxcc(xtfloat, pD), 4 * sizeof(float32_t));
        pH=(const xtfloatx2*)h;
        XT_LSX2IP(t,pX,2*sizeof(float32_t));  XT_SSX2XC(t,pWr,2*sizeof(float32_t));
        XT_LSX2IP(t,pX,2*sizeof(float32_t));  XT_SSX2XC(t,pWr,2*sizeof(float32_t));
        XT_LSX2IP(t,pX,2*sizeof(float32_t));  XT_SSX2XC(t,pWr,2*sizeof(float32_t));
        XT_LSX2IP(t,pX,2*sizeof(float32_t));  XT_SSX2XC(t,pWr,2*sizeof(float32_t));
        XT_LASX2NEGPC(aD,pD);
        XT_LASX2RIC(X23,aD,pD); XT_LASX2RIC(Xab,aD,pD);
        XT_LASX2RIC(X01,aD,pD); XT_LASX2RIC(X89,aD,pD);
        for (m=0; m<(M & ~3); m+=4)
        {
            XT_LSX2IP(H01,pH,2*sizeof(float32_t));
            XT_LSX2IP(H23,pH,2*sizeof(float32_t));
            XT_MADD_SX2(q0,X01,H01);
            XT_MADD_SX2(q1,X23,H01);
            XT_MADD_SX2(q0,X89,H23);
            XT_MADD_SX2(q1,Xab,H23);
            X23=X01; Xab=X89;
            XT_LASX2RIC(X01,aD,pD); XT_LASX2RIC(X89,aD,pD);
        }
        if (M&2)
        {
            XT_LSX2IP(H01,pH,2*sizeof(float32_t));
            XT_MADD_SX2(q0,X01,H01);
            XT_MADD_SX2(q1,X23,H01);
        }
        q01h = XT_SEL32_HH_SX2(q0, q1);
        q01l = XT_SEL32_LL_SX2(q0, q1);
        q0r  = XT_ADD_SX2(q01h, q01l);
        XT_SASX2IP(q0r, ay, pY);
    }
    AE_SA64POS_FP(ay,pY);
    return (float32_t*)pWr;
}
#else
float32_t* firdecf_D4(
                const float32_t* restrict h, int M, 
                float32_t* restrict d, float32_t* restrict p, int L,
                float32_t * restrict y, const float32_t * restrict x, int N, int D )
{
        xtfloat   *restrict pz;
        const xtfloatx2 *restrict px;
        const xtfloatx2 *restrict ph;
        xtfloatx2 *pp;
        const xtfloatx2 *pd;
        ae_valign ax, ah, ad;
        int n, m;
        NASSERT(x);
        NASSERT(y);
        NASSERT(h);
        NASSERT(d);
        NASSERT(p);
        NASSERT_ALIGN(d, 8);
        NASSERT(N > 0);
        NASSERT(M > 0);
        NASSERT(M % 2 == 0);
        NASSERT(N % 8 == 0);
        /* set circular buffer boundaries */
        WUR_AE_CBEGIN0((uintptr_t)(d + 0));
        WUR_AE_CEND0((uintptr_t)(d + L));

        pz = (xtfloat*)y;
        pp = (xtfloatx2*)p;
        for (n = 0; n < (N >> 1); n++, x += 8)
        {
        xtfloatx2 A0, B0, A1, B1, X0, X1, X2, X3, H0, H1;
        xtfloatx2 Y0, Y1, Y2, Y3, Y4;
        A0 = B0 = A1 = B1 = (xtfloatx2)0.0f;

        ph = (const xtfloatx2*)h;
        ah = AE_LA64_PP(ph);

        px = (xtfloatx2*)(x);
        ax = AE_LA64_PP(px);
        XT_LASX2IP(Y0, ax, px);
        XT_LASX2IP(Y1, ax, px);
        XT_LASX2IP(Y2, ax, px);
        pd = (const xtfloatx2*)(((const float32_t*)pp));
        XT_LASX2NEGPC(ad, pd);
        XT_LASX2RIC(Y3, ad, pd);
        XT_LASX2RIC(Y4, ad, pd);
        X3 = XT_SEL32_HL_SX2(Y1, Y0);
        X2 = XT_SEL32_HL_SX2(Y2, Y1);
        X0 = XT_SEL32_HL_SX2(Y0, Y3);
        X1 = Y4;
        for (m = 0; m < (M >> 2); m++)
        {
            XT_LASX2IP(H0, ah, ph);
            XT_LASX2IP(H1, ah, ph);
            XT_MADD_SX2(A0, H0, X0);
            XT_MADD_SX2(A1, H0, X2);
            XT_MADD_SX2(B0, H1, X1);
            XT_MADD_SX2(B1, H1, X3);
            X2 = X0;
            X3 = X1;
            XT_LASX2RIC(X0, ad, pd);
            XT_LASX2RIC(X1, ad, pd);
        }
        if (M & 2)
        {
            XT_LASX2IP(H0, ah, ph);
            XT_MADD_SX2(A0, H0, X0);
            XT_MADD_SX2(A1, H0, X2);
        }
        A0 = A0 + B0;
        A1 = A1 + B1;
        A0 = A0 + XT_SEL32_LH_SX2(A0, A0);
        A1 = A1 + XT_SEL32_LH_SX2(A1, A1);

        px = (xtfloatx2*)(x);
        ax = AE_LA64_PP(px);
        XT_LASX2IP(Y0, ax, px);
        XT_LASX2IP(Y1, ax, px);
        XT_LASX2IP(Y2, ax, px);
        XT_LASX2IP(Y3, ax, px);
        XT_SSX2XC(Y0, pp, sizeof(Y0));
        XT_SSX2XC(Y1, pp, sizeof(Y1));
        XT_SSX2XC(Y2, pp, sizeof(Y2));
        XT_SSX2XC(Y3, pp, sizeof(Y3));
        XT_SSIP(A0, pz, sizeof(float32_t));
        XT_SSIP(A1, pz, sizeof(float32_t));
    }
    return (float32_t*)pp;
}
#endif
#endif
#endif /*HAVE_VFPU*/

