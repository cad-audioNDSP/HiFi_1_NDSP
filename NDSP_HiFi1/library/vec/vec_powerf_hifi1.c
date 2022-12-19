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
DISCARD_FUN(float32_t, vec_powerf, (const float32_t * restrict x, int N))

#else
/*===========================================================================
  Vector matematics:
  vec_power            Power of a Vector
===========================================================================*/
/*-------------------------------------------------------------------------
  Power of a Vector
  These routines compute power of vector with scaling output result by rsh 
  bits. Fixed point rountines make accumulation in the 64-bit wide 
  accumulator and output may scaled down with saturation by rsh bits. 
  So, if representation of x input is Qx, result will be represented in 
  Q(2x-rsh) format.
  Two versions of routines are available: regular versions (vec_power24x24, 
  vec_power32x32, vec_power16x16, vec_powerf) work with arbitrary arguments, 
  faster versions (vec_power24x24_fast, vec_power32x32_fast, 
  vec_power16x16_fast) apply some restrictions.

  Precision: 
  24x24 24x24-bit data, 64-bit output
  32x32 32x32-bit data, 64-bit output
  16x16 16x16-bit data, 64-bit output
  f     single precision floating point

  Input:
  x[N]  input data, Q31, Q15 or floating point
  rsh   right shift of result
  N     length of vector

  Returns: Sum of squares of a vector, Q(2x-rsh)

  Restrictions:
  for vec_power32x32(): rsh in range 31...62
  for vec_power24x24(): rsh in range 15...46
  for vec_power16x16(): rsh in range 0...31
  For regular versions (vec_power24x24, vec_power32x32, 
  vec_power16x16, vec_powerf):
  none

  For faster versions (vec_power24x24_fast, 
  vec_power32x32_fast, vec_power16x16_fast ):
  x - aligned on 8-byte boundary
  N   - multiple of 4
-------------------------------------------------------------------------*/

float32_t   vec_powerf     ( const float32_t * restrict x,int N)
{
#if XCHAL_HAVE_HIFI1_LOW_LATENCY_MAC_FMA
  int n;

  xtfloatx2 vxf, vacc;
  xtfloat xf, zf, acc;

  const xtfloatx2 * restrict px = (const xtfloatx2 *)x;
  ae_valign x_align;
  NASSERT(x);
  if (N <= 0) return 0;
  
  x_align = AE_LA64_PP(px);
  vacc = XT_MOV_SX2(0.f);
  zf = XT_MOV_S(0.f);

  for (n = 0; n<(N>>1); n++)
  {
    XT_LASX2IP(vxf, x_align, px);
    XT_MADD_SX2(vacc, vxf, vxf);
  }

  if(N&1)
  {
    XT_LSIP(xf,(const xtfloat *)px, 4);
    XT_MADD_S(zf,xf, xf);
  }
  acc = XT_RADD_SX2(vacc);
  acc = XT_ADD_S(acc, zf);

#else
 int n;

  xtfloatx2 vxf, vacc,vxf1, vacc1;
  xtfloat xf, zf, acc;

  const xtfloatx2 * restrict px = (const xtfloatx2 *)x;
  ae_valign x_align;
  NASSERT(x);
  if (N <= 0) return 0;
  
  x_align = AE_LA64_PP(px);
  vacc = XT_MOV_SX2(0.f);
  vacc1 = XT_MOV_SX2(0.f);
  zf = XT_MOV_S(0.f);

  for (n = 0; n<(N>>2); n++)
  {
    XT_LASX2IP(vxf, x_align, px);
    XT_MADD_SX2(vacc, vxf, vxf);

    XT_LASX2IP(vxf1, x_align, px);
    XT_MADD_SX2(vacc1, vxf1, vxf1);
    
  }

  vacc=XT_ADD_SX2(vacc,vacc1);

  for (n=4*n; n<N; n++)
  {
    XT_LSIP(xf,(const xtfloat *)px, 4);
    XT_MADD_S(zf,xf, xf);
  }
  acc = XT_RADD_SX2(vacc);
  acc = XT_ADD_S(acc, zf);
#endif
  return acc;
} /* vec_powerf() */

#endif /* #if HAVE_VFPU */

