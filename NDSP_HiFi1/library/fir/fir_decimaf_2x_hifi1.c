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
    Real FIR Filter with decimation (2x)
    C code optimized for HiFi1
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Common helper macros. */
#include "common.h"
/* Filters and transformations */
#include "NatureDSP_Signal.h"

#if !(HAVE_VFPU)
DISCARD_FUN(float32_t *, fir_decimaf_2x, (float32_t * restrict z, float32_t * restrict delay, 
            float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int N))

#else
#include "fir_decimaf_2x.h"

/*
    2x decimator:
    Input/output:
    delay[M] - circular delay line
    Input:
    p        - pointer to delay line
    x[N*2]   - input signal
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
float32_t * fir_decimaf_2x(float32_t * restrict z, float32_t * restrict delay, float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int N)
{
    xtfloat* restrict pz;
    const xtfloatx2 *restrict ph;
    const xtfloatx2 *restrict px;
    xtfloatx2 * pp;
    const xtfloatx2 * pd;
    ae_valign ax, ad, ah;
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

    pp = (xtfloatx2*)p;
    pz = (xtfloat*)z;
    /* process by 8 input samples */
    for (n = 0; n < (N >> 2); n++, x += 8)
    {
        xtfloatx2 A0, A1, A2, A3, XX, X0, X1, X2, X3, H0;

        ph = (const xtfloatx2*)h;
        ah = AE_LA64_PP(ph);

        A0 = A1 = A2 = A3 = (xtfloatx2)0.0f;
        px = (xtfloatx2*)(x);
        ax = AE_LA64_PP(px);
        XT_LASX2IP(X0, ax, px);
        XT_LASX2IP(X1, ax, px);
        XT_LASX2IP(X2, ax, px);
        XT_LASX2IP(X3, ax, px);
        pd = (const xtfloatx2*)(((const float32_t*)pp));
        XT_LASX2NEGPC(ad, pd);
        XT_LASX2RIC(XX, ad, pd);

        X3 = XT_SEL32_HL_SX2(X3, X2);
        X2 = XT_SEL32_HL_SX2(X2, X1);
        X1 = XT_SEL32_HL_SX2(X1, X0);
        X0 = XT_SEL32_HL_SX2(X0, XX);
        for (m = 0; m < M; m += 2)
        {
            XT_LASX2IP(H0, ah, ph);
            XT_MADD_SX2(A0, H0, X0);
            XT_MADD_SX2(A1, H0, X1);
            XT_MADD_SX2(A2, H0, X2);
            XT_MADD_SX2(A3, H0, X3);
            X3 = X2;
            X2 = X1;
            X1 = X0;
            XT_LASX2RIC(X0, ad, pd);
        }
        A0 = A0 + XT_SEL32_LH_SX2(A0, A0);
        A1 = A1 + XT_SEL32_LH_SX2(A1, A1);
        A2 = A2 + XT_SEL32_LH_SX2(A2, A2);
        A3 = A3 + XT_SEL32_LH_SX2(A3, A3);
        px = (xtfloatx2*)(x);
        ax = AE_LA64_PP(px);
        XT_LASX2IP(X0, ax, px);
        XT_LASX2IP(X1, ax, px);
        XT_LASX2IP(X2, ax, px);
        XT_LASX2IP(X3, ax, px);
        XT_SSX2XC(X0, pp, sizeof(X0));
        XT_SSX2XC(X1, pp, sizeof(X1));
        XT_SSX2XC(X2, pp, sizeof(X2));
        XT_SSX2XC(X3, pp, sizeof(X3));

        XT_SSIP(A0, pz, sizeof(float32_t));
        XT_SSIP(A1, pz, sizeof(float32_t));
        XT_SSIP(A2, pz, sizeof(float32_t));
        XT_SSIP(A3, pz, sizeof(float32_t));
    }

    return (float32_t*)pp;
} /* fir_decimaf_2x() */
#else
/*The following code is implemented to improve MAC throughput*/
float32_t* firdecf_D2(
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
    xtfloatx2 X01,X23,X45,X67,H01;
    int n,m;
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
    NASSERT(D==2);

    pX=(const xtfloatx2 *)x;
    WUR_AE_CBEGIN0( (uintptr_t)( d    ) );
    WUR_AE_CEND0  ( (uintptr_t)( d + L) );
    ay=AE_ZALIGN64();
    pY=(xtfloatx2*)y;
    pWr=(xtfloatx2*)p;
    for (n=0; n<N; n+=4)
    {
        xtfloatx2 t; 
        xtfloat   t1;
        xtfloatx2 q0,q1,q2,q3,q01h,q01l,q23h,q23l,q0r,q2r;
        q0=q1=q2=q3=(xtfloatx2)0.f;
        pD=pWr;
        XT_LSXC(t1, castxcc(xtfloat, pD), 6 * sizeof(float32_t));
        pH=(const xtfloatx2*)h;

        XT_LSX2IP(t,pX,2*sizeof(float32_t));  XT_SSX2XC(t,pWr,2*sizeof(float32_t));
        XT_LSX2IP(t,pX,2*sizeof(float32_t));  XT_SSX2XC(t,pWr,2*sizeof(float32_t));
        XT_LSX2IP(t,pX,2*sizeof(float32_t));  XT_SSX2XC(t,pWr,2*sizeof(float32_t));
        XT_LSX2IP(t,pX,2*sizeof(float32_t));  XT_SSX2XC(t,pWr,2*sizeof(float32_t));
        XT_LASX2NEGPC(aD,pD);
        XT_LASX2RIC(X67,aD,pD);
        XT_LASX2RIC(X45,aD,pD);
        XT_LASX2RIC(X23,aD,pD);
        XT_LASX2RIC(X01,aD,pD);
        for (m=0; m<M; m+=2)
        {
            XT_LSX2IP(H01,pH,2*sizeof(float32_t));
            XT_MADD_SX2(q0,X01,H01);
            XT_MADD_SX2(q1,X23,H01);
            XT_MADD_SX2(q2,X45,H01);
            XT_MADD_SX2(q3,X67,H01);
            X67=X45;
            X45=X23;
            X23=X01;
            XT_LASX2RIC(X01,aD,pD);
        }
        // Store 4 filter outputs.
        q01h = XT_SEL32_HH_SX2(q0, q1);
        q01l = XT_SEL32_LL_SX2(q0, q1);
        q0r  = XT_ADD_SX2(q01h, q01l);
        q23h = XT_SEL32_HH_SX2(q2, q3);
        q23l = XT_SEL32_LL_SX2(q2, q3);
        q2r  = XT_ADD_SX2(q23h, q23l);
        XT_SASX2IP(q0r, ay, pY);
        XT_SASX2IP(q2r, ay, pY);
    }
    AE_SA64POS_FP(ay,pY);
    return (float32_t*)pWr;
}
#endif
#endif /*HAVE_VFPU*/

