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
#include "scl_sine_table32.h"
#include "common.h"

/*===========================================================================
  Vector matematics:
  vec_cosine            Cosine    
===========================================================================*/

/*-------------------------------------------------------------------------
  Sine/Cosine 
  Fixed-point functions calculate sin(pi*x) or cos(pi*x) for numbers written 
  in Q31 or Q15 format. Return results in the same format. 
  Floating point functions compute sin(x) or cos(x)
  Two versions of functions available: regular version (vec_sine16x16, 
  vec_cosine16x16, vec_sine24x24, vec_cosine24x24, vec_sine32x32, 
  vec_cosine32x32, , vec_sinef, vec_cosinef) with arbitrary arguments and 
  faster version (vec_sine24x24_fast, vec_cosine24x24_fast, 
  vec_sine32x32_fast, vec_cosine32x32_fast) that apply some restrictions.
  NOTE:
  1.  Scalar floating point functions are compatible with standard ANSI C
      routines and set errno and exception flags accordingly
  2.  Floating point functions limit the range of allowable input values:
      [-102940.0, 102940.0] Whenever the input value does not belong to this
      range, the result is set to NaN.

  Precision: 
  16x16  16-bit inputs, 16-bit output. Accuracy: 2 LSB  
  24x24  24-bit inputs, 24-bit output. Accuracy: 74000(3.4e-5)
  32x32  32-bit inputs, 32-bit output. Accuracy: 1700 (7.9e-7)
  f      floating point. Accuracy 2 ULP

  Input:
  x[N]  input data, Q15, Q31 or floating point
  N     length of vectors
  Output:
  y[N]  output data,Q31,Q15 or floating point

  Restriction:
  Regular versions (vec_sine24x24, vec_cosine24x24, vec_sine32x32, 
  vec_cosine32x32, vec_sinef, vec_cosinef):
  x,y - should not overlap

  Faster versions (vec_sine24x24_fast, vec_cosine24x24_fast, 
  vec_sine32x32_fast, vec_cosine32x32_fast):
  x,y - should not overlap
  x,y - aligned on 8-byte boundary
  N   - multiple of 2

  Scalar versions:
  ----------------
  return result in Q31,Q15 or floating point
-------------------------------------------------------------------------*/

void vec_cosine24x24_fast (  f24 * restrict y, const f24 * restrict x, int N)
#if SIN24X24_ALG==0
{
/*
MATLAB reference code:
function y=approx_sin32(x)
ord=9;
tbl=sin(2*pi*(0:2^ord-1)/(2^ord))*2^31;
tbl=min(2^31-1,max(-2^31,tbl));
sh=(32-ord);
idx=bitshift(x+2^(sh-1),-sh);
y=tbl(idx+1);
dx=x-bitshift(idx,sh);
pi_dx=pi*dx;
c=tbl(idx+1+(2^ord)/4)/2^31 % cosine !!!
s=y/2^31 %s=sin(x0)
c=c-1/2*pi_dx.*s/(2^31)%c=cos-1/2*sin*dx
y=y+pi_dx.*c;%y=sin(x0)+cos*dx-1/2sin*dx^2
*/
  int n;
        ae_int32x2 * restrict py = (      ae_int32x2 *)y;
  const ae_f24x2   * restrict px = (const ae_f24x2   *)x;

  uint32_t      off,offb;
  ae_int32x2    vsw,vaw,vxw,vnw,vlw,vkw,vrw,vxa,vxb,vxc,vxd, vdl,vcs,vsn,vww,vdw;
  ae_f32x2      vsf,vdf,vxf,vdlf,vaf,vbf;
  ae_f24x2      vxl;
  if (N <= 0) return;
  vsw = AE_MOVDA32X2(0xc91000, 0xc91000); //pi,Q22
  vnw = AE_MOVDA32X2(0x4000, 0x4000); //1<<(22)
  vlw = AE_MOVDA32X2(0x200, 0x200); //1<<(9)/4
  vkw = AE_MOVDA32X2(0x7fc, 0x7fc); //1<<(9)-1
  vww = AE_MOVDA32X2(0x00FF8FFF,0x00FF8FFF); 
  NASSERT_ALIGN8(px);
  NASSERT_ALIGN8(py);
  ASSERT((N&1)==0);
   for (n=0; n<N; n+=2)
  {
    AE_L32X2F24_IP(vxl, px, +8); 
    vaw = (vxl);
    vxw = AE_SLAI32(vaw, 8);
    vaw = AE_ADD32S(vaw,vnw);
    vaw = AE_AND32(vaw, vww);
    vaw = AE_SRLI32(vaw,13);

    off = AE_MOVAD32_H(vaw);
    offb = AE_MOVAD32_L(vaw);
    /*Load sine for next numbers*/
    vxa = AE_L32_X((const ae_int32 *)sine_table32, off); //sin
    vxb = AE_L32_X((const ae_int32 *)sine_table32, offb); //sin b
    vsn = AE_SEL32_HH(vxa, vxb); //sin

    vdw = AE_SLAI32(vaw,21);
    vdw = AE_SUB32(vxw, vdw);
    vaf = (vdw);
    vbf = (vsw);
    vdlf = AE_MULFP32X2RAS(vaf, vbf);//
    vdl = (vdlf);
    vdl = AE_SLAI32(vdl, 9); //Q31
    vaw = AE_ADD32S(vaw,vlw);
    vaw = AE_AND32(vaw, vkw);
    off = AE_MOVAD32_H(vaw);
    offb = AE_MOVAD32_L(vaw);
    /*Load cosine*/
    vxc = AE_L32_X((const ae_int32 *)sine_table32, off); //cos 
    vxd = AE_L32_X((const ae_int32 *)sine_table32, offb); //cos 
    vcs = AE_SEL32_HH(vxc, vxd); //cos

    vsf = (vsn);//sin
    vxf = (vcs);//cos
    
    vdf = (vdl);//dx
    AE_MULSFP32X2RAS(vxf, vsf, vdf); //y = cos(x0)-sin(x0)dx,Q31
    vrw = (vxf);
    AE_S32X2_IP(vrw, py, +8);//put answer
 
  }
}
#elif SIN24X24_ALG==1
{
        ae_int32x2     * restrict py = (      ae_int32x2 *)y;
    const ae_int32x2   * restrict px = (const ae_int32x2 *)x;
    ae_f32x2 c0,c1,c2,c3;
    int n;

    NASSERT_ALIGN8(x);
    NASSERT_ALIGN8(y);
    NASSERT((N%2)==0);
    if(N<=0) return;

    c0=AE_L32_I((const ae_int32*)sine_coef32,0*sizeof(int32_t));
    c1=AE_L32_I((const ae_int32*)sine_coef32,1*sizeof(int32_t));
    c2=AE_L32_I((const ae_int32*)sine_coef32,2*sizeof(int32_t));
    c3=AE_L32_I((const ae_int32*)sine_coef32,3*sizeof(int32_t));

    for (n=0; n<(N>>1);n++)
    {
        ae_int32x2 X,Y;
        ae_f32x2 X2;
        ae_f32x2 f;
        xtbool2 sx;
        AE_L32X2_IP(X, px, 8); 
        X=AE_ADD32(X,AE_MOVDA32(0x40000000));
        sx=AE_LT32(X,AE_ZERO32());
        X=AE_SLLI32(X,1);
        X = AE_ABS32(X);

        X2=AE_MULFP32X2RAS(X,X);

        Y=c0;
        f=c1; AE_MULAFP32X2RAS(f,Y,X2); Y=f;
        f=c2; AE_MULAFP32X2RAS(f,Y,X2); Y=f;
        f=c3; AE_MULAFP32X2RAS(f,Y,X2); Y=f;
        f=X ; AE_MULAFP32X2RAS(f,Y,X); Y=f;

        Y =  AE_SRLI32(Y,8);
        X=AE_NEG32(Y);
        AE_MOVT32X2(Y,X,sx);
        Y=AE_SLAI32S(Y,8);
        AE_S32X2_IP(Y, py, 8);
    }
}
#else
#error wrong SIN24X24_ALG
#endif
