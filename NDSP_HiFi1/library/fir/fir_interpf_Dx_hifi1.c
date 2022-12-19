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
DISCARD_FUN(float32_t *, fir_interpf_Dx, (float32_t * restrict z, float32_t * restrict delay, 
            float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int N))
            
#else
#include "fir_interpf_Dx.h"

/*
    Dx interpolator:
    Input/output:
    delay[M*D] - circular delay line
    Input:
    p        - pointer to delay line
    x[N]     - input signal
    h[M*D]   - impulse response
    N        - number of output samples
    Output:
    z[N*D]     - output samples
    Restrictions:
    N>0, M>0
    N   - multiple of 8
    M   - multiple of 4
    D > 1
    delay should be aligned on 8 byte boundary

    Returns:
    updated pointer to delay line
*/
float32_t * fir_interpf_Dx(float32_t * restrict z, float32_t * restrict delay, float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int D, int N)
{
#define P 8
    int n, m, j, _N;
    const xtfloatx2*  restrict pX = (const xtfloatx2*)x;
    //const xtfloat  *  restrict px = (const xtfloat  *)x;
    const xtfloatx2*  restrict px = (const xtfloatx2*)x;
    xtfloatx2  * restrict pD = (xtfloatx2  *)p;
    const xtfloatx2*   restrict pH = (const xtfloatx2*)h;
    xtfloat*          pZ = (xtfloat*)z;
    ae_valign x_align, z_align, d_align, x1_align,h_align;
    NASSERT(x);
    NASSERT(z);
    //M = M;
    NASSERT(N > 0);
    NASSERT(M > 0);
    NASSERT(D > 1);
    NASSERT(N % 8 == 0);
    NASSERT(M % 4 == 0);
    _N = N & (~7);
    //p = p;
    x_align = AE_LA64_PP(pX);
    z_align = AE_ZALIGN64();
    x1_align = AE_LA64_PP(px);
    WUR_AE_CBEGIN0((uintptr_t)(delay));
    WUR_AE_CEND0((uintptr_t)(delay + M));
    for (n = 0; n < _N; n += P)
    {
        xtfloatx2 x0, x1, x2, x3;
        xtfloatx2 acc0, acc1, acc2, acc3;
        xtfloatx2    s0, s1, s2, s3;
        xtfloatx2 hm;
        xtfloatx2 t0, t1, t2;
        __Pragma("loop_count min=5")
            for (j = 0; j < D; j++)
            {
                xtfloatx2 temp;
                pX = (xtfloatx2*)((uintptr_t)(x + n));
                x_align = AE_LA64_PP(pX);
                XT_LASX2IP(x0, x_align, pX);
                XT_LASX2IP(x1, x_align, pX);
                XT_LASX2IP(x2, x_align, pX);
                XT_LASX2IP(x3, x_align, pX);

                d_align = AE_LA64_PP(pD);
                XT_LASX2RIC(temp, d_align, pD);

                x3 = XT_SEL32_LH_SX2(x3, x3);
                x2 = XT_SEL32_LH_SX2(x2, x2);
                x1 = XT_SEL32_LH_SX2(x1, x1);
                x0 = XT_SEL32_LH_SX2(x0, x0);

                pH=(const xtfloatx2*)(h+j * M);

                h_align=AE_LA64_PP(pH);
                XT_LASX2IP(hm,h_align,pH);
                acc0=XT_MULMUX_S(hm,x0,0);
                acc1=XT_MULMUX_S(hm,x1,0);
                acc2=XT_MULMUX_S(hm,x2,0);
                acc3=XT_MULMUX_S(hm,x3,0);

               // __Pragma("loop_count min=1")
                    for (m = 0; m < (M-4); m += 4)
                    {

                        hm = XT_SEL32_LH_SX2(hm, hm);
                        /* select and load next sample */
                        x3 = XT_SEL32_LH_SX2(x3, x2);
                        t2 = XT_SEL32_LH_SX2(x2, x1);
                        t1 = XT_SEL32_LH_SX2(x1, x0);
                        XT_LASX2RIC(temp, d_align, pD);
                        t0 = XT_SEL32_LH_SX2(x0, temp);

                        XT_MADDMUX_S(acc0,hm,t0,0);
                        XT_MADDMUX_S(acc1,hm,t1,0);
                        XT_MADDMUX_S(acc2,hm,t2,0);
                        XT_MADDMUX_S(acc3,hm,x3,0);

                        x3 = x2;
                        x2 = x1;
                        x1 = x0;
                        x0 = temp;

                        XT_LASX2IP(hm,h_align,pH);
                        XT_MADDMUX_S(acc0,hm,x0,0);
                        XT_MADDMUX_S(acc1,hm,x1,0);
                        XT_MADDMUX_S(acc2,hm,x2,0);
                        XT_MADDMUX_S(acc3,hm,x3,0);

                        hm = XT_SEL32_LH_SX2(hm, hm);
                        /* select and load next sample */
                        x3 = XT_SEL32_LH_SX2(x3, x2);
                        t2 = XT_SEL32_LH_SX2(x2, x1);
                        t1 = XT_SEL32_LH_SX2(x1, x0);
                        XT_LASX2RIC(temp, d_align, pD);
                        t0 = XT_SEL32_LH_SX2(x0, temp);

                        XT_MADDMUX_S(acc0,hm,t0,0);
                        XT_MADDMUX_S(acc1,hm,t1,0);
                        XT_MADDMUX_S(acc2,hm,t2,0);
                        XT_MADDMUX_S(acc3,hm,x3,0);

                        x3 = x2;
                        x2 = x1;
                        x1 = x0;
                        x0 = temp;

                        XT_LASX2IP(hm,h_align,pH);
                        XT_MADDMUX_S(acc0,hm,x0,0);
                        XT_MADDMUX_S(acc1,hm,x1,0);
                        XT_MADDMUX_S(acc2,hm,x2,0);
                        XT_MADDMUX_S(acc3,hm,x3,0);
                    }

                hm = XT_SEL32_LH_SX2(hm, hm);
                x3 = XT_SEL32_LH_SX2(x3, x2);
                t2 = XT_SEL32_LH_SX2(x2, x1);
                t1 = XT_SEL32_LH_SX2(x1, x0);
                XT_LASX2RIC(temp, d_align, pD);
                t0 = XT_SEL32_LH_SX2(x0, temp);

                XT_MADDMUX_S(acc0,hm,t0,0);
                XT_MADDMUX_S(acc1,hm,t1,0);
                XT_MADDMUX_S(acc2,hm,t2,0);
                XT_MADDMUX_S(acc3,hm,x3,0);

                x3 = x2;
                x2 = x1;
                x1 = x0;
                x0 = temp;

				XT_LASX2IP(hm,h_align,pH);
				XT_MADDMUX_S(acc0,hm,x0,0);
				XT_MADDMUX_S(acc1,hm,x1,0);
				XT_MADDMUX_S(acc2,hm,x2,0);
				XT_MADDMUX_S(acc3,hm,x3,0);

				hm = XT_SEL32_LH_SX2(hm, hm);
				/* select and load next sample */
				x3 = XT_SEL32_LH_SX2(x3, x2);
				t2 = XT_SEL32_LH_SX2(x2, x1);
				t1 = XT_SEL32_LH_SX2(x1, x0);
				XT_LASX2RIC(temp, d_align, pD);
				t0 = XT_SEL32_LH_SX2(x0, temp);

				XT_MADDMUX_S(acc0,hm,t0,0);
				XT_MADDMUX_S(acc1,hm,t1,0);
				XT_MADDMUX_S(acc2,hm,t2,0);
				XT_MADDMUX_S(acc3,hm,x3,0);

                XT_LASX2IC(temp, d_align, pD);
                pZ = (xtfloat*)((uintptr_t*)z + n * D + j);
                XT_SSXP((xtfloat)acc0, pZ, D * 4);
                acc0 = XT_SEL32_LH_SX2(acc0, acc0);
                XT_SSXP((xtfloat)acc0, pZ, D * 4);
                XT_SSXP((xtfloat)acc1, pZ, D * 4);
                acc1 = XT_SEL32_LH_SX2(acc1, acc1);
                XT_SSXP((xtfloat)acc1, pZ, D * 4);
                XT_SSXP((xtfloat)acc2, pZ, D * 4);
                acc2 = XT_SEL32_LH_SX2(acc2, acc2);
                XT_SSXP((xtfloat)acc2, pZ, D * 4);
                XT_SSXP((xtfloat)acc3, pZ, D * 4);
                acc3 = XT_SEL32_LH_SX2(acc3, acc3);
                XT_SSXP((xtfloat)acc3, pZ, -7 * D * 4 + 4);
    }

        XT_LASX2IP(s0, x1_align, px);
        XT_LASX2IP(s1, x1_align, px);
        XT_LASX2IP(s2, x1_align, px);
        XT_LASX2IP(s3, x1_align, px);

        XT_SSX2XC(s0, pD, 8);
        XT_SSX2XC(s1, pD, 8);
		XT_SSX2XC(s2, pD, 8);
		XT_SSX2XC(s3, pD, 8);
  }
    p = (float32_t*)pD;

    return (float32_t*)pD;
}/* fir_interpf_Dx() */

#endif /*HAVE_VFPU*/
