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
	NatureDSP_Signal library. FFT part
    C code with generic optimization

    twiddle factors for 24x24 real fwd/rev FFT transforms
*/

#include "NatureDSP_Signal.h"
#include "fft_cplx_twiddles_24x24.h"
#include "common.h"

/*
    Twiddle tables are received by sequential writing twiddle factors used in the original fft loops
    However, last radix2/radix4 stage is omitted because they are processed separately and combined 
    with bitreversing
    Higher word of each item is a cosine of a twiddle angle and lower word is a sine. All numbers are
    represented in Q31 with cleared 8 LSBs. Twiddle angle is always an integral number multiplied by 
    2*pi/N where N is FFT length. 
    For example, the sequence of twiddle angles is:
    16-pts transform 
    [0 0 0 1 2 3 2 4 6 3 6 -7]/16
    32-pts transform 
    [0 0 0 1 2 3 2 4 6 3 6 9 4 8 12 5 10 15 6 12 -14 7 14 -11 0 0 0 4 8 12 0 0 0 4 8 12 0 0 0 4 8 12 0 0 0 4 8 12 ]/32
    ....
*/
const int32_t MSC_ALIGNED ifft24x24_twd256[12*32] GCC_ALIGNED = {
(int32_t)0x7fffff00, (int32_t)0x00000000, (int32_t)0x7fffff00, (int32_t)0x00000000, (int32_t)0x7fffff00, (int32_t)0x00000000,(int32_t)0x7ff62100, (int32_t)0x03242a00, (int32_t)0x7fd88700, (int32_t)0x0647d900, (int32_t)0x7fa73600, (int32_t)0x096a9000,
(int32_t)0x7fd88700, (int32_t)0x0647d900, (int32_t)0x7f623600, (int32_t)0x0c8bd300, (int32_t)0x7e9d5500, (int32_t)0x12c81000,(int32_t)0x7fa73600, (int32_t)0x096a9000, (int32_t)0x7e9d5500, (int32_t)0x12c81000, (int32_t)0x7ce3ce00, (int32_t)0x1c0b8200,
(int32_t)0x7f623600, (int32_t)0x0c8bd300, (int32_t)0x7d8a5f00, (int32_t)0x18f8b800, (int32_t)0x7a7d0500, (int32_t)0x25280c00,(int32_t)0x7f099100, (int32_t)0x0fab2700, (int32_t)0x7c29fb00, (int32_t)0x1f19f900, (int32_t)0x776c4e00, (int32_t)0x2e110a00,
(int32_t)0x7e9d5500, (int32_t)0x12c81000, (int32_t)0x7a7d0500, (int32_t)0x25280c00, (int32_t)0x73b5eb00, (int32_t)0x36ba2000,(int32_t)0x7e1d9300, (int32_t)0x15e21400, (int32_t)0x78848400, (int32_t)0x2b1f3400, (int32_t)0x6f5f0200, (int32_t)0x3f174900,
(int32_t)0x7d8a5f00, (int32_t)0x18f8b800, (int32_t)0x7641af00, (int32_t)0x30fbc500, (int32_t)0x6a6d9800, (int32_t)0x471cec00,(int32_t)0x7ce3ce00, (int32_t)0x1c0b8200, (int32_t)0x73b5eb00, (int32_t)0x36ba2000, (int32_t)0x64e88900, (int32_t)0x4ebfe800,
(int32_t)0x7c29fb00, (int32_t)0x1f19f900, (int32_t)0x70e2cb00, (int32_t)0x3c56ba00, (int32_t)0x5ed77c00, (int32_t)0x55f5a400,(int32_t)0x7b5d0300, (int32_t)0x2223a400, (int32_t)0x6dca0d00, (int32_t)0x41ce1e00, (int32_t)0x5842dd00, (int32_t)0x5cb42000,
(int32_t)0x7a7d0500, (int32_t)0x25280c00, (int32_t)0x6a6d9800, (int32_t)0x471cec00, (int32_t)0x5133cc00, (int32_t)0x62f20100,(int32_t)0x798a2300, (int32_t)0x2826b900, (int32_t)0x66cf8100, (int32_t)0x4c3fdf00, (int32_t)0x49b41500, (int32_t)0x68a69e00,
(int32_t)0x78848400, (int32_t)0x2b1f3400, (int32_t)0x62f20100, (int32_t)0x5133cc00, (int32_t)0x41ce1e00, (int32_t)0x6dca0d00,(int32_t)0x776c4e00, (int32_t)0x2e110a00, (int32_t)0x5ed77c00, (int32_t)0x55f5a400, (int32_t)0x398cdd00, (int32_t)0x72552c00,
(int32_t)0x7641af00, (int32_t)0x30fbc500, (int32_t)0x5a827900, (int32_t)0x5a827900, (int32_t)0x30fbc500, (int32_t)0x7641af00,(int32_t)0x7504d300, (int32_t)0x33def200, (int32_t)0x55f5a400, (int32_t)0x5ed77c00, (int32_t)0x2826b900, (int32_t)0x798a2300,
(int32_t)0x73b5eb00, (int32_t)0x36ba2000, (int32_t)0x5133cc00, (int32_t)0x62f20100, (int32_t)0x1f19f900, (int32_t)0x7c29fb00,(int32_t)0x72552c00, (int32_t)0x398cdd00, (int32_t)0x4c3fdf00, (int32_t)0x66cf8100, (int32_t)0x15e21400, (int32_t)0x7e1d9300,
(int32_t)0x70e2cb00, (int32_t)0x3c56ba00, (int32_t)0x471cec00, (int32_t)0x6a6d9800, (int32_t)0x0c8bd300, (int32_t)0x7f623600,(int32_t)0x6f5f0200, (int32_t)0x3f174900, (int32_t)0x41ce1e00, (int32_t)0x6dca0d00, (int32_t)0x03242a00, (int32_t)0x7ff62100,
(int32_t)0x6dca0d00, (int32_t)0x41ce1e00, (int32_t)0x3c56ba00, (int32_t)0x70e2cb00, (int32_t)0xf9b82600, (int32_t)0x7fd88700,(int32_t)0x6c242900, (int32_t)0x447acd00, (int32_t)0x36ba2000, (int32_t)0x73b5eb00, (int32_t)0xf054d800, (int32_t)0x7f099100,
(int32_t)0x6a6d9800, (int32_t)0x471cec00, (int32_t)0x30fbc500, (int32_t)0x7641af00, (int32_t)0xe7074700, (int32_t)0x7d8a5f00,(int32_t)0x68a69e00, (int32_t)0x49b41500, (int32_t)0x2b1f3400, (int32_t)0x78848400, (int32_t)0xdddc5b00, (int32_t)0x7b5d0300,
(int32_t)0x66cf8100, (int32_t)0x4c3fdf00, (int32_t)0x25280c00, (int32_t)0x7a7d0500, (int32_t)0xd4e0cb00, (int32_t)0x78848400,(int32_t)0x64e88900, (int32_t)0x4ebfe800, (int32_t)0x1f19f900, (int32_t)0x7c29fb00, (int32_t)0xcc210d00, (int32_t)0x7504d300,
(int32_t)0x62f20100, (int32_t)0x5133cc00, (int32_t)0x18f8b800, (int32_t)0x7d8a5f00, (int32_t)0xc3a94500, (int32_t)0x70e2cb00,(int32_t)0x60ec3800, (int32_t)0x539b2a00, (int32_t)0x12c81000, (int32_t)0x7e9d5500, (int32_t)0xbb853200, (int32_t)0x6c242900,
(int32_t)0x5ed77c00, (int32_t)0x55f5a400, (int32_t)0x0c8bd300, (int32_t)0x7f623600, (int32_t)0xb3c02000, (int32_t)0x66cf8100,(int32_t)0x5cb42000, (int32_t)0x5842dd00, (int32_t)0x0647d900, (int32_t)0x7fd88700, (int32_t)0xac64d500, (int32_t)0x60ec3800,
(int32_t)0x5a827900, (int32_t)0x5a827900, (int32_t)0x00000000, (int32_t)0x7fffff00, (int32_t)0xa57d8600, (int32_t)0x5a827900,(int32_t)0x5842dd00, (int32_t)0x5cb42000, (int32_t)0xf9b82600, (int32_t)0x7fd88700, (int32_t)0x9f13c700, (int32_t)0x539b2a00,
(int32_t)0x55f5a400, (int32_t)0x5ed77c00, (int32_t)0xf3742c00, (int32_t)0x7f623600, (int32_t)0x99307e00, (int32_t)0x4c3fdf00,(int32_t)0x539b2a00, (int32_t)0x60ec3800, (int32_t)0xed37ef00, (int32_t)0x7e9d5500, (int32_t)0x93dbd600, (int32_t)0x447acd00,
(int32_t)0x5133cc00, (int32_t)0x62f20100, (int32_t)0xe7074700, (int32_t)0x7d8a5f00, (int32_t)0x8f1d3400, (int32_t)0x3c56ba00,(int32_t)0x4ebfe800, (int32_t)0x64e88900, (int32_t)0xe0e60600, (int32_t)0x7c29fb00, (int32_t)0x8afb2c00, (int32_t)0x33def200,
(int32_t)0x4c3fdf00, (int32_t)0x66cf8100, (int32_t)0xdad7f300, (int32_t)0x7a7d0500, (int32_t)0x877b7b00, (int32_t)0x2b1f3400,(int32_t)0x49b41500, (int32_t)0x68a69e00, (int32_t)0xd4e0cb00, (int32_t)0x78848400, (int32_t)0x84a2fc00, (int32_t)0x2223a400,
(int32_t)0x471cec00, (int32_t)0x6a6d9800, (int32_t)0xcf043a00, (int32_t)0x7641af00, (int32_t)0x8275a000, (int32_t)0x18f8b800,(int32_t)0x447acd00, (int32_t)0x6c242900, (int32_t)0xc945df00, (int32_t)0x73b5eb00, (int32_t)0x80f66e00, (int32_t)0x0fab2700,
(int32_t)0x41ce1e00, (int32_t)0x6dca0d00, (int32_t)0xc3a94500, (int32_t)0x70e2cb00, (int32_t)0x80277800, (int32_t)0x0647d900,(int32_t)0x3f174900, (int32_t)0x6f5f0200, (int32_t)0xbe31e100, (int32_t)0x6dca0d00, (int32_t)0x8009de00, (int32_t)0xfcdbd500,
(int32_t)0x3c56ba00, (int32_t)0x70e2cb00, (int32_t)0xb8e31300, (int32_t)0x6a6d9800, (int32_t)0x809dc900, (int32_t)0xf3742c00,(int32_t)0x398cdd00, (int32_t)0x72552c00, (int32_t)0xb3c02000, (int32_t)0x66cf8100, (int32_t)0x81e26c00, (int32_t)0xea1deb00,
(int32_t)0x36ba2000, (int32_t)0x73b5eb00, (int32_t)0xaecc3300, (int32_t)0x62f20100, (int32_t)0x83d60400, (int32_t)0xe0e60600,(int32_t)0x33def200, (int32_t)0x7504d300, (int32_t)0xaa0a5b00, (int32_t)0x5ed77c00, (int32_t)0x8675dc00, (int32_t)0xd7d94600,
(int32_t)0x30fbc500, (int32_t)0x7641af00, (int32_t)0xa57d8600, (int32_t)0x5a827900, (int32_t)0x89be5000, (int32_t)0xcf043a00,(int32_t)0x2e110a00, (int32_t)0x776c4e00, (int32_t)0xa1288300, (int32_t)0x55f5a400, (int32_t)0x8daad300, (int32_t)0xc6732200,
(int32_t)0x2b1f3400, (int32_t)0x78848400, (int32_t)0x9d0dfe00, (int32_t)0x5133cc00, (int32_t)0x9235f200, (int32_t)0xbe31e100,(int32_t)0x2826b900, (int32_t)0x798a2300, (int32_t)0x99307e00, (int32_t)0x4c3fdf00, (int32_t)0x97596100, (int32_t)0xb64bea00,
(int32_t)0x25280c00, (int32_t)0x7a7d0500, (int32_t)0x95926700, (int32_t)0x471cec00, (int32_t)0x9d0dfe00, (int32_t)0xaecc3300,(int32_t)0x2223a400, (int32_t)0x7b5d0300, (int32_t)0x9235f200, (int32_t)0x41ce1e00, (int32_t)0xa34bdf00, (int32_t)0xa7bd2200,
(int32_t)0x1f19f900, (int32_t)0x7c29fb00, (int32_t)0x8f1d3400, (int32_t)0x3c56ba00, (int32_t)0xaa0a5b00, (int32_t)0xa1288300,(int32_t)0x1c0b8200, (int32_t)0x7ce3ce00, (int32_t)0x8c4a1400, (int32_t)0x36ba2000, (int32_t)0xb1401700, (int32_t)0x9b177600,
(int32_t)0x18f8b800, (int32_t)0x7d8a5f00, (int32_t)0x89be5000, (int32_t)0x30fbc500, (int32_t)0xb8e31300, (int32_t)0x95926700,(int32_t)0x15e21400, (int32_t)0x7e1d9300, (int32_t)0x877b7b00, (int32_t)0x2b1f3400, (int32_t)0xc0e8b600, (int32_t)0x90a0fd00,
(int32_t)0x12c81000, (int32_t)0x7e9d5500, (int32_t)0x8582fa00, (int32_t)0x25280c00, (int32_t)0xc945df00, (int32_t)0x8c4a1400,(int32_t)0x0fab2700, (int32_t)0x7f099100, (int32_t)0x83d60400, (int32_t)0x1f19f900, (int32_t)0xd1eef500, (int32_t)0x8893b100,
(int32_t)0x0c8bd300, (int32_t)0x7f623600, (int32_t)0x8275a000, (int32_t)0x18f8b800, (int32_t)0xdad7f300, (int32_t)0x8582fa00,(int32_t)0x096a9000, (int32_t)0x7fa73600, (int32_t)0x8162aa00, (int32_t)0x12c81000, (int32_t)0xe3f47d00, (int32_t)0x831c3100,
(int32_t)0x0647d900, (int32_t)0x7fd88700, (int32_t)0x809dc900, (int32_t)0x0c8bd300, (int32_t)0xed37ef00, (int32_t)0x8162aa00,(int32_t)0x03242a00, (int32_t)0x7ff62100, (int32_t)0x80277800, (int32_t)0x0647d900, (int32_t)0xf6956f00, (int32_t)0x8058c900
};
static const tFftDescr descr={  256,fft_inc256,(cint32_ptr)ifft24x24_twd256};
const fft_handle_t cifft24_256=(const void*)&descr;
const fft_handle_t rifft24_512=(const void*)&descr;
