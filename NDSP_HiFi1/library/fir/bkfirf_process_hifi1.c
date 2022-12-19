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
    Real block FIR filter, floating point
    C code optimized for HiFi1
*/

/*-------------------------------------------------------------------------
  Real FIR filter.
  Computes a real FIR filter (direct-form) using IR stored in vector h. The 
  real data input is stored in vector x. The filter output result is stored 
  in vector y. The filter calculates N output samples using M coefficients 
  and requires last M-1 samples in the delay line.
  NOTE: 
   user application is not responsible for management of delay lines


  Precision: 
  16x16  16-bit data, 16-bit coefficients, 16-bit outputs
  24x24  24-bit data, 24-bit coefficients, 24-bit outputs
  24x24p use 24-bit data packing for internal delay line buffer
         and internal coefficients storage
  32x16    32-bit data, 16-bit coefficients, 32-bit outputs
  32x32    32-bit data, 32-bit coefficients, 32-bit outputs
  f        floating point

  Input:
  x[N]      - input samples, Q31, Q15, floating point
  h[M]      - filter coefficients in normal order, Q31, Q15, floating point
  N         - length of sample block, should be a multiple of 4
  M         - length of filter, should be a multiple of 4
  Output:
  y[N]      - input samples, Q31, Q15, floating point

  Restrictions:
  x,y should not be overlapping
  x,h - aligned on a 8-bytes boundary
  N,M - multiples of 4 
-------------------------------------------------------------------------*/
/* Portable data types. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
/* Common utility and macros declarations. */
#include "common.h"
#include "bkfirf.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, bkfirf_process, (bkfirf_handle_t _bkfir,
                                  float32_t * restrict  y,
                                  const float32_t * restrict  x, int N))
#else
#if 1 
/*The following code is implemented to improve MAC throughput*/
void bkfirf_process( bkfirf_handle_t _bkfir, 
                         float32_t * restrict  y,
                   const float32_t * restrict  x, int N )
{
    const xtfloatx2* restrict pH;
    const xtfloatx2* restrict pX;
    xtfloatx2* restrict pY;
    const xtfloatx2  * pD01;
    const xtfloatx2  * pD12;
    xtfloatx2* p;
    ae_valign  ay;
    int n,m,M;
    const float32_t* h;
    bkfirf_t* bkfir;
    NASSERT(_bkfir);
    bkfir=(bkfirf_t*)_bkfir;
    NASSERT(bkfir->h);
    NASSERT(bkfir->d);
    NASSERT(bkfir->p);
    NASSERT_ALIGN(bkfir->h,8);
    NASSERT_ALIGN(bkfir->d,8);
    NASSERT_ALIGN(bkfir->p,8);
    NASSERT(N%4==0);
    NASSERT_ALIGN(x,8);
    NASSERT((bkfir->M%4)==0);
    NASSERT(x);
    NASSERT(y);
    if(N<=0) return;
    M=bkfir->M;
    NASSERT(N>0);
    NASSERT(M>0);
    h=bkfir->h;
    pX=(const xtfloatx2*)x;
    WUR_AE_CBEGIN0( (uintptr_t)( bkfir->d            ) );
    WUR_AE_CEND0  ( (uintptr_t)( bkfir->d + bkfir->M ) );
    ay=AE_ZALIGN64();
    pY=(xtfloatx2*)y;
    p=(xtfloatx2*)(bkfir->p);

    for (n=0; n<N; n+=4)
    {
        xtfloatx2   q0, q1, q2, q3,q01h,q01l,q23h,q23l,q0r,q1r;
        xtfloatx2   t,H01,H23,X01,X12,X23,X34,XN0,XN1;
        ae_valign   ad01,ad12;

        pH=(xtfloatx2* )h;
        pD01=(const xtfloatx2*)(p);
        pD12=(const xtfloatx2*)(((int32_t*)p)+1);
        XT_LASX2NEGPC(ad01,pD01);
        XT_LASX2NEGPC(ad12,pD12);

        XT_LASX2RIC (X12,ad12,pD12);
        XT_LASX2RIC (t,ad01,pD01);

        XT_LSX2IP(XN0,pX,8);
        XT_LSX2IP(XN1,pX,8);

        X12=XT_SEL32_LH_SX2(XN0,XN0);
        X34=XT_SEL32_LH_SX2(XN1,XN1);
        X01=XT_SEL32_HL_SX2(XN0,t);
        X23=XT_SEL32_HL_SX2(XN1,XN0);

        XT_LSX2IP(H01,pH,8);
        q0=q1=0.0f;
        q2=XT_MUL_SX2(H01,X23);
        q3=XT_MUL_SX2(H01,X34);
        __Pragma("loop_count min=1")
        for (m=0; m<M-2; m+=2)
        {
            XT_LSX2IP(H23,pH,8);
            XT_MADD_SX2(q0,H01,X01);
            XT_MADD_SX2(q1,H01,X12);
            XT_MADD_SX2(q2,H23,X01);
            XT_MADD_SX2(q3,H23,X12);
            H01=H23;
            XT_LASX2RIC(X01,ad01,pD01);
            XT_LASX2RIC(X12,ad12,pD12);
        }
        XT_MADD_SX2(q0,H01,X01);
        XT_MADD_SX2(q1,H01,X12);
        
        q01h=XT_SEL32_HH_SX2(q0,q1);
        q01l=XT_SEL32_LL_SX2(q0,q1);
        q23h=XT_SEL32_HH_SX2(q2,q3);
        q23l=XT_SEL32_LL_SX2(q2,q3);

        q0r=XT_ADD_SX2(q01h,q01l);
        q1r=XT_ADD_SX2(q23h,q23l);

        XT_SSX2XC(XN0,p,8);
        XT_SSX2XC(XN1,p,8);

        XT_SASX2IP( q0r, ay, pY );
        XT_SASX2IP( q1r, ay, pY );

    }
    AE_SA64POS_FP(ay,pY);
    bkfir->p=(float32_t*)p;
}
#else
/* process block of samples */
void bkfirf_process(bkfirf_handle_t _bkfir,
    float32_t * restrict  y,
    const float32_t * restrict  x, int N)
{
    ae_valign yalign,h_align;
    bkfirf_t *state;
    int n, m, M, _N;
    xtfloatx2 h0,h1;
    xtfloatx2 x0, x1, x2, x3;
    xtfloatx2 s0, s1, s2, s3;
    xtfloatx2 acc0, acc1, acc2, acc3;
    const xtfloatx2* restrict pX = (const xtfloatx2*)x;
    xtfloatx2* restrict pD;
    const xtfloatx2*   restrict pH;
    xtfloatx2*          pZ = (xtfloatx2*)y;
    NASSERT(_bkfir);
    state = (bkfirf_t*)_bkfir;
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
    pD = (xtfloatx2*)state->p;
    pH = (const xtfloatx2*)state->h;

    M = state->M;
    NASSERT(N > 0);
    NASSERT(M > 0);
#define P1 8    /* unroll factor for the first loop */
#define P2 2    /* unroll factor for the last loop */

    _N = N & (~(P1 - 1));
    WUR_AE_CBEGIN0((uintptr_t)(state->d));
    WUR_AE_CEND0((uintptr_t)(state->d + M));
    yalign = AE_ZALIGN64();
    for (n = 0; n < _N; n += P1)
    {
        xtfloatx2 t0, t1, t2;
        xtfloatx2 tmp0;

        XT_LSX2IP(x0, pX, 8);
        XT_LSX2IP(x1, pX, 8);
        XT_LSX2IP(x2, pX, 8);
        XT_LSX2IP(x3, pX, 8);
        s0 = x0;
        s1 = x1;
        s2 = x2;
        s3 = x3;

        pH = (const xtfloatx2*)state->h;
        h_align=AE_LA64_PP(pH);

        { xtfloatx2 dummy;  XT_LSX2XC(dummy, pD, -8); }

            XT_LASX2IP(h0,h_align,pH);
            acc0=XT_MULMUX_S(h0,x0,0);
            acc1=XT_MULMUX_S(h0,x1,0);
            acc2=XT_MULMUX_S(h0,x2,0);
            acc3=XT_MULMUX_S(h0,x3,0);
//        __Pragma("loop_count min=2,factor=2")
            for (m = 0; m < (M-4); m += 4)
            {
                /* SIMD multiply */
              /* select and load next sample */
                x3 = XT_SEL32_LH_SX2(x2, x3);
                t2 = XT_SEL32_LH_SX2(x1, x2);
                t1 = XT_SEL32_LH_SX2(x0, x1);
                XT_LSX2XC(tmp0, pD, -8);
                t0 = XT_SEL32_LH_SX2(tmp0, x0);
                /* SIMD multiply */
                h0 = XT_SEL32_LH_SX2(h0, h0);
                XT_MADDMUX_S(acc0,h0,t0,0);
                XT_MADDMUX_S(acc1,h0,t1,0);
                XT_MADDMUX_S(acc2,h0,t2,0);
                XT_MADDMUX_S(acc3,h0,x3,0);
                /* select and load next sample */
                x3 = x2;
                x2 = x1;
                x1 = x0;
                x0 = tmp0;

                /* SIMD multiply */
                XT_LASX2IP(h1,h_align,pH);
                XT_MADDMUX_S(acc0,h1,x0,0);
                XT_MADDMUX_S(acc1,h1,x1,0);
                XT_MADDMUX_S(acc2,h1,x2,0);
                XT_MADDMUX_S(acc3,h1,x3,0);

                /* select and load next sample */
                x3 = XT_SEL32_LH_SX2(x2, x3);
                t2 = XT_SEL32_LH_SX2(x1, x2);
                t1 = XT_SEL32_LH_SX2(x0, x1);
                XT_LSX2XC(tmp0, pD, -8);
                t0 = XT_SEL32_LH_SX2(tmp0, x0);

                /* SIMD multiply */
                h1 = XT_SEL32_LH_SX2(h1, h1);
                XT_MADDMUX_S(acc0,h1,t0,0);
                XT_MADDMUX_S(acc1,h1,t1,0);
                XT_MADDMUX_S(acc2,h1,t2,0);
                XT_MADDMUX_S(acc3,h1,x3,0);
                /* select and load next sample */
                x3 = x2;
                x2 = x1;
                x1 = x0;
                x0 = tmp0;

                XT_LASX2IP(h0,h_align,pH);
                XT_MADDMUX_S(acc0,h0,x0,0);
                XT_MADDMUX_S(acc1,h0,x1,0);
                XT_MADDMUX_S(acc2,h0,x2,0);
                XT_MADDMUX_S(acc3,h0,x3,0);
      }
                x3 = XT_SEL32_LH_SX2(x2, x3);
                t2 = XT_SEL32_LH_SX2(x1, x2);
                t1 = XT_SEL32_LH_SX2(x0, x1);
                XT_LSX2XC(tmp0, pD, -8);
                t0 = XT_SEL32_LH_SX2(tmp0, x0);

                /* SIMD multiply */
                h0 = XT_SEL32_LH_SX2(h0, h0);
                XT_MADDMUX_S(acc0,h0,t0,0);
                XT_MADDMUX_S(acc1,h0,t1,0);
                XT_MADDMUX_S(acc2,h0,t2,0);
                XT_MADDMUX_S(acc3,h0,x3,0);
                /* select and load next sample */
                x3 = x2;
                x2 = x1;
                x1 = x0;
                x0 = tmp0;

                /* SIMD multiply */
                XT_LASX2IP(h1,h_align,pH);
                XT_MADDMUX_S(acc0,h1,x0,0);
                XT_MADDMUX_S(acc1,h1,x1,0);
                XT_MADDMUX_S(acc2,h1,x2,0);
                XT_MADDMUX_S(acc3,h1,x3,0);

                /* select and load next sample */
                x3 = XT_SEL32_LH_SX2(x2, x3);
                t2 = XT_SEL32_LH_SX2(x1, x2);
                t1 = XT_SEL32_LH_SX2(x0, x1);
                XT_LSX2XC(tmp0, pD, -8);
                t0 = XT_SEL32_LH_SX2(tmp0, x0);

                /* SIMD multiply */
                h1 = XT_SEL32_LH_SX2(h1, h1);
                XT_MADDMUX_S(acc0,h1,t0,0);
                XT_MADDMUX_S(acc1,h1,t1,0);
                XT_MADDMUX_S(acc2,h1,t2,0);
                XT_MADDMUX_S(acc3,h1,x3,0);
                /* select and load next sample */
                x3 = x2;
                x2 = x1;
                x1 = x0;
                x0 = tmp0;

            { xtfloatx2 dummy;  XT_LSX2XC(dummy, pD, 8); }

            XT_SASX2IP(acc0, yalign, pZ);
            XT_SASX2IP(acc1, yalign, pZ);
            XT_SASX2IP(acc2, yalign, pZ);
            XT_SASX2IP(acc3, yalign, pZ);

            NASSERT_ALIGN(pD, 8);
            XT_SSX2XC(s0, pD, 8);
            XT_SSX2XC(s1, pD, 8);
            XT_SSX2XC(s2, pD, 8);
            XT_SSX2XC(s3, pD, 8);
    }

    for (; n < N; n += P2)
    {
        xtfloatx2 tmp0;
         pH = (const xtfloatx2*)state->h;
        h_align=AE_LA64_PP(pH);

        XT_LSX2IP(x0, pX, 8);
        s0 = x0;
        { xtfloatx2 dummy;  XT_LSX2XC(dummy, pD, -8); }

        XT_LASX2IP(h0,h_align,pH);
        XT_LASX2IP(h1,h_align,pH);

        acc0=XT_MULMUX_S(h0,x0,0);
        h0 = XT_SEL32_LH_SX2(h0, h0);
        XT_LSX2XC(tmp0, pD, -8);
        x0 = XT_SEL32_LH_SX2(tmp0, x0);
        XT_MADDMUX_S(acc0,h0,x0,0);
        /* select and load next sample */
        x0 = tmp0;
         /* SIMD multiply */
        acc1=XT_MULMUX_S(h1,x0,0);
       // __Pragma("loop_count min=2,factor=2")
            for (m = 0; m < (M-4); m += 4)
            {  
              
                h1 = XT_SEL32_LH_SX2(h1, h1);
                /* select and load next sample */
                XT_LSX2XC(tmp0, pD, -8);
                x0 = XT_SEL32_LH_SX2(tmp0, x0);
                XT_MADDMUX_S(acc1,h1,x0,0);
                /* select and load next sample */
                x0 = tmp0;
                XT_LASX2IP(h0,h_align,pH);
                XT_LASX2IP(h1,h_align,pH);

                XT_MADDMUX_S(acc0,h0,x0,0);
                h0 = XT_SEL32_LH_SX2(h0, h0);
                XT_LSX2XC(tmp0, pD, -8);
                x0 = XT_SEL32_LH_SX2(tmp0, x0);
                XT_MADDMUX_S(acc0,h0,x0,0);
                x0 = tmp0;
                XT_MADDMUX_S(acc1,h1,x0,0);

            }

            h1 = XT_SEL32_LH_SX2(h1, h1);
            /* select and load next sample */
            XT_LSX2XC(tmp0, pD, -8);
            x0 = XT_SEL32_LH_SX2(tmp0, x0);
            XT_MADDMUX_S(acc1,h1,x0,0);
            x0 = tmp0;
            acc0=XT_ADD_SX2(acc0, acc1);
        { xtfloatx2 dummy;  XT_LSX2XC(dummy, pD, 8); }
        XT_SASX2IP(acc0, yalign, pZ);
        NASSERT_ALIGN(pD, 8);
        XT_SSX2XC(s0, pD, 8);
    }
    state->p = (float32_t*)pD;
    AE_SA64POS_FP(yalign, pZ);
} /* bkfirf_process() */
#endif
#endif /*HAVE_VFPU*/

