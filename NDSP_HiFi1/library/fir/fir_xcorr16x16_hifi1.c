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
    Real data circular cross-correlation, 16x16-bit
*/

/*-------------------------------------------------------------------------
  Circular Correlation
  Estimates the circular cross-correlation between vectors x (of length N) 
  and y (of length M)  resulting in vector r of length N. It is a similar 
  to correlation but x is read in opposite direction.

  Precision: 
  16x16     16x16-bit data, 16-bit outputs
  24x24     24x24-bit data, 24-bit outputs
  32x16     32x16-bit data, 32-bit outputs
  32x32     32x32-bit data, 32-bit outputs
  f         floating point (both real and complex data)

  Input:
  x[N]          input data Q31,Q15 or floating point
  y[M]          input data Q31,Q15 or floating point
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

void fir_xcorr16x16(int16_t * restrict r,
    const int16_t * restrict x,
    const int16_t * restrict y,
    int N, int M)
{   
    /* code with Quad MAC option  */
    //
    // Circular cross-correlation algorithm:
    //
    //   r[n] = sum( x[mod(n+m,N)]*y[m] )
    //        m=0..M-1
    //
    //   where n = 0..N-1
    //
    const ae_int16x4 * pX0;
    const ae_int16x4 * pX1;
    const ae_int16x4 * pX2;
    const ae_int16x4 * pX3;
    const ae_int16x4 * S0;
    const ae_int16x4 * S0_1;
    const ae_int16x4 * pY;

    ae_int16x4 * restrict pR;

    ae_int64   q0, q1, q2, q3;
    ae_int64   q4, q5, q6, q7;
    ae_int16x4 x0, x1, x2, x3;
    ae_int16x4 y0;
    ae_f32x2   t0, t1;

    ae_valign ar1, ar2, ar3;

    int n, m;

    NASSERT(r);
    NASSERT(x);
    NASSERT(y);
    NASSERT_ALIGN(r, 8);
    NASSERT_ALIGN(x, 8);
    NASSERT_ALIGN(y, 8);
    NASSERT(M > 0 && M % 4 == 0);
    NASSERT(N > 0 && N % 4 == 0);

    //
    // Setup pointers and circular addressing for array x[N].
    //

    pX0 = (const ae_int16x4 *)x;
    pR  = (ae_int16x4 *)r;

    WUR_AE_CBEGIN0((uintptr_t)(x + 0));
    WUR_AE_CEND0((uintptr_t)(x + N));

    for (n = 0; n < (N & ~7); n += 8){
        pX1 = (const ae_int16x4 *)(x + n + 1);
        pX2 = (const ae_int16x4 *)(x + n + 2);
        pX3 = (const ae_int16x4 *)(x + n + 3);

        AE_LA16X4POS_PC(ar1, pX1);
        AE_LA16X4POS_PC(ar2, pX2);
        AE_LA16X4POS_PC(ar3, pX3);

        AE_L16X4_XC(x0, pX0, +8);
        AE_LA16X4_IC(x1, ar1, pX1);
        AE_LA16X4_IC(x2, ar2, pX2);
        AE_LA16X4_IC(x3, ar3, pX3);

        S0 = pX0;        

        pY = (const ae_int16x4 *)y;
        AE_L16X4_IP(y0, pY, 8);

        q0 = AE_MULZAAAAQ16(x0, y0);
        q1 = AE_MULZAAAAQ16(x1, y0);
        q2 = AE_MULZAAAAQ16(x2, y0);
        q3 = AE_MULZAAAAQ16(x3, y0); 

        AE_L16X4_XC(x0, pX0, +8);
        S0_1 = pX0;
        AE_LA16X4_IC(x1, ar1, pX1);
        AE_LA16X4_IC(x2, ar2, pX2);
        AE_LA16X4_IC(x3, ar3, pX3);

        q4 = AE_MULZAAAAQ16(x0, y0);
        q5 = AE_MULZAAAAQ16(x1, y0);
        q6 = AE_MULZAAAAQ16(x2, y0);
        q7 = AE_MULZAAAAQ16(x3, y0);

        for (m = 0; m < (M >> 2) - 1; m++){

            AE_L16X4_XC(x0, S0, +8);
            AE_L16X4_IP(y0, pY, 8);
            
            AE_MULAAAAQ16(q0, x0, y0);
            AE_MULAAAAQ16(q1, x1, y0);
            AE_MULAAAAQ16(q2, x2, y0);
            AE_MULAAAAQ16(q3, x3, y0);

            AE_L16X4_XC(x0, S0_1, 8);
            AE_LA16X4_IC(x1, ar1, pX1);
            AE_LA16X4_IC(x2, ar2, pX2);
            AE_LA16X4_IC(x3, ar3, pX3);

            AE_MULAAAAQ16(q4, x0, y0);
            AE_MULAAAAQ16(q5, x1, y0);
            AE_MULAAAAQ16(q6, x2, y0);
            AE_MULAAAAQ16(q7, x3, y0);
        }

        t0 = AE_TRUNCA32X2F64S(q0, q1, 33);
        t1 = AE_TRUNCA32X2F64S(q2, q3, 33);
        AE_S16X4_IP(AE_ROUND16X4F32SASYM(t0, t1), pR, 8);

        t0 = AE_TRUNCA32X2F64S(q4, q5, 33);
        t1 = AE_TRUNCA32X2F64S(q6, q7, 33);
        AE_S16X4_IP(AE_ROUND16X4F32SASYM(t0, t1), pR, 8);
    }

    if (N & 4)
    {
        pX1 = (const ae_int16x4 *)(x + n + 1);
        pX2 = (const ae_int16x4 *)(x + n + 2);
        pX3 = (const ae_int16x4 *)(x + n + 3);

        AE_LA16X4POS_PC(ar1, pX1);
        AE_LA16X4POS_PC(ar2, pX2);
        AE_LA16X4POS_PC(ar3, pX3);

        AE_L16X4_XC(x0, pX0, +8);
        AE_LA16X4_IC(x1, ar1, pX1);
        AE_LA16X4_IC(x2, ar2, pX2);
        AE_LA16X4_IC(x3, ar3, pX3);

        S0 = pX0;

        pY = (const ae_int16x4 *)y;
        AE_L16X4_IP(y0, pY, 8);

        q0 = AE_MULZAAAAQ16(x0, y0);
        q1 = AE_MULZAAAAQ16(x1, y0);
        q2 = AE_MULZAAAAQ16(x2, y0);
        q3 = AE_MULZAAAAQ16(x3, y0);

        for (m = 0; m < (M >> 2) - 1; m++){
            AE_L16X4_XC(x0, S0, +8);
            AE_LA16X4_IC(x1, ar1, pX1);
            AE_LA16X4_IC(x2, ar2, pX2);
            AE_LA16X4_IC(x3, ar3, pX3);

            AE_L16X4_IP(y0, pY, 8);
            AE_MULAAAAQ16(q0, x0, y0);
            AE_MULAAAAQ16(q1, x1, y0);
            AE_MULAAAAQ16(q2, x2, y0);
            AE_MULAAAAQ16(q3, x3, y0);
        }

        t0 = AE_TRUNCA32X2F64S(q0, q1, 33);
        t1 = AE_TRUNCA32X2F64S(q2, q3, 33);
        AE_S16X4_IP(AE_ROUND16X4F32SASYM(t0, t1), pR, 8);
    }
} /* fir_xcorr16x16() */
