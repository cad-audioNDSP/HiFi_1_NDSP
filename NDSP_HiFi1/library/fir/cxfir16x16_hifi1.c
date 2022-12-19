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
    Complex block FIR filter, 16x16-bit
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

/* Instance pointer validation number. */
#define MAGIC     0x6B2D04F2

/* Reserve memory for alignment. */
#define ALIGNED_SIZE( size, align ) \
    ((size_t)(size)+(align)-1)

/* Align address on a specified boundary. */
#define ALIGNED_ADDR( addr, align ) \
    (void*)(((uintptr_t)(addr)+((align)-1)) & ~((align)-1))

#define sz_i16   sizeof(int16_t)
//#define sz_i32   sizeof(int32_t)

/* negation -Q31->Q31                            */
inline_ int16_t L_neg_l(int16_t x)
{
    return (x == MIN_INT16) ? MAX_INT16 : -x;
}


/* Filter instance structure. */
typedef struct tag_cxfir16x16_t
{
    uint32_t        magic;     // Instance pointer validation number
    int             M;         // Number of complex filter coefficients
    const int16_t * coef;      // M complex coefficients
    int16_t *       delayLine; // Delay line for complex samples
    int             delayLen;  // Delay line length, in complex samples
    int             wrIx;      // Index of the oldest complex sample

} cxfir16x16_t, *cxfir16x16_ptr_t;
/* Calculate the memory block size for an FIR filter with given attributes. */
size_t cxfir16x16_alloc(int M)
{
    NASSERT(M > 0);
    NASSERT(!(M & 3));
    return (ALIGNED_SIZE(sizeof(cxfir16x16_t), 4)
        + // Delay line
        ALIGNED_SIZE(2 * (M + 2)*sz_i16, 8)
        + // Filter coefficients
        ALIGNED_SIZE(2 * 2 * (M + 2)*sz_i16, 8));
} // cxfir16x16_alloc()

/* Initialize the filter structure. The delay line is zeroed. */
cxfir16x16_handle_t cxfir16x16_init( void *             objmem,
                                     int                M,
                                const complex_fract16 * restrict h)
{
    cxfir16x16_ptr_t cxfir;
    void    *        ptr;
    int16_t *        coef;
    int16_t *        coefB;
    int16_t *        delLine;

    int m;

    NASSERT(objmem && M > 0 && h);
    NASSERT(!(M & 3) && IS_ALIGN(h));

    //
    // Partition the memory block.
    //

    ptr     = objmem;
    cxfir   = (cxfir16x16_ptr_t)ALIGNED_ADDR(ptr, 4);
    ptr     = cxfir + 1;
    delLine = (int16_t *)ALIGNED_ADDR(ptr, 8);
    ptr     = delLine + 2 * (M + 2);

    coef    = (int16_t *)ALIGNED_ADDR(ptr, 8);
    ptr     = coef + 2 * 2 * (M + 2);

    NASSERT((int8_t*)ptr - (int8_t*)objmem <= (int)cxfir16x16_alloc(M));

    //
    // Copy the filter coefficients in reverted order and zero the delay line.
    //
    coefB = coef + 2 * (M + 2);

    coef[0] = coef[1] = coefB[0] = coefB[1] = 0;

    
    for (m = 1; m<M + 1; m++)
    {
        coef[2 * m + 0] = h[(M - m)].s.re;
        coef[2 * m + 1] = L_neg_l(h[(M - m)].s.im);

        coefB[2 * m + 0] = h[(M - m)].s.im;
        coefB[2 * m + 1] = h[(M - m)].s.re;
    }

    for (; m<M + 2; m++)
    {
        coef[2 * m + 0]  = coef[2 * m + 1]  = 0;
        coefB[2 * m + 0] = coefB[2 * m + 1] = 0;
    }


    for (m = 0; m<M + 2; m++)
    {
        delLine[2 * m + 0] = 0;
        delLine[2 * m + 1] = 0;
    }

    //
    // Initialize the filter instance.
    //

    cxfir->magic        = MAGIC;
    cxfir->M            = M;
    cxfir->coef         = coef;
    cxfir->delayLine    = delLine;
    cxfir->delayLen     = M + 2;
    cxfir->wrIx         = 0;

    return (cxfir);
} // cxfir16x16_init()

/* Put a chunk of input signal into the delay line and compute the filter
* response. */
void cxfir16x16_process( cxfir16x16_handle_t _cxfir,
                         complex_fract16 * restrict  y,
                   const complex_fract16 * restrict  x, int N)
{
    cxfir16x16_ptr_t cxfir = (cxfir16x16_ptr_t)_cxfir;

    const ae_int16x4 *          C;
          ae_int16x4 * restrict D_wr;
    const ae_int16x4 *          D_rd0;
    const ae_int16x4 *          D_rd1;
    const ae_int16x4 *          X;
          ae_int16   * restrict Y;
    
    ae_valign D_va1;
    ae_valign ay;

    ae_int16x4 d01, d12;
    ae_int64   q0r, q1r;
    ae_int64   q0i, q1i;
    ae_int64   z;
    ae_int16x4 c0, c0i;
    ae_f32x2   t0, t1;

    int M, wrIx;
    int m, n;

    if (N <= 0) return;
    NASSERT(cxfir && cxfir->magic == MAGIC && y && x);
    NASSERT_ALIGN8(x);

    M = cxfir->M;
    wrIx = cxfir->wrIx;
    NASSERT(!(M & 3) && !(N & 3));
    NASSERT_ALIGN8(cxfir->delayLine);
    NASSERT_ALIGN8(cxfir->coef);

    //
    // Setup pointers and circular delay line buffer.
    //

    D_wr = (ae_int16x4*)(cxfir->delayLine + 2 * wrIx);
    X    = (const ae_int16x4 *)x;
    Y    = (ae_int16     *)y;

    WUR_AE_CBEGIN0((uintptr_t)(cxfir->delayLine));
    WUR_AE_CEND0((uintptr_t)(cxfir->delayLine + 2 * cxfir->delayLen));
    z = AE_ZERO64();

    ay = AE_ZALIGN64();

    //
    // Break the input signal into 4-samples blocks. For each block, store 4
    // samples to the delay line and compute the filter response.
    //
    __Pragma("loop_count min=1")
    for (n = 0; n < (N >> 1); n++)
    {
        // Reset the coefficients pointer. Now it looks at the tap corresponding
        // to the oldest sample in the delay line.
        C = (const ae_int16x4*)cxfir->coef;

        // Load 2 complex input samples.
        AE_L16X4_IP(d01, X, +8);

        // Store 2 complex samples to the delay line buffer with circular address update.
        AE_S16X4_XC(d01, D_wr, +8);

        D_rd0 = D_wr;
        D_rd1 = (const ae_int16x4*)((int16_t *)D_wr + 2);

        // Zero the accumulators.
        q0r = z; q1r = z;
        q0i = z; q1i = z;

        AE_LA16X4POS_PC(D_va1, D_rd1);

        for (m = 0; m < (M >> 1) + 1; m++)
        {
            AE_L16X4_XC(d01, D_rd0, +8);
            AE_LA16X4_IC(d12, D_va1, D_rd1);

            c0i = AE_L16X4_X(C,+2*2*(M + 2));
            AE_L16X4_IP(c0, C, +8);

            AE_MULAAAAQ16(q0r, d01, c0);
            AE_MULAAAAQ16(q1r, d12, c0);

            AE_MULAAAAQ16(q0i, d01, c0i);
            AE_MULAAAAQ16(q1i, d12, c0i);
        }

        t0 = AE_TRUNCA32X2F64S(q0r, q0i, 33);
        t1 = AE_TRUNCA32X2F64S(q1r, q1i, 33);
        AE_SA16X4_IP(AE_ROUND16X4F32SASYM(t0, t1), ay, castxcc(ae_int16x4, Y));
    }
    AE_SA64POS_FP(ay, Y);
    cxfir->wrIx = (int)((int16_t *)D_wr - cxfir->delayLine) >> 1;
} /* cxfir16x16_process */

