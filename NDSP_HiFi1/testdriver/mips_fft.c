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
    test module for testing MCPS performance (Complex FFT)
*/

#include "NatureDSP_Signal.h"
#include "mips.h"
#include "config.h"
#include "vecutils.h"
 
#include "utils.h"
#include "profiler.h"
 

#define PROFILE_INVERTED_FFT_cfft16x16(N,i) PROFILE_INVERTED_FFT_SC( fft_cplx16x16,N,i,( out0.i16, inp1.i16, cfft16x16_##N, i),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_cfft24x24(N,i) PROFILE_INVERTED_FFT_SC( fft_cplx24x24,N,i,( out0.i32, inp1.i32, cfft24_##N, i),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_cfft32x16(N) PROFILE_INVERTED_FFT( fft_cplx32x16,N,( out0.i32, inp1.i32, cfft16_##N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rfft16x16(N,i) PROFILE_INVERTED_FFT_SC( fft_real16x16,N,i,( out0.i16, inp1.i16, rfft16x16_##N, i),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rfft32x16(N) PROFILE_INVERTED_FFT( fft_real32x16,N,( out0.i32, inp1.i32, rfft16_##N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rfft24x24(N,i) PROFILE_INVERTED_FFT_SC( fft_real24x24,N,i,( out0.i32, inp1.i32, rfft24_##N, i),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_cfft32x32(N) PROFILE_INVERTED_FFT( fft_cplx32x32,N,( out0.i32, inp1.i32, cfft32_##N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rfft32x32(N) PROFILE_INVERTED_FFT( fft_real32x32,N,( out0.i32, inp1.i32, rfft32_##N, 3),fout,prf_ptscycle3,N );

#define PROFILE_INVERTED_FFT_cifft16x16(N,i) PROFILE_INVERTED_FFT_SC( ifft_cplx16x16,N,i,( out0.i16, inp1.i16, cifft16x16_##N, i),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_cifft24x24(N,i) PROFILE_INVERTED_FFT_SC( ifft_cplx24x24,N,i,( out0.i32, inp1.i32, cfft24_##N, i),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_cifft32x16(N) PROFILE_INVERTED_FFT( ifft_cplx32x16,N,( out0.i32, inp1.i32, cfft16_##N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rifft16x16(N,i) PROFILE_INVERTED_FFT_SC( ifft_real16x16,N,i,( out0.i16, inp1.i16, rifft16x16_##N, i),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rifft32x16(N) PROFILE_INVERTED_FFT( ifft_real32x16,N,( out0.i32, inp1.i32, rfft16_##N, 3),fout,prf_ptscycle3,N);
#define PROFILE_INVERTED_FFT_rifft24x24(N,i) PROFILE_INVERTED_FFT_SC( ifft_real24x24,N,i,( out0.i32, inp1.i32, rfft24_##N, i),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_cifft32x32(N) PROFILE_INVERTED_FFT( ifft_cplx32x32,N,( out0.i32, inp1.i32, cfft32_##N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rifft32x32(N) PROFILE_INVERTED_FFT( ifft_real32x32,N,( out0.i32, inp1.i32, rifft32_##N, 3),fout,prf_ptscycle3,N );


void mips_fft(int phaseNum, FILE* fout)
{
    perf_info(fout, "\nFixed point complex FFT:\n");
    if ( phaseNum == 0 || phaseNum == 1 )
    {
        PROFILE_INVERTED_FFT_cfft16x16(16,3);
        PROFILE_INVERTED_FFT_cfft16x16(32,3);
        PROFILE_INVERTED_FFT_cfft16x16(64,3);
        PROFILE_INVERTED_FFT_cfft16x16(128,3);
        PROFILE_INVERTED_FFT_cfft16x16(256,3);
        PROFILE_INVERTED_FFT_cfft16x16(512,3);
        PROFILE_INVERTED_FFT_cfft16x16(1024,3);
        PROFILE_INVERTED_FFT_cfft16x16(2048,3);
        PROFILE_INVERTED_FFT_cfft16x16(4096,3);


        PROFILE_INVERTED_FFT_cfft16x16(16,2);
        PROFILE_INVERTED_FFT_cfft16x16(32,2);
        PROFILE_INVERTED_FFT_cfft16x16(64,2);
        PROFILE_INVERTED_FFT_cfft16x16(128,2);
        PROFILE_INVERTED_FFT_cfft16x16(256,2);
        PROFILE_INVERTED_FFT_cfft16x16(512,2);
        PROFILE_INVERTED_FFT_cfft16x16(1024,2);
        PROFILE_INVERTED_FFT_cfft16x16(2048,2);
        PROFILE_INVERTED_FFT_cfft16x16(4096,2);

        PROFILE_INVERTED_FFT_cfft24x24(512, 0);
        PROFILE_INVERTED_FFT_cfft24x24(512, 1);
        PROFILE_INVERTED_FFT_cfft24x24(512, 2);
        PROFILE_INVERTED_FFT_cfft24x24(512, 3);
        PROFILE_INVERTED_FFT_cfft24x24(16,0);
        PROFILE_INVERTED_FFT_cfft24x24(32,0);
        PROFILE_INVERTED_FFT_cfft24x24(64,0);
        PROFILE_INVERTED_FFT_cfft24x24(128,0);
        PROFILE_INVERTED_FFT_cfft24x24(256,0);
        PROFILE_INVERTED_FFT_cfft24x24(16, 3);
        PROFILE_INVERTED_FFT_cfft24x24(32, 3);
        PROFILE_INVERTED_FFT_cfft24x24(64, 3);
        PROFILE_INVERTED_FFT_cfft24x24(128, 3);
        PROFILE_INVERTED_FFT_cfft24x24(256, 3);
        PROFILE_INVERTED_FFT_cfft24x24(1024,2);
        PROFILE_INVERTED_FFT_cfft24x24(2048,3);
        PROFILE_INVERTED_FFT_cfft24x24(4096,0);

        PROFILE_INVERTED_FFT_cfft32x16(16);
        PROFILE_INVERTED_FFT_cfft32x16(32);
        PROFILE_INVERTED_FFT_cfft32x16(64);
        PROFILE_INVERTED_FFT_cfft32x16(128);
        PROFILE_INVERTED_FFT_cfft32x16(256);
        PROFILE_INVERTED_FFT_cfft32x16(512);
        PROFILE_INVERTED_FFT_cfft32x16(1024);
        PROFILE_INVERTED_FFT_cfft32x16(2048);
        PROFILE_INVERTED_FFT_cfft32x16(4096);

        PROFILE_INVERTED_FFT_cfft32x32(16);
        PROFILE_INVERTED_FFT_cfft32x32(32);
        PROFILE_INVERTED_FFT_cfft32x32(64);
        PROFILE_INVERTED_FFT_cfft32x32(128);
        PROFILE_INVERTED_FFT_cfft32x32(256);
        PROFILE_INVERTED_FFT_cfft32x32(512);
        PROFILE_INVERTED_FFT_cfft32x32(1024);
        PROFILE_INVERTED_FFT_cfft32x32(2048);
        PROFILE_INVERTED_FFT_cfft32x32(4096);


        PROFILE_INVERTED_FFT_cifft16x16(16,3);
        PROFILE_INVERTED_FFT_cifft16x16(32,3);
        PROFILE_INVERTED_FFT_cifft16x16(64,3);
        PROFILE_INVERTED_FFT_cifft16x16(128,3);
        PROFILE_INVERTED_FFT_cifft16x16(256,3);
        PROFILE_INVERTED_FFT_cifft16x16(512,3);
        PROFILE_INVERTED_FFT_cifft16x16(1024,3);
        PROFILE_INVERTED_FFT_cifft16x16(2048,3);
        PROFILE_INVERTED_FFT_cifft16x16(4096,3);
        PROFILE_INVERTED_FFT_cifft16x16(16,2);
        PROFILE_INVERTED_FFT_cifft16x16(32,2);
        PROFILE_INVERTED_FFT_cifft16x16(64,2);
        PROFILE_INVERTED_FFT_cifft16x16(128,2);
        PROFILE_INVERTED_FFT_cifft16x16(256,2);
        PROFILE_INVERTED_FFT_cifft16x16(512,2);
        PROFILE_INVERTED_FFT_cifft16x16(1024,2);
        PROFILE_INVERTED_FFT_cifft16x16(2048,2);
        PROFILE_INVERTED_FFT_cifft16x16(4096,2);

        PROFILE_INVERTED_FFT_cifft24x24(512, 0);
        PROFILE_INVERTED_FFT_cifft24x24(512, 1);
        PROFILE_INVERTED_FFT_cifft24x24(512, 2);
        PROFILE_INVERTED_FFT_cifft24x24(512, 3);
        PROFILE_INVERTED_FFT_cifft24x24(16, 0);
        PROFILE_INVERTED_FFT_cifft24x24(32, 0);
        PROFILE_INVERTED_FFT_cifft24x24(64, 0);
        PROFILE_INVERTED_FFT_cifft24x24(128, 0);
        PROFILE_INVERTED_FFT_cifft24x24(256, 0);
        PROFILE_INVERTED_FFT_cifft24x24(16, 3);
        PROFILE_INVERTED_FFT_cifft24x24(32, 3);
        PROFILE_INVERTED_FFT_cifft24x24(64, 3);
        PROFILE_INVERTED_FFT_cifft24x24(128, 3);
        PROFILE_INVERTED_FFT_cifft24x24(256, 3);
        PROFILE_INVERTED_FFT_cifft24x24(1024, 2);
        PROFILE_INVERTED_FFT_cifft24x24(2048, 3);
        PROFILE_INVERTED_FFT_cifft24x24(4096, 0);

        PROFILE_INVERTED_FFT_cifft32x16(16);
        PROFILE_INVERTED_FFT_cifft32x16(32);
        PROFILE_INVERTED_FFT_cifft32x16(64);
        PROFILE_INVERTED_FFT_cifft32x16(128);
        PROFILE_INVERTED_FFT_cifft32x16(256);
        PROFILE_INVERTED_FFT_cifft32x16(512);
        PROFILE_INVERTED_FFT_cifft32x16(1024);
        PROFILE_INVERTED_FFT_cifft32x16(2048);
        PROFILE_INVERTED_FFT_cifft32x16(4096);

        PROFILE_INVERTED_FFT_cifft32x32(16);
        PROFILE_INVERTED_FFT_cifft32x32(32);
        PROFILE_INVERTED_FFT_cifft32x32(64);
        PROFILE_INVERTED_FFT_cifft32x32(128);
        PROFILE_INVERTED_FFT_cifft32x32(256);
        PROFILE_INVERTED_FFT_cifft32x32(512);
        PROFILE_INVERTED_FFT_cifft32x32(1024);
        PROFILE_INVERTED_FFT_cifft32x32(2048);
        PROFILE_INVERTED_FFT_cifft32x32(4096);

        perf_info(fout, "\nFixed point real FFT:\n");

        PROFILE_INVERTED_FFT_rfft16x16(32,3);
        PROFILE_INVERTED_FFT_rfft16x16(64,3);
        PROFILE_INVERTED_FFT_rfft16x16(128,3);
        PROFILE_INVERTED_FFT_rfft16x16(256,3);
        PROFILE_INVERTED_FFT_rfft16x16(512,3);
        PROFILE_INVERTED_FFT_rfft16x16(1024,3);
        PROFILE_INVERTED_FFT_rfft16x16(2048,3);
        PROFILE_INVERTED_FFT_rfft16x16(4096,3);
        PROFILE_INVERTED_FFT_rfft16x16(32,2);
        PROFILE_INVERTED_FFT_rfft16x16(64,2);
        PROFILE_INVERTED_FFT_rfft16x16(128,2);
        PROFILE_INVERTED_FFT_rfft16x16(256,2);
        PROFILE_INVERTED_FFT_rfft16x16(512,2);
        PROFILE_INVERTED_FFT_rfft16x16(1024,2);
        PROFILE_INVERTED_FFT_rfft16x16(2048,2);
        PROFILE_INVERTED_FFT_rfft16x16(4096,2);

        PROFILE_INVERTED_FFT_rfft24x24(512,0);
        PROFILE_INVERTED_FFT_rfft24x24(512, 1);
        PROFILE_INVERTED_FFT_rfft24x24(512, 2);
        PROFILE_INVERTED_FFT_rfft24x24(512, 3);
        PROFILE_INVERTED_FFT_rfft24x24(32, 0);
        PROFILE_INVERTED_FFT_rfft24x24(64, 0);
        PROFILE_INVERTED_FFT_rfft24x24(128, 0);
        PROFILE_INVERTED_FFT_rfft24x24(256, 0);
        PROFILE_INVERTED_FFT_rfft24x24(32, 3);
        PROFILE_INVERTED_FFT_rfft24x24(64, 3);
        PROFILE_INVERTED_FFT_rfft24x24(128, 3);
        PROFILE_INVERTED_FFT_rfft24x24(256, 3);
        PROFILE_INVERTED_FFT_rfft24x24(1024, 2);
        PROFILE_INVERTED_FFT_rfft24x24(2048,  3);
        PROFILE_INVERTED_FFT_rfft24x24(4096, 0);
        PROFILE_INVERTED_FFT_rfft24x24(1024, 3);

        PROFILE_INVERTED_FFT_rfft32x16(32);
        PROFILE_INVERTED_FFT_rfft32x16(64);
        PROFILE_INVERTED_FFT_rfft32x16(128);
        PROFILE_INVERTED_FFT_rfft32x16(256);
        PROFILE_INVERTED_FFT_rfft32x16(512);
        PROFILE_INVERTED_FFT_rfft32x16(1024);
        PROFILE_INVERTED_FFT_rfft32x16(2048);
        PROFILE_INVERTED_FFT_rfft32x16(4096);

        PROFILE_INVERTED_FFT_rfft32x32(32);
        PROFILE_INVERTED_FFT_rfft32x32(64);
        PROFILE_INVERTED_FFT_rfft32x32(128);
        PROFILE_INVERTED_FFT_rfft32x32(256);
        PROFILE_INVERTED_FFT_rfft32x32(512);
        PROFILE_INVERTED_FFT_rfft32x32(1024);
        PROFILE_INVERTED_FFT_rfft32x32(2048);
        PROFILE_INVERTED_FFT_rfft32x32(4096);

        PROFILE_INVERTED_FFT_rifft16x16(32,3);
        PROFILE_INVERTED_FFT_rifft16x16(64,3);
        PROFILE_INVERTED_FFT_rifft16x16(128,3);
        PROFILE_INVERTED_FFT_rifft16x16(256,3);
        PROFILE_INVERTED_FFT_rifft16x16(512,3);
        PROFILE_INVERTED_FFT_rifft16x16(1024,3);
        PROFILE_INVERTED_FFT_rifft16x16(2048,3);
        PROFILE_INVERTED_FFT_rifft16x16(4096,3);
        PROFILE_INVERTED_FFT_rifft16x16(32,2);
        PROFILE_INVERTED_FFT_rifft16x16(64,2);
        PROFILE_INVERTED_FFT_rifft16x16(128,2);
        PROFILE_INVERTED_FFT_rifft16x16(256,2);
        PROFILE_INVERTED_FFT_rifft16x16(512,2);
        PROFILE_INVERTED_FFT_rifft16x16(1024,2);
        PROFILE_INVERTED_FFT_rifft16x16(2048,2);
        PROFILE_INVERTED_FFT_rifft16x16(4096,2);

        PROFILE_INVERTED_FFT_rifft24x24(512,  0);
        PROFILE_INVERTED_FFT_rifft24x24(512,  1);
        PROFILE_INVERTED_FFT_rifft24x24(512,  2);
        PROFILE_INVERTED_FFT_rifft24x24(512,  3);
        PROFILE_INVERTED_FFT_rifft24x24(32,  0);
        PROFILE_INVERTED_FFT_rifft24x24(64,  0);
        PROFILE_INVERTED_FFT_rifft24x24(128,  0);
        PROFILE_INVERTED_FFT_rifft24x24(256,  0);
        PROFILE_INVERTED_FFT_rifft24x24(32,  3);
        PROFILE_INVERTED_FFT_rifft24x24(64,  3);
        PROFILE_INVERTED_FFT_rifft24x24(128,  3);
        PROFILE_INVERTED_FFT_rifft24x24(256,  3);
        PROFILE_INVERTED_FFT_rifft24x24(1024,  2);
        PROFILE_INVERTED_FFT_rifft24x24(2048,  3);
        PROFILE_INVERTED_FFT_rifft24x24(4096,  0);
        PROFILE_INVERTED_FFT_rifft24x24(1024, 3);

        PROFILE_INVERTED_FFT_rifft32x16(32);
        PROFILE_INVERTED_FFT_rifft32x16(64);
        PROFILE_INVERTED_FFT_rifft32x16(128);
        PROFILE_INVERTED_FFT_rifft32x16(256);
        PROFILE_INVERTED_FFT_rifft32x16(512);
        PROFILE_INVERTED_FFT_rifft32x16(1024);
        PROFILE_INVERTED_FFT_rifft32x16(2048);
        PROFILE_INVERTED_FFT_rifft32x16(4096);

        PROFILE_INVERTED_FFT_rifft32x32(32);
        PROFILE_INVERTED_FFT_rifft32x32(64);
        PROFILE_INVERTED_FFT_rifft32x32(128);
        PROFILE_INVERTED_FFT_rifft32x32(256);
        PROFILE_INVERTED_FFT_rifft32x32(512);
        PROFILE_INVERTED_FFT_rifft32x32(1024);
        PROFILE_INVERTED_FFT_rifft32x32(2048);
        PROFILE_INVERTED_FFT_rifft32x32(4096);
    }

}
