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

    twiddle factors for 32x16/16x16 real fwd/rev FFT transforms
*/

#include "NatureDSP_Signal.h"
#include "fft_cplx_twiddles.h"
#include "common.h"

/*
    Twiddle tables are received by sequential writing twiddle factors used in the original fft loops
    However, last radix2/radix4 stage is omitted because they are processed separately and combined 
    with bitreversing
    Higher halfword of each item is a cosine of a twiddle angle and lower halfword is a sine. All numbers are
    represented in Q15. Twiddle angle is always an integral number multiplied by 2*pi/N where N is FFT length. 
    For example, the sequence of twiddle angles is:
    16-pts transform 
    [0 0 0 1 2 3 2 4 6 3 6 -7]/16
    32-pts transform 
    [0 0 0 1 2 3 2 4 6 3 6 9 4 8 12 5 10 15 6 12 -14 7 14 -11 0 0 0 4 8 12 0 0 0 4 8 12 0 0 0 4 8 12 0 0 0 4 8 12 ]/32
    ....
*/
const int16_t MSC_ALIGNED ifft_twd1024[128*12] GCC_ALIGNED = {
(int16_t)0x7fff, (int16_t)0x0000, (int16_t)0x7fff, (int16_t)0x0000, (int16_t)0x7fff, (int16_t)0x0000, (int16_t)0x7fff, (int16_t)0x00c9, (int16_t)0x7ffe, (int16_t)0x0192, (int16_t)0x7ffa, (int16_t)0x025b,
(int16_t)0x7ffe, (int16_t)0x0192, (int16_t)0x7ff6, (int16_t)0x0324, (int16_t)0x7fea, (int16_t)0x04b6, (int16_t)0x7ffa, (int16_t)0x025b, (int16_t)0x7fea, (int16_t)0x04b6, (int16_t)0x7fce, (int16_t)0x0711,
(int16_t)0x7ff6, (int16_t)0x0324, (int16_t)0x7fd9, (int16_t)0x0648, (int16_t)0x7fa7, (int16_t)0x096b, (int16_t)0x7ff1, (int16_t)0x03ed, (int16_t)0x7fc2, (int16_t)0x07d9, (int16_t)0x7f75, (int16_t)0x0bc4,
(int16_t)0x7fea, (int16_t)0x04b6, (int16_t)0x7fa7, (int16_t)0x096b, (int16_t)0x7f38, (int16_t)0x0e1c, (int16_t)0x7fe2, (int16_t)0x057f, (int16_t)0x7f87, (int16_t)0x0afb, (int16_t)0x7ef0, (int16_t)0x1073,
(int16_t)0x7fd9, (int16_t)0x0648, (int16_t)0x7f62, (int16_t)0x0c8c, (int16_t)0x7e9d, (int16_t)0x12c8, (int16_t)0x7fce, (int16_t)0x0711, (int16_t)0x7f38, (int16_t)0x0e1c, (int16_t)0x7e3f, (int16_t)0x151c,
(int16_t)0x7fc2, (int16_t)0x07d9, (int16_t)0x7f0a, (int16_t)0x0fab, (int16_t)0x7dd6, (int16_t)0x176e, (int16_t)0x7fb5, (int16_t)0x08a2, (int16_t)0x7ed6, (int16_t)0x113a, (int16_t)0x7d63, (int16_t)0x19be,
(int16_t)0x7fa7, (int16_t)0x096b, (int16_t)0x7e9d, (int16_t)0x12c8, (int16_t)0x7ce4, (int16_t)0x1c0c, (int16_t)0x7f98, (int16_t)0x0a33, (int16_t)0x7e60, (int16_t)0x1455, (int16_t)0x7c5a, (int16_t)0x1e57,
(int16_t)0x7f87, (int16_t)0x0afb, (int16_t)0x7e1e, (int16_t)0x15e2, (int16_t)0x7bc6, (int16_t)0x209f, (int16_t)0x7f75, (int16_t)0x0bc4, (int16_t)0x7dd6, (int16_t)0x176e, (int16_t)0x7b27, (int16_t)0x22e5,
(int16_t)0x7f62, (int16_t)0x0c8c, (int16_t)0x7d8a, (int16_t)0x18f9, (int16_t)0x7a7d, (int16_t)0x2528, (int16_t)0x7f4e, (int16_t)0x0d54, (int16_t)0x7d3a, (int16_t)0x1a83, (int16_t)0x79c9, (int16_t)0x2768,
(int16_t)0x7f38, (int16_t)0x0e1c, (int16_t)0x7ce4, (int16_t)0x1c0c, (int16_t)0x790a, (int16_t)0x29a4, (int16_t)0x7f22, (int16_t)0x0ee4, (int16_t)0x7c89, (int16_t)0x1d93, (int16_t)0x7840, (int16_t)0x2bdc,
(int16_t)0x7f0a, (int16_t)0x0fab, (int16_t)0x7c2a, (int16_t)0x1f1a, (int16_t)0x776c, (int16_t)0x2e11, (int16_t)0x7ef0, (int16_t)0x1073, (int16_t)0x7bc6, (int16_t)0x209f, (int16_t)0x768e, (int16_t)0x3042,
(int16_t)0x7ed6, (int16_t)0x113a, (int16_t)0x7b5d, (int16_t)0x2224, (int16_t)0x75a6, (int16_t)0x326e, (int16_t)0x7eba, (int16_t)0x1201, (int16_t)0x7aef, (int16_t)0x23a7, (int16_t)0x74b3, (int16_t)0x3497,
(int16_t)0x7e9d, (int16_t)0x12c8, (int16_t)0x7a7d, (int16_t)0x2528, (int16_t)0x73b6, (int16_t)0x36ba, (int16_t)0x7e7f, (int16_t)0x138f, (int16_t)0x7a06, (int16_t)0x26a8, (int16_t)0x72af, (int16_t)0x38d9,
(int16_t)0x7e60, (int16_t)0x1455, (int16_t)0x798a, (int16_t)0x2827, (int16_t)0x719e, (int16_t)0x3af3, (int16_t)0x7e3f, (int16_t)0x151c, (int16_t)0x790a, (int16_t)0x29a4, (int16_t)0x7083, (int16_t)0x3d08,
(int16_t)0x7e1e, (int16_t)0x15e2, (int16_t)0x7885, (int16_t)0x2b1f, (int16_t)0x6f5f, (int16_t)0x3f17, (int16_t)0x7dfb, (int16_t)0x16a8, (int16_t)0x77fb, (int16_t)0x2c99, (int16_t)0x6e31, (int16_t)0x4121,
(int16_t)0x7dd6, (int16_t)0x176e, (int16_t)0x776c, (int16_t)0x2e11, (int16_t)0x6cf9, (int16_t)0x4326, (int16_t)0x7db1, (int16_t)0x1833, (int16_t)0x76d9, (int16_t)0x2f87, (int16_t)0x6bb8, (int16_t)0x4524,
(int16_t)0x7d8a, (int16_t)0x18f9, (int16_t)0x7642, (int16_t)0x30fc, (int16_t)0x6a6e, (int16_t)0x471d, (int16_t)0x7d63, (int16_t)0x19be, (int16_t)0x75a6, (int16_t)0x326e, (int16_t)0x691a, (int16_t)0x490f,
(int16_t)0x7d3a, (int16_t)0x1a83, (int16_t)0x7505, (int16_t)0x33df, (int16_t)0x67bd, (int16_t)0x4afb, (int16_t)0x7d0f, (int16_t)0x1b47, (int16_t)0x7460, (int16_t)0x354e, (int16_t)0x6657, (int16_t)0x4ce1,
(int16_t)0x7ce4, (int16_t)0x1c0c, (int16_t)0x73b6, (int16_t)0x36ba, (int16_t)0x64e9, (int16_t)0x4ec0, (int16_t)0x7cb7, (int16_t)0x1cd0, (int16_t)0x7308, (int16_t)0x3825, (int16_t)0x6371, (int16_t)0x5098,
(int16_t)0x7c89, (int16_t)0x1d93, (int16_t)0x7255, (int16_t)0x398d, (int16_t)0x61f1, (int16_t)0x5269, (int16_t)0x7c5a, (int16_t)0x1e57, (int16_t)0x719e, (int16_t)0x3af3, (int16_t)0x6068, (int16_t)0x5433,
(int16_t)0x7c2a, (int16_t)0x1f1a, (int16_t)0x70e3, (int16_t)0x3c57, (int16_t)0x5ed7, (int16_t)0x55f6, (int16_t)0x7bf9, (int16_t)0x1fdd, (int16_t)0x7023, (int16_t)0x3db8, (int16_t)0x5d3e, (int16_t)0x57b1,
(int16_t)0x7bc6, (int16_t)0x209f, (int16_t)0x6f5f, (int16_t)0x3f17, (int16_t)0x5b9d, (int16_t)0x5964, (int16_t)0x7b92, (int16_t)0x2162, (int16_t)0x6e97, (int16_t)0x4074, (int16_t)0x59f4, (int16_t)0x5b10,
(int16_t)0x7b5d, (int16_t)0x2224, (int16_t)0x6dca, (int16_t)0x41ce, (int16_t)0x5843, (int16_t)0x5cb4, (int16_t)0x7b27, (int16_t)0x22e5, (int16_t)0x6cf9, (int16_t)0x4326, (int16_t)0x568a, (int16_t)0x5e50,
(int16_t)0x7aef, (int16_t)0x23a7, (int16_t)0x6c24, (int16_t)0x447b, (int16_t)0x54ca, (int16_t)0x5fe4, (int16_t)0x7ab7, (int16_t)0x2467, (int16_t)0x6b4b, (int16_t)0x45cd, (int16_t)0x5303, (int16_t)0x616f,
(int16_t)0x7a7d, (int16_t)0x2528, (int16_t)0x6a6e, (int16_t)0x471d, (int16_t)0x5134, (int16_t)0x62f2, (int16_t)0x7a42, (int16_t)0x25e8, (int16_t)0x698c, (int16_t)0x486a, (int16_t)0x4f5e, (int16_t)0x646c,
(int16_t)0x7a06, (int16_t)0x26a8, (int16_t)0x68a7, (int16_t)0x49b4, (int16_t)0x4d81, (int16_t)0x65de, (int16_t)0x79c9, (int16_t)0x2768, (int16_t)0x67bd, (int16_t)0x4afb, (int16_t)0x4b9e, (int16_t)0x6747,
(int16_t)0x798a, (int16_t)0x2827, (int16_t)0x66d0, (int16_t)0x4c40, (int16_t)0x49b4, (int16_t)0x68a7, (int16_t)0x794a, (int16_t)0x28e5, (int16_t)0x65de, (int16_t)0x4d81, (int16_t)0x47c4, (int16_t)0x69fd,
(int16_t)0x790a, (int16_t)0x29a4, (int16_t)0x64e9, (int16_t)0x4ec0, (int16_t)0x45cd, (int16_t)0x6b4b, (int16_t)0x78c8, (int16_t)0x2a62, (int16_t)0x63ef, (int16_t)0x4ffb, (int16_t)0x43d1, (int16_t)0x6c8f,
(int16_t)0x7885, (int16_t)0x2b1f, (int16_t)0x62f2, (int16_t)0x5134, (int16_t)0x41ce, (int16_t)0x6dca, (int16_t)0x7840, (int16_t)0x2bdc, (int16_t)0x61f1, (int16_t)0x5269, (int16_t)0x3fc6, (int16_t)0x6efb,
(int16_t)0x77fb, (int16_t)0x2c99, (int16_t)0x60ec, (int16_t)0x539b, (int16_t)0x3db8, (int16_t)0x7023, (int16_t)0x77b4, (int16_t)0x2d55, (int16_t)0x5fe4, (int16_t)0x54ca, (int16_t)0x3ba5, (int16_t)0x7141,
(int16_t)0x776c, (int16_t)0x2e11, (int16_t)0x5ed7, (int16_t)0x55f6, (int16_t)0x398d, (int16_t)0x7255, (int16_t)0x7723, (int16_t)0x2ecc, (int16_t)0x5dc8, (int16_t)0x571e, (int16_t)0x3770, (int16_t)0x735f,
(int16_t)0x76d9, (int16_t)0x2f87, (int16_t)0x5cb4, (int16_t)0x5843, (int16_t)0x354e, (int16_t)0x7460, (int16_t)0x768e, (int16_t)0x3042, (int16_t)0x5b9d, (int16_t)0x5964, (int16_t)0x3327, (int16_t)0x7556,
(int16_t)0x7642, (int16_t)0x30fc, (int16_t)0x5a82, (int16_t)0x5a82, (int16_t)0x30fc, (int16_t)0x7642, (int16_t)0x75f4, (int16_t)0x31b5, (int16_t)0x5964, (int16_t)0x5b9d, (int16_t)0x2ecc, (int16_t)0x7723,
(int16_t)0x75a6, (int16_t)0x326e, (int16_t)0x5843, (int16_t)0x5cb4, (int16_t)0x2c99, (int16_t)0x77fb, (int16_t)0x7556, (int16_t)0x3327, (int16_t)0x571e, (int16_t)0x5dc8, (int16_t)0x2a62, (int16_t)0x78c8,
(int16_t)0x7505, (int16_t)0x33df, (int16_t)0x55f6, (int16_t)0x5ed7, (int16_t)0x2827, (int16_t)0x798a, (int16_t)0x74b3, (int16_t)0x3497, (int16_t)0x54ca, (int16_t)0x5fe4, (int16_t)0x25e8, (int16_t)0x7a42,
(int16_t)0x7460, (int16_t)0x354e, (int16_t)0x539b, (int16_t)0x60ec, (int16_t)0x23a7, (int16_t)0x7aef, (int16_t)0x740b, (int16_t)0x3604, (int16_t)0x5269, (int16_t)0x61f1, (int16_t)0x2162, (int16_t)0x7b92,
(int16_t)0x73b6, (int16_t)0x36ba, (int16_t)0x5134, (int16_t)0x62f2, (int16_t)0x1f1a, (int16_t)0x7c2a, (int16_t)0x735f, (int16_t)0x3770, (int16_t)0x4ffb, (int16_t)0x63ef, (int16_t)0x1cd0, (int16_t)0x7cb7,
(int16_t)0x7308, (int16_t)0x3825, (int16_t)0x4ec0, (int16_t)0x64e9, (int16_t)0x1a83, (int16_t)0x7d3a, (int16_t)0x72af, (int16_t)0x38d9, (int16_t)0x4d81, (int16_t)0x65de, (int16_t)0x1833, (int16_t)0x7db1,
(int16_t)0x7255, (int16_t)0x398d, (int16_t)0x4c40, (int16_t)0x66d0, (int16_t)0x15e2, (int16_t)0x7e1e, (int16_t)0x71fa, (int16_t)0x3a40, (int16_t)0x4afb, (int16_t)0x67bd, (int16_t)0x138f, (int16_t)0x7e7f,
(int16_t)0x719e, (int16_t)0x3af3, (int16_t)0x49b4, (int16_t)0x68a7, (int16_t)0x113a, (int16_t)0x7ed6, (int16_t)0x7141, (int16_t)0x3ba5, (int16_t)0x486a, (int16_t)0x698c, (int16_t)0x0ee4, (int16_t)0x7f22,
(int16_t)0x70e3, (int16_t)0x3c57, (int16_t)0x471d, (int16_t)0x6a6e, (int16_t)0x0c8c, (int16_t)0x7f62, (int16_t)0x7083, (int16_t)0x3d08, (int16_t)0x45cd, (int16_t)0x6b4b, (int16_t)0x0a33, (int16_t)0x7f98,
(int16_t)0x7023, (int16_t)0x3db8, (int16_t)0x447b, (int16_t)0x6c24, (int16_t)0x07d9, (int16_t)0x7fc2, (int16_t)0x6fc2, (int16_t)0x3e68, (int16_t)0x4326, (int16_t)0x6cf9, (int16_t)0x057f, (int16_t)0x7fe2,
(int16_t)0x6f5f, (int16_t)0x3f17, (int16_t)0x41ce, (int16_t)0x6dca, (int16_t)0x0324, (int16_t)0x7ff6, (int16_t)0x6efb, (int16_t)0x3fc6, (int16_t)0x4074, (int16_t)0x6e97, (int16_t)0x00c9, (int16_t)0x7fff,
(int16_t)0x6e97, (int16_t)0x4074, (int16_t)0x3f17, (int16_t)0x6f5f, (int16_t)0xfe6e, (int16_t)0x7ffe, (int16_t)0x6e31, (int16_t)0x4121, (int16_t)0x3db8, (int16_t)0x7023, (int16_t)0xfc13, (int16_t)0x7ff1,
(int16_t)0x6dca, (int16_t)0x41ce, (int16_t)0x3c57, (int16_t)0x70e3, (int16_t)0xf9b8, (int16_t)0x7fd9, (int16_t)0x6d62, (int16_t)0x427a, (int16_t)0x3af3, (int16_t)0x719e, (int16_t)0xf75e, (int16_t)0x7fb5,
(int16_t)0x6cf9, (int16_t)0x4326, (int16_t)0x398d, (int16_t)0x7255, (int16_t)0xf505, (int16_t)0x7f87, (int16_t)0x6c8f, (int16_t)0x43d1, (int16_t)0x3825, (int16_t)0x7308, (int16_t)0xf2ac, (int16_t)0x7f4e,
(int16_t)0x6c24, (int16_t)0x447b, (int16_t)0x36ba, (int16_t)0x73b6, (int16_t)0xf055, (int16_t)0x7f0a, (int16_t)0x6bb8, (int16_t)0x4524, (int16_t)0x354e, (int16_t)0x7460, (int16_t)0xedff, (int16_t)0x7eba,
(int16_t)0x6b4b, (int16_t)0x45cd, (int16_t)0x33df, (int16_t)0x7505, (int16_t)0xebab, (int16_t)0x7e60, (int16_t)0x6add, (int16_t)0x4675, (int16_t)0x326e, (int16_t)0x75a6, (int16_t)0xe958, (int16_t)0x7dfb,
(int16_t)0x6a6e, (int16_t)0x471d, (int16_t)0x30fc, (int16_t)0x7642, (int16_t)0xe707, (int16_t)0x7d8a, (int16_t)0x69fd, (int16_t)0x47c4, (int16_t)0x2f87, (int16_t)0x76d9, (int16_t)0xe4b9, (int16_t)0x7d0f,
(int16_t)0x698c, (int16_t)0x486a, (int16_t)0x2e11, (int16_t)0x776c, (int16_t)0xe26d, (int16_t)0x7c89, (int16_t)0x691a, (int16_t)0x490f, (int16_t)0x2c99, (int16_t)0x77fb, (int16_t)0xe023, (int16_t)0x7bf9,
(int16_t)0x68a7, (int16_t)0x49b4, (int16_t)0x2b1f, (int16_t)0x7885, (int16_t)0xdddc, (int16_t)0x7b5d, (int16_t)0x6832, (int16_t)0x4a58, (int16_t)0x29a4, (int16_t)0x790a, (int16_t)0xdb99, (int16_t)0x7ab7,
(int16_t)0x67bd, (int16_t)0x4afb, (int16_t)0x2827, (int16_t)0x798a, (int16_t)0xd958, (int16_t)0x7a06, (int16_t)0x6747, (int16_t)0x4b9e, (int16_t)0x26a8, (int16_t)0x7a06, (int16_t)0xd71b, (int16_t)0x794a,
(int16_t)0x66d0, (int16_t)0x4c40, (int16_t)0x2528, (int16_t)0x7a7d, (int16_t)0xd4e1, (int16_t)0x7885, (int16_t)0x6657, (int16_t)0x4ce1, (int16_t)0x23a7, (int16_t)0x7aef, (int16_t)0xd2ab, (int16_t)0x77b4,
(int16_t)0x65de, (int16_t)0x4d81, (int16_t)0x2224, (int16_t)0x7b5d, (int16_t)0xd079, (int16_t)0x76d9, (int16_t)0x6564, (int16_t)0x4e21, (int16_t)0x209f, (int16_t)0x7bc6, (int16_t)0xce4b, (int16_t)0x75f4,
(int16_t)0x64e9, (int16_t)0x4ec0, (int16_t)0x1f1a, (int16_t)0x7c2a, (int16_t)0xcc21, (int16_t)0x7505, (int16_t)0x646c, (int16_t)0x4f5e, (int16_t)0x1d93, (int16_t)0x7c89, (int16_t)0xc9fc, (int16_t)0x740b,
(int16_t)0x63ef, (int16_t)0x4ffb, (int16_t)0x1c0c, (int16_t)0x7ce4, (int16_t)0xc7db, (int16_t)0x7308, (int16_t)0x6371, (int16_t)0x5098, (int16_t)0x1a83, (int16_t)0x7d3a, (int16_t)0xc5c0, (int16_t)0x71fa,
(int16_t)0x62f2, (int16_t)0x5134, (int16_t)0x18f9, (int16_t)0x7d8a, (int16_t)0xc3a9, (int16_t)0x70e3, (int16_t)0x6272, (int16_t)0x51cf, (int16_t)0x176e, (int16_t)0x7dd6, (int16_t)0xc198, (int16_t)0x6fc2,
(int16_t)0x61f1, (int16_t)0x5269, (int16_t)0x15e2, (int16_t)0x7e1e, (int16_t)0xbf8c, (int16_t)0x6e97, (int16_t)0x616f, (int16_t)0x5303, (int16_t)0x1455, (int16_t)0x7e60, (int16_t)0xbd86, (int16_t)0x6d62,
(int16_t)0x60ec, (int16_t)0x539b, (int16_t)0x12c8, (int16_t)0x7e9d, (int16_t)0xbb85, (int16_t)0x6c24, (int16_t)0x6068, (int16_t)0x5433, (int16_t)0x113a, (int16_t)0x7ed6, (int16_t)0xb98b, (int16_t)0x6add,
(int16_t)0x5fe4, (int16_t)0x54ca, (int16_t)0x0fab, (int16_t)0x7f0a, (int16_t)0xb796, (int16_t)0x698c, (int16_t)0x5f5e, (int16_t)0x5560, (int16_t)0x0e1c, (int16_t)0x7f38, (int16_t)0xb5a8, (int16_t)0x6832,
(int16_t)0x5ed7, (int16_t)0x55f6, (int16_t)0x0c8c, (int16_t)0x7f62, (int16_t)0xb3c0, (int16_t)0x66d0, (int16_t)0x5e50, (int16_t)0x568a, (int16_t)0x0afb, (int16_t)0x7f87, (int16_t)0xb1df, (int16_t)0x6564,
(int16_t)0x5dc8, (int16_t)0x571e, (int16_t)0x096b, (int16_t)0x7fa7, (int16_t)0xb005, (int16_t)0x63ef, (int16_t)0x5d3e, (int16_t)0x57b1, (int16_t)0x07d9, (int16_t)0x7fc2, (int16_t)0xae31, (int16_t)0x6272,
(int16_t)0x5cb4, (int16_t)0x5843, (int16_t)0x0648, (int16_t)0x7fd9, (int16_t)0xac65, (int16_t)0x60ec, (int16_t)0x5c29, (int16_t)0x58d4, (int16_t)0x04b6, (int16_t)0x7fea, (int16_t)0xaaa0, (int16_t)0x5f5e,
(int16_t)0x5b9d, (int16_t)0x5964, (int16_t)0x0324, (int16_t)0x7ff6, (int16_t)0xa8e2, (int16_t)0x5dc8, (int16_t)0x5b10, (int16_t)0x59f4, (int16_t)0x0192, (int16_t)0x7ffe, (int16_t)0xa72c, (int16_t)0x5c29,
(int16_t)0x5a82, (int16_t)0x5a82, (int16_t)0x0000, (int16_t)0x7fff, (int16_t)0xa57e, (int16_t)0x5a82, (int16_t)0x59f4, (int16_t)0x5b10, (int16_t)0xfe6e, (int16_t)0x7ffe, (int16_t)0xa3d7, (int16_t)0x58d4,
(int16_t)0x5964, (int16_t)0x5b9d, (int16_t)0xfcdc, (int16_t)0x7ff6, (int16_t)0xa238, (int16_t)0x571e, (int16_t)0x58d4, (int16_t)0x5c29, (int16_t)0xfb4a, (int16_t)0x7fea, (int16_t)0xa0a2, (int16_t)0x5560,
(int16_t)0x5843, (int16_t)0x5cb4, (int16_t)0xf9b8, (int16_t)0x7fd9, (int16_t)0x9f14, (int16_t)0x539b, (int16_t)0x57b1, (int16_t)0x5d3e, (int16_t)0xf827, (int16_t)0x7fc2, (int16_t)0x9d8e, (int16_t)0x51cf,
(int16_t)0x571e, (int16_t)0x5dc8, (int16_t)0xf695, (int16_t)0x7fa7, (int16_t)0x9c11, (int16_t)0x4ffb, (int16_t)0x568a, (int16_t)0x5e50, (int16_t)0xf505, (int16_t)0x7f87, (int16_t)0x9a9c, (int16_t)0x4e21,
(int16_t)0x55f6, (int16_t)0x5ed7, (int16_t)0xf374, (int16_t)0x7f62, (int16_t)0x9930, (int16_t)0x4c40, (int16_t)0x5560, (int16_t)0x5f5e, (int16_t)0xf1e4, (int16_t)0x7f38, (int16_t)0x97ce, (int16_t)0x4a58,
(int16_t)0x54ca, (int16_t)0x5fe4, (int16_t)0xf055, (int16_t)0x7f0a, (int16_t)0x9674, (int16_t)0x486a, (int16_t)0x5433, (int16_t)0x6068, (int16_t)0xeec6, (int16_t)0x7ed6, (int16_t)0x9523, (int16_t)0x4675,
(int16_t)0x539b, (int16_t)0x60ec, (int16_t)0xed38, (int16_t)0x7e9d, (int16_t)0x93dc, (int16_t)0x447b, (int16_t)0x5303, (int16_t)0x616f, (int16_t)0xebab, (int16_t)0x7e60, (int16_t)0x929e, (int16_t)0x427a,
(int16_t)0x5269, (int16_t)0x61f1, (int16_t)0xea1e, (int16_t)0x7e1e, (int16_t)0x9169, (int16_t)0x4074, (int16_t)0x51cf, (int16_t)0x6272, (int16_t)0xe892, (int16_t)0x7dd6, (int16_t)0x903e, (int16_t)0x3e68,
(int16_t)0x5134, (int16_t)0x62f2, (int16_t)0xe707, (int16_t)0x7d8a, (int16_t)0x8f1d, (int16_t)0x3c57, (int16_t)0x5098, (int16_t)0x6371, (int16_t)0xe57d, (int16_t)0x7d3a, (int16_t)0x8e06, (int16_t)0x3a40,
(int16_t)0x4ffb, (int16_t)0x63ef, (int16_t)0xe3f4, (int16_t)0x7ce4, (int16_t)0x8cf8, (int16_t)0x3825, (int16_t)0x4f5e, (int16_t)0x646c, (int16_t)0xe26d, (int16_t)0x7c89, (int16_t)0x8bf5, (int16_t)0x3604,
(int16_t)0x4ec0, (int16_t)0x64e9, (int16_t)0xe0e6, (int16_t)0x7c2a, (int16_t)0x8afb, (int16_t)0x33df, (int16_t)0x4e21, (int16_t)0x6564, (int16_t)0xdf61, (int16_t)0x7bc6, (int16_t)0x8a0c, (int16_t)0x31b5,
(int16_t)0x4d81, (int16_t)0x65de, (int16_t)0xdddc, (int16_t)0x7b5d, (int16_t)0x8927, (int16_t)0x2f87, (int16_t)0x4ce1, (int16_t)0x6657, (int16_t)0xdc59, (int16_t)0x7aef, (int16_t)0x884c, (int16_t)0x2d55,
(int16_t)0x4c40, (int16_t)0x66d0, (int16_t)0xdad8, (int16_t)0x7a7d, (int16_t)0x877b, (int16_t)0x2b1f, (int16_t)0x4b9e, (int16_t)0x6747, (int16_t)0xd958, (int16_t)0x7a06, (int16_t)0x86b6, (int16_t)0x28e5,
(int16_t)0x4afb, (int16_t)0x67bd, (int16_t)0xd7d9, (int16_t)0x798a, (int16_t)0x85fa, (int16_t)0x26a8, (int16_t)0x4a58, (int16_t)0x6832, (int16_t)0xd65c, (int16_t)0x790a, (int16_t)0x8549, (int16_t)0x2467,
(int16_t)0x49b4, (int16_t)0x68a7, (int16_t)0xd4e1, (int16_t)0x7885, (int16_t)0x84a3, (int16_t)0x2224, (int16_t)0x490f, (int16_t)0x691a, (int16_t)0xd367, (int16_t)0x77fb, (int16_t)0x8407, (int16_t)0x1fdd,
(int16_t)0x486a, (int16_t)0x698c, (int16_t)0xd1ef, (int16_t)0x776c, (int16_t)0x8377, (int16_t)0x1d93, (int16_t)0x47c4, (int16_t)0x69fd, (int16_t)0xd079, (int16_t)0x76d9, (int16_t)0x82f1, (int16_t)0x1b47,
(int16_t)0x471d, (int16_t)0x6a6e, (int16_t)0xcf04, (int16_t)0x7642, (int16_t)0x8276, (int16_t)0x18f9, (int16_t)0x4675, (int16_t)0x6add, (int16_t)0xcd92, (int16_t)0x75a6, (int16_t)0x8205, (int16_t)0x16a8,
(int16_t)0x45cd, (int16_t)0x6b4b, (int16_t)0xcc21, (int16_t)0x7505, (int16_t)0x81a0, (int16_t)0x1455, (int16_t)0x4524, (int16_t)0x6bb8, (int16_t)0xcab2, (int16_t)0x7460, (int16_t)0x8146, (int16_t)0x1201,
(int16_t)0x447b, (int16_t)0x6c24, (int16_t)0xc946, (int16_t)0x73b6, (int16_t)0x80f6, (int16_t)0x0fab, (int16_t)0x43d1, (int16_t)0x6c8f, (int16_t)0xc7db, (int16_t)0x7308, (int16_t)0x80b2, (int16_t)0x0d54,
(int16_t)0x4326, (int16_t)0x6cf9, (int16_t)0xc673, (int16_t)0x7255, (int16_t)0x8079, (int16_t)0x0afb, (int16_t)0x427a, (int16_t)0x6d62, (int16_t)0xc50d, (int16_t)0x719e, (int16_t)0x804b, (int16_t)0x08a2,
(int16_t)0x41ce, (int16_t)0x6dca, (int16_t)0xc3a9, (int16_t)0x70e3, (int16_t)0x8027, (int16_t)0x0648, (int16_t)0x4121, (int16_t)0x6e31, (int16_t)0xc248, (int16_t)0x7023, (int16_t)0x800f, (int16_t)0x03ed,
(int16_t)0x4074, (int16_t)0x6e97, (int16_t)0xc0e9, (int16_t)0x6f5f, (int16_t)0x8002, (int16_t)0x0192, (int16_t)0x3fc6, (int16_t)0x6efb, (int16_t)0xbf8c, (int16_t)0x6e97, (int16_t)0x8001, (int16_t)0xff37,
(int16_t)0x3f17, (int16_t)0x6f5f, (int16_t)0xbe32, (int16_t)0x6dca, (int16_t)0x800a, (int16_t)0xfcdc, (int16_t)0x3e68, (int16_t)0x6fc2, (int16_t)0xbcda, (int16_t)0x6cf9, (int16_t)0x801e, (int16_t)0xfa81,
(int16_t)0x3db8, (int16_t)0x7023, (int16_t)0xbb85, (int16_t)0x6c24, (int16_t)0x803e, (int16_t)0xf827, (int16_t)0x3d08, (int16_t)0x7083, (int16_t)0xba33, (int16_t)0x6b4b, (int16_t)0x8068, (int16_t)0xf5cd,
(int16_t)0x3c57, (int16_t)0x70e3, (int16_t)0xb8e3, (int16_t)0x6a6e, (int16_t)0x809e, (int16_t)0xf374, (int16_t)0x3ba5, (int16_t)0x7141, (int16_t)0xb796, (int16_t)0x698c, (int16_t)0x80de, (int16_t)0xf11c,
(int16_t)0x3af3, (int16_t)0x719e, (int16_t)0xb64c, (int16_t)0x68a7, (int16_t)0x812a, (int16_t)0xeec6, (int16_t)0x3a40, (int16_t)0x71fa, (int16_t)0xb505, (int16_t)0x67bd, (int16_t)0x8181, (int16_t)0xec71,
(int16_t)0x398d, (int16_t)0x7255, (int16_t)0xb3c0, (int16_t)0x66d0, (int16_t)0x81e2, (int16_t)0xea1e, (int16_t)0x38d9, (int16_t)0x72af, (int16_t)0xb27f, (int16_t)0x65de, (int16_t)0x824f, (int16_t)0xe7cd,
(int16_t)0x3825, (int16_t)0x7308, (int16_t)0xb140, (int16_t)0x64e9, (int16_t)0x82c6, (int16_t)0xe57d, (int16_t)0x3770, (int16_t)0x735f, (int16_t)0xb005, (int16_t)0x63ef, (int16_t)0x8349, (int16_t)0xe330,
(int16_t)0x36ba, (int16_t)0x73b6, (int16_t)0xaecc, (int16_t)0x62f2, (int16_t)0x83d6, (int16_t)0xe0e6, (int16_t)0x3604, (int16_t)0x740b, (int16_t)0xad97, (int16_t)0x61f1, (int16_t)0x846e, (int16_t)0xde9e,
(int16_t)0x354e, (int16_t)0x7460, (int16_t)0xac65, (int16_t)0x60ec, (int16_t)0x8511, (int16_t)0xdc59, (int16_t)0x3497, (int16_t)0x74b3, (int16_t)0xab36, (int16_t)0x5fe4, (int16_t)0x85be, (int16_t)0xda18,
(int16_t)0x33df, (int16_t)0x7505, (int16_t)0xaa0a, (int16_t)0x5ed7, (int16_t)0x8676, (int16_t)0xd7d9, (int16_t)0x3327, (int16_t)0x7556, (int16_t)0xa8e2, (int16_t)0x5dc8, (int16_t)0x8738, (int16_t)0xd59e,
(int16_t)0x326e, (int16_t)0x75a6, (int16_t)0xa7bd, (int16_t)0x5cb4, (int16_t)0x8805, (int16_t)0xd367, (int16_t)0x31b5, (int16_t)0x75f4, (int16_t)0xa69c, (int16_t)0x5b9d, (int16_t)0x88dd, (int16_t)0xd134,
(int16_t)0x30fc, (int16_t)0x7642, (int16_t)0xa57e, (int16_t)0x5a82, (int16_t)0x89be, (int16_t)0xcf04, (int16_t)0x3042, (int16_t)0x768e, (int16_t)0xa463, (int16_t)0x5964, (int16_t)0x8aaa, (int16_t)0xccd9,
(int16_t)0x2f87, (int16_t)0x76d9, (int16_t)0xa34c, (int16_t)0x5843, (int16_t)0x8ba0, (int16_t)0xcab2, (int16_t)0x2ecc, (int16_t)0x7723, (int16_t)0xa238, (int16_t)0x571e, (int16_t)0x8ca1, (int16_t)0xc890,
(int16_t)0x2e11, (int16_t)0x776c, (int16_t)0xa129, (int16_t)0x55f6, (int16_t)0x8dab, (int16_t)0xc673, (int16_t)0x2d55, (int16_t)0x77b4, (int16_t)0xa01c, (int16_t)0x54ca, (int16_t)0x8ebf, (int16_t)0xc45b,
(int16_t)0x2c99, (int16_t)0x77fb, (int16_t)0x9f14, (int16_t)0x539b, (int16_t)0x8fdd, (int16_t)0xc248, (int16_t)0x2bdc, (int16_t)0x7840, (int16_t)0x9e0f, (int16_t)0x5269, (int16_t)0x9105, (int16_t)0xc03a,
(int16_t)0x2b1f, (int16_t)0x7885, (int16_t)0x9d0e, (int16_t)0x5134, (int16_t)0x9236, (int16_t)0xbe32, (int16_t)0x2a62, (int16_t)0x78c8, (int16_t)0x9c11, (int16_t)0x4ffb, (int16_t)0x9371, (int16_t)0xbc2f,
(int16_t)0x29a4, (int16_t)0x790a, (int16_t)0x9b17, (int16_t)0x4ec0, (int16_t)0x94b5, (int16_t)0xba33, (int16_t)0x28e5, (int16_t)0x794a, (int16_t)0x9a22, (int16_t)0x4d81, (int16_t)0x9603, (int16_t)0xb83c,
(int16_t)0x2827, (int16_t)0x798a, (int16_t)0x9930, (int16_t)0x4c40, (int16_t)0x9759, (int16_t)0xb64c, (int16_t)0x2768, (int16_t)0x79c9, (int16_t)0x9843, (int16_t)0x4afb, (int16_t)0x98b9, (int16_t)0xb462,
(int16_t)0x26a8, (int16_t)0x7a06, (int16_t)0x9759, (int16_t)0x49b4, (int16_t)0x9a22, (int16_t)0xb27f, (int16_t)0x25e8, (int16_t)0x7a42, (int16_t)0x9674, (int16_t)0x486a, (int16_t)0x9b94, (int16_t)0xb0a2,
(int16_t)0x2528, (int16_t)0x7a7d, (int16_t)0x9592, (int16_t)0x471d, (int16_t)0x9d0e, (int16_t)0xaecc, (int16_t)0x2467, (int16_t)0x7ab7, (int16_t)0x94b5, (int16_t)0x45cd, (int16_t)0x9e91, (int16_t)0xacfd,
(int16_t)0x23a7, (int16_t)0x7aef, (int16_t)0x93dc, (int16_t)0x447b, (int16_t)0xa01c, (int16_t)0xab36, (int16_t)0x22e5, (int16_t)0x7b27, (int16_t)0x9307, (int16_t)0x4326, (int16_t)0xa1b0, (int16_t)0xa976,
(int16_t)0x2224, (int16_t)0x7b5d, (int16_t)0x9236, (int16_t)0x41ce, (int16_t)0xa34c, (int16_t)0xa7bd, (int16_t)0x2162, (int16_t)0x7b92, (int16_t)0x9169, (int16_t)0x4074, (int16_t)0xa4f0, (int16_t)0xa60c,
(int16_t)0x209f, (int16_t)0x7bc6, (int16_t)0x90a1, (int16_t)0x3f17, (int16_t)0xa69c, (int16_t)0xa463, (int16_t)0x1fdd, (int16_t)0x7bf9, (int16_t)0x8fdd, (int16_t)0x3db8, (int16_t)0xa84f, (int16_t)0xa2c2,
(int16_t)0x1f1a, (int16_t)0x7c2a, (int16_t)0x8f1d, (int16_t)0x3c57, (int16_t)0xaa0a, (int16_t)0xa129, (int16_t)0x1e57, (int16_t)0x7c5a, (int16_t)0x8e62, (int16_t)0x3af3, (int16_t)0xabcd, (int16_t)0x9f98,
(int16_t)0x1d93, (int16_t)0x7c89, (int16_t)0x8dab, (int16_t)0x398d, (int16_t)0xad97, (int16_t)0x9e0f, (int16_t)0x1cd0, (int16_t)0x7cb7, (int16_t)0x8cf8, (int16_t)0x3825, (int16_t)0xaf68, (int16_t)0x9c8f,
(int16_t)0x1c0c, (int16_t)0x7ce4, (int16_t)0x8c4a, (int16_t)0x36ba, (int16_t)0xb140, (int16_t)0x9b17, (int16_t)0x1b47, (int16_t)0x7d0f, (int16_t)0x8ba0, (int16_t)0x354e, (int16_t)0xb31f, (int16_t)0x99a9,
(int16_t)0x1a83, (int16_t)0x7d3a, (int16_t)0x8afb, (int16_t)0x33df, (int16_t)0xb505, (int16_t)0x9843, (int16_t)0x19be, (int16_t)0x7d63, (int16_t)0x8a5a, (int16_t)0x326e, (int16_t)0xb6f1, (int16_t)0x96e6,
(int16_t)0x18f9, (int16_t)0x7d8a, (int16_t)0x89be, (int16_t)0x30fc, (int16_t)0xb8e3, (int16_t)0x9592, (int16_t)0x1833, (int16_t)0x7db1, (int16_t)0x8927, (int16_t)0x2f87, (int16_t)0xbadc, (int16_t)0x9448,
(int16_t)0x176e, (int16_t)0x7dd6, (int16_t)0x8894, (int16_t)0x2e11, (int16_t)0xbcda, (int16_t)0x9307, (int16_t)0x16a8, (int16_t)0x7dfb, (int16_t)0x8805, (int16_t)0x2c99, (int16_t)0xbedf, (int16_t)0x91cf,
(int16_t)0x15e2, (int16_t)0x7e1e, (int16_t)0x877b, (int16_t)0x2b1f, (int16_t)0xc0e9, (int16_t)0x90a1, (int16_t)0x151c, (int16_t)0x7e3f, (int16_t)0x86f6, (int16_t)0x29a4, (int16_t)0xc2f8, (int16_t)0x8f7d,
(int16_t)0x1455, (int16_t)0x7e60, (int16_t)0x8676, (int16_t)0x2827, (int16_t)0xc50d, (int16_t)0x8e62, (int16_t)0x138f, (int16_t)0x7e7f, (int16_t)0x85fa, (int16_t)0x26a8, (int16_t)0xc727, (int16_t)0x8d51,
(int16_t)0x12c8, (int16_t)0x7e9d, (int16_t)0x8583, (int16_t)0x2528, (int16_t)0xc946, (int16_t)0x8c4a, (int16_t)0x1201, (int16_t)0x7eba, (int16_t)0x8511, (int16_t)0x23a7, (int16_t)0xcb69, (int16_t)0x8b4d,
(int16_t)0x113a, (int16_t)0x7ed6, (int16_t)0x84a3, (int16_t)0x2224, (int16_t)0xcd92, (int16_t)0x8a5a, (int16_t)0x1073, (int16_t)0x7ef0, (int16_t)0x843a, (int16_t)0x209f, (int16_t)0xcfbe, (int16_t)0x8972,
(int16_t)0x0fab, (int16_t)0x7f0a, (int16_t)0x83d6, (int16_t)0x1f1a, (int16_t)0xd1ef, (int16_t)0x8894, (int16_t)0x0ee4, (int16_t)0x7f22, (int16_t)0x8377, (int16_t)0x1d93, (int16_t)0xd424, (int16_t)0x87c0,
(int16_t)0x0e1c, (int16_t)0x7f38, (int16_t)0x831c, (int16_t)0x1c0c, (int16_t)0xd65c, (int16_t)0x86f6, (int16_t)0x0d54, (int16_t)0x7f4e, (int16_t)0x82c6, (int16_t)0x1a83, (int16_t)0xd898, (int16_t)0x8637,
(int16_t)0x0c8c, (int16_t)0x7f62, (int16_t)0x8276, (int16_t)0x18f9, (int16_t)0xdad8, (int16_t)0x8583, (int16_t)0x0bc4, (int16_t)0x7f75, (int16_t)0x822a, (int16_t)0x176e, (int16_t)0xdd1b, (int16_t)0x84d9,
(int16_t)0x0afb, (int16_t)0x7f87, (int16_t)0x81e2, (int16_t)0x15e2, (int16_t)0xdf61, (int16_t)0x843a, (int16_t)0x0a33, (int16_t)0x7f98, (int16_t)0x81a0, (int16_t)0x1455, (int16_t)0xe1a9, (int16_t)0x83a6,
(int16_t)0x096b, (int16_t)0x7fa7, (int16_t)0x8163, (int16_t)0x12c8, (int16_t)0xe3f4, (int16_t)0x831c, (int16_t)0x08a2, (int16_t)0x7fb5, (int16_t)0x812a, (int16_t)0x113a, (int16_t)0xe642, (int16_t)0x829d,
(int16_t)0x07d9, (int16_t)0x7fc2, (int16_t)0x80f6, (int16_t)0x0fab, (int16_t)0xe892, (int16_t)0x822a, (int16_t)0x0711, (int16_t)0x7fce, (int16_t)0x80c8, (int16_t)0x0e1c, (int16_t)0xeae4, (int16_t)0x81c1,
(int16_t)0x0648, (int16_t)0x7fd9, (int16_t)0x809e, (int16_t)0x0c8c, (int16_t)0xed38, (int16_t)0x8163, (int16_t)0x057f, (int16_t)0x7fe2, (int16_t)0x8079, (int16_t)0x0afb, (int16_t)0xef8d, (int16_t)0x8110,
(int16_t)0x04b6, (int16_t)0x7fea, (int16_t)0x8059, (int16_t)0x096b, (int16_t)0xf1e4, (int16_t)0x80c8, (int16_t)0x03ed, (int16_t)0x7ff1, (int16_t)0x803e, (int16_t)0x07d9, (int16_t)0xf43c, (int16_t)0x808b,
(int16_t)0x0324, (int16_t)0x7ff6, (int16_t)0x8027, (int16_t)0x0648, (int16_t)0xf695, (int16_t)0x8059, (int16_t)0x025b, (int16_t)0x7ffa, (int16_t)0x8016, (int16_t)0x04b6, (int16_t)0xf8ef, (int16_t)0x8032,
(int16_t)0x0192, (int16_t)0x7ffe, (int16_t)0x800a, (int16_t)0x0324, (int16_t)0xfb4a, (int16_t)0x8016, (int16_t)0x00c9, (int16_t)0x7fff, (int16_t)0x8002, (int16_t)0x0192, (int16_t)0xfda5, (int16_t)0x8006
};
static const cint32_ptr seqifft_twd1024[]={(cint32_ptr)ifft_twd1024,(cint32_ptr)ifft_twd256,(cint32_ptr)ifft_twd64,(cint32_ptr)ifft_twd16};
static const tFftDescr descr={  1024,fft_inc1024,(cint32_ptr)seqifft_twd1024};
const fft_handle_t cifft16_1024=(const void*)&descr;
const fft_handle_t rifft16_2048=(const void*)&descr;

