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
    Real data circular convolution, floating point
    C code optimized for HiFi1
*/

/*-------------------------------------------------------------------------
  Circular Convolution
  Performs circular convolution between vectors x (of length N) and y (of 
  length M)  resulting in vector r of length N.

  Precision: 
  16x16  16x16-bit data, 16-bit outputs
  24x24  24x24-bit data, 24-bit outputs
  32x16  32x16-bit data, 32-bit outputs (both real and complex)
  32x32  32x32-bit data, 32-bit outputs 
  f      floating point 

  Input:
  x[N]          input data (Q31,Q15 or floating point)
  y[M]          input data (Q31,Q15 or floating point)
  N             length of x
  M             length of y
  Output:
  r[N]          output data,Q31,Q15 or floating point

  Restriction:
  x,y,r should not overlap
  x,y,r - aligned on an 8-bytes boundary
  N,M   - multiples of 4 and >0
-------------------------------------------------------------------------*/

/* Portable data types. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
/* Common utility and macros declarations. */
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, fir_convolf, (float32_t * restrict r,
            const float32_t * restrict x,
            const float32_t * restrict y,
            int N, int M))

#else
void fir_convolf(float32_t * restrict r,
    const float32_t * restrict x,
    const float32_t * restrict y,
    int N, int M)
{
    //
    // Circular convolution algorithm:
    //
    //   r[n] = sum( x[mod(n-m,N)]*y[m] )
    //        m=0..M-1
    //
    //   where n = 0..N-1
    //

    xtfloatx2 A10, A32, A54, A76, X10, X32, X0_, X21, Y0, Y1;
    const xtfloatx2 * restrict pX;
    const xtfloatx2 * restrict pX1;
    const ae_int32* restrict pY;
    xtfloatx2 * restrict pR;
    ae_valign ax1;
    int n, m;

    NASSERT(r);
    NASSERT(x);
    NASSERT(y);
    NASSERT_ALIGN(r, 8);
    NASSERT_ALIGN(x, 8);
    NASSERT_ALIGN(y, 8);
    NASSERT(M > 0 && N > 0);
    NASSERT(M % 4 == 0 && N % 4 == 0);

    if (N <= 0 || M <= 0) return;
    pR = (xtfloatx2*)r;
    /* set circular buffer boundaries */
    WUR_AE_CBEGIN0((uintptr_t)(x + 0));
    WUR_AE_CEND0((uintptr_t)(x + N));

    x += 2;
    for (n = 0; n < (N >> 2); n++, x += 4)
    {
        A10 = A32 = A54 = A76 = (xtfloat)0.f;
        pY = (const ae_int32*)y;
        pX = pX1 = (const xtfloatx2*)(x);
        AE_LA32X2NEG_PC(ax1, castxcc(const ae_int32x2, pX1));
        XT_LSX2RIC(X32, pX);
        XT_LASX2RIC(X21, ax1, pX1);
        __Pragma("loop_count min=1");
        for (m = 0; m < (M >> 1); m++)
        {
            ae_int32x2 tmp;
            XT_LSX2RIC(X10, pX);
            XT_LASX2RIC(X0_, ax1, pX1);
            AE_L32_IP(tmp, pY, sizeof(float32_t)); Y0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(tmp);
            AE_L32_IP(tmp, pY, sizeof(float32_t)); Y1 = XT_AE_MOVXTFLOATX2_FROMINT32X2(tmp);
            XT_MADD_SX2(A10, X10, Y0);
            XT_MADD_SX2(A32, X32, Y0);
            XT_MADD_SX2(A54, X0_, Y1);
            XT_MADD_SX2(A76, X21, Y1);
            X32 = X10;
            X21 = X0_;
        }
        A10 = XT_ADD_SX2(A10, A54);
        A32 = XT_ADD_SX2(A32, A76);
        A10 = XT_SEL32_LH_SX2(A10, A10);
        A32 = XT_SEL32_LH_SX2(A32, A32);
        XT_SSX2IP(A10, pR, sizeof(A10));
        XT_SSX2IP(A32, pR, sizeof(A32));
    }
} /* fir_convolf() */

#endif /*HAVE_VFPU*/

