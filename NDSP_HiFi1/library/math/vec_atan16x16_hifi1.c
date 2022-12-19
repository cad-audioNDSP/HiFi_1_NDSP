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
#include "polyatan16x16q23_tbl.h"

#include "common.h"
/*-------------------------------------------------------------------------
  Arctangent 
  Functions calculate arctangent of number. Fixed point functions scale 
  output by pi which corresponds to the real phases +pi/4 and represent 
  input and output in Q15 or Q31
  NOTE:
  1.  Scalar floating point function is compatible with standard ANSI C
      routines and sets errno and exception flags accordingly

  Precision: 
  16x16  16-bit inputs, 16-bit output. Accuracy: 2 LSB
  24x24  24-bit inputs, 24-bit output. Accuracy: 74000 (3.4e-5)
  32x32  32-bit inputs, 32-bit output. Accuracy: 42    (2.0e-8)
  f      floating point. Accuracy: 2 ULP


  Input:
  x[N]   input data, Q15, Q31 or floating point
  N      length of vectors
  Output:
  z[N]   result, Q15, Q31 or floating point

  Restriction:
  x,z should not overlap

  Scalar versions:
  ----------------
  return result, Q15, Q31 or floating point
-------------------------------------------------------------------------*/

void vec_atan16x16 (int16_t   * restrict z, 
              const int16_t   * restrict x, int N )
{
    const ae_int16x4 * restrict px;
          ae_int16x4 * restrict pz;
    ae_valign ax,az;
    ae_int16x4 x0,x1,x2,z0;
    ae_f32x2 Y1,Y0,f0,f1;

    int n;
    NASSERT(x);
    NASSERT(z);
    if (N<=0) return ;
    px=(const ae_int16x4 *)x;
    pz=(      ae_int16x4 *)z;
    ax = AE_LA64_PP(px);
    az = AE_ZALIGN64();
    for (n=0; n<(N&~3); n+=4)
    {
        /* take input and convert to Q23 */
        AE_LA16X4_IP(x0,ax,px);
        /* compute polynomial */
        ae_int16x4 X20_16ae = AE_MULFP16X4RAS(x0,x0);
        Y0=Y1=polyatan16x16q23[0]; //Y0 = AE_SLLI32(Y0,8); //Y1 = AE_SLLI32(Y1,8);
        f0=f1=polyatan16x16q23[1]; //f0 = AE_SLLI32(f0,8); //f1 = AE_SLLI32(f1,8);
        AE_MULAFP32X16X2RAS_L(f0,Y0,X20_16ae); Y0=f0;
        AE_MULAFP32X16X2RAS_H(f1,Y1,X20_16ae); Y1=f1;
        f0=f1=polyatan16x16q23[2]; //f0 = AE_SLLI32(f0,8); f1 = AE_SLLI32(f1,8);
        AE_MULAFP32X16X2RAS_L(f0,Y0,X20_16ae); Y0=f0;
        AE_MULAFP32X16X2RAS_H(f1,Y1,X20_16ae); Y1=f1;
        f0=f1=polyatan16x16q23[3]; //f0 = AE_SLLI32(f0,8); f1 = AE_SLLI32(f1,8);
        AE_MULAFP32X16X2RAS_L(f0,Y0,X20_16ae); Y0=f0;
        AE_MULAFP32X16X2RAS_H(f1,Y1,X20_16ae); Y1=f1;
        Y0=AE_MULFP32X16X2RAS_L(Y0,x0); 
        Y1=AE_MULFP32X16X2RAS_H(Y1,x0); 
        /* convert result to Q15 */
        Y0=AE_SLAI32(Y0,8);
        Y1=AE_SLAI32(Y1,8);
        z0=AE_ROUND16X4F32SASYM(Y1,Y0);
        AE_SA16X4_IP(z0,az,pz);
    }
    AE_SA64POS_FP(az, pz);
    N&=3;
    if (N)
    {
        ae_int16 *pz_=(ae_int16 *)pz;
        AE_LA16X4_IP(x0,ax,px);
        /* compute polynomial */
        ae_int16x4 X20_16ae = AE_MULFP16X4RAS(x0,x0);
        Y0=Y1=polyatan16x16q23[0]; //Y0 = AE_SLLI32(Y0,8); //Y1 = AE_SLLI32(Y1,8);
        f0=f1=polyatan16x16q23[1]; //f0 = AE_SLLI32(f0,8); //f1 = AE_SLLI32(f1,8);
        AE_MULAFP32X16X2RAS_L(f0,Y0,X20_16ae); Y0=f0;
        AE_MULAFP32X16X2RAS_H(f1,Y1,X20_16ae); Y1=f1;
        f0=f1=polyatan16x16q23[2]; //f0 = AE_SLLI32(f0,8); f1 = AE_SLLI32(f1,8);
        AE_MULAFP32X16X2RAS_L(f0,Y0,X20_16ae); Y0=f0;
        AE_MULAFP32X16X2RAS_H(f1,Y1,X20_16ae); Y1=f1;
        f0=f1=polyatan16x16q23[3]; //f0 = AE_SLLI32(f0,8); f1 = AE_SLLI32(f1,8);
        AE_MULAFP32X16X2RAS_L(f0,Y0,X20_16ae); Y0=f0;
        AE_MULAFP32X16X2RAS_H(f1,Y1,X20_16ae); Y1=f1;
        Y0=AE_MULFP32X16X2RAS_L(Y0,x0); 
        Y1=AE_MULFP32X16X2RAS_H(Y1,x0); 
        /* convert result to Q15 */
        Y0=AE_SLAI32(Y0,8);
        Y1=AE_SLAI32(Y1,8);
        z0=AE_ROUND16X4F32SASYM(Y1,Y0);
        x0 = AE_SEL16_6543(z0, z0); 
        x1 = AE_SEL16_5432(z0, z0);
        x2 = AE_SEL16_5432(x0, x0);
        AE_S16_0_IP(x0, pz_, 2);
        if(N>1) AE_S16_0_IP(x1, pz_, 2);
        if(N>2) AE_S16_0_IP(x2, pz_, 2);
    }
}

