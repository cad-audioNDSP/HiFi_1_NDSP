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
    Real FIR Filter with decimation
    C code optimized for HiFi1
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Common helper macros. */
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(float32_t *, fir_decimaf_Dx, (float32_t * restrict z, float32_t * restrict delay, 
            float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int N))
            
#else
#include "fir_decimaf_Dx.h"

/*
    universal decimator:
    Input/output:
    delay[M] - circular delay line
    Input:
    p        - pointer to delay line
    x[N*D]   - input signal
    h[M]     - impulse response
    D          decimation factor
    N        - number of output samples
    Output:
    z[N]     - output samples
    Restrictions:
    N>0, M>0
    D>4
    M multiple of 2
    N multiple of 8
    delay should be aligned on 8 byte boundary

    Returns:
    updated pointer to delay line
*/
#if 0
float32_t * fir_decimaf_Dx(float32_t * restrict z, float32_t * restrict delay, float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int D, int N)
{
    const xtfloat   *px;
    xtfloat   *pz;
    const xtfloatx2 *ph;
    xtfloat   *pp;
    const xtfloatx2 *pd;
    xtfloatx2 A0, A1, A2, A3, XX, X0, X1, X2, X3, H0, H1, H2, H3;
    ae_valign ah, ad;

    int m, n, d;
    NASSERT(N > 0 && M > 0);
    NASSERT(M % 2 == 0);
    NASSERT(N % 8 == 0);
    NASSERT(D > 4);
    NASSERT_ALIGN(delay, 8);
    WUR_AE_CBEGIN0((uintptr_t)(delay + 0));
    WUR_AE_CEND0((uintptr_t)(delay + M));
    px = (const xtfloat*)x;
    pz = (xtfloat*)z;
    pp = (xtfloat*)p;
    for (n = 0; n < N; n += 1)
    {
        ph = (const xtfloatx2*)h;
        ah = AE_LA64_PP(ph);

        A0 = A1 = A2 = A3 = (xtfloatx2)0.0f;
        pd = (const xtfloatx2*)pp;
        XX = XT_LSI(px, 0);
        XT_LASX2NEGPC(ad, pd);
        XT_LASX2RIC(X0, ad, pd);
        XT_LASX2RIC(X1, ad, pd);
        XT_LASX2RIC(X2, ad, pd);
        XT_LASX2RIC(X3, ad, pd);
        X0 = XT_SEL32_HL_SX2(XX, X0);
        for (m = 0; m < (M&~7); m += 8)
        {
            XT_LASX2IP(H0, ah, ph);
            XT_LASX2IP(H1, ah, ph);
            XT_LASX2IP(H2, ah, ph);
            XT_LASX2IP(H3, ah, ph);
            XT_MADD_SX2(A0, H0, X0);
            XT_MADD_SX2(A1, H1, X1);
            XT_MADD_SX2(A2, H2, X2);
            XT_MADD_SX2(A3, H3, X3);
            XT_LASX2RIC(X0, ad, pd);
            XT_LASX2RIC(X1, ad, pd);
            XT_LASX2RIC(X2, ad, pd);
            XT_LASX2RIC(X3, ad, pd);
        }
        if (M & 4)
        {
            XT_LASX2IP(H0, ah, ph);
            XT_LASX2IP(H1, ah, ph);
            XT_MADD_SX2(A0, H0, X0);
            XT_MADD_SX2(A1, H1, X1);
            X0 = X2;
        }
        if (M & 2)
        {
            XT_LASX2IP(H0, ah, ph);
            XT_MADD_SX2(A0, H0, X0);
        }
        A0 = A0 + A1;
        A2 = A2 + A3;
        A0 = A0 + A2;
        A0 = A0 + XT_SEL32_LH_SX2(A0, A0);
        __Pragma("loop_count min=4");
        for (d = 0; d < D; d++)
        {
            xtfloat t;
            XT_LSIP(t, px, sizeof(float32_t));
            XT_SSXC(t, pp, sizeof(float32_t));
        }
        XT_SSIP(A0, pz, sizeof(float32_t));
    }
    return (float32_t*)pp;
} /* fir_decimaf_Dx() */
#else
/*The following code is implemented to improve MAC throughput*/
float32_t* firdecf_Dx(
    const float32_t* h, int M, 
    float32_t* d, float32_t* p, int L,
    float32_t * restrict y, const float32_t * x, int N, int D )
{
    const xtfloatx2 *restrict pX;
          xtfloatx2 *restrict pY;
    const xtfloatx2* pD0;
    const xtfloatx2* pD1;
    const xtfloatx2* pD2;
    const xtfloatx2* pD3;
    const xtfloatx2* pH;
    xtfloatx2 * pWr;
    ae_valign ay;
    xtfloatx2 X01,X23,X45,X67,H01;
    int m,n,j;
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
    NASSERT(D>1);

    pX=(const xtfloatx2 *)x;
    WUR_AE_CBEGIN0( (uintptr_t)( d    ) );
    WUR_AE_CEND0  ( (uintptr_t)( d + L) );
    ay=AE_ZALIGN64();
    pY=(xtfloatx2 *)y;
    pWr=(xtfloatx2*)p;
    for (n=0; n<N; n+=4)
    {
        xtfloatx2 q0,q1,q2,q3,q01h,q01l,q23h,q23l,q0r,q2r;
        xtfloatx2 t;
        xtfloat   t1;
        ae_valign ad0,ad1,ad2,ad3;
        pH=(const xtfloatx2*)h;
        q0=q1=q2=q3=(xtfloatx2)0.f;
        pD0=(xtfloatx2*)(pWr);
        for(j=0; j<2*D; j++) 
        {
             XT_LSX2IP(t,pX,2*sizeof(float32_t));  
             XT_SSX2XC(t,pWr,2*sizeof(float32_t));
        }
        pD1=pD0; XT_LSXC(t1,castxcc(xtfloat, pD1),D*sizeof(float32_t)); 
        pD2=pD1; XT_LSXC(t1,castxcc(xtfloat, pD2),D*sizeof(float32_t)); 
        pD3=pD2; XT_LSXC(t1,castxcc(xtfloat, pD3),D*sizeof(float32_t)); 

        XT_LASX2NEGPC(ad0,pD0);
        XT_LASX2NEGPC(ad1,pD1);
        XT_LASX2NEGPC(ad2,pD2);
        XT_LASX2NEGPC(ad3,pD3);
        for (m=0; m<M; m+=2)
        {
            XT_LASX2RIC(X01,ad0,pD0); 
            XT_LASX2RIC(X23,ad1,pD1);
            XT_LASX2RIC(X45,ad2,pD2); 
            XT_LASX2RIC(X67,ad3,pD3);
            XT_LSX2IP(H01,pH,2*sizeof(float32_t));
            XT_MADD_SX2(q0,X01,H01);
            XT_MADD_SX2(q1,X23,H01);
            XT_MADD_SX2(q2,X45,H01);
            XT_MADD_SX2(q3,X67,H01);
        }
        q01h = XT_SEL32_HH_SX2(q0,q1); 
        q01l = XT_SEL32_LL_SX2(q0,q1); 
        q0r  = XT_ADD_SX2(q01h, q01l);
        XT_SASX2IP(q0r, ay, pY);
        
        q23h = XT_SEL32_HH_SX2(q2,q3); 
        q23l = XT_SEL32_LL_SX2(q2,q3); 
        q2r  = XT_ADD_SX2(q23h, q23l);
        XT_SASX2IP(q2r, ay, pY);

    }
    AE_SA64POS_FP(ay,pY);
    return (float32_t*)pWr;
}
#endif
#endif /*HAVE_VFPU*/

