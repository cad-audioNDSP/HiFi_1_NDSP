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

/* DSP Library API */
#include "NatureDSP_Signal.h"
/* Common helper macros. */
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, vec_antilognf, (float32_t * restrict y, const float32_t* restrict x, int N))

#else
/* Tables */
#include "expf_tbl.h"
/* sNaN/qNaN, single precision. */
#include "nanf_tbl.h"

/*===========================================================================
  Vector matematics:
  vec_antilog          Antilogarithm         
===========================================================================*/

/*-------------------------------------------------------------------------
  Antilogarithm
  These routines calculate antilogarithm (base2, natural and base10). 32 
  and 24-bit fixed-point functions accept inputs in Q6.25 and form outputs 
  in Q16.15 format and return 0x7FFFFFFF in case of overflow and 0 in case 
  of underflow. 16-bit fixed-point functions accept inputs in Q3.12 and 
  form outputs in Q8.7 format and return 0x7FFF in case of overflow and 
  0 in case of underflow.

  Precision:
  16x16  16-bit inputs, 16-bit outputs. Accuracy: 2 LSB
  32x32  32-bit inputs, 32-bit outputs. Accuracy: 8*e-6*y+1LSB
  24x24  24-bit inputs, 24-bit outputs. Accuracy: 8*e-6*y+1LSB
  f      floating point: Accuracy: 2 ULP
  NOTE:
  1.  Although 32 and 24 bit functions provide the similar accuracy, 32-bit
      functions have better input/output resolution (dynamic range).
  2.  Floating point functions are compatible with standard ANSI C routines 
      and set errno and exception flags accordingly.

  Input:
  x[N]  input data,Q6.25 (for 32 and 24-bit functions), Q3.12 (for 16-bit 
        functions) or floating point
  N     length of vectors
  Output:
  y[N]  output data,Q16.15 (for 32 and 24-bit functions), Q8.7 (for 16-bit 
        functions) or floating point  

  Restriction:
  x,y should not overlap

  Scalar versions:
  ----------------
  fixed point functions return result, Q16.15 (for 32 and 24-bit functions), 
  Q8.7 (for 16-bit functions) 

-------------------------------------------------------------------------*/
void vec_antilognf( float32_t * restrict y, const float32_t* restrict x, int N )
{
  /*
    int32_t t, y;
    int e;
    int64_t a;

    if (isnan(x)) return x;
    if (x>expfminmax[1].f) x = expfminmax[1].f;
    if (x<expfminmax[0].f) x = expfminmax[0].f;

    / scale input to 1/ln(2) and convert to Q31 /
    x = frexpf(x, &e);

    t = (int32_t)STDLIB_MATH(ldexpf)(x, e + 24);
    a = ((int64_t)t*invln2_Q30) >> 22; / Q24*Q30->Q32 /
    t = ((uint32_t)a) >> 1;
    e = (int32_t)(a >> 32);
    / compute 2^t in Q30 where t is in Q31 /
    y = expftblpq[0];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftblpq[1];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftblpq[2];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftblpq[3];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftblpq[4];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftblpq[5];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftblpq[6];
    / convert back to the floating point /
    x = STDLIB_MATH(ldexpf)((float32_t)y, e - 30);
  */

  const xtfloatx2 *          X   = (xtfloatx2*)x;
        xtfloatx2 * restrict Y   = (xtfloatx2*)y;
  const xtfloat  *          TBL  = (xtfloat *)expftbl_flaot;

  ae_valign X_va, Y_va;
  
  xtfloatx2 x0, y0, y1;
  xtfloatx2 tb0, tb1, tb2, tb3, tb4, tb5, tb6, u0f;
  ae_int64 wh, wl, wh1, wl1, wh0, wl0;
  ae_int32x2 e0, e1, u0;
  xtfloatx2 f0, n0;
  xtbool2 b_nan;

  int n;

  if ( N<=0 ) return;

  tb0 = XT_LSI( TBL, 0*4 );
  tb1 = XT_LSI( TBL, 1*4 );
  tb2 = XT_LSI( TBL, 2*4 );
  tb3 = XT_LSI( TBL, 3*4 );
  tb4 = XT_LSI( TBL, 4*4 );
  tb5 = XT_LSI( TBL, 5*4 );
  tb6 = XT_LSI( TBL, 6*4 );
  
  X_va = AE_LA64_PP(X);
  Y_va = AE_ZALIGN64();

  for ( n=0; n<(N>>1); n++ )
  {
    XT_LASX2IP(x0, X_va, X);

    b_nan = XT_UN_SX2(x0, x0);

	u0 = XT_TRUNC_SX2(x0, 24);
    wh = AE_MUL32_HH(u0, invln2_Q30);
    wl = AE_MUL32_LL(u0, invln2_Q30);
    wh0 = AE_SRAI64(wh, 22);
    wl0 = AE_SRAI64(wl, 22);
    e0 = AE_TRUNCI32X2F64S(wh0, wl0, 0);
    wh1 = AE_SLLI64(wh0, 32);
    wl1 = AE_SLLI64(wl0, 32);
    u0 = AE_TRUNCI32X2F64S(wh1, wl1, 0);
    u0 = AE_SRLI32(u0, 1);

	u0f = XT_FLOAT_SX2(u0, 31);

    n0 = tb0; f0 = tb1;
    XT_MADD_SX2(f0, u0f, n0); n0 = f0; f0 = tb2;
    XT_MADD_SX2(f0, u0f, n0); n0 = f0; f0 = tb3;
    XT_MADD_SX2(f0, u0f, n0); n0 = f0; f0 = tb4;
    XT_MADD_SX2(f0, u0f, n0); n0 = f0; f0 = tb5;
    XT_MADD_SX2(f0, u0f, n0); n0 = f0; f0 = tb6;
    XT_MADD_SX2(f0, u0f, n0); x0 = f0;

    e0 = AE_ADD32( e0, 254 );
    e1 = AE_SRAI32(e0, 1);
    e0 = AE_SUB32(e0, e1);
    e0 = AE_SLAI32(e0, 23);
    e1 = AE_SLAI32(e1, 23);
    y0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(e0);
    y1 = XT_AE_MOVXTFLOATX2_FROMINT32X2(e1);

    XT_MOVT_SX2(y1, qNaNf.f, b_nan);

    y0 = XT_MUL_SX2(y0, y1);
    y0 = XT_MUL_SX2(x0, y0);

    XT_SASX2IP(y0, Y_va, Y);
  }

  XT_SASX2POSFP(Y_va, Y);

  if ( N&1 )
  {
    x0 = XT_LSI( (xtfloat*)X, 0 );

    b_nan = XT_UN_SX2(x0, x0);

	u0 = XT_TRUNC_SX2(x0, 24);
    wh = AE_MUL32_HH(u0, invln2_Q30);
    wl = AE_MUL32_LL(u0, invln2_Q30);
    wh0 = AE_SRAI64(wh, 22);
    wl0 = AE_SRAI64(wl, 22);
    e0 = AE_TRUNCI32X2F64S(wh0, wl0, 0);
    wh1 = AE_SLLI64(wh0, 32);
    wl1 = AE_SLLI64(wl0, 32);
    u0 = AE_TRUNCI32X2F64S(wh1, wl1, 0);
    u0 = AE_SRLI32(u0, 1);
	
	u0f = XT_FLOAT_SX2(u0, 31);

    n0 = tb0; f0 = tb1;
	XT_MADD_SX2(f0, u0f, n0); n0 = f0; f0 = tb2;
    XT_MADD_SX2(f0, u0f, n0); n0 = f0; f0 = tb3;
    XT_MADD_SX2(f0, u0f, n0); n0 = f0; f0 = tb4;
    XT_MADD_SX2(f0, u0f, n0); n0 = f0; f0 = tb5;
    XT_MADD_SX2(f0, u0f, n0); n0 = f0; f0 = tb6;
    XT_MADD_SX2(f0, u0f, n0); x0 = f0;

    e0 = AE_ADD32( e0, 254 );
    e1 = AE_SRAI32(e0, 1);
    e0 = AE_SUB32(e0, e1);
    e0 = AE_SLAI32(e0, 23);
    e1 = AE_SLAI32(e1, 23);
    y0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(e0);
    y1 = XT_AE_MOVXTFLOATX2_FROMINT32X2(e1);

    XT_MOVT_SX2(y1, qNaNf.f, b_nan);

    y0 = XT_MUL_SX2(y0, y1);
    y0 = XT_MUL_SX2(x0, y0);

    XT_SSI( y0, (xtfloat*)Y, 0 );
  }

}
#endif /* #if HAVE_VFPU */

