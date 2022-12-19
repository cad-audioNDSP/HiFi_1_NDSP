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

void vec_recip24x24 (
                  f24       * restrict frac,
                  int16_t   * restrict exp,
                  const f24 * restrict x,
                  int N)

{
    int n;
    static const ALIGN(8) int32_t cnst01[]={0,1};
    ae_int32x2 _01;


    const ae_int32x2 * restrict pfRd;
    const ae_int32x2 * restrict pfWr;
          ae_valign    wr_align,rd_align;

    if(N<=0) return;
	#if XCHAL_HAVE_HIFI1_LOW_LATENCY_MAC_FMA
        const ae_int32x2   * restrict px = (const ae_int32x2   *)x;
        ae_int32x2   * restrict pf = (      ae_int32x2   *)frac;
        ae_int16x4   * restrict expPtr = (  ae_int16x4   *)exp;
        ae_int16x4 nsa;
        xtbool2 isZero1,isZero2;
        xtbool4 isZero;
        ae_valign  expalign, Dalign, DalignSav;
        ae_int16x4 nsa16x4_L0,nsa16x4_L1,nsa16x4_H0,nsa16x4_H1;
        ae_int32x2 X, XT_L,XT_H;
        Dalign = AE_LA64_PP(px);
    	expalign = AE_ZALIGN64();
    	DalignSav = AE_ZALIGN64();
        for(n=0; n<(N>>2); n++)
        {
        	//-------------Set 1---------------------------------------------------
        	AE_LA32X2_IP(X,Dalign,px);
			X=AE_SRAI32(X,8);
			isZero1=AE_EQ32(X,AE_MOVDA32(0)); //compare and keep it aside

			nsa16x4_L0 =(ae_int16) AE_NSA32_L(X);//process lower 32 bit

			XT_L=AE_SLAA32S(X, (ae_int16)nsa16x4_L0);
			nsa16x4_L1 = (ae_int16)AE_NSA32_L(AE_INTSWAP(X)); //apply NSA on higher 32 bits

			XT_H=AE_SLAA32S(AE_INTSWAP(X),(ae_int16)nsa16x4_L1);
			X = AE_SEL32_LL(XT_H,XT_L);

			AE_SA32X2_IP(X,DalignSav,pf);
			//-------------------------Set 2-----------------------------------------
			AE_LA32X2_IP(X,Dalign,px);
			X=AE_SRAI32(X,8);
			isZero2=AE_EQ32(X,AE_MOVDA32(0)); //compare and keep it aside
			nsa16x4_H0 =(ae_int16) AE_NSA32_L(X);//process lower 32 bit

			XT_L=AE_SLAA32S(X, (ae_int16)nsa16x4_H0);
			nsa16x4_H1 = (ae_int16)AE_NSA32_L(AE_INTSWAP(X)); //apply NSA on higher 32 bits

			XT_H=AE_SLAA32S(AE_INTSWAP(X),(ae_int16)nsa16x4_H1);

			X = AE_SEL32_LL(XT_H,XT_L);
			AE_SA32X2_IP(X,DalignSav,pf);
            //-----------------------------------------------------------------------
			nsa16x4_L0 = AE_SEL16_7362(nsa16x4_L1,nsa16x4_L0);
			nsa16x4_H0 = AE_SEL16_7362(nsa16x4_H1,nsa16x4_H0);
			nsa = AE_SEL16_7610(nsa16x4_L0,nsa16x4_H0);
			nsa=AE_SUB16(nsa, AE_MOVDA16(7)); //subtract 7 from H and L
			isZero = vbool2_join_vbool4(isZero2, isZero1);
			AE_MOVT16X4(nsa,0x28,(isZero));  // return biggest exponent for zero input
			AE_SA16X4_IP(nsa,expalign,expPtr);
	    }
        AE_SA64POS_FP(expalign, expPtr);
        AE_SA64POS_FP(DalignSav, pf);
        //tail handling
        if(N&2)
         {
        	AE_LA32X2_IP(X,Dalign,px);
			X=AE_SRAI32(X,8);
			X = AE_INTSWAP(X); //process in reverse way
			isZero1=AE_EQ32(X,AE_MOVDA32(0)); //compare and keep it aside

			nsa16x4_L1 =(ae_int16) AE_NSA32_L(X);//process lower 32 bit

			XT_H=AE_SLAA32S(X, (ae_int16)nsa16x4_L1);
			nsa16x4_L0 = (ae_int16)AE_NSA32_L(AE_INTSWAP(X)); //apply NSA on higher 32 bits

			XT_L=AE_SLAA32S(AE_INTSWAP(X),(ae_int16)nsa16x4_L0);
			X = AE_SEL32_LL(XT_H,XT_L);

			AE_SA32X2_IP(X,DalignSav,pf);

			nsa = AE_SEL16_7362(nsa16x4_L0,nsa16x4_L1); //2 higher 2 lower
			nsa=AE_SUB16(nsa, AE_MOVDA16(7)); //subtract 7 from all
			isZero = vbool2_join_vbool4(isZero1, isZero1);
			AE_MOVT16X4(nsa,0x28,(isZero));
			AE_S16_0_IP(nsa,(ae_int16*)expPtr,(sizeof(ae_int16)));
			nsa = AE_SHORTSWAP(nsa);

			AE_S16_0_IP(nsa,(ae_int16*)expPtr,(sizeof(ae_int16)));
			AE_SA64POS_FP(DalignSav, pf);
          }

        if(N&1)
        {
          AE_L32_IP(X, (ae_int32*)px, sizeof(ae_int32));
		  X=AE_SRAI32(X,8);
		  isZero1=AE_EQ32(X,AE_MOVDA32(0)); //compare and keep it aside
		  isZero = vbool2_join_vbool4(isZero1, isZero1);

          nsa16x4_H0 = (ae_int16)AE_NSA32_L(X); //apply NSA on higher 32 bits
		  XT_H=AE_SLAA32S(X,(ae_int16)nsa16x4_H0);
		  X = AE_SEL32_LL(XT_H,XT_H);
		  nsa=AE_SUB16(nsa16x4_H0, AE_MOVDA16(7)); //subtract 7 from H and L
		  AE_MOVT16X4(nsa,0x28,(isZero));
		  AE_S16_0_IP(nsa,(ae_int16*)expPtr,0);
		  AE_S32_L_IP(X,(ae_int32*)pf,0);
        }

    __Pragma("no_reorder");
    pfRd = (const ae_int32x2*)frac;
    pfWr = (      ae_int32x2*)frac;
    wr_align=AE_ZALIGN64();
    rd_align=AE_LA64_PP(pfRd);
    _01 = AE_L32X2_I((const ae_int32x2*)cnst01,0);
    for(n=N; n>0; n-=2)
     {
		  xtbool2 sx,wr_mask,isZero;
		  ae_f32x2 f;
		  ae_int32x2 X0,X,Y,E,_0x400000;
		  _0x400000=AE_MOVDA32(0x400000);
		  AE_LA32X2_IP(X0,rd_align,pfRd);
		  sx=AE_LT32(X0,AE_ZERO32());
		  X=AE_INT32X2_ABS32S(X0);
		  isZero=AE_EQ32(X,AE_ZERO32());
		  AE_MOVT32X2(X,AE_MOVDA32(0x7fffffff),isZero);

		  X=AE_SRAI32(X,8);
		  Y=AE_SUB32(AE_MOVDA32((int32_t)0x00BAEC00),X); /* first approximation */
		  X=AE_SLAI32(X,8);

		  /* 3 iterations to achieve 1 LSB accuracy in mantissa */
		  f=(_0x400000); AE_MULSFP32X2RAS(f,(X),(Y)); E=(f);
		  E=AE_SLLI32(E,9);
		  f=(Y); AE_MULAFP32X2RAS(f,(E),(Y)); Y=(f);
		  f=(_0x400000); AE_MULSFP32X2RAS(f,(X),(Y)); E=(f);
		  E=AE_SLLI32(E,9);
		  f=(Y); AE_MULAFP32X2RAS(f,(E),(Y)); Y=(f);
		  f=(_0x400000); AE_MULSFP32X2RAS(f,(X),(Y)); E=(f);
		  E=AE_SLLI32(E,9);
		  f=(Y); AE_MULAFP32X2RAS(f,(E),(Y)); Y=(f);

		  /* restore original sign */
		  X=AE_NEG32(Y);
		  AE_MOVT32X2(Y,X,sx);
		  Y=(AE_SLLI32S((Y),8));
		  wr_mask=AE_LT32(_01,AE_MOVDA32(n)); /* compute mask for last incomplete iteration */
		  AE_MOVT32X2(X0,Y,wr_mask);
		  AE_SA32X2_IP(X0, wr_align, castxcc(ae_int32x2, pfWr));
	   }
       AE_SA64POS_FP(wr_align, castxcc(void, pfWr));
        #else
       const ae_int32   * restrict px = (const ae_int32   *)x;
       	   	 ae_int32   * restrict pf = (      ae_int32   *)frac;
       /* compute exponent and normalize inputs */
           px = (const ae_int32  *)x;
           pf = (      ae_int32*)frac;
           for(n=0; n<N; n++)
           {
               xtbool isZero;
               unsigned nsa;
               ae_int32x2 X;
               ae_int64   Z;
               AE_L32_IP(X, px, sizeof(int32_t));
       	       X=AE_SRAI32(X,8);
               Z = AE_MOVINT64_FROMINT32X2(X);
               isZero=AE_EQ64(Z,AE_ZERO64());
               nsa=AE_NSAZ32_L(X);
               XT_MOVT(nsa,0x28,(isZero));  /* return biggest exponent for zero input */
               X=AE_SLAA32S(X,nsa);
               exp[n]=(int16_t)(nsa-7);
               AE_S32_L_IP(X,pf,sizeof(int32_t));
           }
           __Pragma("no_reorder");
           pfRd = (const ae_int32x2*)frac;
           pfWr = (      ae_int32x2*)frac;
           wr_align=AE_ZALIGN64();
           rd_align=AE_LA64_PP(pfRd);
           _01 = AE_L32X2_I((const ae_int32x2*)cnst01,0);
           for(n=N; n>0; n-=2)
           {
               xtbool2 sx,wr_mask,isZero;
               ae_f32x2 f;
               ae_int32x2 X0,X,Y,E,_0x400000;
               _0x400000=AE_MOVDA32(0x400000);
               AE_LA32X2_IP(X0,rd_align,pfRd);
               sx=AE_LT32(X0,AE_ZERO32());
               X=AE_INT32X2_ABS32S(X0);
               isZero=AE_EQ32(X,AE_ZERO32());
               AE_MOVT32X2(X,AE_MOVDA32(0x7fffffff),isZero);

               X=AE_SRAI32(X,8);
               Y=AE_SUB32(AE_MOVDA32((int32_t)0x00BAEC00),X); /* first approximation */
               X=AE_SLAI32(X,8);

               /* 3 iterations to achieve 1 LSB accuracy in mantissa */
               f=(_0x400000); AE_MULSFP32X2RAS(f,(X),(Y)); E=(f);
               E=AE_SLLI32(E,9);
               f=(Y); AE_MULAFP32X2RAS(f,(E),(Y)); Y=(f);
               f=(_0x400000); AE_MULSFP32X2RAS(f,(X),(Y)); E=(f);
               E=AE_SLLI32(E,9);
               f=(Y); AE_MULAFP32X2RAS(f,(E),(Y)); Y=(f);
               f=(_0x400000); AE_MULSFP32X2RAS(f,(X),(Y)); E=(f);
               E=AE_SLLI32(E,9);
               f=(Y); AE_MULAFP32X2RAS(f,(E),(Y)); Y=(f);

               /* restore original sign */
               X=AE_NEG32(Y);
               AE_MOVT32X2(Y,X,sx);
               Y=(AE_SLLI32S((Y),8));
               wr_mask=AE_LT32(_01,AE_MOVDA32(n)); /* compute mask for last incomplete iteration */
               AE_MOVT32X2(X0,Y,wr_mask);
               AE_SA32X2_IP(X0, wr_align, castxcc(ae_int32x2, pfWr));
           }
           AE_SA64POS_FP(wr_align, castxcc(void, pfWr));
#endif
}

