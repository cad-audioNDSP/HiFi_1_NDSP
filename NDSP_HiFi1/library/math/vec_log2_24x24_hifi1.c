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
#include "vec_log_table.h"
#include "common.h"

/*===========================================================================
  Vector matematics:
  vec_log              Logarithm 
===========================================================================*/

/*-------------------------------------------------------------------------
  Logarithm:
  Different kinds of logarithm (base 2, natural, base 10). 32 and 24-bit 
  fixed point functions interpret input as Q16.15 and represent results in 
  Q25 format or return 0x80000000 on negative of zero input. 16-bit fixed-
  point functions interpret input as Q8.7 and represent result in Q3.12 or
  return 0x8000 on negative of zero input

  Precision:
  16x16  16-bit inputs, 16-bit outputs
  24x24  24-bit inputs, 24-bit outputs
  32x32  32-bit inputs, 32-bit outputs
  f      floating point

  Accuracy :
  16x16 functions                                                    2 LSB
  vec_log2_32x32,scl_log2_32x32  , vec_log2_24x24,scl_log2_24x24     730 (2.2e-5)
  vec_logn_32x32,scl_logn_32x32  , vec_logn_24x24,scl_logn_24x24     510 (1.5e-5)
  vec_log10_32x32,scl_log10_32x32, vec_log10_24x24,scl_log10_24x24   230 (6.9e-6)
  floating point                                                     2 ULP

  NOTES:
  1.  Although 32 and 24 bit functions provide the same accuracy, 32-bit 
      functions have better input/output resolution (dynamic range)
  2.  Scalar Floating point functions are compatible with standard ANSI C routines 
      and set errno and exception flags accordingly.
  3.  Floating point functions limit the range of allowable input values:
      A) If x<0, the result is set to NaN. In addition, scalar floating point
         functions assign the value EDOM to errno and raise the "invalid" 
         floating-point exception.
      B) If x==0, the result is set to minus infinity. Scalar floating  point
         functions assign the value ERANGE to errno and raise the "divide-by-zero"
         floating-point exception.

  Input:
  x[N]  input data, Q16.15 (32 or 24-bit functions), Q8.7 (16-bit functions) or 
        floating point 
  N     length of vectors
  Output:
  y[N]  result, Q6.25 (32 or 24-bit functions), Q3.12 (16-bit functions) or 
        floating point 

  Restriction:
  x,y should not overlap

  Scalar versions:
  ----------------
  return result result, Q6.25 (32 or 24-bit functions), Q3.12 (16-bit 
  functions) or floating point
-------------------------------------------------------------------------*/
/*algorithm
  f(x) = log2(x)   
  f(x) = (f(x0) + f'(x0)*(x1-x0))+nsa
  log2(x0) = f(x0)
  (1/x0)*log2(e) = f'(x0)
  x = (2^nsa)*x1
  x1-x0 = dx
  x0 in 0.5..1
*/

void vec_log2_24x24 (f24 * restrict y, const f24 * restrict x, int N)
{
  int         n, nsa, off;

  ae_int32x2  vxw, vyw, vnw, vdw, vew, vmw, vsw, vhw, vzw, viw;

  ae_f32x2    vxf, vdf;
  ae_f64      vyf;
  ae_int64    vyq;
  xtbool2     inf;

  vmw = AE_MOVDA32X2(0x7fffff, 0x7fffff);
  vsw = AE_MOVDA32X2(0x400000, 0x400000);
  vhw = AE_SLAI32(vsw, 8);
  vew = AE_MOVDA32X2(24, 24);
  vzw = AE_MOVI(0);
  viw = AE_MOVDA32X2(MIN_INT32, MIN_INT32);

if(N>0)
{
  ae_f32x2    vxl;
  ae_int32x2 * restrict py = (      ae_int32x2 *)y;
  const ae_f32x2   * restrict px = (const ae_f32x2   *)x;
  ae_valign a1 = AE_LA64_PP(px);
  ae_valign a2 = AE_ZALIGN64();
  for (n=0; n<N>>1; n++)
  {
    AE_LA32X2_IP(vxl, a1,px);
    vxl = AE_SRAI32(vxl,8);
    vxw = (vxl);
    inf = AE_LE32(vxw, vzw);
    int nsa_1 = AE_NSAZ32_L(vxw);
    int nsa_0 = AE_NSAZ32_L(AE_SEL32_HH(vxw,vxw));
    ae_int32x2 tmp2 = AE_SLAA32S(vxw, nsa_1);
    ae_int32x2 tmp1 = AE_SLAA32S(vxw, nsa_0);
    vxw = AE_SEL32_HL(tmp1,tmp2);

    vxw = AE_SUB32(vxw, vhw);
    vdw = AE_AND32(vxw, vmw);
    vdw = AE_SUB32(vdw, vsw);
    vdw = AE_SLAI32(vdw, 2);       /*x1-x0, Q.33 */
    vxw = AE_SRAI32(vxw, 23);
    vxw = AE_SLAI32(vxw, 3);

    int off_0 = AE_MOVAD32_H(vxw);
    int off_1 = AE_MOVAD32_L(vxw);

    ae_int32x2 vxw_0 = AE_L32X2_X((const ae_int32x2 *)log2_table, off_0);
    ae_int32x2 vxw_1 = AE_L32X2_X((const ae_int32x2 *)log2_table, off_1);

    ae_int64 vyf_0 = AE_CVT64F32_H(vxw_0);     /* Q.63 */
    ae_int64 vyf_1 = AE_CVT64F32_H(vxw_1);     /* Q.63 */
    
    ae_int32x2 vxf_0 = (vxw_0);
    ae_int32x2 vxf_1 = (vxw_1);
    vdf = (vdw);

    AE_MULAF32S_LH(vyf_0, vxf_0, vdf);   /* log2(x0)+(1/x0)*log2(e)*(x1-x0), Q.29 * Q.33 -> Q.63 */
    AE_MULAF32S_LL(vyf_1, vxf_1, vdf);   /* log2(x0)+(1/x0)*log2(e)*(x1-x0), Q.29 * Q.33 -> Q.63 */

    vyf_0 = AE_SRAI64(vyf_0,6);
    vyf_1 = AE_SRAI64(vyf_1,6);

    vnw = AE_MOVDA32X2(nsa_0, nsa_1);/*Q.0*/
    vnw = AE_SUB32(vew, vnw);
    vnw = AE_SLAI32(vnw, 25);/*Q.25*/

    vxf = AE_ROUND32X2F64SASYM(vyf_0,vyf_1);
    vyw = (vxf);
    vyw = AE_ADD32(vyw, vnw);/*(log2(x0)+(1/x0)*log2(e)*(x1-x0))+nsa, Q.25*/
    AE_MOVT32X2(vyw, viw, inf);

    AE_SA32X2_IP(vyw, a2,py);

  }
  AE_SA64POS_FP(a2,py);

  const ae_f32* px1 = (ae_f32 *)px;
  ae_int32* py1 = (ae_int32 *)py;
  
  if(N&1)
  {
    vxl = AE_L32_I(px1, 0);
    vxl = AE_SRAI32(vxl,8);
    vxw = (vxl);
    inf = AE_LE32(vxw, vzw);
    nsa = AE_NSAZ32_L(vxw);
    vxw = AE_SLAA32S(vxw, nsa);
    vxw = AE_SUB32(vxw, vhw);
    vdw = AE_AND32(vxw, vmw);
    vdw = AE_SUB32(vdw, vsw);
    vdw = AE_SLAI32(vdw, 2);       /*x1-x0, Q.33 */
    vxw = AE_SRAI32(vxw, 23);
    vxw = AE_SLAI32(vxw, 3);
    off = AE_MOVAD32_H(vxw);
    vxw = AE_L32X2_X((const ae_int32x2 *)log2_table, off);
    vyf = AE_CVT64F32_H(vxw);     /* Q.63 */
    vxf = (vxw);
    vdf = (vdw);
    AE_MULAF32S_LH(vyf, vxf, vdf); /* log2(x0)+(1/x0)*log2(e)*(x1-x0), Q.29 * Q.33 -> Q.63 */
    vyq = (vyf);
    vyq = AE_SRAI64(vyq, 6);/*Q.57*/
    vyf = (vyq);
    vnw = AE_MOVDA32X2(nsa, nsa);/*Q.0*/
    vnw = AE_SUB32(vew, vnw);
    vnw = AE_SLAI32(vnw, 25);/*Q.25*/
    vxf = AE_ROUND32F64SASYM(vyf);
    vyw = (vxf);
    vyw = AE_ADD32(vyw, vnw);/*(log2(x0)+(1/x0)*log2(e)*(x1-x0))+nsa, Q.25*/
    AE_MOVT32X2(vyw, viw, inf);
    AE_S32_L_I(vyw, py1, 0);
  }
}
else
{
  const ae_f32* px1 = (ae_f32 *)x;
  ae_int32* py1 = (ae_int32 *)y;
  ae_f32x2 vxl;
  for(n=0;n<N;++n)
  {
    vxl = AE_L32_I(px1, 0);
    vxl = AE_SRAI32(vxl,8);
    vxw = (vxl);
    inf = AE_LE32(vxw, vzw);
    nsa = AE_NSAZ32_L(vxw);
    vxw = AE_SLAA32S(vxw, nsa);
    vxw = AE_SUB32(vxw, vhw);
    vdw = AE_AND32(vxw, vmw);
    vdw = AE_SUB32(vdw, vsw);
    vdw = AE_SLAI32(vdw, 2);       /*x1-x0, Q.33 */
    vxw = AE_SRAI32(vxw, 23);
    vxw = AE_SLAI32(vxw, 3);
    off = AE_MOVAD32_H(vxw);
    vxw = AE_L32X2_X((const ae_int32x2 *)log2_table, off);
    vyf = AE_CVT64F32_H(vxw);     /* Q.63 */
    vxf = (vxw);
    vdf = (vdw);
    AE_MULAF32S_LH(vyf, vxf, vdf);   /* log2(x0)+(1/x0)*log2(e)*(x1-x0), Q.29 * Q.33 -> Q.63 */
    vyq = (vyf);
    vyq = AE_SRAI64(vyq, 6);/*Q.57*/
    vyf = (vyq);
    vnw = AE_MOVDA32X2(nsa, nsa);/*Q.0*/
    vnw = AE_SUB32(vew, vnw);
    vnw = AE_SLAI32(vnw, 25);/*Q.25*/
    vxf = AE_ROUND32F64SASYM(vyf);
    vyw = (vxf);
    vyw = AE_ADD32(vyw, vnw);/*(log2(x0)+(1/x0)*log2(e)*(x1-x0))+nsa, Q.25*/
    AE_MOVT32X2(vyw, viw, inf);
    AE_S32_L_I(vyw, py1, 0);
  }
 }

}
