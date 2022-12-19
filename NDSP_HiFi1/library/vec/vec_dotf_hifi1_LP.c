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
DISCARD_FUN(float32_t, vec_dotf, (const float32_t * restrict x, const float32_t * restrict y, int N))

#else
/*===========================================================================
  Vector matematics:
  vec_dot              Vector Dot Product
===========================================================================*/
/*-------------------------------------------------------------------------
  Vector Dot product
  These routines take two vectors and calculates their dot product. Two 
  versions of routines are available: regular versions (vec_dot24x24, 
  vec_dot32x16, vec_dot32x32, vec_dot16x16, vec_dotf) work with arbitrary 
  arguments, faster versions (vec_dot24x24_fast, vec_dot32x16_fast, 
  vec_dot32x32_fast, vec_dot16x16_fast) apply some restrictions.

  Precision: 
  16x16  16x16-bit data, 64-bit output for regular version and 32-bit for 
         fast version
  24x24  24x24-bit data, 64-bit output
  32x16  32x16-bit data, 64-bit output
  32x32  32x32-bit data, 64-bit output
  f      floating point

  Input:
  x[N]  input data, Q31 or floating point
  y[N]  input data, Q31, Q15, or floating point
  N	length of vectors
  Returns: dot product of all data pairs, Q31 or floating point

  Restrictions:
  Regular versions:
  None
  Faster versions:
  x,y - aligned on 8-byte boundary
  N   - multiple of 4

  vec_dot16x16_fast utilizes 32-bit saturating accumulator, so, input data 
  should be scaled properly to avoid erroneous results
-------------------------------------------------------------------------*/

float32_t vec_dotf   (const float32_t * restrict x,const float32_t * restrict y,int N)

{
  int n=0;

  xtfloatx2 vxf, vyf, vacc,vacc1;
  xtfloat xf, yf, zf, acc;

  const xtfloatx2 * restrict px = (const xtfloatx2 *)x;
  const xtfloatx2 * restrict py = (const xtfloatx2 *)y;
  ae_valign x_align, y_align;
  NASSERT(x);
  NASSERT(y);
  if (N <= 0) return 0;

  x_align = AE_LA64_PP(px);
  y_align = AE_LA64_PP(py);

  vacc = XT_MOV_SX2(0.f);
  vacc1 = XT_MOV_SX2(0.f);
  zf = XT_MOV_S(0.f);

#if XCHAL_HAVE_HIFI1_LOW_LATENCY_MAC_FMA
  for (n = 0; n<(N>>1); n++)
  {
    XT_LASX2IP(vxf, x_align, px);
    XT_LASX2IP(vyf, y_align, py);
    XT_MADD_SX2(vacc, vxf, vyf);
  }
  vacc=XT_ADD_SX2(vacc,vacc1);

  if(N&1)
  {
    XT_LSIP(xf,(const xtfloat *)px, 4);
    XT_LSIP(yf,(const xtfloat *)py, 4);
    XT_MADD_S(zf,xf, yf);
  }
  acc=XT_RADD_SX2(vacc);
  acc = XT_ADD_S(acc,zf);

#else
  for (n = 0; n<(N>>2); n++)
  {
    XT_LASX2IP(vxf, x_align, px);
    XT_LASX2IP(vyf, y_align, py);
    XT_MADD_SX2(vacc, vxf, vyf);
    
    XT_LASX2IP(vxf, x_align, px);
    XT_LASX2IP(vyf, y_align, py);
    XT_MADD_SX2(vacc1, vxf, vyf);
   
  }
  vacc=XT_ADD_SX2(vacc,vacc1);

  for (n=4*n; n<N; n++)
  {
    XT_LSIP(xf,(const xtfloat *)px, 4);
    XT_LSIP(yf,(const xtfloat *)py, 4);
    XT_MADD_S(zf,xf, yf);
  }
  acc=XT_RADD_SX2(vacc);
  acc = XT_ADD_S(acc,zf);
#endif
  return acc;
} /* vec_dotf() */

#endif /* #if HAVE_VFPU */

