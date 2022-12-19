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

#include <errno.h>

/* DSP Library API */
#include "NatureDSP_Signal.h"
/* Common helper macros. */
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(float32_t, scl_antilog2f, (float32_t x))

#else
/* Inline functions for floating-point exceptions and environment control. */
#include "__fenv.h"
/* Tables */
#include "expf_tbl.h"
#include "alog2f_tbl.h"
/* +/-Infinity, single precision */
#include "inff_tbl.h"
/* sNaN/qNaN, single precision. */
#include "nanf_tbl.h"

/* If non-zero, set errno and raise floating-point exceptions on errors. */
#define SCL_ANTILOG2F_ERRH     1

/*===========================================================================
  Scalar matematics:
  scl_antilog          Antilogarithm         
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

float32_t scl_antilog2f (float32_t x)
{
  /*
  * Reference C code:
  *
  *   int32_t t,y;
  *   int e;
  *
  *   e=STDLIB_MATH(floorf)(x);   // -149...128
  *   x=x-(float32_t)e;
  *   t=(int32_t)STDLIB_MATH(ldexpf)(x,31);
  *   // compute 2^t in Q30 where t is in Q31
  *   y=                                           expftbl_Q30[0];
  *   y=satQ31((((int64_t)t*y)+(1LL<<(31-1)))>>31)+expftbl_Q30[1];
  *   y=satQ31((((int64_t)t*y)+(1LL<<(31-1)))>>31)+expftbl_Q30[2];
  *   y=satQ31((((int64_t)t*y)+(1LL<<(31-1)))>>31)+expftbl_Q30[3];
  *   y=satQ31((((int64_t)t*y)+(1LL<<(31-1)))>>31)+expftbl_Q30[4];
  *   y=satQ31((((int64_t)t*y)+(1LL<<(31-1)))>>31)+expftbl_Q30[5];
  *   y=satQ31((((int64_t)t*y)+(1LL<<(31-1)))>>31)+expftbl_Q30[6];
  *   // convert back to the floating point
  *   x=STDLIB_MATH(ldexpf)((float32_t)y,e-30);
  *   return x;
  */

  /* Input value; saturated input value; integer part of input number. */
  xtfloatx2 x0, x1, x2;
  /* Output value; polynomial approximation */
  xtfloatx2 y0;//, q;
  /* Polynomial coeffs. */
  xtfloatx2 cf0, cf1, cf2, cf3, cf4, cf5, cf6;
  /* Fractional part; polynomial value. */
 // ae_int32x2 u0;
  /* Exponential parts. */
  ae_int32x2 e0, e1;
  /* Scale factors. */
  xtfloatx2 s0, s1;
  /* Auxiliary var. */
  xtfloatx2 t, n0;

#if SCL_ANTILOG2F_ERRH != 0
  /* Floating-point Status and Control Register values. */
  ae_int64 fstate;

  x0 = (xtfloatx2)x;
  
  if ( xtbool2_extract_0( XT_UN_SX2( x0, x0 ) ) )
  {
    __Pragma( "frequency_hint never" );
    errno = EDOM;
    return (qNaNf.f);
  }
#else  
  x0 = (xtfloatx2)x;  
  if ( xtbool2_extract_0( XT_UN_SX2( x0, x0 ) ) )
  {
    __Pragma( "frequency_hint never" );
    return (qNaNf.f);
  }
#endif

  x1 = XT_MAX_SX2(alog2fminmax[0].f, x0);
  x1 = XT_MIN_SX2(alog2fminmax[1].f, x1);

#if SCL_ANTILOG2F_ERRH != 0
	//__fenv_t fenv;
  /* Latch floating-point exception flags and clear exception enable bits. */
  //__feholdexcept(&fenv);

  /* Sample floating-point exception flags. */
  fstate = XT_AE_MOVVFCRFSR();
#endif

  x2 = XT_FIFLOOR_SX2(x1);
  e0 = XT_TRUNC_SX2(x2, 0);
  x2 = XT_SUB_SX2(x1, x2);

  /*
  * Compute 2^fract(x) in Q30, where x is in Q31. Use a combination of Estrin's
  * rule and Horner's method to evaluate the polynomial.
  */

  cf0 = expftbl_flaot[0];
  cf1 = expftbl_flaot[1];
  cf2 = expftbl_flaot[2];
  cf3 = expftbl_flaot[3];
  cf4 = expftbl_flaot[4];
  cf5 = expftbl_flaot[5];
  cf6 = expftbl_flaot[6];

  n0 = cf0; t = cf1; XT_MADD_SX2(t, x2, n0);
  n0 = t;   t = cf2; XT_MADD_SX2(t, x2, n0);
  n0 = t;   t = cf3; XT_MADD_SX2(t, x2, n0);
  n0 = t;   t = cf4; XT_MADD_SX2(t, x2, n0);
  n0 = t;   t = cf5; XT_MADD_SX2(t, x2, n0);
  n0 = t;   t = cf6; XT_MADD_SX2(t, x2, n0);


  /*
  * Calculate 2^int(x) * 2^fract(x)
  */

  e0 = AE_ADD32(e0, 254);
  e1 = AE_SRAI32(e0, 1);
  e0 = AE_SUB32(e0, e1);
  e0 = AE_SLAI32(e0, 23);
  e1 = AE_SLAI32(e1, 23);

  s0 = XT_AE_MOVXTFLOATX2_FROMINT32X2(e0);
  s1 = XT_AE_MOVXTFLOATX2_FROMINT32X2(e1);

  s0 = XT_MUL_SX2(s0, s1);
  y0 = XT_MUL_SX2(s0, t);

#if SCL_ANTILOG2F_ERRH != 0
  {
    /* Is input less than infinity; is output equal to infinity. */
    xtbool2 b_xfin, b_yinf;

    /* Restore floating-point exception flags and exception control bits. */
    //__fesetenv(&fenv);
	
	  /* Suppress spurious exception flags and restore original status flags. */
    XT_AE_MOVFCRFSRV( fstate );

    b_xfin = XT_OLT_SX2(x0, plusInff.f);
    b_yinf = XT_OEQ_SX2(y0, plusInff.f);

    /* Check for an overflow. */
    if (XT_ANDB(xtbool2_extract_0(b_xfin), xtbool2_extract_0(b_yinf)))
    {
      __Pragma("frequency_hint never");
      __feraiseexcept(FE_OVERFLOW);
      errno = ERANGE;
    }
  }
#endif

  return (XT_HIGH_S(y0));

} /* scl_antilog2f() */

#endif /* #if HAVE_VFPU */

