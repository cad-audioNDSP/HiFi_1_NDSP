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
    Complex block FIR filter, floating point
    C code optimized for HiFi1
*/

/*-------------------------------------------------------------------------
  Complex Block FIR Filter
  Computes a complex FIR filter (direct-form) using complex IR stored in 
  vector h. The complex data input is stored in vector x. The filter output
  result is stored in vector r. The filter calculates N output samples using 
  M coefficients and requires last M-1 samples in the delay line. Real and
  imaginary parts are interleaved and real parts go first (at even indexes).
  NOTE: 
  User application is not responsible for management of delay lines

  Precision: 
  16x16     16-bit data, 16-bit coefficients, 16-bit outputs
  24x24     24-bit data, 24-bit coefficients, 24-bit outputs
  32x16     32-bit data, 16-bit coefficients, 32-bit outputs
  32x32     32-bit data, 32-bit coefficients, 32-bit outputs
  f         floating point
  Input:
  h[M] complex filter coefficients; h[0] is to be multiplied with the newest 
       sample , Q31, Q15, floating point
  x[N] input samples , Q15, Q31 or floating point
  N    length of sample block (in complex samples) 
  M    length of filter 
  Output:
  y[N] output samples , Q15, Q31 or floating point

  Restriction:
  x,y - should not overlap
  x,h - aligned on a 8-bytes boundary
  N,M - multiples of 4
-------------------------------------------------------------------------*/
/* Portable data types. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
/* Common utility and macros declarations. */
#include "common.h"
#include "cxfirf.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, cxfirf_process, (cxfirf_handle_t _cxfir, complex_float * restrict  y, const complex_float * restrict  x, int N))

#else
void cxfirf_process(cxfirf_handle_t _cxfir,
    complex_float * restrict  y,
    const complex_float * restrict  x, int N)
{
    int n, m, M, _N;
    xtfloatx2 x0, x1, x2, x3;
    xtfloatx2 s0, s1, s2, s3;
    xtfloatx2 acc0, acc1, acc2, acc3;
    const xtfloatx2* restrict pX = (const xtfloatx2*)x;
    xtfloatx2* restrict pD;
    const xtfloatx2* restrict pH;
    xtfloatx2*          pZ = (xtfloatx2*)y;
    cxfirf_t* state;
    NASSERT(_cxfir);
    state = (cxfirf_t*)_cxfir;
    NASSERT(state->h);
    NASSERT(state->d);
    NASSERT(state->p);
    NASSERT_ALIGN(state->h, 8);
    NASSERT_ALIGN(state->d, 8);
    NASSERT_ALIGN(state->p, 8);
    NASSERT(N % 4 == 0);
    NASSERT_ALIGN(x, 8);
    NASSERT((state->M % 4) == 0);
    NASSERT(x);
    NASSERT(y);
    if (N <= 0) return;
    M = state->M;
    pD = (xtfloatx2*)state->p;
    pH = (const xtfloatx2*)state->h;
    NASSERT(N > 0);
    NASSERT(M > 0);
#define P1 4    /* unroll factor for the first loop */

    _N = N & (~(P1 - 1));
    WUR_AE_CBEGIN0((uintptr_t)(state->d));
    WUR_AE_CEND0((uintptr_t)(state->d + M));
    for (n = 0; n < _N; n += P1)
    {
        XT_LSX2IP(x0, pX, 8);
        XT_LSX2IP(x1, pX, 8);
        XT_LSX2IP(x2, pX, 8);
        XT_LSX2IP(x3, pX, 8);
        s0 = x0;
        s1 = x1;
        s2 = x2;
        s3 = x3;
        acc0 =
            acc1 =
            acc2 =
            acc3 = (xtfloatx2)0.0f;
        pH = (const xtfloatx2 *)state->h;
        { xtfloatx2 dummy;  XT_LSX2XC(dummy, pD, -8); }
        __Pragma("loop_count min=1,factor=4")
            for (m = 0; m < M; m++)
            {
                xtfloatx2 hm;
                XT_LSX2IP(hm, pH, 8);
                /*Complex SIMD multiply */
                XT_MADDC_S(acc0, hm, x0);
                XT_MADDC_S(acc1, hm, x1);
                XT_MADDC_S(acc2, hm, x2);
                XT_MADDC_S(acc3, hm, x3);
                /*load next sample */
                x3 = x2; x2 = x1; x1 = x0;
                XT_LSX2XC(x0, pD, -8);
            }
        { xtfloatx2 dummy;  XT_LSX2XC(dummy, pD, 8); }

        XT_SSX2IP(acc0, pZ, 8);
        XT_SSX2IP(acc1, pZ, 8);
        XT_SSX2IP(acc2, pZ, 8);
        XT_SSX2IP(acc3, pZ, 8);

        NASSERT_ALIGN(pD, 8);
        XT_SSX2XC(s0, pD, 8);
        XT_SSX2XC(s1, pD, 8);
        XT_SSX2XC(s2, pD, 8);
        XT_SSX2XC(s3, pD, 8);
    }
    __Pragma("loop_count max=3")
        for (; n < N; n++)
        {
            XT_LSX2IP(x0, pX, 8);
            s0 = x0;
            acc0 =
                acc1 = (xtfloatx2)0.0f;
            pH = (const xtfloatx2 *)state->h;
            { xtfloatx2 dummy;  XT_LSX2XC(dummy, pD, -8); }
            __Pragma("loop_count min=1,factor=4")
                for (m = 0; m < M; m++)
                {
                    xtfloatx2 hm;
                    XT_LSX2IP(hm, pH, 8);
                    XT_MADDMUX_S(acc0, hm, x0, 0);
                    XT_MADDMUX_S(acc1, hm, x0, 1);
                    XT_LSX2XC(x0, pD, -8);
                }
            { xtfloatx2 dummy;  XT_LSX2XC(dummy, pD, 8); }
            acc0 = XT_ADD_SX2(acc0, acc1);
            XT_SSX2IP(acc0, pZ, 8);
            NASSERT_ALIGN(pD, 8);
            XT_SSX2XC(s0, pD, 8);
    }
    state->p = (complex_float*)pD;
} /* cxfirf_process() */

#endif
