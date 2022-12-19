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
    C code optimized for HiFi1
*/
/*===========================================================================

    fft_unpack24to32_ie:
    Unpack 24 bits packed data to 32 bits unpacked.

    fft_pack32to24_ie:
    Pack 32-bits data into 24-bits packed. 

    N - number of input/output words.

    Called from: 
    fft_real_32x16_ie_24p
    fft_real_24x24_ie_24p
    ifft_real_32x16_ie_24p
    ifft_real_24x24_ie_24p

===========================================================================*/
#include "NatureDSP_Signal.h"
#include "common.h"

#define AE_SA24X2X4_IP_HIFI1(t0, t1, t2, t3, v, pout)	\
{														\
    ae_int32x2 t0_tmp, t0_tmp1, t0_tmp2, t1_tmp, t1_tmp1, t1_tmp2, t2_tmp, t2_tmp1, t2_tmp2;\
	t0_tmp  = AE_SEL32_LH(t0, t0);\
	t0_tmp1 = AE_SLAI32(t0_tmp, 24);\
	t0_tmp1 = AE_OR32(t0, t0_tmp1);\
	t0_tmp  = AE_SEL32_LH(t1, t1);\
	t0_tmp  = AE_SLAI32(t0_tmp, 16);\
	t0      = AE_SRLI32(t0, 8);\
	t0_tmp  = AE_OR32(t0_tmp, t0);\
	t0_tmp2 = AE_SEL32_HL(t0_tmp1, t0_tmp);\
	AE_SA32X2_IP(t0_tmp2, v, pout);\
	t1_tmp  = AE_SEL32_LH(t1, t1);\
	t1_tmp1 = AE_SRLI32(t1, 16);\
	t1_tmp  = AE_SLAI32(t1_tmp, 8);\
	t1_tmp  = AE_OR32(t1_tmp1, t1_tmp);\
	t1_tmp1 = AE_SEL32_LH(t2, t2);\
	t1_tmp1 = AE_SLAI32(t1_tmp1, 24);\
	t1_tmp1 = AE_OR32(t2, t1_tmp1);\
	t1_tmp2 = AE_SEL32_HH(t1_tmp, t1_tmp1);\
	AE_SA32X2_IP(t1_tmp2, v, pout);\
	t2_tmp  = AE_SEL32_LH(t2, t2);\
	t2_tmp  = AE_SRLI32(t2_tmp, 8);\
	t2_tmp1 = AE_SLAI32(t3, 16);\
	t2_tmp  = AE_OR32(t2_tmp, t2_tmp1);\
	t2_tmp1 = AE_SEL32_LH(t3, t3);\
	t3 		= AE_SLAI32(t3, 8);\
	t2_tmp1 = AE_SRLI32(t2_tmp1, 16);\
	t2_tmp1 = AE_OR32(t3, t2_tmp1);\
	t2_tmp2 = AE_SEL32_HL(t2_tmp, t2_tmp1);\
	AE_SA32X2_IP(t2_tmp2, v, pout);\
}													\

void fft_pack32to24_32x16_ie(uint8_t *x,  uint8_t *y, int N)
{
    ae_f24x2 d; 
    ae_valign v = AE_ZALIGN64();
    ae_f32x2 *restrict pin  = (ae_f32x2 *)x; 
    void *restrict pout = (ae_f32x2 *)y; 
    int i; 

    NASSERT_ALIGN8(x);
    NASSERT_ALIGN8(y);
    NASSERT((N&1)==0);

    for(i=0; i<N>>3; i++)
    {
        ae_int32x2 t0, t1, t2, t3;
        AE_L32X2_IP(t0, castxcc(ae_int32x2, pin), sizeof(*pin) );
        AE_L32X2_IP(t1, castxcc(ae_int32x2, pin), sizeof(*pin) );
        AE_L32X2_IP(t2, castxcc(ae_int32x2, pin), sizeof(*pin) );
        AE_L32X2_IP(t3, castxcc(ae_int32x2, pin), sizeof(*pin) );
        /* Do rounding before packing */
        t0 = AE_ADD32(t0, 0x80);
        t0 = AE_SRLI32(t0, 8);
		t1 = AE_ADD32(t1, 0x80);
		t1 = AE_SRLI32(t1, 8);
		t2 = AE_ADD32(t2, 0x80);
		t2 = AE_SRLI32(t2, 8);
		t3 = AE_ADD32(t3, 0x80);
        t3 = AE_SRLI32(t3, 8);
        AE_SA24X2X4_IP_HIFI1(t0, t1, t2, t3, v, (ae_f32x2 *)pout); 
    }
	if(N%8==2)
    {
        ae_int32x2 t;
        AE_L32X2_IP(t, castxcc(ae_int32x2, pin), sizeof(*pin) );
        /* Do rounding before packing */
        t = AE_ADD32(t, 0x80);
        t = AE_SRAI32(t, 8);
        d = AE_MOVF24X2_FROMINT32X2(t);
        AE_SA24X2_IP(d, v, (ae_f24x2 *)pout); 
    }
    AE_SA64POS_FP(v, pout); 
}

void fft_pack32to24_ie(uint8_t *x,  uint8_t *y, int N)
{
    ae_f24x2 d; 
    ae_valign v = AE_ZALIGN64();
    ae_f32x2 *restrict pin  = (ae_f32x2 *)x; 
    void *restrict pout = (ae_f32x2 *)y; 
    int i; 
	ae_int32x2 d_msbz = AE_MOVDA32(0x00FFFFFF);

    NASSERT_ALIGN8(x);
    NASSERT_ALIGN8(y);
    NASSERT((N&1)==0);

    for(i=0; i<N>>3; i++)
    {
        ae_int32x2 t0, t1, t2, t3;
        AE_L32X2_IP(t0, castxcc(ae_int32x2, pin), sizeof(*pin) );
        AE_L32X2_IP(t1, castxcc(ae_int32x2, pin), sizeof(*pin) );
        AE_L32X2_IP(t2, castxcc(ae_int32x2, pin), sizeof(*pin) );
        AE_L32X2_IP(t3, castxcc(ae_int32x2, pin), sizeof(*pin) );
        t0 = AE_AND32(t0, d_msbz);
        t1 = AE_AND32(t1, d_msbz);
        t2 = AE_AND32(t2, d_msbz);
        t3 = AE_AND32(t3, d_msbz);
        AE_SA24X2X4_IP_HIFI1(t0, t1, t2, t3, v, (ae_f32x2 *)pout); 
    }
	if(N%8==2)
    {
        ae_int32x2 t;
        AE_L32X2_IP(t, castxcc(ae_int32x2, pin), sizeof(*pin) );
        d = AE_MOVF24X2_FROMINT32X2(t);
        AE_SA24X2_IP(d, v, (ae_f24x2 *)pout); 
    }
    AE_SA64POS_FP(v, pout); 
}


#define AE_LA24X2X4_IP_HIFI1(d24x2_0, d24x2_1, d24x2_2, d24x2_3, v, pin) 				 \
{																		  				 \
	ae_int32x2 d0, d1, d2, d0_tmp, d0_tmp1, d1_tmp, d1_tmp1, d2_tmp, d2_tmp1, d3_tmp, d3_tmp1, d_lsbz;\
	d_lsbz = AE_MOVDA32(0xFFFFFF00);										 			 \
	AE_LA32X2_IP(d0, v, (ae_int32x2*)pin);												 \
	AE_LA32X2_IP(d1, v, (ae_int32x2*)pin);                                               \
	AE_LA32X2_IP(d2, v, (ae_int32x2*)pin);                                               \
	d0_tmp  = AE_SLAI32(d0, 8);                                                          \
	d0_tmp1 = AE_SEL32_LH(d0, d0);                                                       \
	d0_tmp1 = AE_MOVINT32X2_FROMINT64(AE_SRAI64(AE_MOVINT64_FROMINT32X2(d0_tmp1), 16));  \
	d0_tmp1 = AE_AND32(d0_tmp1, d_lsbz);                                                 \
	d24x2_0 = AE_SEL32_HL(d0_tmp, d0_tmp1);                                              \
	d1_tmp  = AE_SEL32_HL(d1, d0);                                                       \
	d1_tmp1 = AE_MOVINT32X2_FROMINT64(AE_SRAI64(AE_MOVINT64_FROMINT32X2(d1_tmp), 8));    \
	d1_tmp1 = AE_AND32(d1_tmp1, d_lsbz);                                                 \
	d1_tmp  = AE_AND32(d1_tmp, d_lsbz);                                                  \
	d24x2_1 = AE_SEL32_LH(d1_tmp1, d1_tmp);                                              \
	d2_tmp  = AE_SLAI32(d1, 8);                                                          \
	d2_tmp1 = AE_SEL32_HL(d2, d1);                                                       \
	d2_tmp1 = AE_MOVINT32X2_FROMINT64(AE_SLAA64(AE_MOVINT64_FROMINT32X2(d2_tmp1), 16));  \
	d2_tmp1 = AE_AND32(d2_tmp1, d_lsbz);                                                 \
	d24x2_2 = AE_SEL32_LH(d2_tmp, d2_tmp1);                                              \
	d3_tmp  = AE_SEL32_LH(d2, d2);                                                       \
	d3_tmp1 = AE_MOVINT32X2_FROMINT64(AE_SRAI64(AE_MOVINT64_FROMINT32X2(d3_tmp), 8));    \
	d3_tmp1 = AE_AND32(d3_tmp1, d_lsbz);                                                 \
	d3_tmp  = AE_AND32(d3_tmp, d_lsbz);                                                  \
	d24x2_3 = AE_SEL32_LH(d3_tmp1, d3_tmp);                                              \
}\

void fft_unpack24to32_ie(uint8_t *x,  uint8_t *y, int N)
{
    ae_int32x2 d24x2_0, d24x2_1, d24x2_2, d24x2_3;
    ae_int24x2 d; 
	d24x2_0 = d24x2_1 = d24x2_2 = d24x2_3 = AE_ZERO32();
    ae_valign v = AE_LA64_PP(x);
    void *pin  = x; 
    ae_f32x2 *pout = (ae_f32x2 *)y; 
    int i; 

    NASSERT_ALIGN8(x);
    NASSERT_ALIGN8(y);
    NASSERT((N&1)==0);
    for(i=0; i<(N>>3); i++)
    {
        AE_LA24X2X4_IP_HIFI1(d24x2_0, d24x2_1, d24x2_2, d24x2_3, v, pin);
		AE_S32X2_IP(d24x2_0, pout, sizeof(*pout));
        AE_S32X2_IP(d24x2_1, pout, sizeof(*pout));
        AE_S32X2_IP(d24x2_2, pout, sizeof(*pout));
        AE_S32X2_IP(d24x2_3, pout, sizeof(*pout));
    }
	if(N%8==2)
	{
		AE_LA24X2_IP(d, v, pin);
        AE_S32X2F24_IP(d, (ae_f24x2*)pout, sizeof(*pout)); 
    }

}
