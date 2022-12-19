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
    C code optimized for HiFi1
*/
/*===========================================================================
Fast Fourier Transforms:
fft_cplx             FFT on Complex Data
fft_real             FFT on Real Data
ifft_cplx            IFFT on Complex Data
ifft_real            Inverse FFT Forming Real Data
fft_cplx<prec>_ie    FFT on Complex Data with optimized memory usage
fft_real<prec>_ie    FFT on Real Data with optimized memory usage
ifft_cplx<prec>_ie   IFFT on Complex Data with optimized memory usage
ifft_real<prec>_ie   Inverse FFT Forming Real Data with optimized memory usage
dct                  Discrete Cosine Transform

There are limited combinations of precision and scaling options available:
----------------+---------------------------------------------------------------
FFT/IFFT        | Scaling options                        | Restrictions on the
                |                                        | input dynamic range
----------------+---------------------------------------------------------------
cplx24x24       | 0 � no scaling                         | input signal < 2^23/(2*N),
                |                                        | N-fft-size
real24x24       | 1 � 24-bit scaling                     |        none
                | 2 � 32-bit scaling on the first stage  |        none
                | and 24-bit scaling later               |        none
                | 3 � fixed scaling before each stage    |        none
------------------------------------------------------------------------------------
cplx32x16       | 3 � fixed scaling before each stage    |        none
cplx16x16       | 3 � fixed scaling before each stage    |        none
cplx32x16_ie    | 3 � fixed scaling before each stage    |        none
cplx24x24_ie    | 3 � fixed scaling before each stage    |        none
real32x16       | 3 � fixed scaling before each stage    |        none
real16x16       | 3 � fixed scaling before each stage    |        none
real32x16_ie    | 3 � fixed scaling before each stage    |        none
real24x24_ie    | 3 � fixed scaling before each stage    |        none
real32x16_ie_24p| 3 � fixed scaling before each stage    |        none
----------------+---------------------------------------------------------------
real24x24_ie_24p| 1 � 24-bit scaling                     |        none
----------------+---------------------------------------------------------------
DCT:            |
----------------+---------------------------------------------------------------
24x24	          | 0 � no scaling                         |        none
32x16	          | 0 � no scaling                         |        none
----------------+---------------------------------------------------------------

===========================================================================*/

#include "NatureDSP_Signal.h"
#include "common.h"

extern int fft_stage_last_ie( int32_t *x, 
                               int32_t *y, 
                               int N); 


/*-------------------------------------------------------------------------
  These functions make FFT on complex data with optimized memory usage.
  Scaling  : Fixed data scaling at each stage
  NOTES:
  1. Bit-reversing reordering is done here.
  2. FFT runs in-place algorithm so INPUT DATA WILL APPEAR DAMAGED after 
     the call
  3. Forward/inverse FFT of size N may be supplied with constant data
     (twiddle factors) of a larger-sized FFT = N*twdstep.

  Precision: 
  24x24_ie      24-bit input/outputs, 24-bit twiddles
  32x16_ie      32-bit input/outputs, 16-bit twiddles
  f             floating point
 
  Input:
  x[N]                  complex input signal. Real and imaginary data are interleaved 
                        and real data goes first
  twd[N*twdstep*3/4]    twiddle factor table of a complex-valued FFT of size N*twdstep
  N                     FFT size
  twdstep               twiddle step 
  scalingOpt            scaling option = 3 (fixed scaling)
  Output:
  y[N]                  output spectrum. Real and imaginary data are interleaved and 
                        real data goes first

  Returned value: total number of right shifts occurred during scaling 
                  procedure

  Restrictions:
  x,y - should not overlap
  x,y - aligned on 8-bytes boundary
  N   - 256, 512, 1024 for fixed-point routines, power of 2 and >=8 for 
        floating point

-------------------------------------------------------------------------*/
static inline ae_f32x2 AE_MULFC24RA_LE(ae_f32x2 x, ae_f32x2 y)
{
    ae_f64 re, im;
    re = AE_MULF32S_HH(x, y);
    im = AE_MULF32S_LH(x, y);
    AE_MULSF32S_LL(re, x, y);
    AE_MULAF32S_LH(im, y, x);
    ae_f32x2 out = AE_ROUND32X2F64SASYM(re, im);
    /*ae_f32x2 out = AE_ROUND24X2F48SASYM(re, im);*/

   return out;
}
int fft_cplx24x24_ie(complex_fract32* y,complex_fract32* x, const complex_fract32* twd, int twdstep, int N, int scalingOpt)
{
    NASSERT_ALIGN8(x);
    NASSERT_ALIGN8(y);
    NASSERT(scalingOpt==3);
    NASSERT(N==128||N==256||N==512||N==1024);

    int shift = 0; 
    int s;    
    int stride = N/4;     
    int M = N*twdstep; 

    ae_int32x2   * px32;
    ae_int32x2   * py32;

    ae_int32x2   * restrict ptw1 = (ae_int32x2*)  twd; 
    ae_int32x2   * restrict ptw2 = (ae_int32x2*) (twd+1*M/4);
    ae_int32x2   * restrict ptw3 = (ae_int32x2*) (twd+2*M/4);
 
    ae_int32x2  vA0, vA1, vA2, vA3, vB0, vB1, vB2, vB3, vC0, vC1, vC2, vC3;

    ae_int32x2 vT1, vT2, vT3;
    int i; 
    int log2n = 0; 

    WUR_AE_CBEGIN0((unsigned)x); 
    WUR_AE_CEND0((unsigned)(uintptr_t)(x+(N-1)));     

    s = 3;

    /*ae_int32x2 rounding_const = AE_MOVDA32X2(0, 1<<30);
 *     ae_f64 rounding_const64 = AE_MOVF64_FROMF32X2(rounding_const);*/

    while( stride > 1 )
    {
        WUR_AE_SAR(s); 
        unsigned acc = 0; 
        const unsigned acc_inc  = (log2n==0)? 0: (0x80000000 >> (log2n-1)); 

        shift += s;       
        log2n += 2;   
        px32 = py32 = (ae_int32x2*)x; 

        ptw1 = (ae_int32x2*)  twd; 
        ptw2 = (ae_int32x2*) (twd+1*M/4);
        ptw3 = (ae_int32x2*) (twd+2*M/4);

        i = N/4;
        do 
        {
            int offset_inc = 0; 
            acc+=acc_inc;
            XT_MOVEQZ(offset_inc, twdstep*8, acc); 
            /* Load 24-bit values using 32-bit load. Shift the loaded values for right-alignment */
            vA3 = AE_L32X2_X ( px32, 3*8*stride);
            vA2 = AE_L32X2_X ( px32, 2*8*stride); 
            vA1 = AE_L32X2_X ( px32, 1*8*stride); 
            AE_L32X2_XC(vA0, px32,   4*8*stride); 
            vA3 = AE_SRAI32(vA3, 8); 
            vA2 = AE_SRAI32(vA2, 8); 
            vA1 = AE_SRAI32(vA1, 8);
            vA0 = AE_SRAI32(vA0, 8); 

            vB0 = AE_ADD32S(vA0, vA2);
            vB2 = AE_SUB32S(vA0, vA2);
            vB1 = AE_ADD32S(vA1, vA3);
            vB3 = AE_SUB32S(vA1, vA3);
            
            /*vB3 = AE_SEL32_LH(vB3, vB3);*/
            
            vC0 = AE_ADD32S(vB0, vB1);
            vC2 = AE_SUB32S(vB0, vB1);
            vC3 = AE_SUBADD32S_HL_LH(vB2, vB3); 
            vC1 = AE_ADDSUB32S_HL_LH(vB2, vB3); 
    #if 1        
            vC0 = AE_SRAS32(vC0); 
            vC2 = AE_SRAS32(vC2); 
            vC3 = AE_SRAS32(vC3); 
            vC1 = AE_SRAS32(vC1); 
    #else
            vC0 = AE_SRAA32RS(vC0, s); 
            vC2 = AE_SRAA32RS(vC2, s); 
            vC3 = AE_SRAA32RS(vC3, s); 
            vC1 = AE_SRAA32RS(vC1, s); 
    #endif
            /* Load 24-bit twiddle factors as 32-bits. To be interpreted as Q31 instead of Q23 */
            AE_L32X2_XP(vT1, ptw1, offset_inc);
            AE_L32X2_XP(vT2, ptw2, offset_inc);
            AE_L32X2_XP(vT3, ptw3, offset_inc);
            /*vT1 = AE_SRAI32(vT1, 8); 
 *             vT2 = AE_SRAI32(vT2, 8); 
 *                         vT3 = AE_SRAI32(vT3, 8);*/

            vC1 = AE_MULFC24RA_LE(vC1, vT1);
            /*vC1 = AE_MULFC24RA(t1, vT1); */
            vC2 = AE_MULFC24RA_LE(vC2, vT2);
            /*vC2 = AE_MULFC24RA(t2, vT2);*/
            vC3 = AE_MULFC24RA_LE(vC3, vT3);
            /*vC3 = AE_MULFC24RA(t3, vT3);*/

            /* Store the results in-place */
            vC0 = AE_SLAI32(vC0, 8);
            vC1 = AE_SLAI32(vC1, 8);
            vC2 = AE_SLAI32(vC2, 8);
            vC3 = AE_SLAI32(vC3, 8);
            AE_S32X2_X(vC3,  py32, 3*8*stride);
            AE_S32X2_X(vC2,  py32, 1*8*stride);
            AE_S32X2_X(vC1,  py32, 2*8*stride);
            AE_S32X2_XC(vC0, py32, 4*8*stride);     

        } while(--i);

        s = 2;  /*Scaling for other stages*/
        stride>>=2;  
        twdstep<<=2;
    }    /*while( stride > 1 )*/

    shift += fft_stage_last_ie((int32_t*)x, (int32_t*)y, N); 

    return shift;
}

