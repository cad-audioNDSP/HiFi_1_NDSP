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
    Real data circular cross-correlation, complex floating point, no requirements on vectors 
    length and alignment.
    C code optimized for HiFi1
*/

/*-------------------------------------------------------------------------
  Circular Correlation
  Estimates the circular cross-correlation between vectors x (of length N) 
  and y (of length M)  resulting in vector r of length N. It is a similar 
  to correlation but x is read in opposite direction.
  These functions implement the circular correlation algorithm described in
  the previous chapter with no limitations on x and y vectors length and
  alignment at the cost of increased processing complexity. In addition, this
  implementation variant requires scratch memory area.

  Precision: 
  16x16     16x16-bit data, 16-bit outputs
  24x24     24x24-bit data, 24-bit outputs
  32x16     32x16-bit data, 32-bit outputs
  32x32     32x32-bit data, 32-bit outputs
  f         floating point (both real and complex data)

  Input:
  s[]           Scratch memory, 
                FIR_XCORRA16X16_SCRATCH_SIZE( N, M ) 
                FIR_XCORRA24X24_SCRATCH_SIZE( N, M ) 
                FIR_XCORRA32X16_SCRATCH_SIZE( N, M ) 
                FIR_XCORRA32X32_SCRATCH_SIZE( N, M ) 
                FIR_XCORRAF_SCRATCH_SIZE( N, M )     
                CXFIR_XCORRAF_SCRATCH_SIZE( N, M )   
                bytes
  x[N]          input data Q31, Q15  or floating point
  y[M]          input data Q31, Q15 or floating point
  N             length of x
  M             length of y
  Output:
  r[N]          output data, Q31, Q15  or floating point

  Restrictions:
  x,y,r should not overlap
  s        - must be aligned on an 8-bytes boundary
  N,M      - must be >0
  N >= M-1 - minimum allowed length of vector x is the length of y minus one
-------------------------------------------------------------------------*/
/* Portable data types. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
/* Common utility and macros declarations. */
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, cxfir_xcorraf, (void * restrict s, complex_float  * restrict r, const complex_float  * restrict x, const complex_float  * restrict y, int N, int M))

#else
void cxfir_xcorraf( void * restrict s,
                    complex_float  * restrict r,
                    const complex_float  * restrict x,
                    const complex_float  * restrict y,
                    int N, int M 
                  )
{
    //
    // Circular cross-correlation algorithm:
    //
    //   r[n] = sum( x[mod(n+m,N)]*y[m] )
    //        m=0..M-1
    //
    //   where n = 0..N-1
    //
    xtfloatx2 A0, A1, A2, A3, X0, X1, X2, X3, Y;
    const xtfloatx2 * restrict pX;
    const xtfloatx2 * restrict pY;
    xtfloatx2 * restrict pR;

    int n, m;
    NASSERT(r);
    NASSERT(x);
    NASSERT(y);
    NASSERT_ALIGN(r, 8);
    NASSERT_ALIGN(x, 8);
    NASSERT_ALIGN(y, 8);
    (void)s;
    NASSERT(N > 0 && M > 0);
    NASSERT(N >= M - 1);

    pR = (xtfloatx2*)r;
    /* set circular buffer boundaries */
    WUR_AE_CBEGIN0((uintptr_t)(x + 0));
    WUR_AE_CEND0((uintptr_t)(x + N));

    for (n = 0; n < (N&~3); n += 4, x += 4)
    {
        pX = (const xtfloatx2 *)(x);
        pY = (const xtfloatx2 *)y;
        XT_LSX2XC(X0, pX, sizeof(X0));
        XT_LSX2XC(X1, pX, sizeof(X1));
        XT_LSX2XC(X2, pX, sizeof(X2));
        A0 = A1 = A2 = A3 = (xtfloatx2)0.0f;
        __Pragma("loop_count min=1")
            for (m = 0; m < M; m++)
            {
                XT_LSX2XC(X3, pX, sizeof(X3));
                XT_LSX2IP(Y, pY, sizeof(Y));
                XT_MADDCCONJ_S(A0, Y, X0);
                XT_MADDCCONJ_S(A1, Y, X1);
                XT_MADDCCONJ_S(A2, Y, X2);
                XT_MADDCCONJ_S(A3, Y, X3);
                X0 = X1; X1 = X2; X2 = X3;
            }
        XT_SSX2IP(A0, pR, sizeof(A0));
        XT_SSX2IP(A1, pR, sizeof(A0));
        XT_SSX2IP(A2, pR, sizeof(A0));
        XT_SSX2IP(A3, pR, sizeof(A0));
    }
    for (; n < (N); n++, x++)
    {
        pX = (const xtfloatx2 *)(x);
        pY = (const xtfloatx2 *)y;
        A0 = A1 = A2 = A3 = (xtfloatx2)0.0f;
        XT_LSX2XC(X0, pX, sizeof(X0));
        for (m = 0; m < (M&~1); m += 2)
        {
            XT_LSX2IP(Y, pY, sizeof(Y));
            XT_MADDMUX_S(A0, Y, X0, 4);
            XT_MADDMUX_S(A1, Y, X0, 5);
            XT_LSX2XC(X0, pX, sizeof(X0));

            XT_LSX2IP(Y, pY, sizeof(Y));
            XT_MADDMUX_S(A2, Y, X0, 4);
            XT_MADDMUX_S(A3, Y, X0, 5);
            XT_LSX2XC(X0, pX, sizeof(X0));
        }
        if (M & 1)
        {
            XT_LSX2IP(Y, pY, sizeof(Y));
            XT_MADDMUX_S(A0, Y, X0, 4);
            XT_MADDMUX_S(A1, Y, X0, 5);
        }
        A0 = A0 + A2;
        A1 = A1 + A3;
        A0 = A0 + A1;
        XT_SSX2IP(A0, pR, sizeof(A0));
    }
} /* cxfir_xcorraf() */

#endif /*HAVE_VFPU*/


