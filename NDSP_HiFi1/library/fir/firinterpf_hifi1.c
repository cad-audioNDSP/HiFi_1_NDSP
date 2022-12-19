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
    Interpolating block real FIR filter, floating point
    C code optimized for HiFi1
*/

/*-------------------------------------------------------------------------
  Interpolating Block Real/Complex FIR Filter
  Computes a real FIR filter (direct-form) with interpolation using IR stored 
  in vector h. The real data input is stored in vector x. The filter output 
  result is stored in vector y. The filter calculates N*D output samples 
  using M*D coefficients and requires last N+M*D-1 samples on the delay line.
  NOTE:
  user application is not responsible for management of delay lines

  Precision: 
  16x16     16-bit real data, 16-bit coefficients, 16-bit real outputs 
  16x16     16-bit complex data, 16-bit coefficients, 16-bit complex outputs
  24x24     24-bit real data, 24-bit coefficients, 24-bit real outputs 
  32x16     32-bit real data, 16-bit coefficients, 32-bit real outputs 
  32x32     32-bit real data, 32-bit coefficients, 32-bit real outputs 
  f         floating point

  Input:
  h[M*D]        filter coefficients; h[0] is to be multiplied with the 
                newest sample,Q31, Q15, floating point
  D             interpolation ratio
  N             length of input sample block
  M             length of subfilter. Total length of filter is M*D
  x[N]          input samples,Q15, Q31 or floating point
  Output:
  y[N*D]        output samples, Q15, Q31 or floating point

  Restrictions:
  x,h,y should not overlap
  x,h - aligned on an 8-bytes boundary
  N   - multiple of 8
  M   - multiple of 4
  D should be >1

  PERFORMANCE NOTE:
  for optimum performance follow rules:
  D   - 2, 3 or 4

-------------------------------------------------------------------------*/
/* Portable data types. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
/* Common utility and macros declarations. */
#include "common.h"
#if !(HAVE_VFPU)
DISCARD_FUN(size_t, firinterpf_alloc, (int D, int M))
DISCARD_FUN(firinterpf_handle_t, firinterpf_init, (void * objmem, int D, int M, const float32_t * restrict h))
DISCARD_FUN(void, firinterpf_process, (firinterpf_handle_t _firinterp, float32_t * restrict y, const float32_t * restrict x, int N))

#else
#if 0
#include "fir_interpf_2x.h"
#include "fir_interpf_3x.h"
#include "fir_interpf_4x.h"
#include "fir_interpf_Dx.h"

/* Interpolator instance structure. */
typedef struct tag_firinterpf_t
{
  int               M; /* Filter length                   */
  int               D; /* Interpolation/decimation factor */
  const float32_t * h; /* Filter coefficients (M*D)       */
        float32_t * d; /* Delay line of length (M*D)      */
        float32_t * p; /* Pointer into the delay line     */

} firinterpf_t;

/* Calculate the memory block size for an interpolator with given 
 * attributes. */
size_t firinterpf_alloc( int D, int M )
{
  NASSERT( D > 1 && M > 0 );
  NASSERT( M%4==0);
  return M*(D+1)*sizeof(float32_t) + sizeof(firinterpf_t) + 7;
} // firinterpf_alloc()

/* Initialize the interpolator structure. The delay line is zeroed. */
firinterpf_handle_t firinterpf_init ( void * objmem, 
                                      int D, int M, 
                                      const float32_t * restrict h 
                                    )
{
    firinterpf_t* firinterp;
    void *           ptr;
    float32_t * restrict pd;
    float32_t * restrict ph;
    int m,d;
    NASSERT( D > 1 && M > 0 );
    NASSERT( M%4==0);
    NASSERT(h);
    NASSERT(objmem);
    NASSERT_ALIGN(h,8);
    /* partition the memblock */
    ptr     = objmem;
    ph = (float32_t*)((((uintptr_t)ptr)+7)&~7);
    pd = ph+M*D;
    firinterp=(firinterpf_t*)(pd+M);
    firinterp->M   = M;
    firinterp->D   = D;
    firinterp->h = ph;
    firinterp->d = firinterp->p = pd;
    /* copy/rearrange coefficients and clean up delay line */
    for (m=0; m<M; m++) 
    for (d=0; d<D; d++) 
        ph[d*M+m]=h[m*D+d]*D;
    for (m=0; m<M; m++) pd[m]=0.f;
    return firinterp;

} // firinterpf_init()

/* Put a chunk of input signal into the delay line and compute the filter
 * response. */
void firinterpf_process( firinterpf_handle_t _firinterp, 
                             float32_t * restrict      y,
                       const float32_t * restrict      x, int N )
{
    typedef float32_t* (*fn_interp)(float32_t *, float32_t *, float32_t *, const float32_t *, const float32_t *, int, int);
    static const fn_interp fn[3] = { &fir_interpf_2x, &fir_interpf_3x, &fir_interpf_4x };

    firinterpf_t *state;
    int M, D;
    float32_t* d;
    float32_t* p;
    const float32_t* h;

    NASSERT(_firinterp);
    state=(firinterpf_t *)_firinterp;
    NASSERT(x);
    NASSERT(y);
    NASSERT(state);
    NASSERT_ALIGN(state->d,8);
    M = state->M;
    D = state->D;
    if(N<=0) return;
    NASSERT(N>0);
    NASSERT(M>0);
    NASSERT(D>1);
    NASSERT(M%4==0);
    h = state->h;
    p = state->p;
    d = state->d;
    if (D <= 4)
    {
    p = fn[D - 2](y, d, p, h, x, M, N);
    }
    else
    {
    p = fir_interpf_Dx(y, d, p, h, x, M, D, N);
    }
    state->p = p;
} // firinterpf_process()
#else
/*The following code is implemented to improve MAC throughput*/
/* Interpolator instance structure. */
typedef struct tag_firinterpf_t
{
  int               M; /* Filter length                   */
  int               D; /* Interpolation/decimation factor */
  const float32_t * h; /* Filter coefficients (M*D)       */
        float32_t * d; /* Delay line of length (M*D)      */
        float32_t * p; /* Pointer into the delay line     */

} firinterpf_t;

/* Calculate the memory block size for an interpolator with given 
 * attributes. */
size_t firinterpf_alloc( int D, int M )
{
  NASSERT( D > 1 && M > 0 );
  NASSERT( M%4==0);
  return M*(D+1)*sizeof(float32_t) + sizeof(firinterpf_t) + 7;
} // firinterpf_alloc()

/* Initialize the interpolator structure. The delay line is zeroed. */
firinterpf_handle_t firinterpf_init ( void * objmem, 
                                      int D, int M, 
                                      const float32_t * restrict h 
                                    )
{
    firinterpf_t* firinterp;
    void *           ptr;
    float32_t * restrict pd;
    float32_t * restrict ph;
    int m,d;
    NASSERT( D > 1 && M > 0 );
    NASSERT( M%4==0);
    NASSERT(h);
    NASSERT(objmem);
    NASSERT_ALIGN(h,8);
    /* partition the memblock */
    ptr     = objmem;
    ph = (float32_t*)((((uintptr_t)ptr)+7)&~7);
    pd = ph+M*D;
    firinterp=(firinterpf_t*)(pd+M);
    firinterp->M   = M;
    firinterp->D   = D;
    firinterp->h = ph;
    firinterp->d = firinterp->p = pd;
    /* copy/rearrange coefficients and clean up delay line */
    for (m=0; m<M; m++) 
    for (d=0; d<D; d++) 
        ph[d*M+m]=h[m*D+d]*D;
    for (m=0; m<M; m++) pd[m]=0.f;
    return firinterp;

} // firinterpf_init()

/* process block of input samples */
void firinterpf_process( firinterpf_handle_t _firinterp, 
                             float32_t * restrict      y,
                       const float32_t * restrict      x, int N )
{
    const xtfloatx2* restrict pX;
    const xtfloatx2* restrict pX0;
    const xtfloatx2* restrict pX1;
    const xtfloatx2* restrict pH;
    xtfloatx2* pWr;
    firinterpf_t *state;
    int n, m, M, D, j;
    const float32_t* h;
    xtfloatx2* pY;
    ae_valign ay;
    xtfloatx2 X01,X23,X12,X34,H01,H23,t;
    ae_valign aX0,aX1;
    xtfloatx2 q0,q1,q2,q3;

    NASSERT(_firinterp);
    state=(firinterpf_t *)_firinterp;
    NASSERT(x);
    NASSERT(y);
    NASSERT(state);
    NASSERT_ALIGN(state->d,8);
    M = state->M;
    D = state->D;
    if(N<=0) return;
    NASSERT(N>0);
    NASSERT(M>0);
    NASSERT(D>1);
    NASSERT(M%2==0);
    h = state->h;
    ay=AE_ZALIGN64();
    pWr=(xtfloatx2*)state->p;
    pX =(const xtfloatx2*)x;
    /* setup circular buffer boundaries */
    WUR_AE_CBEGIN0( (uintptr_t)( state->d     ) );
    WUR_AE_CEND0  ( (uintptr_t)( state->d + M ) );
    /* process by 4 samples */
    for (n = 0; n<N; n += 4)
    {
        pH =(xtfloatx2*)h;
        pY =(xtfloatx2*)y;
        for (j = 0; j<D; j++)
        {
            /* filtering loop */
            pX0=(const xtfloatx2*)(pWr);
            pX1=(const xtfloatx2*)(((float32_t*)pWr)+1);
            XT_LASX2NEGPC(aX0,pX0);
            XT_LASX2NEGPC(aX1,pX1);
            XT_LASX2RIC(X12,aX1,pX1);
            XT_LASX2RIC(t  ,aX0,pX0);
            X12=XT_LSX2I (pX,0*sizeof(float32_t));
            X34=XT_LSX2I (pX,2*sizeof(float32_t));
            X12=XT_SEL32_LH_SX2(X12,X12);
            X34=XT_SEL32_LH_SX2(X34,X34);
            X01=AE_SEL32_LL_SX2(X12,t);
            X23=AE_SEL32_LH_SX2(X34,X12);

            XT_LSX2IP(H01,pH,2*sizeof(float32_t));
            q0=q1=q2=q3=(xtfloatx2)0.f;
            XT_MADD_SX2(q2,H01,X23);
            XT_MADD_SX2(q3,H01,X34);
            __Pragma("loop_count min=1")
            for (m=0; m<M-2; m+=2)
            {
                XT_LSX2IP(H23,pH,2*sizeof(float32_t));
                XT_MADD_SX2(q0,H01,X01);
                XT_MADD_SX2(q1,H01,X12);
                XT_MADD_SX2(q2,H23,X01);
                XT_MADD_SX2(q3,H23,X12);
                H01=H23;
                XT_LASX2RIC(X01,aX0,pX0);
                XT_LASX2RIC(X12,aX1,pX1);
            }
            XT_MADD_SX2(q0,H01,X01);
            XT_MADD_SX2(q1,H01,X12);

            q0 = q0 + XT_SEL32_LH_SX2(q0, q0);
            q1 = q1 + XT_SEL32_LH_SX2(q1, q1);
            q2 = q2 + XT_SEL32_LH_SX2(q2, q2);
            q3 = q3 + XT_SEL32_LH_SX2(q3, q3);
            AE_SSXP( q0, castxcc(xtfloat,pY), 4*D );
            AE_SSXP( q1, castxcc(xtfloat,pY), 4*D );
            AE_SSXP( q2, castxcc(xtfloat,pY), 4*D );
            AE_SSXP( q3, castxcc(xtfloat,pY), (1-3*D)*4 );
        }
        /* update delay line */
        XT_LSX2IP(X01,pX,8); 
        XT_LSX2IP(X23,pX,8); 
        XT_SSX2XC(X01,pWr,sizeof(float32_t)*2);
        XT_SSX2XC(X23,pWr,sizeof(float32_t)*2);
        state->p =(float32_t*)pWr;
        y+=4*D;
    }
    state->p =(float32_t*)pWr;
} // firinterpf_process()
#endif
#endif

