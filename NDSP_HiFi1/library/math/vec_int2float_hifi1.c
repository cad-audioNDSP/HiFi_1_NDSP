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
DISCARD_FUN(void, vec_int2float, (float32_t  * restrict y, const int32_t  * restrict x, int t, int N))

#else
/*===========================================================================
  Vector matematics:
  vec_int2float            integer to float converion    
===========================================================================*/
/*-------------------------------------------------------------------------
  integer to float conversion
  routine converts integer to float and scales result up by 2^t.

  Precision: 
  f     single precision floating point

  Output:
  y[N]  output data
  Input:
  x[N]  input data
  t     scale factor
  N     length of vector

  Restriction:
  t should be in range -126...126
-------------------------------------------------------------------------*/

void   vec_int2float ( float32_t  * restrict y, const int32_t  * restrict x, int t, int N)
{
  /*
    float32_t y;
    ufloat32uint32 s;
    ASSERT(t>=-126 && t<=126);
    s.u = ((uint32_t)(t + 127)) << 23;
    y=((float32_t)x);
    y=y*s.f;
    return y;
  */
   const ae_int32x2 * restrict X = (const ae_int32x2 *)x;
          xtfloatx2 * restrict Y = ( xtfloatx2 *)y;
  
  int n;
  xtfloatx2 y0, y1;
  xtfloat f0;
  ae_int32x2 x0, t0;
  int32_t h0; 
  ae_valign X_va, Y_va;
  NASSERT(x);
  NASSERT(y);
  NASSERT(t >= -126 && t <= 126);
  if (N <= 0) return;
  t0 = AE_MOVDA32(((uint32_t)(t + 127)) << 23);
  y1 = XT_AE_MOVXTFLOATX2_FROMINT32X2(t0);
  X_va = AE_LA64_PP(X);
  Y_va = AE_ZALIGN64();
  for (n = 0; n<(N >> 1); n++)
  {
    AE_LA32X2_IP(x0, X_va, X);
    y0 = XT_FLOAT_SX2(x0, 0);
    y0 = XT_MUL_SX2(y0, y1);
    XT_SASX2IP(y0, Y_va, Y);
  }
  XT_SASX2POSFP(Y_va, Y); 
  if (N & 1)
  {
    h0 = XT_L32I((int *)X, 0);
    f0 = XT_FLOAT_S(h0, 0);
    f0 = XT_MUL_S(f0, y1);
    XT_SSI(f0, (xtfloat *)Y, 0);
  }
} /* vec_int2float() */

#endif /* #if HAVE_VFPU */

