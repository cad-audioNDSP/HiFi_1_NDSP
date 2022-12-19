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
    NatureDSP Signal Processing Library. FFT part
    Code optimized for HiFi1
    Floating point DCT 
*/

#include "NatureDSP_Signal.h"
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(int, dctf, (float32_t *  y, float32_t * x, int N))
  
#else
/*-------------------------------------------------------------------------
  Discrete Cosine Transform.
  These functions apply DCT (Type II, Type IV) to input.
    Scaling  : 
      +-----------------------+--------------------------------------+
      |      Function         |           Scaling options            |
      +-----------------------+--------------------------------------+
      |       dct_16x16       |  3 - fixed scaling before each stage |
      |       dct_24x24       |  3 - fixed scaling before each stage |
      |       dct_32x16       |  3 - fixed scaling before each stage |
      |       dct_32x32       |  3 - fixed scaling before each stage |
      |       dct4_24x24      |  3 - fixed scaling before each stage |
      |       dct4_32x16      |  3 - fixed scaling before each stage |
      |       dct4_32x32      |  3 - fixed scaling before each stage |
      +-----------------------+--------------------------------------+
  NOTES:
     1. DCT runs in-place algorithm so INPUT DATA WILL APPEAR DAMAGED after 
     the call.
     2. N - DCT size (depends on selected DCT handle)

  Precision: 
  16x16  16-bit input/outputs, 16-bit twiddles
  24x24  24-bit input/outputs, 24-bit twiddles
  32x16  32-bit input/outputs, 16-bit twiddles
  32x32  32-bit input/outputs, 32-bit twiddles
  f      floating point

  Input:
  x[N]        input signal
  h           DCT handle
  scalingOpt  scaling option (see table above) 
              not applicable to the floating point function
  Output:
  y[N]        transform output
  
  Returned value:
              total number of right shifts occurred during scaling 
              procedure 
  Restriction:
  x,y         should not overlap
  x,y         aligned on 8-bytes boundary
-------------------------------------------------------------------------*/
#define SZ_CF32 (sizeof(complex_float))
#define SZ_F32  (sizeof(float32_t))

/*
exp(i*pi/2*(0:N-1)/N), N=32
*/
static const union ufloat32uint32 ALIGN(8) dct_twd32[64]=
{
{0x3f800000UL},{0x00000000UL},{0x3f7fb10fUL},{0x3d48fb30UL},{0x3f7ec46dUL},{0x3dc8bd36UL},{0x3f7d3aacUL},{0x3e164083UL},
{0x3f7b14beUL},{0x3e47c5c2UL},{0x3f7853f8UL},{0x3e78cfccUL},{0x3f74fa0bUL},{0x3e94a031UL},{0x3f710908UL},{0x3eac7cd4UL},
{0x3f6c835eUL},{0x3ec3ef15UL},{0x3f676bd8UL},{0x3edae880UL},{0x3f61c598UL},{0x3ef15aeaUL},{0x3f5b941aUL},{0x3f039c3dUL},
{0x3f54db31UL},{0x3f0e39daUL},{0x3f4d9f02UL},{0x3f187fc0UL},{0x3f45e403UL},{0x3f226799UL},{0x3f3daef9UL},{0x3f2beb4aUL},
{0x3f3504f3UL},{0x3f3504f3UL},{0x3f2beb4aUL},{0x3f3daef9UL},{0x3f226799UL},{0x3f45e403UL},{0x3f187fc0UL},{0x3f4d9f02UL},
{0x3f0e39daUL},{0x3f54db31UL},{0x3f039c3dUL},{0x3f5b941aUL},{0x3ef15aeaUL},{0x3f61c598UL},{0x3edae880UL},{0x3f676bd8UL},
{0x3ec3ef15UL},{0x3f6c835eUL},{0x3eac7cd4UL},{0x3f710908UL},{0x3e94a031UL},{0x3f74fa0bUL},{0x3e78cfccUL},{0x3f7853f8UL},
{0x3e47c5c2UL},{0x3f7b14beUL},{0x3e164083UL},{0x3f7d3aacUL},{0x3dc8bd36UL},{0x3f7ec46dUL},{0x3d48fb30UL},{0x3f7fb10fUL}
};
/*
N=16;  
twd = exp(-2j*pi*[1;2;3]*(0:N/4-1)/N);
twd_ri = reshape([real(twd(:).');imag(twd(:).')],1,2*numel(twd));
*/
static const union ufloat32uint32 ALIGN(8) fft_twd16[24]=
{
{0x3f800000UL},{0x00000000UL},{0x3f800000UL},{0x00000000UL},{0x3f800000UL},{0x00000000UL},
{0x3f6c835eUL},{0xbec3ef15UL},{0x3f3504f3UL},{0xbf3504f3UL},{0x3ec3ef15UL},{0xbf6c835eUL},
{0x3f3504f3UL},{0xbf3504f3UL},{0x248d3132UL},{0xbf800000UL},{0xbf3504f3UL},{0xbf3504f3UL},
{0x3ec3ef15UL},{0xbf6c835eUL},{0xbf3504f3UL},{0xbf3504f3UL},{0xbf6c835eUL},{0x3ec3ef15UL}
};


/*
exp(i*pi/2*(0:N-1)/N), N=64
*/
static const union ufloat32uint32 ALIGN(8) dct_twd64[128]=
{
{0x3f800000UL},{0x00000000UL},{0x3f7fec43UL},{0x3cc90ab0UL},{0x3f7fb10fUL},{0x3d48fb30UL},{0x3f7f4e6dUL},{0x3d96a905UL},
{0x3f7ec46dUL},{0x3dc8bd36UL},{0x3f7e1324UL},{0x3dfab273UL},{0x3f7d3aacUL},{0x3e164083UL},{0x3f7c3b28UL},{0x3e2f10a2UL},
{0x3f7b14beUL},{0x3e47c5c2UL},{0x3f79c79dUL},{0x3e605c13UL},{0x3f7853f8UL},{0x3e78cfccUL},{0x3f76ba07UL},{0x3e888e93UL},
{0x3f74fa0bUL},{0x3e94a031UL},{0x3f731447UL},{0x3ea09ae5UL},{0x3f710908UL},{0x3eac7cd4UL},{0x3f6ed89eUL},{0x3eb8442aUL},
{0x3f6c835eUL},{0x3ec3ef15UL},{0x3f6a09a7UL},{0x3ecf7bcaUL},{0x3f676bd8UL},{0x3edae880UL},{0x3f64aa59UL},{0x3ee63375UL},
{0x3f61c598UL},{0x3ef15aeaUL},{0x3f5ebe05UL},{0x3efc5d27UL},{0x3f5b941aUL},{0x3f039c3dUL},{0x3f584853UL},{0x3f08f59bUL},
{0x3f54db31UL},{0x3f0e39daUL},{0x3f514d3dUL},{0x3f13682aUL},{0x3f4d9f02UL},{0x3f187fc0UL},{0x3f49d112UL},{0x3f1d7fd1UL},
{0x3f45e403UL},{0x3f226799UL},{0x3f41d870UL},{0x3f273656UL},{0x3f3daef9UL},{0x3f2beb4aUL},{0x3f396842UL},{0x3f3085bbUL},
{0x3f3504f3UL},{0x3f3504f3UL},{0x3f3085bbUL},{0x3f396842UL},{0x3f2beb4aUL},{0x3f3daef9UL},{0x3f273656UL},{0x3f41d870UL},
{0x3f226799UL},{0x3f45e403UL},{0x3f1d7fd1UL},{0x3f49d112UL},{0x3f187fc0UL},{0x3f4d9f02UL},{0x3f13682aUL},{0x3f514d3dUL},
{0x3f0e39daUL},{0x3f54db31UL},{0x3f08f59bUL},{0x3f584853UL},{0x3f039c3dUL},{0x3f5b941aUL},{0x3efc5d27UL},{0x3f5ebe05UL},
{0x3ef15aeaUL},{0x3f61c598UL},{0x3ee63375UL},{0x3f64aa59UL},{0x3edae880UL},{0x3f676bd8UL},{0x3ecf7bcaUL},{0x3f6a09a7UL},
{0x3ec3ef15UL},{0x3f6c835eUL},{0x3eb8442aUL},{0x3f6ed89eUL},{0x3eac7cd4UL},{0x3f710908UL},{0x3ea09ae5UL},{0x3f731447UL},
{0x3e94a031UL},{0x3f74fa0bUL},{0x3e888e93UL},{0x3f76ba07UL},{0x3e78cfccUL},{0x3f7853f8UL},{0x3e605c13UL},{0x3f79c79dUL},
{0x3e47c5c2UL},{0x3f7b14beUL},{0x3e2f10a2UL},{0x3f7c3b28UL},{0x3e164083UL},{0x3f7d3aacUL},{0x3dfab273UL},{0x3f7e1324UL},
{0x3dc8bd36UL},{0x3f7ec46dUL},{0x3d96a905UL},{0x3f7f4e6dUL},{0x3d48fb30UL},{0x3f7fb10fUL},{0x3cc90ab0UL},{0x3f7fec43UL}
};


/*
N=32;  
twd = exp(-2j*pi*[1;2;3]*(0:N/4-1)/N);
twd_ri = reshape([real(twd(:).');imag(twd(:).')],1,2*numel(twd));
*/
static const union ufloat32uint32 ALIGN(8) fft_twd32[48]=
{
{0x3f800000UL},{0x00000000UL},{0x3f800000UL},{0x00000000UL},{0x3f800000UL},{0x00000000UL},
{0x3f7b14beUL},{0xbe47c5c2UL},{0x3f6c835eUL},{0xbec3ef15UL},{0x3f54db31UL},{0xbf0e39daUL},
{0x3f6c835eUL},{0xbec3ef15UL},{0x3f3504f3UL},{0xbf3504f3UL},{0x3ec3ef15UL},{0xbf6c835eUL},
{0x3f54db31UL},{0xbf0e39daUL},{0x3ec3ef15UL},{0xbf6c835eUL},{0xbe47c5c2UL},{0xbf7b14beUL},
{0x3f3504f3UL},{0xbf3504f3UL},{0x248d3132UL},{0xbf800000UL},{0xbf3504f3UL},{0xbf3504f3UL},
{0x3f0e39daUL},{0xbf54db31UL},{0xbec3ef15UL},{0xbf6c835eUL},{0xbf7b14beUL},{0xbe47c5c2UL},
{0x3ec3ef15UL},{0xbf6c835eUL},{0xbf3504f3UL},{0xbf3504f3UL},{0xbf6c835eUL},{0x3ec3ef15UL},
{0x3e47c5c2UL},{0xbf7b14beUL},{0xbf6c835eUL},{0xbec3ef15UL},{0xbf0e39daUL},{0x3f54db31UL}
};

/* 1/sqrt(2.0) */
static const union ufloat32uint32 _invsqrt2f_ = { 0x3f3504f3 };
int dctf     ( float32_t * restrict y,float32_t * restrict x, int N)
/*
    Reference Matlab code:
    function y=dctf(x)
    N=numel(x);
    y(1:N/2)     =x(1:2:N);
    y(N:-1:N/2+1)=x(2:2:N);
    % take fft of N/2
    y=fft(y(1:2:N)+j*y(2:2:N));
    w=exp(i*pi/2*(0:N-1)/N);
    % DCT split algorithm
    Y0=y(1);
    T0=real(Y0)+imag(Y0);
    T1=real(Y0)-imag(Y0);
    z(1      )= real(T0);%*sqrt(2)/2;
    z(N/2+1  )= real(T1)*sqrt(2)/2;
    for k=2:N/4
        Y0=y(k);
        Y1=y(N/2+2-k);
        COSI=(w(4*(k-1)+1));
        W1=w(k);
        W2=w(N/2+2-k);
        S=Y0+Y1;
        D=Y0-Y1;
        T0=i*real(D)+imag(S);
        T1=i*imag(D)+real(S);
        Y0=  ( imag(T0)*imag(COSI)-real(T0)*real(COSI)) + ...
           i*( real(T0)*imag(COSI)+imag(T0)*real(COSI));
        T0=0.5*(T1-Y0);
        T1=0.5*(T1+Y0);
        z(k      )= real(T0)*real(W1)+imag(T0)*imag(W1);
        z(N+2-k  )= real(T0)*imag(W1)-imag(T0)*real(W1);
        z(N/2+2-k)= real(T1)*real(W2)-imag(T1)*imag(W2);
        z(N/2+k  )= real(T1)*imag(W2)+imag(T1)*real(W2);
    end
    W1=w(N/4+1);
    T0=y(N/4+1);
    z(N/4+1  )= real(T0)*real(W1)-imag(T0)*imag(W1);
    z(N+1-N/4)= real(T0)*imag(W1)+imag(T0)*real(W1);
    y=z;
*/
{
    const union ufloat32uint32 *rfft_split_twd; 
/*    const tdct2_twd *descr=(const tdct2_twd *)h;
    int N;*/
    const xtfloatx2 *restrict p0_twd;
    const xtfloatx2 *restrict p1_twd;
    const xtfloatx2 *restrict p2_twd;
    const xtfloatx2 *restrict p0_ld;
    const xtfloatx2 *restrict p1_ld;
          xtfloatx2 *restrict p0_stx2;
          xtfloatx2 *restrict p1_stx2;
          xtfloat   *restrict p0_st;
          xtfloat   *restrict p1_st;
          xtfloat   *restrict p2_st;
          xtfloat   *restrict p3_st;
    xtfloatx2 t0, t1, y0, y1,
              w1, w2, s, d, cosi, c05;
    xtfloat b0, b1, re, im, invsqrt2f;
    ae_int32x2 t32x2;
    int k, n;
    int N2, N4;

    NASSERT_ALIGN(x,8);
    NASSERT_ALIGN(y,8);
    NASSERT(x!=y);
/*    NASSERT(descr->magic==MAGIC_DCT2_F);
    NASSERT(descr->N==32 || descr->N==64);
    N=descr->N;
    rfft_split_twd=(const union ufloat32uint32 *)descr->rfft_split_twd;*/
    NASSERT(N==32 || N==64);
    N2 = N>>1;
    N4 = N2>>1;

    const union ufloat32uint32 * fft_twd = ((N == 32) ? fft_twd16 : fft_twd32) ; /* N is either 32 or 64 */
    rfft_split_twd = ((N == 32) ? dct_twd32 : dct_twd64) ; /* N is either 32 or 64 */

    /* permute inputs */
    p0_ld  = (const xtfloatx2 *)x;
    p0_stx2 = (xtfloatx2 *)y;
    p1_stx2 = (xtfloatx2 *)(y+N-2);
    __Pragma("loop_count min=1")
    for (n=0; n<N4; n++)
    {
      /* y[n]    =x[2*n+0] */
      /* y[N-1-n]=x[2*n+1] */
      XT_LSX2IP(t0, p0_ld, SZ_CF32);
      XT_LSX2IP(t1, p0_ld, SZ_CF32);
      y0 = XT_SEL32_HH_SX2(t0, t1);
      y1 = XT_SEL32_LL_SX2(t1, t0);
      XT_SSX2IP(y0, p0_stx2,       SZ_CF32);
      XT_SSX2XP(y1, p1_stx2, -(int)SZ_CF32);
    }

    /* compute fft(N/2) */
    /* set twiddle stride to 2 for N=32 or 1 for N=64 */
    fft_cplxf_ie((complex_float*)x,(complex_float*)y,(complex_float*)fft_twd,1,N2);

    /* make final DCT transformation of FFT outputs */
    p0_ld  = (const xtfloatx2 *)x;
    p1_ld  = (const xtfloatx2 *)x+N2-1;
    p0_twd = (const xtfloatx2 *)rfft_split_twd+4;
    p1_twd = (const xtfloatx2 *)rfft_split_twd+1;
    p2_twd = (const xtfloatx2 *)rfft_split_twd+(N2-1);
    p0_st = (xtfloat *)y;
    p1_st = p0_st+N2;
    p2_st = p1_st-1;
    p3_st = p2_st+N2;

    /* Load constants */
    c05 = (xtfloatx2)0.5f;/* 0.5 */
    invsqrt2f = XT_LSI((xtfloat *)&_invsqrt2f_, 0);/* 1/sqrt(2) */

    XT_LSX2IP(y0, p0_ld, SZ_CF32);
    /* b0 = y0.re + y0.im */
    /* b1 = y0.re - y0.im */
    re = XT_HIGH_S(y0);
    im = XT_LOW_S (y0);
    b0=XT_ADD_S(re, im);
    b1=XT_SUB_S(re, im);
    XT_SSIP(b0, p0_st, SZ_F32);
    b1 = XT_MUL_S(b1, invsqrt2f);
    XT_SSIP(b1, p1_st, SZ_F32);

    __Pragma("loop_count min=2")
    for (k=1; k<N4; k++)
    {
      XT_LSX2IP(y0, p0_ld,       SZ_CF32);
      XT_LSX2XP(y1, p1_ld, -(int)SZ_CF32);
      XT_LSX2IP(cosi, p0_twd, 4*SZ_CF32);
      XT_LSX2IP(w1  , p1_twd,   SZ_CF32);
      XT_LSX2XP(w2  , p2_twd, -(int)SZ_CF32);
      
      s  = y0 + y1;
      d  = y0 - y1;
      /* t0.re = s.im; t0.im = d.re */
      t0 = XT_SEL32_LH_SX2(s, d);
      /* t0.re = s.re; t0.im = d.im */
      t1 = XT_SEL32_HL_SX2(s, d);

      y0 = XT_MULC_S(t0, cosi);
      /* t0 = 0.5*(t1+conj(y0)) */
      /* t1 = 0.5*(t1-conj(y0)) */
      t0 = t1 = t1*c05;
      XT_MADDMUX_S(t0, c05, y0, 4);
      XT_MADDMUX_S(t1, c05, y0, 6);
      t0 = XT_MULCCONJ_S(w1, t0);
      t1 = XT_MULC_S    (w2, t1);
      
      /* y[k    ]= t0.re */
      /* y[N-k  ]= t0.im */
      re = XT_HIGH_S(t0);
      t32x2 = XT_AE_MOVINT32X2_FROMXTFLOATX2(t0);
      XT_SSIP(re, p0_st, SZ_F32);/* save real part */
      AE_S32_L_IP(t32x2, castxcc(ae_int32,p3_st), -(int)SZ_F32);/* save imag part */
      /* y[N/2-k]= t1.re */
      /* y[N/2+k]= t1.im */
      re = XT_HIGH_S(t1);
      t32x2 = XT_AE_MOVINT32X2_FROMXTFLOATX2(t1);
      XT_SSIP(re, p2_st, -(int)SZ_F32);/* save real part */
      AE_S32_L_IP(t32x2, castxcc(ae_int32,p1_st), SZ_F32);/* save imag part*/
    }
    t0 = XT_LSX2I(p0_ld, 0);
    w1 = XT_LSX2I(p1_twd, 0);
    t0 = XT_MULC_S(t0, w1);

    re = XT_HIGH_S(t0);
    im = XT_LOW_S (t0);
    XT_SSI(re, p0_st, 0);
    XT_SSI(im, p3_st, 0);

    return 0;
} /* dctf() */

#endif /* HAVE_VFPU */
