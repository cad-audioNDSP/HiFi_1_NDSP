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
    Real interpolating FIR Filter
    C code optimized for HiFi1
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Common helper macros. */
#include "common.h"
/* Filters and transformations */
#include "NatureDSP_Signal.h"

#if !(HAVE_VFPU)
DISCARD_FUN(float32_t *, fir_interpf_4x, (float32_t * restrict z, float32_t * restrict delay, 
            float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int N))
            
#else
#include "fir_interpf_4x.h"

/*
    4x interpolator:
    Input/output:
    delay[M*4] - circular delay line
    Input:
    p        - pointer to delay line
    x[N]     - input signal
    h[M*4]   - impulse response
    N        - number of output samples
    Output:
    z[N*4]     - output samples
    Restrictions:
    N>0, M>0
    N   - multiple of 8
    M   - multiple of 4
    delay should be aligned on 8 byte boundary

    Returns:
    updated pointer to delay line
*/

float32_t * fir_interpf_4x(float32_t * restrict z, float32_t * restrict delay, float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int N)
{

#define P 4
    int n, m, _N;
    const xtfloatx2*  restrict pX = (const xtfloatx2*)x;
    const xtfloat  *  restrict px = (const xtfloat  *)x;
    xtfloat  * restrict pD = (xtfloat  *)p;
    const xtfloat*   restrict pH = (const xtfloat*)h;
    xtfloatx2*          pZ = (xtfloatx2*)z;
    ae_valign x_align, z_align;
    NASSERT(x);
    NASSERT(z);
    //M = M;
    NASSERT(N > 0);
    NASSERT(M > 0);
    NASSERT(N % 8 == 0);
    NASSERT(M % 4 == 0);
    _N = N & (~3);
    //p = p;
    x_align = AE_LA64_PP(pX);
    z_align = AE_ZALIGN64();
    WUR_AE_CBEGIN0((uintptr_t)(delay));
    WUR_AE_CEND0((uintptr_t)(delay + M));
    pZ = (xtfloatx2*)((uintptr_t*)z);
    z_align = AE_ZALIGN64();
    pX = (xtfloatx2*)((uintptr_t)(x));
    x_align = AE_LA64_PP(pX);
    for (n = 0; n < _N; n += P)
    {
        xtfloatx2 x0, x1;
        xtfloatx2 acc0, acc1, acc2, acc3, acc4, acc5, acc6, acc7;
        xtfloat    s0, s1, s2, s3;
        xtfloat tmp;

        XT_LASX2IP(x0, x_align, pX);
        XT_LASX2IP(x1, x_align, pX);
        acc0 = acc1 = acc2 = acc3 = acc4 = acc5 = acc6 = acc7 = (xtfloatx2)0.0f;
        XT_LSXC(tmp, pD, -4);

        __Pragma("loop_count min=2")
            for (m = 0; m < M; m++)
            {
                xtfloatx2 hm;
                hm = pH[m + 0];
                XT_MADD_SX2(acc0, hm, x0);
                XT_MADD_SX2(acc1, hm, x1);
                hm = pH[m + M];
                XT_MADD_SX2(acc2, hm, x0);
                XT_MADD_SX2(acc3, hm, x1);
                hm = pH[m + 2 * M];
                XT_MADD_SX2(acc4, hm, x0);
                XT_MADD_SX2(acc5, hm, x1);
                hm = pH[m + 3 * M];
                XT_MADD_SX2(acc6, hm, x0);
                XT_MADD_SX2(acc7, hm, x1);
                /* select and load next sample */
                x1 = XT_SEL32_LH_SX2(x0, x1);
                XT_LSXC(tmp, pD, -4);
                x0 = XT_SEL32_LH_SX2((xtfloatx2)tmp, x0);
            }
        XT_LSXC(tmp, pD, 4);
        {
            xtfloatx2 temp;
            temp = XT_SEL32_HH_SX2(acc0, acc2);        XT_SASX2IP(temp, z_align, pZ);
            temp = XT_SEL32_HH_SX2(acc4, acc6);        XT_SASX2IP(temp, z_align, pZ);
            temp = XT_SEL32_LL_SX2(acc0, acc2);        XT_SASX2IP(temp, z_align, pZ);
            temp = XT_SEL32_LL_SX2(acc4, acc6);        XT_SASX2IP(temp, z_align, pZ);
            temp = XT_SEL32_HH_SX2(acc1, acc3);        XT_SASX2IP(temp, z_align, pZ);
            temp = XT_SEL32_HH_SX2(acc5, acc7);        XT_SASX2IP(temp, z_align, pZ);
            temp = XT_SEL32_LL_SX2(acc1, acc3);        XT_SASX2IP(temp, z_align, pZ);
            temp = XT_SEL32_LL_SX2(acc5, acc7);        XT_SASX2IP(temp, z_align, pZ);
        }
        XT_LSIP(s0, px, 4);
        XT_LSIP(s1, px, 4);
        XT_LSIP(s2, px, 4);
        XT_LSIP(s3, px, 4);
        XT_SSXC(s0, pD, 4);
        XT_SSXC(s1, pD, 4);
        XT_SSXC(s2, pD, 4);
        XT_SSXC(s3, pD, 4);
    }
    AE_SA64POS_FP(z_align, pZ);
    return (float32_t*)pD;
} /* fir_interpf_4x() */

#endif /*HAVE_VFPU*/
