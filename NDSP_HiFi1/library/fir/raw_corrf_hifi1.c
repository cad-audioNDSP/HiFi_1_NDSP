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
    NatureDSP Signal Processing Library. FIR part
    Real data circular convolution/correlation, floating point, helper file
    Code optimized for HiFi1
*/

/* Portable data types. */
#include "NatureDSP_types.h"
/* Common utility and macros declarations. */
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, raw_corrf, (float32_t * restrict r,
            const float32_t * restrict x,
            const float32_t * restrict y,
            int N, int M))
            
            
#else
#include "raw_corrf.h"
/*
    raw linear correlation: 
    Restrictions:
    x - aligned on 8 byte boundary
    N>0 && M>0
    N>=M-1
*/
void raw_corrf( float32_t  * restrict r,
                const float32_t  * restrict x,
                const float32_t  * restrict y,
                int N, int M )
{
    //
    // Circular cross-correlation algorithm:
    //
    //   r[n] = sum( x[mod(n+m,N)]*y[m] )
    //        m=0..M-1
    //
    //   where n = 0..N-1
    //
    ae_int32x2 tmp;
    const xtfloatx2*   restrict px0;
    const xtfloatx2*   restrict px1;
    const ae_int32*    restrict py;
    xtfloatx2*   restrict pr;
    ae_valign ax1, ar;
    xtfloatx2 A0, A1, A2, A3, X01, X23, X45, X67, X12, X34, X56, X78, Y0, Y1;

    int n, m;
    NASSERT(r);
    NASSERT(x);
    NASSERT(y);
    NASSERT_ALIGN(x, 8);
    NASSERT(N > 0 && M > 0);
    NASSERT(N >= M - 1);
    ar = AE_ZALIGN64();
    pr = (xtfloatx2*)r;
    for (n = 0; n < (N&~7); n += 8)
    {
        px0 = (const xtfloatx2*)(x + n);
        px1 = (const xtfloatx2*)(x + n + 1);
        ax1 = AE_LA64_PP(px1);
        py = (const ae_int32*)y;
        A0 = A1 = A2 = A3 = (xtfloatx2)0.0f;
        XT_LSX2IP(X01, px0, sizeof(X01));
        XT_LSX2IP(X23, px0, sizeof(X23));
        XT_LSX2IP(X45, px0, sizeof(X45));
        XT_LASX2IP(X12, ax1, px1);
        XT_LASX2IP(X34, ax1, px1);
        XT_LASX2IP(X56, ax1, px1);
        for (m = 0; m < (M&~1); m += 2)
        {
            XT_LSX2IP(X67, px0, sizeof(X67));
            XT_LASX2IP(X78, ax1, px1);
            AE_L32_IP(tmp, py, sizeof(float32_t)); Y0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(tmp);
            AE_L32_IP(tmp, py, sizeof(float32_t)); Y1 = XT_AE_MOVXTFLOATX2_FROMINT32X2(tmp);
            XT_MADD_SX2(A0, X01, Y0);
            XT_MADD_SX2(A1, X23, Y0);
            XT_MADD_SX2(A2, X45, Y0);
            XT_MADD_SX2(A3, X67, Y0);
            XT_MADD_SX2(A0, X12, Y1);
            XT_MADD_SX2(A1, X34, Y1);
            XT_MADD_SX2(A2, X56, Y1);
            XT_MADD_SX2(A3, X78, Y1);
            X01 = X23; X23 = X45; X45 = X67;
            X12 = X34; X34 = X56; X56 = X78;
        }
        if (M & 1)
        {
            XT_LSX2IP(X67, px0, sizeof(X67));
            AE_L32_IP(tmp, py, sizeof(float32_t)); Y0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(tmp);
            XT_MADD_SX2(A0, X01, Y0);
            XT_MADD_SX2(A1, X23, Y0);
            XT_MADD_SX2(A2, X45, Y0);
            XT_MADD_SX2(A3, X67, Y0);
        }
        XT_SASX2IP(A0, ar, pr);
        XT_SASX2IP(A1, ar, pr);
        XT_SASX2IP(A2, ar, pr);
        XT_SASX2IP(A3, ar, pr);
    }
    XT_SASX2POSFP(ar, pr);
    /* last 1...7 iterations */
    N &= 7;
    for (; N > 0; n += 4, N -= 4)
    {
        px0 = (const xtfloatx2*)(x + n);
        px1 = (const xtfloatx2*)(x + n + 1);
        ax1 = AE_LA64_PP(px1);
        py = (const ae_int32*)y;
        A0 = A1 = A2 = A3 = (xtfloatx2)0.0f;
        XT_LSX2IP(X01, px0, sizeof(X01));
        XT_LASX2IP(X12, ax1, px1);
        for (m = 0; m < (M&~1); m += 2)
        {
            XT_LSX2IP(X23, px0, sizeof(X67));
            XT_LASX2IP(X34, ax1, px1);
            AE_L32_IP(tmp, py, sizeof(float32_t)); Y0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(tmp);
            AE_L32_IP(tmp, py, sizeof(float32_t)); Y1 = XT_AE_MOVXTFLOATX2_FROMINT32X2(tmp);
            XT_MADD_SX2(A0, X01, Y0);
            XT_MADD_SX2(A1, X23, Y0);
            XT_MADD_SX2(A2, X12, Y1);
            XT_MADD_SX2(A3, X34, Y1);
            X01 = X23;
            X12 = X34;
        }
        if (M & 1)
        {
            XT_LSX2IP(X23, px0, sizeof(X67));
            /*XT_LASX2IP(X34,ax1,px1);*/
            AE_L32_IP(tmp, py, sizeof(float32_t)); Y0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(tmp);
            XT_MADD_SX2(A0, X01, Y0);
            XT_MADD_SX2(A1, X23, Y0);
        }
        A0 = A0 + A2;
        A1 = A1 + A3;
        if (N > 3) XT_SSI((A1), (xtfloat*)pr, 3 * sizeof(xtfloat));
        if (N > 2) XT_SSI(XT_HIGH_S(A1), (xtfloat*)pr, 2 * sizeof(xtfloat));
        if (N > 1) XT_SSI(A0, (xtfloat*)pr, 1 * sizeof(xtfloat));
        if (N > 0) XT_SSI(XT_HIGH_S(A0), (xtfloat*)pr, 0 * sizeof(xtfloat));
        pr += 2;
    }
} /* raw_corrf() */

#endif /*HAVE_VFPU*/

