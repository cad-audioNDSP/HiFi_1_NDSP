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
/*
NatureDSP Signal Processing Library. FFT part
*/
#include "NatureDSP_Signal.h"
#include "common.h"

/*===========================================================================
Unpack to 32 bits  and calculate block exponent
Called from :

fft_real_24x24_ie_24p
ifft_real_24x24_ie_24p

===========================================================================*/
#define AE_LA24X2X4_IP_HIFI1(d24x2_0, d24x2_1, d24x2_2, d24x2_3, v, pin) 				 \
{																		  				 \
	ae_int32x2 d0, d1, d2, d0_tmp1, d1_tmp, d1_tmp1, d2_tmp1, d3_tmp, d3_tmp1;\
	AE_LA32X2_IP(d0, v, (ae_int32x2*)pin);												 \
	AE_LA32X2_IP(d1, v, (ae_int32x2*)pin);                                               \
	AE_LA32X2_IP(d2, v, (ae_int32x2*)pin);                                               \
	d0_tmp1 = AE_SEL32_LH(d0, d0);                                                       \
	d0_tmp1 = AE_MOVINT32X2_FROMINT64(AE_SLAA64(AE_MOVINT64_FROMINT32X2(d0_tmp1), 8));   \
	d24x2_0 = AE_SEL32_HH(d0, d0_tmp1);                                              	 \
	d1_tmp  = AE_SEL32_HL(d1, d0);                                                       \
	d1_tmp1 = AE_MOVINT32X2_FROMINT64(AE_SLAA64(AE_MOVINT64_FROMINT32X2(d1_tmp), 16));   \
	d1_tmp  = AE_SRLI32(d1, 8);                                                          \
	d24x2_1 = AE_SEL32_HH(d1_tmp1, d1_tmp);                                              \
	d2_tmp1 = AE_SEL32_HL(d2, d1);                                                       \
	d2_tmp1 = AE_MOVINT32X2_FROMINT64(AE_SLAA64(AE_MOVINT64_FROMINT32X2(d2_tmp1), 8));   \
	d24x2_2 = AE_SEL32_LH(d1, d2_tmp1);                                              \
	d3_tmp  = AE_SEL32_LH(d2, d2);                                                       \
	d3_tmp1 = AE_MOVINT32X2_FROMINT64(AE_SLAA64(AE_MOVINT64_FROMINT32X2(d3_tmp), 16));   \
	d3_tmp  = AE_SRLI32(d2, 8);                                                  		 \
	d24x2_3 = AE_SEL32_HL(d3_tmp1, d3_tmp);                                              \
	d24x2_0 = AE_SEXT32X2D24(AE_MOVINT24X2_FROMINT32X2(d24x2_0));						 \
	d24x2_1 = AE_SEXT32X2D24(AE_MOVINT24X2_FROMINT32X2(d24x2_1));						 \
	d24x2_2 = AE_SEXT32X2D24(AE_MOVINT24X2_FROMINT32X2(d24x2_2));						 \
	d24x2_3 = AE_SEXT32X2D24(AE_MOVINT24X2_FROMINT32X2(d24x2_3));						 \
}																						 \

int fft_unpack24to32_s1_ie(uint8_t *x, uint8_t *y, int N)
{  
    ae_int32x2 d24x2_0, d24x2_1, d24x2_2, d24x2_3;
	ae_int24x2 d;
	d24x2_0 = d24x2_1 = d24x2_2 = d24x2_3 = AE_ZERO32();

    ae_valign v = AE_LA64_PP(x);
    void *pin = x;
    ae_f32x2 *pout = (ae_f32x2 *)y;
    int i;
    ae_int32x2  maxV = 0, minV = 0;
    ae_int32x2 tmp32;
    int exp;

    NASSERT_ALIGN8(x);
    NASSERT_ALIGN8(y);
    NASSERT((N & 1) == 0);
    for (i = 0; i<(N>>3); i++)
    {
        AE_LA24X2X4_IP_HIFI1(d24x2_0, d24x2_1, d24x2_2, d24x2_3, v, pin);
        tmp32 = d24x2_0;
        maxV = AE_MAX32(maxV, tmp32);
        minV = AE_MIN32(minV, tmp32);
		tmp32 = d24x2_1;
		maxV = AE_MAX32(maxV, tmp32);
		minV = AE_MIN32(minV, tmp32);
		tmp32 = d24x2_2;
		maxV = AE_MAX32(maxV, tmp32);
		minV = AE_MIN32(minV, tmp32);
		tmp32 = d24x2_3;
		maxV = AE_MAX32(maxV, tmp32);
		minV = AE_MIN32(minV, tmp32);
        AE_S32X2_IP(d24x2_0, pout, sizeof(*pout));
        AE_S32X2_IP(d24x2_1, pout, sizeof(*pout));
        AE_S32X2_IP(d24x2_2, pout, sizeof(*pout));
        AE_S32X2_IP(d24x2_3, pout, sizeof(*pout));
    }
	/*N is multiple of 8*/
	if(N%8==2)
	{
		AE_LA24X2_IP(d, v, pin);
        tmp32 = d;
        maxV = AE_MAX32(maxV, tmp32);
        minV = AE_MIN32(minV, tmp32);
        AE_S32X2_IP(tmp32, pout, sizeof(*pout));
	}
	maxV = AE_MAX32(maxV, AE_NEG32S(minV));
    maxV = AE_MAX32(maxV, AE_SEL32_LH(maxV, maxV));
    exp = AE_NSAZ32_L(maxV);
    return exp;
}
