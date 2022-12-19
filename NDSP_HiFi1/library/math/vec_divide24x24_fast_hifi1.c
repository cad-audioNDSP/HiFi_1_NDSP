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

void vec_divide24x24_fast 
(
  int32_t * restrict        frac,
  int16_t *                   exp,
  const int32_t * restrict  x,
  const int32_t * restrict  y,
  int                       M
)
{
    ae_int32x2 X,Y,E;
    int n,N=M;
    const ae_int32x2  * restrict px;
    const ae_int32x2  * restrict py;
    ae_int32x2 * restrict pfWr;

    if(N<=0) return;
    NASSERT_ALIGN8(x);
    NASSERT_ALIGN8(y);
    NASSERT_ALIGN8(frac);
    NASSERT(N%4==0);
    /* take exponent and normalize inputs. Y is saved to the scratch */
    px=(const ae_int32x2*)x;
    py=(const ae_int32x2*)y;
    pfWr = (      ae_int32x2*)frac;
    __Pragma("loop_count min=1,factor=2")
    for (n=0; n<(N>>1); n++)
    {
        ae_int32x2 Xl,Xh,Yl,Yh,XX,YY;
        ae_int32x2 _0x400000=AE_MOVDA32(0x400000);
        ae_f32x2 f;
        xtbool2 sy;
        int expx,expy;
        /* normalization */
        AE_L32X2_IP(XX, px, sizeof(XX)); 
		XX=AE_SRAI32(XX,8);
        AE_L32X2_IP(YY, py, sizeof(YY)); 
		YY=AE_SRAI32(YY,8);

        expx = AE_NSAZ32_L(XX);
        expy = AE_NSAZ32_L(YY);
        Xl=AE_SLAA32S(XX,expx);
        Yl=AE_SLAA32S(YY,expy);
        exp[1]=(int16_t)(expy-expx+1);
        XX=AE_SEL32_LH(XX,XX);
        YY=AE_SEL32_LH(YY,YY);
        expx = AE_NSAZ32_L(XX);
        expy = AE_NSAZ32_L(YY);
        Xh=AE_SLAA32S(XX,expx);
        Yh=AE_SLAA32S(YY,expy);
        exp[0]=(int16_t)(expy-expx+1);
        XX=AE_SEL32_LL(Xh,Xl);
        YY=AE_SEL32_LL(Yh,Yl);
        exp+=2;
        sy=AE_LT32(YY,AE_ZERO32());
        YY=AE_INT32X2_ABS32S(YY);
        X=AE_SRAI32(YY,8);
        /* first approximation */
        Y=AE_SUB32(AE_MOVDA32((int32_t)0x00BAEC00),X); /* first approximation */
        X=AE_SLAI32(X,8);
            /* 3 iterations to achieve 1 LSB accuracy in mantissa */
        f=(_0x400000); AE_MULSFP32X2RAS(f,(X),(Y)); E=(f);
        E=AE_SLLI32(E,9);
        f=(Y); AE_MULAFP32X2RAS(f,(E),(Y)); Y=(f);
        f=(_0x400000); AE_MULSFP32X2RAS(f,(X),(Y)); E=(f);
        E=AE_SLLI32(E,9);
        f=(Y); AE_MULAFP32X2RAS(f,(E),(Y)); Y=(f);
        f=(_0x400000); AE_MULSFP32X2RAS(f,(X),(Y)); E=(f);
        E=AE_SLLI32(E,9);
        f=(Y); AE_MULAFP32X2RAS(f,(E),(Y)); Y=(f);
        /* restore original sign */
        X=AE_NEG32(Y);
        AE_MOVT32X2(Y,X,sy);
        Y=AE_SLLI32S(Y,8);
        /* multiply by X */
		Y=AE_MULFP32X2RAS(XX,Y);
        AE_S32X2_IP(Y,pfWr,sizeof(ae_int32x2));
    }
}

