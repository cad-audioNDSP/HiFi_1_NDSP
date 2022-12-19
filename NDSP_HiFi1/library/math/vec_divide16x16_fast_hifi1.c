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
#include "NatureDSP_Signal.h"
#include "common.h"

/*===========================================================================
  Vector matematics:
  vec_divide           Division of Q31/Q15 Numbers
===========================================================================*/
/*-------------------------------------------------------------------------
  Division 
Fixed point routines perform pair-wise division of vectors written in Q31 or 
Q15 format. They return the fractional and exponential portion of the division 
result. Since the division may generate result greater than 1, it returns 
fractional portion frac in Q(31-exp) or or Q(15-exp) format and exponent 
exp so true division result in the Q0.31 may be found by shifting 
fractional part left by exponent value.
For division to 0, the result is not defined 

For fixed point finctions, mantissa accuracy is 2 LSB, so relative accuracy is:
vec_divide16x16, scl_divide16x16                   1.2e-4 
vec_divide24x24, scl_divide32x32, scl_divide24x24  4.8e-7 
vec_divide32x32                                    1.8e-9

Floating point routines operate with standard floating point numbers. Functions 
return +/-infinity in case of overflow and provide accuracy of 2 ULP.

Two versions of routines are available: regular versions (vec_divide32x32, 
vec_divide24x24, vec_divide16x16) work with arbitrary arguments, faster 
versions (vec_divide32x3_fast, vec_divide24x24_fast, vec_divide16x16_fast) 
apply some restrictions.

  Precision: 
  32x32  32-bit inputs, 32-bit output. 
  24x24  24-bit inputs, 24-bit output. 
  16x16  16-bit inputs, 16-bit output. 
  f      floating point

  Input:
  x[N]    nominator,Q31, Q15, floating point
  y[N]    denominator,Q31, Q15, floating point
  N       length of vectors
  Output:
  frac[N] fractional parts of result, Q(31-exp) or Q(15-exp) (for fixed 
          point functions)
  exp[N]  exponents of result (for fixed point functions) 
  z[N]    result (for floating point function)

  Restriction:
  For regular versions (vec_divide32x32, vec_divide24x24, 
  vec_divide16x16, vec_dividef) :
  x,y,frac,exp should not overlap

  For faster versions (vec_divide32x3_fast, vec_divide24x24_fast, 
  vec_divide16x16_fast) :
  x,y,frac,exp should not overlap
  x, y, frac to be aligned by 8-byte boundary, N - multiple of 4.

  Scalar versions:
  ----------------
  Return packed value (for fixed point functions): 
  scl_divide24x24(),scl_divide32x32():
  bits 23:0 fractional part
  bits 31:24 exponent
  scl_divide16x16():
  bits 15:0 fractional part
  bits 31:16 exponent
-------------------------------------------------------------------------*/

void vec_divide16x16_fast 
(
  int16_t *       restrict  frac,
  int16_t *       restrict  exp,
  const int16_t * restrict  x,
  const int16_t * restrict  y,
  int M)
{
#define SCR_SZ (MAX_ALLOCA_SZ/sizeof(int16_t))
    int16_t ALIGN(8) scr[SCR_SZ];   /* local scratch */
    xtbool4 mask5;
    int n, N;
    const ae_int16x4 * restrict px;
    const ae_int16x4 * restrict py;
    ae_int16x4 * restrict pf;
    ae_int16x4 * restrict ps;

    const ae_int16x4 * restrict pfRd;
    ae_int16x4 * restrict pfWr;
    const ae_int16x4 * restrict psRd16;

    static const int16_t ALIGN(8) _0101[] = { 0,1,0,1 };
    mask5 = AE_EQ16(AE_L16X4_I((const ae_int16x4*)_0101, 0), AE_ZERO16());
    while (M > 0)
    {
        N = XT_MIN(SCR_SZ, M); /* size of portion */
        NASSERT_ALIGN(x, 8);
        NASSERT_ALIGN(y, 8);
        NASSERT_ALIGN(frac, 8);
        NASSERT(N % 4 == 0);
        /* take exponent and normalize inputs. Y is saved to the scratch */
        px = (const ae_int16x4 *)x;
        py = (const ae_int16x4 *)y;
        pf = (ae_int16x4*)frac;
        ps = (ae_int16x4*)scr;
        for (n = 0; n < (N >> 2); n++)
        {
            ae_int16x4 X0, X1, X2, X3, Y0, Y1, Y2, Y3;
            ae_int16x4 X, Y;
            int expx, expy;
            AE_L16X4_IP(X, px, sizeof(X));
            AE_L16X4_IP(Y, py, sizeof(Y));
            expx = AE_NSAZ16_0(X);
            expy = AE_NSAZ16_0(Y);
            X0 = AE_SLAA16S(X, expx);
            Y0 = AE_SLAA16S(Y, expy);
            exp[3] = (int16_t)(expy - expx + 1);
            X = AE_SEL16_4321(X, X);
            Y = AE_SEL16_4321(Y, Y);

            expx = AE_NSAZ16_0(X);
            expy = AE_NSAZ16_0(Y);
            X1 = AE_SLAA16S(X, expx);
            Y1 = AE_SLAA16S(Y, expy);
            exp[2] = (int16_t)(expy - expx + 1);
            X = AE_SEL16_4321(X, X);
            Y = AE_SEL16_4321(Y, Y);

            expx = AE_NSAZ16_0(X);
            expy = AE_NSAZ16_0(Y);
            X2 = AE_SLAA16S(X, expx);
            Y2 = AE_SLAA16S(Y, expy);
            exp[1] = (int16_t)(expy - expx + 1);
            X = AE_SEL16_4321(X, X);
            Y = AE_SEL16_4321(Y, Y);

            expx = AE_NSAZ16_0(X);
            expy = AE_NSAZ16_0(Y);
            X3 = AE_SLAA16S(X, expx);
            Y3 = AE_SLAA16S(Y, expy);

            X = AE_SEL16_6420(X2, X0);
            X1 = AE_SEL16_6420(X3, X1);
            X1 = AE_SEL16_6543(X1, X1);
            AE_MOVT16X4(X, X1, mask5);
            AE_S16X4_IP(X, pf, sizeof(X));
            Y = AE_SEL16_6420(Y2, Y0);
            Y1 = AE_SEL16_6420(Y3, Y1);
            Y1 = AE_SEL16_6543(Y1, Y1);
            AE_MOVT16X4(Y, Y1, mask5);
            AE_S16X4_IP(Y, ps, sizeof(Y));

            exp[0] = (int16_t)(expy - expx + 1);
            exp += 4;
        }


        pfRd = (const ae_int16x4*)(frac);
        ae_valign align1 = AE_LA64_PP(pfRd);
        pfWr = (ae_int16x4*)(frac);
        ae_valign align2 = AE_ZALIGN64();

        psRd16 = (const ae_int16x4*)(scr);

        for (n = 0; n < (N >> 2); n++)
        {
            ae_int32x2 Y, E, Z;
            ae_int16x4 P;
            ae_f32x2 f;
            xtbool2 sy,sy1;
            ae_int16x4 X;
            ae_int16x4 rd;
            ae_int32x2 tX_temp,tY,temp1,temp2;
            ae_int32x2 _0x400000 = AE_MOVDA32(0x400000);

            AE_L16X4_IP(rd, psRd16, 8);
            X = AE_ABS16S(rd);

            tX_temp=AE_SEXT32X2D16_32(X);
            tY = AE_SLAI32(tX_temp, 8);
            Y = AE_SUB32(AE_MOVDA32((int32_t)0x00BAEC00), tY);

            temp1=AE_SEXT32X2D16_32(rd);
            temp2=AE_SEXT32X2D16_10(rd);
            sy=AE_LT32(temp1, AE_ZERO32());
            sy1=AE_LT32(temp2, AE_ZERO32());

            /* 3 iterations to achieve 1 LSB accuracy in mantissa */
            f = AE_MOVF32X2_FROMINT32X2(_0x400000);
            AE_MULSFP32X16X2RAS_H(f, AE_MOVF32X2_FROMINT32X2(Y),AE_MOVF16X4_FROMINT16X4(X));
            E = AE_MOVINT32X2_FROMF32X2(f);
            E = AE_SLLI32(E, 1+8);
            f = AE_MOVF32X2_FROMINT32X2(Y); AE_MULAFP32X2RAS(f, AE_MOVF32X2_FROMINT32X2(E), AE_MOVF32X2_FROMINT32X2(Y)); Y = AE_MOVINT32X2_FROMF32X2(f);
            f = AE_MOVF32X2_FROMINT32X2(_0x400000);

            AE_MULSFP32X16X2RAS_H(f, AE_MOVF32X2_FROMINT32X2(Y),AE_MOVF16X4_FROMINT16X4(X));
            E = AE_MOVINT32X2_FROMF32X2(f);
            E = AE_SLLI32(E, 1+8);
            f = AE_MOVF32X2_FROMINT32X2(Y); AE_MULAFP32X2RAS(f, AE_MOVF32X2_FROMINT32X2(E), AE_MOVF32X2_FROMINT32X2(Y)); Y = AE_MOVINT32X2_FROMF32X2(f);
            f = AE_MOVF32X2_FROMINT32X2(_0x400000);

            AE_MULSFP32X16X2RAS_H(f, AE_MOVF32X2_FROMINT32X2(Y),AE_MOVF16X4_FROMINT16X4(X));
            E = AE_MOVINT32X2_FROMF32X2(f);
            E = AE_SLLI32(E, 1+8);
            f = AE_MOVF32X2_FROMINT32X2(Y); AE_MULAFP32X2RAS(f, AE_MOVF32X2_FROMINT32X2(E), AE_MOVF32X2_FROMINT32X2(Y)); Y = AE_MOVINT32X2_FROMF32X2(f);
            
            /* restore original sign */
            Z = AE_NEG32(Y);
            AE_MOVT32X2(Y, Z, sy);
            Y = AE_SLAI32S(Y, 8);
            /* multiply by X */

            AE_LA16X4_IP(P,align1,pfRd);
            ae_int32x2 Z0 = AE_MULFP32X16X2RAS_H(AE_MOVF32X2_FROMINT32X2(Y),P);


            tX_temp=AE_SEXT32X2D16_10(X);
            tY = AE_SLAI32(tX_temp, 8);
            Y = AE_SUB32(AE_MOVDA32((int32_t)0x00BAEC00), tY);

            f = AE_MOVF32X2_FROMINT32X2(_0x400000);
            AE_MULSFP32X16X2RAS_L(f, AE_MOVF32X2_FROMINT32X2(Y),AE_MOVF16X4_FROMINT16X4(X));
            E = AE_MOVINT32X2_FROMF32X2(f);
            E = AE_SLLI32(E, 1+8);
            f = AE_MOVF32X2_FROMINT32X2(Y); AE_MULAFP32X2RAS(f, AE_MOVF32X2_FROMINT32X2(E), AE_MOVF32X2_FROMINT32X2(Y)); Y = AE_MOVINT32X2_FROMF32X2(f);
            f = AE_MOVF32X2_FROMINT32X2(_0x400000);
            AE_MULSFP32X16X2RAS_L(f, AE_MOVF32X2_FROMINT32X2(Y),AE_MOVF16X4_FROMINT16X4(X));
            E = AE_MOVINT32X2_FROMF32X2(f);
            E = AE_SLLI32(E, 1+8);
            f = AE_MOVF32X2_FROMINT32X2(Y); AE_MULAFP32X2RAS(f, AE_MOVF32X2_FROMINT32X2(E), AE_MOVF32X2_FROMINT32X2(Y)); Y = AE_MOVINT32X2_FROMF32X2(f);
            f = AE_MOVF32X2_FROMINT32X2(_0x400000);
            AE_MULSFP32X16X2RAS_L(f, AE_MOVF32X2_FROMINT32X2(Y),AE_MOVF16X4_FROMINT16X4(X));
            E = AE_MOVINT32X2_FROMF32X2(f);
            E = AE_SLLI32(E, 1+8);
            f = AE_MOVF32X2_FROMINT32X2(Y); AE_MULAFP32X2RAS(f, AE_MOVF32X2_FROMINT32X2(E), AE_MOVF32X2_FROMINT32X2(Y)); Y = AE_MOVINT32X2_FROMF32X2(f);

            /* restore original sign */
            Z = AE_NEG32(Y);
            AE_MOVT32X2(Y, Z, sy1);
            Y = AE_SLAI32S(Y, 8);
            /* multiply by X */
            ae_int32x2 Z1 = AE_MULFP32X16X2RAS_L(AE_MOVF32X2_FROMINT32X2(Y),P);

            ae_int16x4 res = AE_TRUNC16X4F32(Z0,Z1);
            AE_SA16X4_IP(res,align2,pfWr);

        }
        AE_SA64POS_FP(align2,pfWr);

        /* process next portion */
        M -= N;
        x += N;
        y += N;
        frac += N;
        exp += N;
    }
}

