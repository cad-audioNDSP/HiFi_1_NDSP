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

#if !(HAVE_VFPU)
DISCARD_FUN(void, vec_antilog2f, (float32_t * restrict y, const float32_t* restrict x, int N))

#else
#include "alog2f_tbl.h"
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

void vec_antilog2f( float32_t * restrict y, const float32_t* restrict x, int N )
{
  /*
    int32_t t,y;
    int e;

    if(isnan(x)) return x;
    if (x>alog2fminmax[1].f) x= alog2fminmax[1].f;
    if (x<alog2fminmax[0].f) x= alog2fminmax[0].f;

    e=STDLIB_MATH(floorf)(x);    -149...128 
    x = x - (float32_t)e;
    t = (int32_t)STDLIB_MATH(ldexpf)(x, 31);
     compute 2^t in Q30 where t is in Q31 
    y = expftbl_Q30[0];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftbl_Q30[1];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftbl_Q30[2];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftbl_Q30[3];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftbl_Q30[4];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftbl_Q30[5];
    y = satQ31((((int64_t)t*y) + (1LL << (31 - 1))) >> 31) + expftbl_Q30[6];
     convert back to the floating point 
    x = STDLIB_MATH(ldexpf)((float32_t)y, e - 30);
    return x;
  */

  const xtfloatx2 *          X   = (xtfloatx2*)x;
        xtfloatx2 * restrict Y   = (xtfloatx2*)y;
  const xtfloat  *           TBL = (xtfloat *)expftbl_flaot;

  ae_valign X_va, Y_va;
  
  xtfloatx2 x0, y0, y1, n0, f0;
  xtfloatx2 tb0, tb1, tb2, tb3, tb4, tb5, tb6;
  ae_int32x2 u0, u1, e0, e1;
  xtbool2 b_nan;

  int n;

  if ( N<=0 ) return;

  X_va = AE_LA64_PP(X);
  Y_va = AE_ZALIGN64();

  tb0 = XT_LSI( TBL, 0*4 );
  tb1 = XT_LSI( TBL, 1*4 );
  tb2 = XT_LSI( TBL, 2*4 );
  tb3 = XT_LSI( TBL, 3*4 );
  tb4 = XT_LSI( TBL, 4*4 );
  tb5 = XT_LSI( TBL, 5*4 );
  tb6 = XT_LSI( TBL, 6*4 );

  for ( n=0; n<(N>>1); n++ )
  {
    XT_LASX2IP(x0, X_va, X);

    x0 = XT_MAX_SX2(alog2fminmax[0].f, x0);
    x0 = XT_MIN_SX2(alog2fminmax[1].f, x0);

    y0 = XT_FIFLOOR_SX2(x0);
    e0 = XT_TRUNC_SX2(y0, 0);
    x0 = XT_SUB_SX2(x0, y0);
    
    n0 = tb0; f0 = tb1;
    XT_MADD_SX2(f0, x0, n0); n0 = f0; f0 = tb2;
    XT_MADD_SX2(f0, x0, n0); n0 = f0; f0 = tb3;
    XT_MADD_SX2(f0, x0, n0); n0 = f0; f0 = tb4;
    XT_MADD_SX2(f0, x0, n0); n0 = f0; f0 = tb5;
    XT_MADD_SX2(f0, x0, n0); n0 = f0; f0 = tb6;
    XT_MADD_SX2(f0, x0, n0); x0 = f0;

    b_nan = XT_UN_SX2(y0, y0);
    XT_MOVT_SX2(x0, qNaNf.f, b_nan);

    e0 = AE_ADD32( e0, 254 );
    e1 = AE_SRAI32(e0, 1);
    e0 = AE_SUB32(e0, e1);

    u0 = AE_SLAI32(e0, 23);
    u1 = AE_SLAI32(e1, 23);
    y0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(u0);
    y1 = XT_AE_MOVXTFLOATX2_FROMINT32X2(u1);

    y0 = XT_MUL_SX2(y0, y1);
    y0 = XT_MUL_SX2(x0, y0);

    XT_SASX2IP(y0, Y_va, Y);
  }

  XT_SASX2POSFP(Y_va, Y);

  if ( N&1 )
  {
    x0 = XT_LSI( (xtfloat*)X, 0 );

    x0 = XT_MAX_SX2(alog2fminmax[0].f, x0);
    x0 = XT_MIN_SX2(alog2fminmax[1].f, x0);

    y0 = XT_FIFLOOR_SX2(x0);
    e0 = XT_TRUNC_SX2(y0, 0);
    x0 = XT_SUB_SX2(x0, y0);
    
    n0 = tb0; f0 = tb1;
	  XT_MADD_SX2(f0, x0, n0); n0 = f0; f0 = tb2;
    XT_MADD_SX2(f0, x0, n0); n0 = f0; f0 = tb3;
    XT_MADD_SX2(f0, x0, n0); n0 = f0; f0 = tb4;
    XT_MADD_SX2(f0, x0, n0); n0 = f0; f0 = tb5;
    XT_MADD_SX2(f0, x0, n0); n0 = f0; f0 = tb6;
    XT_MADD_SX2(f0, x0, n0); x0 = f0;

    b_nan = XT_UN_SX2(y0, y0);
    XT_MOVT_SX2(x0, qNaNf.f, b_nan);

    e0 = AE_ADD32( e0, 254 );
    e1 = AE_SRAI32(e0, 1);
    e0 = AE_SUB32(e0, e1);

    u0 = AE_SLAI32(e0, 23);
    u1 = AE_SLAI32(e1, 23);
    y0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(u0);
    y1 = XT_AE_MOVXTFLOATX2_FROMINT32X2(u1);

    y0 = XT_MUL_SX2(y0, y1);
    y0 = XT_MUL_SX2(x0, y0);

    XT_SSI( y0, (xtfloat*)Y, 0 );
  }

} /* vec_antilog2f() */
#endif /* #if HAVE_VFPU */

