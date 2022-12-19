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
 * Test module for testing cycle performance (real FFT functions)
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Filters and transformations API. */
#include "NatureDSP_Signal.h"
/* Measurement utilities */
#include "mips.h"


#define PROFILE_INVERTED_FFT_rfft32x16_ie(N, twstep) PROFILE_INVERTED_FFT( fft_real32x16_ie,N,( out0.ci32, inp1.i32, inp0.ci16, twstep, N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rfft24x24_ie(N, twstep) PROFILE_INVERTED_FFT( fft_real24x24_ie,N,( out0.ci32, inp1.i32, inp0.ci32, twstep, N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rfft32x16_ie_24p(N, twstep) PROFILE_INVERTED_FFT( fft_real32x16_ie_24p,N,( out0.u8, inp1.u8, inp0.ci16, twstep, N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rfft24x24_ie_24p(N, twstep) PROFILE_INVERTED_FFT( fft_real24x24_ie_24p,N,( out0.u8, inp1.u8, inp0.ci32, twstep, N, 1),fout,prf_ptscycle3,N );

#define PROFILE_INVERTED_FFT_rifft32x16_ie(N, twstep) PROFILE_INVERTED_FFT( ifft_real32x16_ie,N,( out0.i32, inp1.ci32, inp0.ci16, twstep, N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rifft24x24_ie(N, twstep) PROFILE_INVERTED_FFT( ifft_real24x24_ie,N,( out0.i32, inp1.ci32, inp0.ci32, twstep, N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rifft32x16_ie_24p(N, twstep) PROFILE_INVERTED_FFT( ifft_real32x16_ie_24p,N,( out0.u8, inp1.u8, inp0.ci16, twstep, N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_rifft24x24_ie_24p(N, twstep) PROFILE_INVERTED_FFT( ifft_real24x24_ie_24p,N,( out0.u8, inp1.u8, inp0.ci32, twstep, N, 1),fout,prf_ptscycle3,N );

#define PROFILE_RFFT(fun,N,suffix1,suffix2,suffix3,...)                 PROFILE_INVERTED(fun,(out1.suffix2,inp0.suffix1,inp1.suffix3,1,N), \
                                                                 fout,"N=" #N,prf_ptscycle3,(N));
#define PROFILE_RFFT_INPL(fun,N,suffix1,suffix2,suffix3, ...)           PROFILE_INVERTED(fun,(out0.suffix2, out0.suffix1,inp1.suffix3,1,N), \
                                                                 fout,"N=" #N " [inplace]",prf_ptscycle3,(N));
#define PROFILE_RFFT_SCL(fun,N,suffix1,suffix2,suffix3,scl_mtd)       { int bexp; \
                                                PROFILE_INVERTED(fun,(out1.suffix2,inp0.suffix1,inp1.suffix3,1,N,&bexp,scl_mtd), \
                                                                 fout,"N=" #N " [scl=" #scl_mtd "]", prf_ptscycle3,(N)) };
#define PROFILE_RFFT_SCL_INPL(fun,N,suffix1,suffix2,suffix3,scl_mtd)  { int bexp; \
                                                PROFILE_INVERTED(fun,(out0.suffix2,out0.suffix1,inp1.suffix3,1,N,&bexp,scl_mtd), \
                                                                 fout,"N=" #N " [scl=" #scl_mtd " inplace]", prf_ptscycle3,(N)) };

#define PROFILE_BATCH( _PROFILER, fxn,suffix1,suffix2,suffix3, ... ) {   \
              _PROFILER( fxn,     8,suffix1,suffix2,suffix3, __VA_ARGS__ ); \
              _PROFILER( fxn,    16,suffix1,suffix2,suffix3, __VA_ARGS__ ); \
              _PROFILER( fxn,    32,suffix1,suffix2,suffix3, __VA_ARGS__ ); \
              _PROFILER( fxn,    64,suffix1,suffix2,suffix3, __VA_ARGS__ ); \
              _PROFILER( fxn,   128,suffix1,suffix2,suffix3, __VA_ARGS__ ); \
              _PROFILER( fxn,   256,suffix1,suffix2,suffix3, __VA_ARGS__ ); \
              _PROFILER( fxn,   512,suffix1,suffix2,suffix3, __VA_ARGS__ ); \
              _PROFILER( fxn,  1024,suffix1,suffix2,suffix3, __VA_ARGS__ ); \
              _PROFILER( fxn,  2048,suffix1,suffix2,suffix3, __VA_ARGS__ ); \
              _PROFILER( fxn,  4096,suffix1,suffix2,suffix3, __VA_ARGS__ ); \
}

void mips_rfft( int phaseNum, FILE * fout )
{
    perf_info( fout, "\nReal FFT with memory improved usage:\n" );
    /*  Stage 1*/
    if ( phaseNum == 0 || phaseNum == 1 )
    {
        PROFILE_INVERTED_FFT_rfft32x16_ie(256, 1);
        PROFILE_INVERTED_FFT_rfft32x16_ie(512, 1);
        PROFILE_INVERTED_FFT_rfft32x16_ie(1024,1);
        PROFILE_INVERTED_FFT_rfft32x16_ie_24p(256, 1);
        PROFILE_INVERTED_FFT_rfft32x16_ie_24p(512, 1);
        PROFILE_INVERTED_FFT_rfft32x16_ie_24p(1024,1);
        PROFILE_INVERTED_FFT_rfft24x24_ie(256, 1);
        PROFILE_INVERTED_FFT_rfft24x24_ie(512, 1);
        PROFILE_INVERTED_FFT_rfft24x24_ie(1024,1);
        PROFILE_INVERTED_FFT_rfft24x24_ie_24p(256, 1);
        PROFILE_INVERTED_FFT_rfft24x24_ie_24p(512, 1);
        PROFILE_INVERTED_FFT_rfft24x24_ie_24p(1024,1);

        PROFILE_INVERTED_FFT_rifft32x16_ie(256, 1);
        PROFILE_INVERTED_FFT_rifft32x16_ie(512, 1);
        PROFILE_INVERTED_FFT_rifft32x16_ie(1024,1);
        PROFILE_INVERTED_FFT_rifft32x16_ie_24p(256, 1);
        PROFILE_INVERTED_FFT_rifft32x16_ie_24p(512, 1);
        PROFILE_INVERTED_FFT_rifft32x16_ie_24p(1024,1);
        PROFILE_INVERTED_FFT_rifft24x24_ie(256, 1);
        PROFILE_INVERTED_FFT_rifft24x24_ie(512, 1);
        PROFILE_INVERTED_FFT_rifft24x24_ie(1024,1);
        PROFILE_INVERTED_FFT_rifft24x24_ie_24p(256, 1);
        PROFILE_INVERTED_FFT_rifft24x24_ie_24p(512, 1);
        PROFILE_INVERTED_FFT_rifft24x24_ie_24p(1024,1);
    }
    /* Stage 2 */
    if ( phaseNum == 0 || phaseNum == 2 )
    {
        PROFILE_BATCH( PROFILE_RFFT         , fft_realf_ie ,f32,cf32,cf32);
        PROFILE_BATCH( PROFILE_RFFT         ,ifft_realf_ie ,cf32,f32,cf32);
    }
} /* mips_rfft() */
