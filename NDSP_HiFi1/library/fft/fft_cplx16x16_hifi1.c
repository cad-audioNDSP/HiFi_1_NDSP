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
 *         NatureDSP Signal Processing Library. FFT part
 *             C code optimized for HiFi1
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
cplx24x24       | 0 ÿ no scaling                         | input signal < 2^23/(2*N),
                |                                        | N-fft-size
real24x24       | 1 ÿ 24-bit scaling                     |        none
                | 2 ÿ 32-bit scaling on the first stage  |        none
                | and 24-bit scaling later               |        none
                | 3 ÿ fixed scaling before each stage    |        none
------------------------------------------------------------------------------------
cplx32x16       | 3 ÿ fixed scaling before each stage    |        none
cplx16x16       | 3 ÿ fixed scaling before each stage    |        none
cplx32x16_ie    | 3 ÿ fixed scaling before each stage    |        none
cplx24x24_ie    | 3 ÿ fixed scaling before each stage    |        none
real32x16       | 3 ÿ fixed scaling before each stage    |        none
real16x16       | 3 ÿ fixed scaling before each stage    |        none
real32x16_ie    | 3 ÿ fixed scaling before each stage    |        none
real24x24_ie    | 3 ÿ fixed scaling before each stage    |        none
real32x16_ie_24p| 3 ÿ fixed scaling before each stage    |        none
----------------+---------------------------------------------------------------
real24x24_ie_24p| 1 ÿ 24-bit scaling                     |        none
----------------+---------------------------------------------------------------
DCT:            |
----------------+---------------------------------------------------------------
24x24	          | 0 ÿ no scaling                         |        none
32x16	          | 0 ÿ no scaling                         |        none
----------------+---------------------------------------------------------------

===========================================================================*/
#include "NatureDSP_Signal.h"
#include "common.h"
#include "fft_cplx_twiddles.h"

//#define FIRST_STAGE_SCALE 3

#ifdef COMPILER_XTENSA
//#define ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
//#define ATTRIBUTE_NEVER_INLINE  __attribute__((noinline))
#else
#define ATTRIBUTE_ALWAYS_INLINE
#define ATTRIBUTE_NEVER_INLINE
#endif

#define FIRST_STAGE_SCALE 3

#define DFT4XI2(x0, x1, x2, x3)/* output x0, x3, x1, x2*/\
{\
    ae_int16x4 t1, t2, t3;\
    AE_ADDANDSUBRNG16RAS_S1(x0, x2);\
    AE_ADDANDSUBRNG16RAS_S1(x1, x3);\
\
    x3 = AE_MOVINT16X4_FROMF16X4(AE_MUL16JS(AE_MOVF16X4_FROMINT16X4(x3)));\
\
    AE_ADDANDSUBRNG16RAS_S2(x0, x1);\
    AE_ADDANDSUBRNG16RAS_S2(x2, x3);\
\
    t1 = x3; \
    t2 = x1; \
    t3 = x2; \
    x1 = t1; \
    x2 = t2;\
    x3 = t3;\
}

inline_ void MULCx2(ae_int16x4 *x, const ae_int16x4 *t)
{
    ae_f16x4 f;
    f = AE_MULFC16RAS_L(AE_MOVF16X4_FROMINT16X4(*x), AE_MOVF16X4_FROMINT16X4(*t));
    AE_MULFC16RAS_H(f, AE_MOVF16X4_FROMINT16X4(*x), AE_MOVF16X4_FROMINT16X4(*t));
    *x = AE_MOVINT16X4_FROMF16X4(f);
}
static void stage_first_DFT4xI2_inpl(int16_t *x, int16_t *twd, int N)
{
    ae_int16x4 * restrict px = (ae_int16x4 *)x;
    ae_int16x4 * restrict py = (ae_int16x4 *)x;
    ae_int32x2 * restrict ptw32x2 = (ae_int32x2 *)twd;
    ae_int32x2 r1, r2, r3;
    int stride = N / 4 * sizeof(int16_t) * 2;
        
    int i = N >> 3;
    ae_int16x4 x0, x1, x2, x3;
    ae_int16x4 w1, w2, w3;

    WUR_AE_SAR(((FIRST_STAGE_SCALE - 1) << 1) + 1);
    __Pragma("loop_count min=2");
    for (i = 0; i < (N >> 3); i++)
    {
        x1 = AE_L16X4_X(px, stride);
        x3 = AE_L16X4_X(px, stride * 3);
        x2 = AE_L16X4_X(px, stride * 2);
        AE_L16X4_XP(x0, px, 8);

        DFT4XI2(x0, x1, x2, x3);

        /* Use AE_L32X2_XP for reading twiddles ! */
        AE_L32X2_XP(r1, ptw32x2, 8);
        AE_L32X2_XP(r2, ptw32x2, 8);
        AE_L32X2_XP(r3, ptw32x2, 8);

        w1 = AE_MOVINT16X4_FROMF32X2(r1);
        w2 = AE_MOVINT16X4_FROMF32X2(r2);
        w3 = AE_MOVINT16X4_FROMF32X2(r3);

        MULCx2(&x1, &w1);
        MULCx2(&x2, &w2);
        MULCx2(&x3, &w3);

        AE_S16X4RNG_XP(x0, py, stride);
        AE_S16X4RNG_XP(x2, py, stride);
        AE_S16X4RNG_XP(x1, py, stride);
        AE_S16X4RNG_XP(x3, py, 8 - 3 * stride);
    }//for (i = 0; i < (N >> 3); i++)
} //stage_first_DFT4xI2_inpl
static void stage_last_DFT4xI2_inpl(int16_t *x, int16_t *y, int N)
{

    int i = N >> 3;
    const int shift = 2;
    ae_int16x4 * restrict px0 = (ae_int16x4 *)x;

    WUR_AE_SAR(((shift - 1) << 1) + 1);

    ae_int16x4 * restrict y10 = (ae_int16x4 *)(y);
    ae_int16x4 * restrict y11 = (ae_int16x4 *)(y + N );
    ae_int16x4 * restrict y12 = (ae_int16x4 *)(y + N / 2);
    ae_int16x4 * restrict y13 = (ae_int16x4 *)(y + 3 * N / 2);

    int ibitrev = 0;
    int ibitrev_off = 0x80000000 >> (29 - XT_NSA(N));
    __Pragma("loop_count min=2");
    for (i=0;i<(N/4);i+=2)
    {
        ae_int16x4 e0, e1, e3, e4, e2, e5, h2, h3, h4, h5;
        e0 = px0[i];
        e1 = px0[i+1];
        e3 = px0[i+N/4];
        e4 = px0[i+1+N/4];

        AE_ADDANDSUBRNG16RAS_S1(e0, e1);
        e2 = e1; 

        AE_ADDANDSUBRNG16RAS_S1(e3, e4);
        e5 = e4; 

        h4 = AE_SEL16_7632(e0, e3);
        h5 = AE_SEL16_5410(e0, e3);
        AE_ADDANDSUBRNG16RAS_S2(h4, h5);

        h3 = AE_SEL16_7632(e1, e4);
        h2 = AE_SEL16_5410(e2, e5);
        h2 = AE_MUL16JS(h2);
        AE_ADDANDSUBRNG16RAS_S2(h3, h2);

        /* Store the 2 bfly outputs together  */
        AE_S16X4RNG_X(h4 , y10, ibitrev);
        AE_S16X4RNG_X(h5 , y11, ibitrev);
        AE_S16X4RNG_X(h2 , y12, ibitrev);
        AE_S16X4RNG_X(h3 , y13, ibitrev);

        ibitrev = AE_ADDBRBA32(ibitrev,ibitrev_off);
    }

} //stage_last_DFT4xI2_inpl
static void stage_inner_DFT4xI2_inpl_merged(int16_t *x, int16_t *y, const int16_t *twd, int N, int stride)
 {
     const int shift = 2;
     ae_int16x4 * restrict px = (ae_int16x4 *)x;
     ae_int16x4 * restrict py = (ae_int16x4 *)y;
     ae_int32x2 * restrict ptw = (ae_int32x2 *)twd;

     uint32_t flag = 0;
     uint32_t flag_inc = 0x80000000 >> (XT_NSA(stride) - XT_NSA(N) - 1); 

     int i;
     ae_int16x4 x0, x1, x2, x3;// , t;
     ae_int32x2 r1, r2, r3;
     ae_int16x4 w1, w2, w3;

     WUR_AE_CBEGIN0((unsigned)x);
     WUR_AE_CEND0((unsigned)(x + 2 * N - 4));
     WUR_AE_SAR(((shift - 1) << 1) + 1);

     __Pragma("loop_count min=2");
     for (i = 0; i < N / 8; i++)
     {   
        int tw_inc = 0;
        flag += flag_inc;
        XT_MOVEQZ(tw_inc, 24, flag);

        r2 = AE_L32X2_I(ptw, 8);
        r3 = AE_L32X2_I(ptw, 16);
        AE_L32X2_XP(r1, ptw, tw_inc);

        w1 = AE_MOVINT16X4_FROMF32X2(r1);
        w2 = AE_MOVINT16X4_FROMF32X2(r2);
        w3 = AE_MOVINT16X4_FROMF32X2(r3);

        AE_L16X4_XP(x0, px, stride);
        AE_L16X4_XP(x1, px, stride);
        AE_L16X4_XP(x2, px, stride);
        AE_L16X4_XC(x3, px, stride);

        DFT4XI2(x0, x1, x2, x3);

        MULCx2(&x1, &w1);
        MULCx2(&x2, &w2);
        MULCx2(&x3, &w3);
           
        AE_S16X4RNG_XP(x0, py, stride);
        AE_S16X4RNG_XP(x2, py, stride);
        AE_S16X4RNG_XP(x1, py, stride);
        AE_S16X4_XC   (x3, py, stride);        
     }
 } //stage_inner_DFT4xI2_inpl_merged
static void stage_inner_DFT4xI2_inpl_ref(int16_t *x, int16_t *y, const int16_t *twd, int N, int stride)
 {
     const int shift = 2;
     ae_int16x4 * restrict px = (ae_int16x4 *)x;
     ae_int16x4 * restrict py = (ae_int16x4 *)y;
     ae_int32x2 * restrict ptw = (ae_int32x2 *)twd;

     int M = 1 << (XT_NSA(stride) - XT_NSA(N)); // (N / stride);

     int i;
     int j;

     ae_int16x4 x0, x1, x2, x3;
     ae_int32x2 r1, r2, r3;
     ae_int16x4 w1, w2, w3;


     WUR_AE_SAR(((shift - 1) << 1) + 1);

     __Pragma("loop_count min=2"); 
    for (i = 0; i < stride /8; i++)
    {   
        py = (ae_int16x4 *)(8 * i + (uintptr_t)y);
        px = (ae_int16x4 *)(8 * i + (uintptr_t)x);

        AE_L32X2_XP(r1, ptw, 8);
        AE_L32X2_XP(r2, ptw, 8);
        AE_L32X2_XP(r3, ptw, 8);

        w1 = AE_MOVINT16X4_FROMF32X2(r1);
        w2 = AE_MOVINT16X4_FROMF32X2(r2);
        w3 = AE_MOVINT16X4_FROMF32X2(r3);

        __Pragma("loop_count min=2");
        for (j = 0; j < M ; j++)
        {

            AE_L16X4_XP(x0, px, stride);
            AE_L16X4_XP(x1, px, stride);
            AE_L16X4_XP(x2, px, stride);
            AE_L16X4_XP(x3, px, stride);
                 
            DFT4XI2(x0, x1, x2, x3);

            MULCx2(&x1, &w1);
            MULCx2(&x2, &w2);
            MULCx2(&x3, &w3);

            AE_S16X4RNG_XP(x0, py, stride);
            AE_S16X4RNG_XP(x2, py, stride);
            AE_S16X4RNG_XP(x1, py, stride);
            AE_S16X4RNG_XP(x3, py, stride);
        }
    }
 } //stage_inner_DFT4xI2_inpl_ref
static void stage_last_DFT2xI2_inpl(int16_t *x, int16_t *y, int N)
{
    ae_int16x4 * restrict px = (ae_int16x4 *)x;
    int i;
    WUR_AE_SAR(1);

    ae_int16x4  * restrict y10, 
                * restrict y11, 
                * restrict y12, 
                * restrict y13;
    ae_int16x4 h2, h3, h4, h5;
    ae_int16x4 e0, e1, e2, e3;
    int ibitrev_off = 0x80000000 >> (29 - XT_NSA(N));
    int ibitrev = 0;

    y10 = (ae_int16x4 *)(y);
    y11 = (ae_int16x4 *)(y + N );
    y12 = (ae_int16x4 *)(y + N / 2);
    y13 = (ae_int16x4 *)(y + 3 * N / 2);

    //last stage with bit-reverse
    __Pragma("loop_count min=2");
    for (i = 0; i<(N / 4); i += 2)
    {
        e0 = px[i];
        e1 = px[i + 1];
        e2 = px[i + N / 4];
        e3 = px[i + 1 + N / 4];

        h5 = AE_SEL16_5410(e0, e2);
        h4 = AE_SEL16_7632(e0, e2);

        AE_ADDANDSUBRNG16RAS_S1(h4, h5);

        h3 = AE_SEL16_5410(e1, e3);
        h2 = AE_SEL16_7632(e1, e3);

        AE_ADDANDSUBRNG16RAS_S1(h2, h3);

        /* Store the 2 bfly outputs together */
        AE_S16X4RNG_X(h4, y10, ibitrev);
        AE_S16X4RNG_X(h5, y11, ibitrev);
        AE_S16X4RNG_X(h2, y12, ibitrev);
        AE_S16X4RNG_X(h3, y13, ibitrev);

        ibitrev = AE_ADDBRBA32(ibitrev, ibitrev_off);
    }
} //stage_last_DFT2xI2_inpl


 static  int stage_second_DFT4xI2_inpl_ref_ds(int16_t *x, int16_t *y, const int16_t *twd, int N, int stride)
{
    int shift;
    ae_int16x4 * restrict px = (ae_int16x4 *)x;
    ae_int16x4 * restrict py = (ae_int16x4 *)y;
    ae_int32x2 * restrict ptw = (ae_int32x2 *)twd;
    int i;

    ae_int16x4 x0, x1, x2, x3;
    ae_int32x2 r1, r2, r3;
    ae_int16x4 w1, w2, w3;

    shift = AE_CALCRNG3();
    //__Pragma("loop_count min=2");
    for (i = 0; i < stride / 8; i++)
    {   
        //py = (ae_int16x4 *)(8 * i + (uintptr_t)y);
        //px = (ae_int16x4 *)(8 * i + (uintptr_t)x);

        AE_L32X2_XP(r1, ptw, 8);
        AE_L32X2_XP(r2, ptw, 8);
        AE_L32X2_XP(r3, ptw, 8);

        w1 = AE_MOVINT16X4_FROMF32X2(r1);
        w2 = AE_MOVINT16X4_FROMF32X2(r2);
        w3 = AE_MOVINT16X4_FROMF32X2(r3);
        
        
        __Pragma("loop_count min=2");
        //for (j = 0; j < M; j++)
        {
            AE_L16X4_XP(x0, px, stride);
            AE_L16X4_XP(x1, px, stride);
            AE_L16X4_XP(x2, px, stride);
            AE_L16X4_XP(x3, px, stride);

            DFT4XI2(x0, x1, x2, x3);
            MULCx2(&x1, &w1);
            MULCx2(&x2, &w2);
            MULCx2(&x3, &w3);

            AE_S16X4RNG_XP(x0, py, stride);
            AE_S16X4RNG_XP(x2, py, stride);
            AE_S16X4RNG_XP(x1, py, stride);
            AE_S16X4RNG_XP(x3, py, stride);
        }
        {

            AE_L16X4_XP(x0, px, stride);
            AE_L16X4_XP(x1, px, stride);
            AE_L16X4_XP(x2, px, stride);
            AE_L16X4_XP(x3, px, stride);

            DFT4XI2(x0, x1, x2, x3);
            MULCx2(&x1, &w1);
            MULCx2(&x2, &w2);
            MULCx2(&x3, &w3);

            AE_S16X4RNG_XP(x0, py, stride);
            AE_S16X4RNG_XP(x2, py, stride);
            AE_S16X4RNG_XP(x1, py, stride);
            AE_S16X4RNG_XP(x3, py, stride);
        }
        {

            AE_L16X4_XP(x0, px, stride);
            AE_L16X4_XP(x1, px, stride);
            AE_L16X4_XP(x2, px, stride);
            AE_L16X4_XP(x3, px, stride);

            DFT4XI2(x0, x1, x2, x3);
            MULCx2(&x1, &w1);
            MULCx2(&x2, &w2);
            MULCx2(&x3, &w3);

            AE_S16X4RNG_XP(x0, py, stride);
            AE_S16X4RNG_XP(x2, py, stride);
            AE_S16X4RNG_XP(x1, py, stride);
            AE_S16X4RNG_XP(x3, py, stride);
        }
        {

            AE_L16X4_XP(x0, px, stride);
            AE_L16X4_XP(x1, px, stride);
            AE_L16X4_XP(x2, px, stride);
            AE_L16X4_XP(x3, px, sizeof(x0)-15 * stride);

            DFT4XI2(x0, x1, x2, x3);
            MULCx2(&x1, &w1);
            MULCx2(&x2, &w2);
            MULCx2(&x3, &w3);

            AE_S16X4RNG_XP(x0, py, stride);
            AE_S16X4RNG_XP(x2, py, stride);
            AE_S16X4RNG_XP(x1, py, stride);
            AE_S16X4RNG_XP(x3, py, sizeof(x0)-15 * stride);
        }
    }
    return shift;
} //stage_second_DFT4xI2_inpl_ref_ds


static int stage_inner_DFT4xI2_inpl_ref_ds(int16_t *x, int16_t *y, const int16_t *twd, int N, int stride)
{
    int shift;
    ae_int16x4 * restrict px = (ae_int16x4 *)x;
    ae_int16x4 * restrict py = (ae_int16x4 *)y;
    ae_int32x2 * restrict ptw = (ae_int32x2 *)twd;

    int M = 1 << (XT_NSA(stride) - XT_NSA(N)); // (N / stride);
    int i;
    int j;
    ae_int16x4 x0, x1, x2, x3;
    ae_int32x2 r1, r2, r3;
    ae_int16x4 w1, w2, w3;

    shift = AE_CALCRNG3();
 

    __Pragma("loop_count min=2");
    for (i = 0; i < stride / 8; i++)
    {   
        py = (ae_int16x4 *)(8 * i + (uintptr_t)y);
        px = (ae_int16x4 *)(8 * i + (uintptr_t)x);

        AE_L32X2_XP(r1, ptw, 8);
        AE_L32X2_XP(r2, ptw, 8);
        AE_L32X2_XP(r3, ptw, 8);

        w1 = AE_MOVINT16X4_FROMF32X2(r1);
        w2 = AE_MOVINT16X4_FROMF32X2(r2);
        w3 = AE_MOVINT16X4_FROMF32X2(r3);

        ASSERT(M>=4); 
        __Pragma("loop_count min=2");
        for (j = 0; j < M; j++)
        {

            AE_L16X4_XP(x0, px, stride);
            AE_L16X4_XP(x1, px, stride);
            AE_L16X4_XP(x2, px, stride);
            AE_L16X4_XP(x3, px, stride);

            DFT4XI2(x0, x1, x2, x3);
            MULCx2(&x1, &w1);
            MULCx2(&x2, &w2);
            MULCx2(&x3, &w3);

            AE_S16X4RNG_XP(x0, py, stride);
            AE_S16X4RNG_XP(x2, py, stride);
            AE_S16X4RNG_XP(x1, py, stride);
            AE_S16X4RNG_XP(x3, py, stride);
        }
    }
    return shift;
} //stage_inner_DFT4xI2_inpl_ref_ds


/*
fist stage with  dynamic scaling
*/
static int stage_first_DFT4xI2_inpl_ds(int16_t *x, int16_t *twd, int N, int bexp)
{
    ae_int16x4 * restrict px = (ae_int16x4 *)x;
    ae_int16x4 * restrict py = (ae_int16x4 *)x;
    ae_int32x2 * restrict ptw32x2 = (ae_int32x2 *)twd;
    ae_int32x2 r1, r2, r3;
    int stride = N / 4 * sizeof(int16_t)* 2;

    int i = N >> 3;
    ae_int16x4 x0, x1, x2, x3;
    ae_int16x4 w1, w2, w3;

    WUR_AE_SAR(((FIRST_STAGE_SCALE - 1) << 1) + 1);
    __Pragma("loop_count min=2");

    for (i = 0; i < (N >> 3); i++)
    {
        x1 = AE_L16X4_X(px, stride);
        x3 = AE_L16X4_X(px, stride * 3);
        x2 = AE_L16X4_X(px, stride * 2);
        AE_L16X4_XP(x0, px, 8);

        // Normalize input data
        x0 = AE_SLAA16S(x0, bexp);
        x1 = AE_SLAA16S(x1, bexp);
        x2 = AE_SLAA16S(x2, bexp);
        x3 = AE_SLAA16S(x3, bexp);
        DFT4XI2(x0, x1, x2, x3);

        /* Use AE_L32X2_XP for reading twiddles ! */
        AE_L32X2_XP(r1, ptw32x2, 8);
        AE_L32X2_XP(r2, ptw32x2, 8);
        AE_L32X2_XP(r3, ptw32x2, 8);

        w1 = AE_MOVINT16X4_FROMF32X2(r1);
        w2 = AE_MOVINT16X4_FROMF32X2(r2);
        w3 = AE_MOVINT16X4_FROMF32X2(r3);

        MULCx2(&x1, &w1);
        MULCx2(&x2, &w2);
        MULCx2(&x3, &w3);

        AE_S16X4RNG_XP(x0, py, stride);
        AE_S16X4RNG_XP(x2, py, stride);
        AE_S16X4RNG_XP(x1, py, stride);
        AE_S16X4RNG_XP(x3, py, 8 - 3 * stride);
    }//for (i = 0; i < (N >> 3); i++)

    return 3 - bexp;
} //stage_first_DFT4xI2_inpl_ds

static int stage_last_DFT4xI2_inpl_ds(int16_t *x, int16_t *y, int N)
{

    int i = N >> 3;
    int shift = 0;
    ae_int16x4 * restrict px0 = (ae_int16x4 *)x;

    ae_int16x4 * restrict y10 = (ae_int16x4 *)(y);
    ae_int16x4 * restrict y11 = (ae_int16x4 *)(y + N);
    ae_int16x4 * restrict y12 = (ae_int16x4 *)(y + N / 2);
    ae_int16x4 * restrict y13 = (ae_int16x4 *)(y + 3 * N / 2);

    int ibitrev = 0;
    int ibitrev_off = 0x80000000 >> (29 - XT_NSA(N));


    shift = AE_CALCRNG3();

    __Pragma("loop_count min=2");
    for (i = 0; i<(N / 4); i += 2)
    {
        ae_int16x4 e0, e1, e3, e4, e2, e5, h2, h3, h4, h5;
        
        e0 = px0[i];
        e1 = px0[i + 1];
        e3 = px0[i + N / 4];
        e4 = px0[i + 1 + N / 4];
        AE_ADDANDSUBRNG16RAS_S1(e0, e1);
        e2 = e1;

        AE_ADDANDSUBRNG16RAS_S1(e3, e4);
        e5 = e4;

        h4 = AE_SEL16_7632(e0, e3);
        h5 = AE_SEL16_5410(e0, e3);
        AE_ADDANDSUBRNG16RAS_S2(h4, h5);

        h3 = AE_SEL16_7632(e1, e4);
        h2 = AE_SEL16_5410(e2, e5);
        h2 = AE_MUL16JS(h2);
        AE_ADDANDSUBRNG16RAS_S2(h3, h2);

        /* Store the 2 bfly outputs together  */
        AE_S16X4RNG_X(h4, y10, ibitrev);
        AE_S16X4RNG_X(h5, y11, ibitrev);
        AE_S16X4RNG_X(h2, y12, ibitrev);
        AE_S16X4RNG_X(h3, y13, ibitrev);

        ibitrev = AE_ADDBRBA32(ibitrev, ibitrev_off);
    }
    return shift;
} //stage_last_DFT4xI2_inpl_ds

static int stage_last_DFT2xI2_inpl_ds(int16_t *x, int16_t *y, int N)
{

    int sar = RUR_AE_SAR();
    ae_int16x4 * restrict px = (ae_int16x4 *)x;
    int i;

    sar = 1 & (sar >> 5);


    WUR_AE_SAR(sar);

    ae_int16x4  * restrict y10,
        *restrict y11,
        *restrict y12,
        *restrict y13;
    ae_int16x4 h2, h3, h4, h5;
    ae_int16x4 e0, e1, e2, e3;
    int ibitrev_off = 0x80000000 >> (29 - XT_NSA(N));
    int ibitrev = 0;

    y10 = (ae_int16x4 *)(y);
    y11 = (ae_int16x4 *)(y + N);
    y12 = (ae_int16x4 *)(y + N / 2);
    y13 = (ae_int16x4 *)(y + 3 * N / 2);

    //last stage with bit-reverse
    __Pragma("loop_count min=2");
    for (i = 0; i<(N / 4); i += 2)
    {
        e0 = px[i];
        e1 = px[i + 1];
        e2 = px[i + N / 4];
        e3 = px[i + 1 + N / 4];

        h5 = AE_SEL16_5410(e0, e2);
        h4 = AE_SEL16_7632(e0, e2);
        AE_ADDANDSUBRNG16RAS_S1(h4, h5);

        h3 = AE_SEL16_5410(e1, e3);
        h2 = AE_SEL16_7632(e1, e3);

        AE_ADDANDSUBRNG16RAS_S1(h2, h3);

        /* Store the 2 bfly outputs together */
        AE_S16X4RNG_X(h4, y10, ibitrev);
        AE_S16X4RNG_X(h5, y11, ibitrev);
        AE_S16X4RNG_X(h2, y12, ibitrev);
        AE_S16X4RNG_X(h3, y13, ibitrev);

        ibitrev = AE_ADDBRBA32(ibitrev, ibitrev_off);
    }
    return sar;
} //stage_last_DFT2xI2_inpl_ds


/*-------------------------------------------------------------------------
  FFT on Complex Data
  These functions make FFT on complex data.
  NOTES:
  1. Bit-reversing permutation is done here. 
  2. FFT runs in-place algorithm so INPUT DATA WILL APPEAR DAMAGED after 
     the call

  Precision: 
  32x32  32-bit input/outputs, 32-bit twiddles
  24x24  24-bit input/outputs, 24-bit twiddles
  32x16  32-bit input/outputs, 16-bit twiddles
  16x16  16-bit input/outputs, 16-bit twiddles
 
  Input:
  x[2*N]     complex input signal. Real and imaginary data are interleaved 
             and real data goes first
  N          FFT size
  scalingOpt scaling option
  Output:
  y[2*N]     output spectrum. Real and imaginary data are interleaved and 
             real data goes first

  Returned value: total number of right shifts occurred during scaling 
                  procedure

  Restrictions:
  x,y should not overlap
  x,y - aligned on a 8-bytes boundary
  N   - 2^m, 16...4096

-------------------------------------------------------------------------*/
int fft_cplx16x16( int16_t* y,int16_t* x,fft_handle_t h,int scalingOption)
{
    /* with Quad MAC option */
    int N;
    const tFftDescr *pDescr = (const tFftDescr*)h;
    int stride;
    cint32_ptr *p = (cint32_ptr *)pDescr->twd;
    ae_int16x4 * restrict px; 
    int16_t *p_twd = (int16_t*)*p++;

    NASSERT_ALIGN8(x);
    NASSERT_ALIGN8(y);
    NASSERT(scalingOption == 3 || scalingOption == 2);
    N = pDescr->N;
    stride = N; // The stride is quartered with every iteration of the outer loop.
   // AE_CALCRNG3()
    if (scalingOption == 3)
    {

        stage_first_DFT4xI2_inpl(x, (int16_t*)p_twd, N);
        stride >>= 2;
        if (stride > 4)
        {
            p_twd = (int16_t*)*p++;
            stage_inner_DFT4xI2_inpl_merged(x, x, p_twd, N, stride);
            stride >>= 2;
            while (stride > 4)
            {
                p_twd = (int16_t*)*p++;
                stage_inner_DFT4xI2_inpl_ref(x, x, p_twd, N, stride);
                stride >>= 2;
            }
        }
        if (stride == 4)
        {
            stage_last_DFT4xI2_inpl(x, y, N);
        }
        else
        {
            stage_last_DFT2xI2_inpl(x, y, N);
        }
        return  31 - XT_NSA(N);
    }
    else // if (scalingOption == 3)
    {
        int shift; 
        int bexp; 
        
        {
            int i; 
            ae_int16x4 acc = AE_MOVINT16X4_FROMINT32X2( AE_MOVI(0) ), tmp; 

            __Pragma("loop_count min=4 factor=4"); 
            px = (ae_int16x4*)x;
            for (i = 0; i < (N >> 1); i++)
            {
                AE_L16X4_IP(tmp, px, sizeof(*px) );
                tmp=AE_ABS16S(tmp);
                acc = AE_MAX16(acc, tmp); 
            }
            acc = AE_MAX16(acc, AE_SEL16_5432(acc, acc));
            acc = AE_MAX16(acc, AE_SHORTSWAP(acc) );

            i = AE_MOVAD16_0(acc);
            bexp = XT_NSA(i)-16;
            XT_MOVEQZ(bexp, 0, i);
        }

        ASSERT(bexp >= 0); 
       
        shift = stage_first_DFT4xI2_inpl_ds(x, (int16_t*)p_twd, N, bexp);
        stride >>= 2;
        //todo stage_second need here
        if (stride > 4)
        {
            p_twd = (int16_t*)*p++;
            shift += stage_second_DFT4xI2_inpl_ref_ds(x, x, p_twd, N, stride);
            stride >>= 2;
            while (stride > 4)
            {
                p_twd = (int16_t*)*p++;
                shift += stage_inner_DFT4xI2_inpl_ref_ds(x, x, p_twd, N, stride);
                stride >>= 2;
            }
        }
        if (stride == 4)
        {
            shift += stage_last_DFT4xI2_inpl_ds(x, y, N);
        }
        else
        {
           shift += stage_last_DFT2xI2_inpl_ds(x, y, N);
        }

       
        return  shift;
    } //if (scalingOption == 3) else
}

