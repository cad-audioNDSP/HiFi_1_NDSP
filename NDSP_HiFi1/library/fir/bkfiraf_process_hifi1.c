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
  These functions implement FIR filter described in previous chapter with no 
  limitation on size of data block, alignment and length of impulse response 
  for the cost of performance.
  NOTE: 
  User application is not responsible for management of delay lines

  Precision: 
  16x16    16-bit data, 16-bit coefficients, 16-bit outputs
  24x24    24-bit data, 24-bit coefficients, 24-bit outputs
  32x16    32-bit data, 16-bit coefficients, 32-bit outputs
  f        floating point
  Input:
  x[N]      - input samples, Q31, floating point
  h[M]      - filter coefficients in normal order, Q31, Q15, floating point
  N         - length of sample block
  M         - length of filter
  Output:
  y[N]      - input samples, Q31, floating point 

  Restrictions:
  x,y should not be overlapping
-------------------------------------------------------------------------*/
/* Portable data types. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
/* Common utility and macros declarations. */
#include "common.h"
#include "bkfiraf.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, bkfiraf_process, (bkfiraf_handle_t _bkfir, float32_t * restrict  y, const float32_t * restrict  x, int N))

#else
/* process block of samples */
#if 1 
/*The following code is implemented to improve MAC throughput */
void bkfiraf_process(bkfiraf_handle_t _bkfir,
    float32_t * restrict  y,
    const float32_t * restrict  x, int N)
{
    const xtfloatx2* restrict pH;
    const xtfloatx2* restrict pX;
          xtfloatx2* restrict pY;
    const xtfloatx2  * pD01;
    const xtfloatx2  * pD12;
    xtfloatx2* p;
    ae_valign  ay,ax;
    int n,m,M;
    const float32_t* h;
    bkfiraf_t* bkfir;
    NASSERT(_bkfir);
    bkfir=(bkfiraf_t*)_bkfir;
    NASSERT(bkfir->h);
    NASSERT(bkfir->d);
    NASSERT(bkfir->p);
    NASSERT_ALIGN(bkfir->h,8);
    NASSERT_ALIGN(bkfir->d,8);
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
    pY=(xtfloatx2*)y;
    p=(xtfloatx2*)(bkfir->p);
    ax=AE_LASX2PP(pX);
    ay=AE_ZALIGN64();
    for (n=0; n<(N&~3); n+=4)
    {
        xtfloatx2  q0, q1, q2, q3,q01h,q01l,q23h,q23l,q0r,q1r;
        xtfloatx2  t,H01,H23,X01,X12,X23,X34,XN0,XN1;
        ae_valign  ad01,ad12;

        pH=(xtfloatx2* )h;
        pD01=(const xtfloatx2*)(p);
        pD12=(const xtfloatx2*)(((int32_t*)p)+1);
        XT_LASX2NEGPC(ad01,pD01);
        XT_LASX2NEGPC(ad12,pD12);
        XT_LASX2RIC(X12,ad12,pD12);
        XT_LASX2RIC(t,ad01,pD01);
        XT_LASX2IP(XN0,ax,pX);
        XT_LASX2IP(XN1,ax,pX);
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

        XT_SSXC(XT_SEL32_LH_SX2(XN0,XN0),(xtfloat*)p,4);
        XT_SSXC(XN0,(xtfloat*)p,4);
        XT_SSXC(XT_SEL32_LH_SX2(XN1,XN1),(xtfloat*)p,4);
        XT_SSXC(XN1,(xtfloat*)p,4);

        XT_SASX2IP( q0r, ay, pY );
        XT_SASX2IP( q1r, ay, pY );

    }
    AE_SA64POS_FP(ay,pY);
    /* process tail (1...3 samples) */
    N&=3;
    if(N)
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

        XT_LASX2IP(XN0,ax,pX);
        XT_LASX2IP(XN1,ax,pX);

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

        XT_SSXC(XT_SEL32_LH_SX2(XN0,XN0),(xtfloat*)p,4);
        XT_SSIP(XT_SEL32_LH_SX2(q0r,q0r),(xtfloat*)pY,4);

        if(N>1)
        {
          XT_SSXC(XN0,(xtfloat*)p,4);
          XT_SSIP(q0r,(xtfloat*)pY,4);
        }
        if(N>2)
        {
          XT_SSXC(XT_SEL32_LH_SX2(XN1,XN1),(xtfloat*)p,4);
          XT_SSIP(XT_SEL32_LH_SX2(q1r,q1r),(xtfloat*)pY,4);
        }
    }
    bkfir->p=(float32_t*)p;
}
#else
void bkfiraf_process(bkfiraf_handle_t _bkfir,
    float32_t * restrict  y,
    const float32_t * restrict  x, int N)
{
    bkfiraf_t *state;
    int n, m, M, _N;
    xtfloat tmp;
    xtfloatx2 h0;
    xtfloatx2 x0, x1, x2, x3;
    xtfloat   a, s0, s1, s2, s3, s4, s5, s6, s7;
    xtfloatx2 acc0, acc1, acc2, acc3;
    ae_valign x_align, z_align;
    const xtfloatx2*  restrict pX = (const xtfloatx2*)x;
    const xtfloatx2*  restrict px = (const xtfloatx2*)x;
    const xtfloatx2* restrict pD;
    const xtfloat*   restrict pH;
    xtfloatx2*          pZ = (xtfloatx2*)y;
    NASSERT(_bkfir);
    state = (bkfiraf_t*)_bkfir;
    NASSERT(state->h);
    NASSERT(state->d);
    NASSERT(state->p);
    NASSERT_ALIGN(state->h, 8);
    NASSERT_ALIGN(state->d, 8);
    NASSERT((state->M % 4) == 0);
    NASSERT(x);
    NASSERT(y);
    if (N <= 0) return;
    pD = (const xtfloatx2*)state->p;
    pH = (const xtfloat*)state->h;
    M = state->M;
    NASSERT(N > 0);
    NASSERT(M > 0);
#define P 8    /* unroll factor for the first loop */

    _N = N & (~(P - 1));
    WUR_AE_CBEGIN0((uintptr_t)(state->d));
    WUR_AE_CEND0((uintptr_t)(state->d + M));
    x_align = AE_LA64_PP(pX);
    z_align = AE_ZALIGN64();
    for (n = 0; n < _N; n += P)
    {
        XT_LASX2IP(x0, x_align, pX);
        XT_LASX2IP(x1, x_align, pX);
        XT_LASX2IP(x2, x_align, pX);
        XT_LASX2IP(x3, x_align, pX);

        acc0 =
            acc1 =
            acc2 =
            acc3 = (xtfloatx2)0.0f;
        XT_LSXC(tmp, castxcc(const xtfloat, pD), -4);
        __Pragma("loop_count min=1,factor=4")
            for (m = 0; m < M; m++)
            {
                /* SIMD multiply */
                h0 = pH[m];
                XT_MADD_SX2(acc0, h0, x0);
                XT_MADD_SX2(acc1, h0, x1);
                XT_MADD_SX2(acc2, h0, x2);
                XT_MADD_SX2(acc3, h0, x3);

                /* select and load next sample */
                x3 = XT_SEL32_LH_SX2(x2, x3);
                x2 = XT_SEL32_LH_SX2(x1, x2);
                x1 = XT_SEL32_LH_SX2(x0, x1);
                XT_LSXC(tmp, castxcc(const xtfloat, pD), -4);
                x0 = XT_SEL32_LH_SX2((xtfloatx2)tmp, x0);
            }
        XT_LSXC(tmp, castxcc(const xtfloat, pD), 4);
        XT_SASX2IP(acc0, z_align, pZ);
        XT_SASX2IP(acc1, z_align, pZ);
        XT_SASX2IP(acc2, z_align, pZ);
        XT_SASX2IP(acc3, z_align, pZ);

        XT_LSIP(s0, castxcc(const xtfloat, px), 4);
        XT_LSIP(s1, castxcc(const xtfloat, px), 4);
        XT_LSIP(s2, castxcc(const xtfloat, px), 4);
        XT_LSIP(s3, castxcc(const xtfloat, px), 4);
        XT_LSIP(s4, castxcc(const xtfloat, px), 4);
        XT_LSIP(s5, castxcc(const xtfloat, px), 4);
        XT_LSIP(s6, castxcc(const xtfloat, px), 4);
        XT_LSIP(s7, castxcc(const xtfloat, px), 4);

        XT_SSXC(s0, castxcc(xtfloat, pD), 4);
        XT_SSXC(s1, castxcc(xtfloat, pD), 4);
        XT_SSXC(s2, castxcc(xtfloat, pD), 4);
        XT_SSXC(s3, castxcc(xtfloat, pD), 4);
        XT_SSXC(s4, castxcc(xtfloat, pD), 4);
        XT_SSXC(s5, castxcc(xtfloat, pD), 4);
        XT_SSXC(s6, castxcc(xtfloat, pD), 4);
        XT_SSXC(s7, castxcc(xtfloat, pD), 4);
    }
    AE_SA64POS_FP(z_align, pZ);
    for (; n < N; n++)
    {
        XT_LSIP(s0, castxcc(const xtfloat, pX), 4);
        a = (xtfloatx2)0.0f;
        XT_LSXC(tmp, castxcc(const xtfloat, pD), -4);
        __Pragma("loop_count min=1,factor=4")
            for (m = 0; m < M; m++)
            {
                s1 = pH[m];
                XT_MADD_S(a, s1, s0);
                XT_LSXC(s0, castxcc(const xtfloat, pD), -4);
            }
        XT_LSXC(tmp, castxcc(const xtfloat, pD), 4);
        XT_SSIP(a, castxcc(xtfloat, pZ), 4);

        XT_LSIP(s0, castxcc(const xtfloat, px), 4);
        XT_SSXC(s0, castxcc(xtfloat, pD), 4);
    }
    state->p = (float32_t*)pD;
} /* bkfiraf_process() */
#endif
#endif /*HAVE_VFPU*/

