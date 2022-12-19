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
/*===========================================================================
  Vector matematics:
  vec_recip            Reciprocal on Q31/Q15 Numbers
===========================================================================*/

/*-------------------------------------------------------------------------
  Reciprocal 
  Fixed point routines return the fractional and exponential portion of the 
  reciprocal of a vector x of Q31 or Q15 numbers. Since the reciprocal is 
  always greater than 1, it returns fractional portion frac in Q(31-exp) 
  or Q(15-exp) format and exponent exp so true reciprocal value in the 
  Q0.31/Q0.15 may be found by shifting fractional part left by exponent 
  value.

  Mantissa accuracy is 1 LSB, so relative accuracy is:
  vec_recip16x16, scl_recip16x16                   6.2e-5 
  vec_recip24x24, scl_recip32x32, scl_recip24x24   2.4e-7 
  vec_recip32x32                                   9.2e-10

  Floating point routines operate with standard floating point numbers. 
  Functions return +/-infinity on zero or denormalized input and provide 
  accuracy of 1 ULP.

  Precision: 
  32x32   32-bit input, 32-bit output. 
  24x24   24-bit input, 24-bit output. 
  16x16   16-bit input, 16-bit output. 
  f       floating point


  Input:
  x[N]    input data, Q31,Q15 or floating point
  N       length of vectors

  Output:
  frac[N] fractional part of result, Q(31-exp) or Q(15-exp) (fixed point 
          functions)
  exp[N]  exponent of result (fixed point functions) 
  y[N]    result (floating point function)

  Restriction:
  x,frac,exp should not overlap

  Scalar versions:
  ----------------
  Return packed value for fixed-point functions: 
  scl_recip24x24(),scl_recip32x32():
  bits 23:0 fractional part
  bits 31:24 exponent
  scl_recip16x16():
  bits 15:0 fractional part
  bits 31:16 exponent
-------------------------------------------------------------------------*/

void vec_recip16x16
(
  int16_t * restrict frac, 
  int16_t * exp, 
  const int16_t * restrict x, 
  int N
)
{

#if ( XCHAL_HW_VERSION >= XTENSA_HWVERSION_RI_2022_9 )
	    int16_t ALIGN(8) temp[MAX_ALLOCA_SZ / sizeof(int16_t)];
	    vbool4 iszero;
	    int n=0;
	    const ae_int16x4   * restrict px = (const ae_int16x4    *)x;
	    	  ae_int16x4   * restrict pf =        (ae_int16x4   *)frac;
	    	  ae_int16x4   * restrict expPtr =    ( ae_int16x4   *)exp;


	    const ae_p16x2s  * restrict pfRd;
	    const ae_int16x4  * restrict pfRd16;
	    const ae_int16   * restrict pfWr;

	    ae_int16x4 nsa;
		ae_valign  expalign, Dalign, DalignSav;
		ae_int16x4 nsa16x4_L0,nsa16x4_L1,nsa16x4_H0,nsa16x4_H1;
		ae_int16x4 X, XT_L0,XT_L1,XT_H0,XT_H1, X_Temp;
	    while (N > 0)
	    {
	        int M;
	        M = MAX_ALLOCA_SZ / sizeof(int16_t);
	        if (M > N) M = N;
	        /* compute exponent and normalize inputs */
	        pf = (ae_int16*)temp;

//	        XT_L0,XT_L1,XT_H0,XT_H1;
	        Dalign = AE_LA64_PP(px);
	        expalign = AE_ZALIGN64();
	        DalignSav = AE_ZALIGN64();

     		for(n=0; n<(M>>2); n++)
 			{
				//-------------Set 1---------------------------------------------------
				AE_LA16X4_IP(X,Dalign,px);
				iszero = AE_EQ16(X,AE_MOVDA16(0));
				nsa16x4_L0 = AE_NSA16_0(X);//process lower 16 bit
				XT_L0=AE_SLAA16S(X, (ae_int16)nsa16x4_L0);

				X_Temp = AE_SEL16_7531(X,X); //1st element of X to be processed 753 doesnot matter
				nsa16x4_L1 = AE_NSA16_0(X_Temp);//process lower 16 bit
				XT_L1=AE_SLAA16S(X_Temp, (ae_int16)nsa16x4_L1);

				X_Temp = AE_SEL16_7632(X,X); //2nd element of X to be processed 763 doesnot matter
				nsa16x4_H0 = AE_NSA16_0(X_Temp);//process lower 16 bit
				XT_H0=AE_SLAA16S(X_Temp, (ae_int16)nsa16x4_H0);

				X_Temp = AE_SEL16_6543(X,X); //3rd element of X to be processed 654 doesnot matter
				nsa16x4_H1 = AE_NSA16_0(X_Temp);//process lower 16 bit
				XT_H1=AE_SLAA16S(X_Temp, (ae_int16)nsa16x4_H1);

				X_Temp = AE_SEL16_5140(XT_L1,XT_L0); //Xtemp = (u,u,L1,L0)
				X = AE_SEL16_5140(XT_H0,XT_H1);// (z1,z2,H0,H1)
				X = AE_SHORTSWAP(X);//(H1,H0,z2,z1)
				X = AE_SEL16_7610(X,X_Temp);// (H1,H0,L1,L0)

				nsa16x4_L0 = AE_SEL16_7362(nsa16x4_L1,nsa16x4_L0); //L1 L0, L1,L0
				nsa16x4_H0 = AE_SEL16_7362(nsa16x4_H1,nsa16x4_H0);  //H1 H0, H1,H0
				nsa = AE_SEL16_7610(nsa16x4_H0,nsa16x4_L0); //H1 H0, L1,L0

				nsa=AE_ADD16(nsa, AE_MOVDA16(1)); //add 1 from H and L
				AE_MOVT16X4(nsa, 0x10, iszero);
				AE_SA16X4_IP(X,DalignSav,pf);
				AE_SA16X4_IP(nsa,expalign,expPtr);
			}
		   AE_SA64POS_FP(expalign, expPtr);
		   AE_SA64POS_FP(DalignSav, pf);
			   //tail handling
		   if(N&2)
			{
				AE_L16_IP(X,(ae_int16*)px,sizeof(ae_int16));
				iszero = AE_EQ16(X,AE_MOVDA16(0));
				nsa16x4_H1 = AE_NSA16_0(X);//process lower 16 bit
				XT_H1=AE_SLAA16S(X, (ae_int16)nsa16x4_H1);

				AE_L16_IP(X,(ae_int16*)px,sizeof(ae_int16));
				nsa16x4_H0 = AE_NSA16_0(X);//process lower 16 bit
				XT_H0=AE_SLAA16S(X, (ae_int16)nsa16x4_H0);

				AE_S16_0_IP(XT_H1,(ae_int16*)pf,(sizeof(ae_int16)));//H1
				AE_S16_0_IP(XT_H0,(ae_int16*)pf,(sizeof(ae_int16)));//H0

				nsa = AE_SEL16_7362(nsa16x4_H0,nsa16x4_H1);  //H0 H1, H0,H1
				nsa=AE_ADD16(nsa, AE_MOVDA16(1)); //add 1
				AE_MOVT16X4(nsa, 0x10, iszero);
				AE_S16_0_IP(nsa,(ae_int16*)expPtr,(sizeof(ae_int16)));//H1
				nsa = AE_SHORTSWAP(nsa);//(H1,H0,z2,z1)
				AE_S16_0_IP(nsa,(ae_int16*)expPtr,(sizeof(ae_int16)));//H0
			 }

		   if(N&1)
		   {
			 AE_L16_IP(X, (ae_int16*)px, sizeof(ae_int16));
			 iszero = AE_EQ16(X,AE_MOVDA16(0));
			 nsa16x4_H0 = (ae_int16)AE_NSA16_0(X); //apply NSA on higher 32 bits
			 X=AE_SLAA16S(X,(ae_int16)nsa16x4_H0);
			 nsa=AE_ADD16(nsa16x4_H0, AE_MOVDA16(1)); //add 1 H and L
			 AE_MOVT16X4(nsa, 0x10, iszero);
			 AE_S16_0_IP(nsa,(ae_int16*)expPtr,0);
			 AE_S16_0_IP(X,(ae_int16*)pf,0);
		   }

	        __Pragma("no_reorder");	    
	        pfRd16=(const ae_int16x4*)(temp);
	        pfWr = (ae_int16 *)frac;
	        ae_int32x2 _0x400000 = AE_MOVDA32(0x00400000);
	        ae_int32x2 _0x00BAEC00 = AE_MOVDA32(0x00BAEC00);
	        ae_int16x4 t_0x7fff00 = AE_MOVDA16(0x7fff);
	        ae_valign align1 = AE_ZALIGN64();
	        for (n = 0; n < (M&~3); n += 4)
	        {
	            xtbool4 sx2;
	            ae_int16x4 rd;
	            ae_int32x2 Y, E;
	            ae_int16x4 X;
	            ae_int32x2 tX_temp,tY;
	            xtbool4  isZero;//tsx,
	            ae_int32x2 f,f1;
	            AE_L16X4_IP(rd, pfRd16, 8);
				X = AE_ABS16S(rd);
				isZero = AE_EQ16(X, AE_ZERO16());
				AE_MOVT16X4(X, t_0x7fff00, isZero);
				sx2 = AE_LT16(rd,AE_ZERO16());
				tX_temp=AE_SEXT32X2D16_32(X);
				tY = AE_SLAI32(tX_temp, 8);
				Y = AE_SUB32(_0x00BAEC00, tY); /* first approximation */

	            /* 3 iterations */
	            f = (_0x400000);
	            AE_MULSFP32X16X2RAS_H(f, (Y),AE_MOVF16X4_FROMINT16X4(X));

	            E = (f);
	            E = AE_SLLI32(E, 9);
	            f1 = (Y);
	            AE_MULAFP32X2RAS(f1, (E), (Y));
	            Y = (f1);

	            f = (_0x400000);
	            AE_MULSFP32X16X2RAS_H(f, (Y),AE_MOVF16X4_FROMINT16X4(X));
	            E = (f);
	            E = AE_SLLI32(E, 9);
	            f1 = (Y);
	            AE_MULAFP32X2RAS(f1, (E), (Y));
	            Y = (f1);

	            f = (_0x400000);
	            AE_MULSFP32X16X2RAS_H(f, (Y),AE_MOVF16X4_FROMINT16X4(X));
	            E = (f);
	            E = AE_SLLI32(E, 9);
	            f1 = (Y);
	            AE_MULAFP32X2RAS(f1, (E), (Y));
	            Y = (f1);

	            ae_int16x4 tX1;
	            /* restore original sign */
	            ae_int32x2 Y0 = (AE_SLLI32S((Y), 8));
	           /* Y = AE_SRAI32(Y, 16);
	            AE_S16_0_I(AE_MOVINT16X4_FROMINT32X2(Y), castxcc(ae_int16, pfWr), 2);
	            Y = AE_INTSWAP(Y);
	            AE_S16_0_IP(AE_MOVINT16X4_FROMINT32X2(Y), castxcc(ae_int16, pfWr), 4); */

				tX_temp=AE_SEXT32X2D16_10(X);
				tY = AE_SLAI32(tX_temp, 8);
				Y = AE_SUB32(_0x00BAEC00, tY); /* first approximation */

	            /* 3 iterations */
				      f1 = (_0x400000);
	            AE_MULSFP32X16X2RAS_L(f1, (Y),AE_MOVF16X4_FROMINT16X4(X));

	            E = (f1);
	            E = AE_SLLI32(E, 9);
	            f = (Y);
	            AE_MULAFP32X2RAS(f, (E), (Y));
	            Y = (f);

	            f1 = (_0x400000);
	            AE_MULSFP32X16X2RAS_L(f1, (Y),AE_MOVF16X4_FROMINT16X4(X));
	            E = (f1);
	            E = AE_SLLI32(E, 9);
	            f = (Y);
	            AE_MULAFP32X2RAS(f, (E), (Y));
	            Y = (f);

	            f1 = (_0x400000);
	            AE_MULSFP32X16X2RAS_L(f1, (Y),AE_MOVF16X4_FROMINT16X4(X));
	            E = (f1);
	            E = AE_SLLI32(E, 9);
	            f = (Y);
	            AE_MULAFP32X2RAS(f, (E), (Y));
	            Y = (f);
	            /* restore original sign */

	            ae_int32x2 Y1 = (AE_SLLI32S((Y), 8));
	            ae_int16x4 result_ae = AE_TRUNC16X4F32(Y0,Y1);

	            tX1 = AE_NEG16S(result_ae);
	            AE_MOVT16X4(result_ae, tX1, sx2);

	            AE_SA16X4_IP(result_ae,align1,castxcc(ae_int16x4,pfWr));

	        }
	        AE_SA64POS_FP(align1,castxcc(ae_int16x4,pfWr));
	        pfRd = (const ae_p16x2s*)(temp+n-2);
	        if (N & 2)
	        {
	            xtbool2 sx, isZero;
	            ae_f32x2 rd;
	            ae_int32x2 X, X0, Y, E, X_temp;
	            ae_int32x2 _0x400000 = AE_MOVDA32(0x00400000);
	            ae_int32x2 _0x00BAEC00 = AE_MOVDA32(0x00BAEC00);
	            ae_int32x2 _0x7fff00 = AE_MOVDA32(0x007fff00);
	            ae_f32x2 f;

	            AE_L16X2M_IU(rd, pfRd, 4);
	            X0 = (rd);
	            sx = AE_LT32(X0, AE_ZERO32());
	            X0 = AE_SLAI32(X0, 8);
	            X = AE_ABS32S(X0);
	            isZero = AE_EQ32(X, AE_ZERO32());
	            AE_MOVT32X2(X, _0x7fff00, isZero);
	            X_temp = AE_SRAI32(X, 8);
	            Y = AE_SUB32(_0x00BAEC00, X_temp); /* first approximation */


	            /* 3 iterations */
	            f = (_0x400000);
	            AE_MULSFP32X2RAS(f, (X), (Y));

	            E = (f);
	            E = AE_SLLI32(E, 1+8);
	            f = (Y);
	            AE_MULAFP32X2RAS(f, (E), (Y));
	            Y = (f);

	            f = (_0x400000);
	            AE_MULSFP32X2RAS(f, (X), (Y));
	            E = (f);
	            E = AE_SLLI32(E, 1+8);
	            f = (Y);
	            AE_MULAFP32X2RAS(f, (E), (Y));
	            Y = (f);

	            f = (_0x400000);
	            AE_MULSFP32X2RAS(f, (X), (Y));
	            E = (f);
	            E = AE_SLLI32(E, 1+8);
	            f = (Y);
	            AE_MULAFP32X2RAS(f, (E), (Y));
	            Y = (f);
	            /* restore original sign */
	            X = AE_NEG32(Y);
	            AE_MOVT32X2(Y, X, sx);
	            Y = (AE_SLLI32S((Y), 8));
	            Y = AE_SRAI32(Y, 16);
	            AE_S16_0_I(AE_MOVINT16X4_FROMINT32X2(Y), castxcc(ae_int16, pfWr), 2);
	            Y = AE_INTSWAP(Y);
	            AE_S16_0_IP(AE_MOVINT16X4_FROMINT32X2(Y), castxcc(ae_int16, pfWr), 4);
	        }
	       if (N & 1)
	        {
	            xtbool2 sx, isZero;
	            ae_f32x2 rd;
	            ae_int32x2 X, X0, Y, E, X_temp;
	            ae_int32x2 _0x400000 = AE_MOVDA32(0x00400000);
	            ae_int32x2 _0x00BAEC00 = AE_MOVDA32(0x00BAEC00);
	            ae_int32x2 _0x7fff00 = AE_MOVDA32(0x007fff00);
	            ae_f32x2 f;

	            AE_L16M_IU(rd, pfRd, 4);
	            X0 = (rd);
	            sx = AE_LT32(X0, AE_ZERO32());
	            X0 = AE_SLAI32(X0, 8);
	            X = AE_ABS32S(X0);
	            isZero = AE_EQ32(X, AE_ZERO32());
	            AE_MOVT32X2(X, _0x7fff00, isZero);
	            X_temp = AE_SRAI32(X, 8);
	            Y = AE_SUB32(_0x00BAEC00, X_temp); /* first approximation */
	            /* 3 iterations */
	            f = (_0x400000); AE_MULSFP32X2RAS_L(f, (X), (Y)); E = (f);
	            E = AE_SLLI32(E, 1 + 8);
	            f = (Y); AE_MULAFP32X2RAS_L(f, (E), (Y)); Y = (f);

	            f = (_0x400000); AE_MULSFP32X2RAS_L(f, (X), (Y)); E = (f);
	            E = AE_SLLI32(E, 1 + 8);
	            f = (Y); AE_MULAFP32X2RAS_L(f, (E), (Y)); Y = (f);

	            f = (_0x400000); AE_MULSFP32X2RAS_L(f, (X), (Y)); E = (f);
	            E = AE_SLLI32(E, 1 + 8);
	            f = (Y); AE_MULAFP32X2RAS_L(f, (E), (Y)); Y = (f);
	            /* restore original sign */
	            X = AE_NEG32(Y);
	            AE_MOVT32X2(Y, X, sx);
	            Y = (AE_SLLI32S((Y), 8));
	            Y = AE_SRAI32(Y, 16);
	            AE_S16_0_IP(AE_MOVINT16X4_FROMINT32X2(Y), castxcc(ae_int16, pfWr), 4);
	        }

	        exp += M;
	        frac += M;
	        x += M;
	        N = N - M;
	}
#else
    int16_t ALIGN(8) temp[MAX_ALLOCA_SZ / sizeof(int16_t)];
    int n=0;
    const ae_int16   * restrict px = (const ae_int16    *)x;
    ae_int16   * restrict pf = (ae_int16   *)frac;
    const ae_p16x2s  * restrict pfRd;
    const ae_int16x4  * restrict pfRd16;
    const ae_int16   * restrict pfWr;

    while (N > 0)
    {
        int M;
        M = MAX_ALLOCA_SZ / sizeof(int16_t);
        if (M > N) M = N;
        /* compute exponent and normalize inputs */
        px = (const ae_int16 *)x;
        pf = (ae_int16*)temp;
        for (n = 0; n < M; n++)
        {
            unsigned nsa;
            ae_int16x4 X;
            AE_L16_IP(X, px, sizeof(int16_t));

            nsa = AE_NSA16_0(X);
            XT_MOVEQZ(nsa,0x10,AE_MOVAD16_3(X));
            X = AE_SLAA16S(X, nsa);
            exp[n] = (int16_t)(nsa + 1);
            AE_S16_0_IP(X, pf, sizeof(int16_t));
        }
        __Pragma("no_reorder");
        pfRd = (const ae_p16x2s*)(temp - 2);
        pfRd16=(const ae_int16x4*)(temp);
        pfWr = (ae_int16 *)frac;
            ae_int32x2 _0x400000 = AE_MOVDA32(0x00400000);
            ae_int32x2 _0x00BAEC00 = AE_MOVDA32(0x00BAEC00);
            ae_int16x4 t_0x7fff00 = AE_MOVDA16(0x7fff);
            ae_valign align1 = AE_ZALIGN64();
        for (n = 0; n < (M&~3); n += 4)
        {
            xtbool2 sx,sx1;
            ae_int32x2 tX;
            ae_int16x4 rd;
            ae_int32x2 Y, E;
            ae_int16x4 X;
            ae_int32x2 tX_temp,tY,temp1,temp2;
            xtbool4  isZero;//tsx,

            ae_int32x2 f,f1;

            AE_L16X4_IP(rd, pfRd16, 8);
			      X = AE_ABS16S(rd);
			      isZero = AE_EQ16(X, AE_ZERO16());
			      AE_MOVT16X4(X, t_0x7fff00, isZero);
			      temp1=AE_SEXT32X2D16_32(rd);
			      temp2=AE_SEXT32X2D16_10(rd);
			      sx=AE_LT32(temp1, AE_ZERO32());
			      sx1=AE_LT32(temp2, AE_ZERO32());
			      tX_temp=AE_SEXT32X2D16_32(X);
			      tY = AE_SLAI32(tX_temp, 8);
			      Y = AE_SUB32(_0x00BAEC00, tY); /* first approximation */

            /* 3 iterations */
            f = (_0x400000);
            AE_MULSFP32X16X2RAS_H(f, (Y),AE_MOVF16X4_FROMINT16X4(X));

            E = (f);
            E = AE_SLLI32(E, 9);
            f1 = (Y);
            AE_MULAFP32X2RAS(f1, (E), (Y));
            Y = (f1);

            f = (_0x400000);
            AE_MULSFP32X16X2RAS_H(f, (Y),AE_MOVF16X4_FROMINT16X4(X));
            E = (f);
            E = AE_SLLI32(E, 9);
            f1 = (Y);
            AE_MULAFP32X2RAS(f1, (E), (Y));
            Y = (f1);

            f = (_0x400000);
            AE_MULSFP32X16X2RAS_H(f, (Y),AE_MOVF16X4_FROMINT16X4(X));
            E = (f);
            E = AE_SLLI32(E, 9);
            f1 = (Y);
            AE_MULAFP32X2RAS(f1, (E), (Y));
            Y = (f1);
            /* restore original sign */
            tX = AE_NEG32(Y);
            AE_MOVT32X2(Y, tX, sx);
            ae_int32x2 Y0 = (AE_SLLI32S((Y), 8));
           /* Y = AE_SRAI32(Y, 16);
            AE_S16_0_I(AE_MOVINT16X4_FROMINT32X2(Y), castxcc(ae_int16, pfWr), 2);
            Y = AE_INTSWAP(Y);
            AE_S16_0_IP(AE_MOVINT16X4_FROMINT32X2(Y), castxcc(ae_int16, pfWr), 4); */

			      tX_temp=AE_SEXT32X2D16_10(X);
			      tY = AE_SLAI32(tX_temp, 8);
			      Y = AE_SUB32(_0x00BAEC00, tY); /* first approximation */

            /* 3 iterations */
			      f1 = (_0x400000);
            AE_MULSFP32X16X2RAS_L(f1, (Y),AE_MOVF16X4_FROMINT16X4(X));

            E = (f1);
            E = AE_SLLI32(E, 9);
            f = (Y);
            AE_MULAFP32X2RAS(f, (E), (Y));
            Y = (f);

            f1 = (_0x400000);
            AE_MULSFP32X16X2RAS_L(f1, (Y),AE_MOVF16X4_FROMINT16X4(X));
            E = (f1);
            E = AE_SLLI32(E, 9);
            f = (Y);
            AE_MULAFP32X2RAS(f, (E), (Y));
            Y = (f);

            f1 = (_0x400000);
            AE_MULSFP32X16X2RAS_L(f1, (Y),AE_MOVF16X4_FROMINT16X4(X));
            E = (f1);
            E = AE_SLLI32(E, 9);
            f = (Y);
            AE_MULAFP32X2RAS(f, (E), (Y));
            Y = (f);
            /* restore original sign */
            tX = AE_NEG32(Y);
            AE_MOVT32X2(Y, tX, sx1);
            ae_int32x2 Y1 = (AE_SLLI32S((Y), 8));
            
            ae_int16x4 result_ae = AE_TRUNC16X4F32(Y0,Y1);
            AE_SA16X4_IP(result_ae,align1,castxcc(ae_int16x4,pfWr));

        }
        AE_SA64POS_FP(align1,castxcc(ae_int16x4,pfWr));
        pfRd = (const ae_p16x2s*)(temp+n-2);
        if (N & 2)
        {
            xtbool2 sx, isZero;
            ae_f32x2 rd;
            ae_int32x2 X, X0, Y, E, X_temp;
            ae_int32x2 _0x400000 = AE_MOVDA32(0x00400000);
            ae_int32x2 _0x00BAEC00 = AE_MOVDA32(0x00BAEC00);
            ae_int32x2 _0x7fff00 = AE_MOVDA32(0x007fff00);
            ae_f32x2 f;

            AE_L16X2M_IU(rd, pfRd, 4);
            X0 = (rd);
            sx = AE_LT32(X0, AE_ZERO32());
            X0 = AE_SLAI32(X0, 8);
            X = AE_ABS32S(X0);
            isZero = AE_EQ32(X, AE_ZERO32());
            AE_MOVT32X2(X, _0x7fff00, isZero);
            X_temp = AE_SRAI32(X, 8);
            Y = AE_SUB32(_0x00BAEC00, X_temp); /* first approximation */


            /* 3 iterations */
            f = (_0x400000);
            AE_MULSFP32X2RAS(f, (X), (Y));

            E = (f);
            E = AE_SLLI32(E, 1+8);
            f = (Y);
            AE_MULAFP32X2RAS(f, (E), (Y));
            Y = (f);

            f = (_0x400000);
            AE_MULSFP32X2RAS(f, (X), (Y));
            E = (f);
            E = AE_SLLI32(E, 1+8);
            f = (Y);
            AE_MULAFP32X2RAS(f, (E), (Y));
            Y = (f);

            f = (_0x400000);
            AE_MULSFP32X2RAS(f, (X), (Y));
            E = (f);
            E = AE_SLLI32(E, 1+8);
            f = (Y);
            AE_MULAFP32X2RAS(f, (E), (Y));
            Y = (f);
            /* restore original sign */
            X = AE_NEG32(Y);
            AE_MOVT32X2(Y, X, sx);
            Y = (AE_SLLI32S((Y), 8));
            Y = AE_SRAI32(Y, 16);
            AE_S16_0_I(AE_MOVINT16X4_FROMINT32X2(Y), castxcc(ae_int16, pfWr), 2);
            Y = AE_INTSWAP(Y);
            AE_S16_0_IP(AE_MOVINT16X4_FROMINT32X2(Y), castxcc(ae_int16, pfWr), 4);
        }
       if (N & 1)
        {
            xtbool2 sx, isZero;
            ae_f32x2 rd;
            ae_int32x2 X, X0, Y, E, X_temp;
            ae_int32x2 _0x400000 = AE_MOVDA32(0x00400000);
            ae_int32x2 _0x00BAEC00 = AE_MOVDA32(0x00BAEC00);
            ae_int32x2 _0x7fff00 = AE_MOVDA32(0x007fff00);
            ae_f32x2 f;

            AE_L16M_IU(rd, pfRd, 4);
            X0 = (rd);
            sx = AE_LT32(X0, AE_ZERO32());
            X0 = AE_SLAI32(X0, 8);
            X = AE_ABS32S(X0);
            isZero = AE_EQ32(X, AE_ZERO32());
            AE_MOVT32X2(X, _0x7fff00, isZero);
            X_temp = AE_SRAI32(X, 8);
            Y = AE_SUB32(_0x00BAEC00, X_temp); /* first approximation */
            /* 3 iterations */
            f = (_0x400000); AE_MULSFP32X2RAS_L(f, (X), (Y)); E = (f);
            E = AE_SLLI32(E, 1 + 8);
            f = (Y); AE_MULAFP32X2RAS_L(f, (E), (Y)); Y = (f);

            f = (_0x400000); AE_MULSFP32X2RAS_L(f, (X), (Y)); E = (f);
            E = AE_SLLI32(E, 1 + 8);
            f = (Y); AE_MULAFP32X2RAS_L(f, (E), (Y)); Y = (f);

            f = (_0x400000); AE_MULSFP32X2RAS_L(f, (X), (Y)); E = (f);
            E = AE_SLLI32(E, 1 + 8);
            f = (Y); AE_MULAFP32X2RAS_L(f, (E), (Y)); Y = (f);
            /* restore original sign */
            X = AE_NEG32(Y);
            AE_MOVT32X2(Y, X, sx);
            Y = (AE_SLLI32S((Y), 8));
            Y = AE_SRAI32(Y, 16);
            AE_S16_0_IP(AE_MOVINT16X4_FROMINT32X2(Y), castxcc(ae_int16, pfWr), 4);
        }

        exp += M;
        frac += M;
        x += M;
        N = N - M;
    }
#endif
} /* vec_recip16x16() */

