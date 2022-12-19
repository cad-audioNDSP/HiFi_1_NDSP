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
#include "vec_recip_table.h"
#include "common.h"

/*===========================================================================
  Scalar matematics:
  scl_tan             Tangent    
===========================================================================*/

/*-------------------------------------------------------------------------
  Tangent 
  Fixed point functions calculate tan(pi*x) for number written in Q15 or 
  Q31. Floating point functions compute tan(x)
  
  Precision:
  16x16  16-bit inputs (Q15), 16-bit outputs (Q8.7). Accuracy: 1 LSB
  24x24  24-bit inputs, 32-bit outputs. Accuracy: (1.3e-4*y+1LSB) 
                                        if abs(y)<=464873(14.19 in Q15) 
                                        or abs(x)<pi*0.4776
  32x32  32-bit inputs, 32-bit outputs. Accuracy: (1.3e-4*y+1LSB)
                                        if abs(y)<=464873(14.19 in Q15) 
                                        or abs(x)<pi*0.4776
  f      floating point.                Accuracy: 2 ULP

  NOTE:
  1.  Scalar floating point function is compatible with standard ANSI C routines 
      and set errno and exception flags accordingly
  2.  Floating point functions limit the range of allowable input values: [-9099, 9099]
      Whenever the input value does not belong to this range, the result is set to NaN.

  Input:
  x[N]   input data, Q15, Q31 or floating point
  N      length of vectors
  Output:
  y[N]   result, Q8.7 (16-bit function), Q16.15 (24 or 32-bit functions) 
         or floating point

  Restriction:
  x,y - should not overlap

  Scalar versions:
  ----------------
  Return result, Q8.7 (16-bit function), Q16.15 (24 or 32-bit functions) 
  or floating point
-------------------------------------------------------------------------*/
int32_t scl_tan24x24 (f24 x)
{
    ae_int32x2 X,Y,E;
    int32_t s;
    xtbool2 sx;
    ae_f32x2 t,X2;
    ae_f32x2 f;
    ae_int32x2 _0x400000=AE_MOVDA32(0x400000);
    ae_int32x2 _0x800000=AE_MOVDA32(0x800000);
    int nsa;
    //s=scl_sine24x24(x);
    ae_f32x2 c0,c1,c2,c3;
    c0=AE_L32_I((const ae_int32*)sine_coef24,0*sizeof(int32_t));
    c1=AE_L32_I((const ae_int32*)sine_coef24,1*sizeof(int32_t));
    c2=AE_L32_I((const ae_int32*)sine_coef24,2*sizeof(int32_t));
    c3=AE_L32_I((const ae_int32*)sine_coef24,3*sizeof(int32_t));
    /* compute sine */
    X = AE_MOVDA32(x);
    sx=AE_LT32(X,AE_ZERO32());
    X=AE_SLLI32(X,1);
    X=AE_SRLI32(X,8);
    X=AE_SUB32(_0x800000,AE_ABS32(AE_SUB32(X,_0x800000)));
    ae_int32x2 X_LS=AE_SLLI32(X,8);
    X2=AE_MULFP32X2RAS_L(X,X_LS);
	  X2 = AE_SLLI32(X2,8);	
    Y=c0;
    f=c1; AE_MULAFP32X2RAS_L(f,Y,X2); Y=f;
    f=c2; AE_MULAFP32X2RAS_L(f,Y,X2); Y=f;
    f=c3; AE_MULAFP32X2RAS_L(f,Y,X2); Y=f;
    f=X ; AE_MULAFP32X2RAS_L(f,Y,X_LS); Y=f;
    Y = AE_AND32(Y, AE_MOVDA32(0x00FFFFFF));
    X=AE_NEG32(Y);
    AE_MOVT32X2(Y,X,sx);
    Y=AE_SLAI32S(Y,8);
    s= (int32_t)AE_MOVAD32_L(Y);
    /* compute cosine */
    X = AE_MOVDA32(AE_ADD32(x,0x40000000));
    sx=AE_LT32(X,AE_ZERO32());
    X=AE_SLLI32(X,1);
    X=AE_SRLI32(X,8);
    X=AE_SUB32(_0x800000,AE_ABS32(AE_SUB32(X,_0x800000)));
    X_LS=AE_SLLI32(X,8);
    X2=AE_MULFP32X2RAS_L(X,X_LS);
	  X2 = AE_SLLI32(X2,8);	
    Y=c0;
    f=c1; AE_MULAFP32X2RAS_L(f,Y,X2); Y=f;
    f=c2; AE_MULAFP32X2RAS_L(f,Y,X2); Y=f;
    f=c3; AE_MULAFP32X2RAS_L(f,Y,X2); Y=f;
    f=X ; AE_MULAFP32X2RAS_L(f,Y,X_LS); Y=f;
    Y = AE_AND32(Y, AE_MOVDA32(0x00FFFFFF));
    X=AE_NEG32(Y);
    AE_MOVT32X2(Y,X,sx);
    X=AE_SLAI32S(Y,8);

    sx=AE_LT32(X,AE_ZERO32());
    X=AE_ABS32S(X);
    /* normalize */
    nsa = AE_NSAZ32_L(X);
    X=AE_SLAA32(X,nsa);
    /* find initial approximation of reciprocal */
    X=AE_SRAI32(X,8);
    Y=AE_SUB32(AE_MOVDA32(0x00BAEC00),X);
    ae_int32x2 Y_LS= AE_SLLI32(Y,8);    
    
    /* 3 iterations to achieve 1 LSB accuracy in mantissa */    
    f=_0x400000; AE_MULSFP32X2RAS_L(f,X,Y_LS); E=f;
    E=AE_SLLI32(E,1);
    f=Y; AE_MULAFP32X2RAS_L(f,E,Y); Y=f; Y_LS = AE_SLLI32(Y,8);
    f=_0x400000; AE_MULSFP32X2RAS_L(f,X,Y_LS); E=f;
    E=AE_SLLI32(E,1);
    f=Y; AE_MULAFP32X2RAS_L(f,E,Y_LS); Y=f; Y_LS = AE_SLLI32(Y,8);
    f=_0x400000; AE_MULSFP32X2RAS_L(f,X,Y_LS); E=f;
    E=AE_SLLI32(E,1);
    f=Y; AE_MULAFP32X2RAS_L(f,E,Y_LS); Y=f;

    Y = AE_AND32(Y,AE_MOVDA32(0x00FFFFFF));
    /* restore original sign */
    X=AE_NEG32(Y);
    AE_MOVT32X2(Y,X,sx);
    Y=AE_SAT24S(Y);
    t=AE_L32_I((ae_int32*)&s,0); 
    f = AE_MULFP32X2RAS_L(Y,t);
    X=AE_MOVINT32X2_FROMF32X2(f);
    X=AE_SLAA32(X,nsa-7);
    return (int32_t)AE_MOVAD32_L(X);
}

