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
DISCARD_FUN(float32_t *, fir_interpf_2x, (float32_t * restrict z, float32_t * restrict delay, 
            float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int N))
            
#else
#include "fir_interpf_2x.h"

/*
    2x interpolator:
    Input/output:
    delay[M*2] - circular delay line
    Input:
    p        - pointer to delay line
    x[N]     - input signal
    h[M*2]   - impulse response
    N        - number of output samples
    Output:
    z[N*2]     - output samples
    Restrictions:
    N>0, M>0
    N   - multiple of 8
    M   - multiple of 4
    delay should be aligned on 8 byte boundary

    Returns:
    updated pointer to delay line
*/

float32_t * fir_interpf_2x(float32_t * restrict z, float32_t * restrict delay, float32_t * restrict p, const float32_t * restrict h, const float32_t * restrict x, int M, int N)
{
#define P 4
    int n, m, _N;
    const xtfloatx2*  restrict pX = (const xtfloatx2*)x;
    //const xtfloat  *  restrict px = (const xtfloat  *)x;
    const xtfloatx2*  restrict px = (const xtfloatx2*)x;
    xtfloatx2* restrict pD = (xtfloatx2*)p;
    const xtfloatx2*   restrict pH = (const xtfloatx2*)h;
    const xtfloatx2*   restrict pH1 = (const xtfloatx2*)(h+M);

    xtfloatx2*          pZ = (xtfloatx2*)z;
    ae_valign x_align, z_align, d_align,x1_align,h_align,h1_align;
    NASSERT(x);
    NASSERT(z);
    NASSERT(N > 0);
    NASSERT(M > 0);
    NASSERT(N % 8 == 0);
    NASSERT(M % 4 == 0);

    _N = N & (~3);
    x_align = AE_LA64_PP(pX);
    x1_align = AE_LA64_PP(px);
    z_align = AE_ZALIGN64();
    WUR_AE_CBEGIN0((uintptr_t)(delay));
    WUR_AE_CEND0((uintptr_t)(delay + M));
    for (n = 0; n < _N; n += P)
    {
        xtfloatx2 temp;
        xtfloatx2 x0, x1;
        xtfloatx2 acc0, acc1, acc2, acc3;
        xtfloatx2    s0, s1;// s2, s3;
        xtfloatx2 h0, h1;
        xtfloatx2 t0, t1;

        XT_LASX2IP(x0, x_align, pX);
        XT_LASX2IP(x1, x_align, pX);
        pH = (const xtfloatx2*)h;
        pH1 = (const xtfloatx2*)(h+M);
        h_align=AE_LA64_PP(pH);
        h1_align=AE_LA64_PP(pH1);

        d_align = AE_LA64_PP(pD);
        XT_LASX2RIC(temp, d_align, pD);
        x1 = XT_SEL32_LH_SX2(x1, x1);
        x0 = XT_SEL32_LH_SX2(x0, x0);


        XT_LASX2IP(h0,h_align,pH);
        XT_LASX2IP(h1,h1_align,pH1);


        acc0=XT_MULMUX_S(h0,x0,0);
        acc1=XT_MULMUX_S(h0,x1,0);
        acc2=XT_MULMUX_S(h1,x0,0);
        acc3=XT_MULMUX_S(h1,x1,0);

       // __Pragma("loop_count min=1")
		for (m = 0; m < (M-2); m += 2)
		{

			/* select and load next sample */
			t1 = XT_SEL32_LH_SX2(x1, x0);
			XT_LASX2RIC(temp, d_align, pD);
			t0 = XT_SEL32_LH_SX2(x0, temp);

			h0 = XT_SEL32_LH_SX2(h0, h0);
			h1 = XT_SEL32_LH_SX2(h1, h1);

			XT_MADDMUX_S(acc0,h0,t0,0);
			XT_MADDMUX_S(acc1,h0,t1,0);
			XT_MADDMUX_S(acc2,h1,t0,0);
			XT_MADDMUX_S(acc3,h1,t1,0);
			/* select next sample */
			x1 = x0;
			x0 = temp;

			XT_LASX2IP(h0,h_align,pH);
			XT_LASX2IP(h1,h1_align,pH1);

			XT_MADDMUX_S(acc0,h0,x0,0);
			XT_MADDMUX_S(acc1,h0,x1,0);
			XT_MADDMUX_S(acc2,h1,x0,0);
			XT_MADDMUX_S(acc3,h1,x1,0);
		}

		t1 = XT_SEL32_LH_SX2(x1, x0);
		XT_LASX2RIC(temp, d_align, pD);
		t0 = XT_SEL32_LH_SX2(x0, temp);

		h0 = XT_SEL32_LH_SX2(h0, h0);
		h1 = XT_SEL32_LH_SX2(h1, h1);

		XT_MADDMUX_S(acc0,h0,t0,0);
		XT_MADDMUX_S(acc1,h0,t1,0);
		XT_MADDMUX_S(acc2,h1,t0,0);
		XT_MADDMUX_S(acc3,h1,t1,0);
		/* select next sample */
		x1 = x0;
		x0 = temp;

        XT_LASX2IC(temp, d_align, pD);
        temp = XT_SEL32_LL_SX2(acc0, acc2);    XT_SASX2IP(temp, z_align, pZ);
        temp = XT_SEL32_HH_SX2(acc0, acc2);    XT_SASX2IP(temp, z_align, pZ);
        temp = XT_SEL32_LL_SX2(acc1, acc3);    XT_SASX2IP(temp, z_align, pZ);
        temp = XT_SEL32_HH_SX2(acc1, acc3);    XT_SASX2IP(temp, z_align, pZ);

        XT_LASX2IP(s0, x1_align, px);
        XT_LASX2IP(s1, x1_align, px);

        XT_SSX2XC(s0, pD, 8);
        XT_SSX2XC(s1, pD, 8);

  }
    AE_SA64POS_FP(z_align, pZ);

    return (float32_t*)pD;
} /* fir_interpf_2x() */

#endif /*HAVE_VFPU*/
