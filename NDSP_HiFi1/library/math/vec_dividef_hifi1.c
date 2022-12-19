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
#include "NatureDSP_types.h"
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(  void, vec_dividef, (float32_t *restrict z, 
              const float32_t * restrict x, const float32_t * restrict y, int N ))

#else
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
#if 0 //HAVE_VFPU -Giving run-time failure.
void vec_dividef (  float32_t *restrict z, 
                    const float32_t * restrict x, const float32_t * restrict y, int N )
{
    int n;
    NASSERT(x);
    NASSERT(y);
    NASSERT(z);
    xtfloatx2 *restrict pz =  (xtfloatx2 *)z;
    ae_valign a1 = AE_ZALIGN64();
    const xtfloatx2 *restrict px =  (const xtfloatx2 *)x;
    ae_valign a2 = AE_LA64_PP(px);
    const xtfloatx2 *restrict py =  (const xtfloatx2 *)y;
    ae_valign a3 = AE_LA64_PP(py);
    xtfloatx2 vxf0,vyf0,vzf0;
    xtfloat vxf,vyf,vzf;
    for (n=0; n<(N>>1); n++)
    {
        XT_LASX2IP(vxf0,a2,px);
        XT_LASX2IP(vyf0,a3,py);
        vzf0  = XT_DIV_SX2(vxf0,vyf0);
        XT_SASX2IP(vzf0,a1,pz);
    }
    if(N&1)
    {
        XT_LSIP(vxf,(xtfloat *)px,sizeof(float32_t));
        XT_LSIP(vyf,(xtfloat *)py,sizeof(float32_t));
        vzf  = XT_DIV_S(vxf,vyf);
        XT_SSIP(vzf,(xtfloat *)pz,sizeof(float32_t));
    }
    XT_SASX2POSFP(a1,pz);
}
#else
void vec_dividef  ( float32_t *restrict z, 
                    const float32_t * restrict x, const float32_t * restrict y, int N )
{
    int n;
    NASSERT(x);
    NASSERT(y);
    NASSERT(z);
    for (n=0; n<N; n++)
    {
        z[n]=XT_DIV_S(x[n],y[n]);
    }
}
#endif 

#endif  /* #if HAVE_VFPU */
