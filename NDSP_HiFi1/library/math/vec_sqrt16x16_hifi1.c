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
#include "polyrsqrtq23_tbl.h"

/*-------------------------------------------------------------------------
  Square Root
  These routines calculate square root.
  NOTES: 
  1. Fixed point functions return 0x80000000 (for 24 and 32-bit functions), 
     0x8000 (for 16-bit functions) on negative argument
  2. For floating point function, whenever an input value is negative, 
     functions raise the "invalid" floating-point exception, assign EDOM 
     to errno and set output value to NaN. Negative zero is considered as
     a valid input, the result is also -0
  Two versions of functions available: regular version (vec_sqrt16x16, 
  vec_sqrt24x24, vec_sqrt32x32, vec_sqrtf) with arbitrary arguments and
  faster version (vec_sqrt24x24_fast, vec_sqrt32x32_fast) that apply some 
  restrictions.

  Precision: 
  16x16  16-bit inputs, 16-bit output. Accuracy: (2 LSB)
  24x24  24-bit inputs, 24-bit output. Accuracy: (2.6e-7*y+1LSB)
  32x32  32-bit inputs, 32-bit output. Accuracy: (2.6e-7*y+1LSB)
  f      floating point. Accuracy 1 ULP

  Input:
  x[N]  input data,Q31, Q15 or floating point
  N     length of vectors
  Output:
  y[N]  output data,Q31, Q15 or floating point

  Restriction:
  Regular versions (vec_sqrt24x24, vec_sqrt32x32):
  x,y - should not overlap

  Faster versions (vec_sqrt24x24_fast, vec_sqrt32x32_fast):
  x,y - should not overlap
  x,y - aligned on 8-byte boundary
  N   - multiple of 2

  Scalar versions:
  ----------------
  return result, Q31, Q15 or floating point
-------------------------------------------------------------------------*/

void vec_sqrt16x16(int16_t * z, const int16_t * x,  int N )
{
    const ae_int16 * restrict px;
          ae_int16 * restrict pz;         
    ae_int32x2 X0,X1,X,R,D,Y;
    ae_f32x2 f0,f1,f;
    ae_int32x2 R0,R1,D0,D1,Y0,Y1,X2,X3;
    xtbool2 lezero;
    int sh0,sh1,sh2,sh3;
    int n;
    NASSERT(x);
    NASSERT(z);
    if(N<=0) return;
    ae_int16x4 *__restrict ptr_out_ae = (ae_int16x4 *)z;
    ae_int16x4 *__restrict ptr_in_ae = (ae_int16x4 *)x;
    ae_valign a2 = AE_LA64_PP(ptr_in_ae);
    ae_valign a1 = AE_ZALIGN64();
    ae_int16x4 inp_ae;
    ae_int32x2 X_2;
    for(n=0; n<(N>>2); n++)
    {
        /* load, take exponent */
        AE_LA16X4_IP(inp_ae,a2,ptr_in_ae);

        ae_int16x4 tmp1_ae = AE_SEL16_4321(inp_ae,inp_ae);
        ae_int16x4 tmp2_ae = AE_SEL16_4321(tmp1_ae,tmp1_ae);
        ae_int16x4 tmp3_ae = AE_SEL16_4321(tmp2_ae,tmp2_ae);

        sh0 = AE_NSAZ16_0(inp_ae)&~1;
        sh1 = AE_NSAZ16_0(tmp1_ae)&~1;
        sh2 = AE_NSAZ16_0(tmp2_ae)&~1;
        sh3 = AE_NSAZ16_0(tmp3_ae)&~1;

        ae_int16x4 x_3 = AE_SLAA16S(inp_ae,sh0);
        ae_int16x4 x_2 = AE_SLAA16S(tmp1_ae,sh1);
        ae_int16x4 x_1 = AE_SLAA16S(tmp2_ae,sh2);
        ae_int16x4 x_0 = AE_SLAA16S(tmp3_ae,sh3);

        xtbool2 lezero_1 = AE_LE32(AE_SEXT32X2D16_32(inp_ae),0);
        xtbool2 lezero_2 = AE_LE32(AE_SEXT32X2D16_10(inp_ae),0);

        /* compute rsqrt */
        R0=R1=polyrsqrtq23[0];
        f0=f1=polyrsqrtq23[1]; 

        ae_int16x4 inp_X_ae = AE_SEL16_6420(AE_SEL16_5410(x_0,x_1),AE_SEL16_5410(x_2,x_3));

        AE_MULAFP32X16X2RAS_H(f0, R0, inp_X_ae);R0=f0;
        AE_MULAFP32X16X2RAS_L(f1, R1, inp_X_ae);R1=f1;

        f0=f1=polyrsqrtq23[2]; 
        AE_MULAFP32X16X2RAS_H(f0, R0, inp_X_ae);R0=f0;
        AE_MULAFP32X16X2RAS_L(f1, R1, inp_X_ae);R1=f1;

        f0=f1=polyrsqrtq23[3]; 
        AE_MULAFP32X16X2RAS_H(f0, R0, inp_X_ae);R0=f0;
        AE_MULAFP32X16X2RAS_L(f1, R1, inp_X_ae);R1=f1;

        f0=f1=polyrsqrtq23[4]; 
        AE_MULAFP32X16X2RAS_H(f0, R0, inp_X_ae);R0=f0;
        AE_MULAFP32X16X2RAS_L(f1, R1, inp_X_ae);R1=f1;

        /* reiterate rsqrt */
        R0 = AE_SLAI32S(R0, 3);
        R1 = AE_SLAI32S(R1, 3);        
        R0 = AE_SAT24S(R0);
        R1 = AE_SAT24S(R1);
        ae_int16x4 R_tmp_X16 = AE_TRUNCI16X4F32S(R0,R1,8);
        D0=AE_MULFP32X16X2RAS_H((R0),R_tmp_X16);
        D1=AE_MULFP32X16X2RAS_L((R1),R_tmp_X16);

        f0 = 0x80000; AE_MULSFP32X16X2RAS_H(f0, D0, inp_X_ae); D0=f0;
        f1 = 0x80000; AE_MULSFP32X16X2RAS_L(f1, D1, inp_X_ae); D1=f1;

        D0=AE_MULFP32X16X2RAS_H((D0),R_tmp_X16);
        D1=AE_MULFP32X16X2RAS_L((D1),R_tmp_X16);

        D0=AE_SLAI32(D0,3);
        D1=AE_SLAI32(D1,3);
        R0=AE_ADD32S(R0,D0);
        R1=AE_ADD32S(R1,D1);
        /* compute sqrt and rescale back */
        Y0 = AE_MULFP32X16X2RAS_H(R0, inp_X_ae) ;
        Y1 = AE_MULFP32X16X2RAS_L(R1, inp_X_ae) ;

        X0=AE_SLAA32S(Y0,10-(sh3>>1));      
        X1=AE_SLAA32S(Y0,10-(sh2>>1));
        X=AE_SEL32_HL(X0,X1);
        X2=AE_SLAA32S(Y1,10-(sh1>>1));
        X3=AE_SLAA32S(Y1,10-(sh0>>1));
        X_2=AE_SEL32_HL(X2,X3);

        AE_MOVT32X2(X,0,lezero_1);
        AE_MOVT32X2(X_2,0,lezero_2);
        
        ae_int16x4 result = AE_SEL16_7531(AE_MOVF16X4_FROMINT32X2(X), AE_MOVF16X4_FROMINT32X2(X_2));
        AE_SA16X4_IP(result,a1,ptr_out_ae);
    }
    AE_SA64POS_FP(a1,ptr_out_ae);
    pz = (ae_int16 *)ptr_out_ae;
    px = (ae_int16 *)ptr_in_ae;
    for(n=0;n<(N&3);n++)
    {
        /* load, take exponent */
        AE_L16_IP (inp_ae,px,2);
        
        sh0 = AE_NSAZ16_0(inp_ae)&~1;
        ae_int16x4 tmp_X16 = AE_SLAA16S(inp_ae,sh0);

        lezero=AE_LE32(AE_SEXT32X2D16_32(inp_ae),0);
        /* compute rsqrt */
        R=polyrsqrtq23[0];
        f=polyrsqrtq23[1];AE_MULAFP32X16X2RAS_L(f, R, tmp_X16); R=f;
        f=polyrsqrtq23[2];AE_MULAFP32X16X2RAS_L(f, R, tmp_X16); R=f;
        f=polyrsqrtq23[3];AE_MULAFP32X16X2RAS_L(f, R, tmp_X16); R=f;
        f=polyrsqrtq23[4];AE_MULAFP32X16X2RAS_L(f, R, tmp_X16); R=f;
        /* reiterate rsqrt */
        R=AE_SLAI24S(AE_MOVF24X2_FROMINT32X2(R),3);
	    ae_int32x2 R_tmp=AE_SLAI32(R,8);
        D=AE_MULFP32X2RAS_L((R),R_tmp);
        f = 0x80000; AE_MULSFP32X16X2RAS_L(f, D, tmp_X16); D=f;
        D=AE_MULFP32X2RAS_L((D),R_tmp);
        D=AE_SLAI32(D,3);
        R=AE_ADD32S(R,D);
        /* compute sqrt and rescale back */
        Y = AE_MULFP32X16X2RAS_L(R, tmp_X16);
        X=AE_SLAA32S(Y,10-(sh0>>1));
        AE_MOVT32X2(X,0,lezero);
        ae_int16x4 res = AE_SEL16_7531(AE_MOVINT16X4_FROMINT32X2(X),AE_MOVINT16X4_FROMINT32X2(X));

        AE_S16_0_IP(res,pz,2);
    }
} /* vecsqrt_16b() */

