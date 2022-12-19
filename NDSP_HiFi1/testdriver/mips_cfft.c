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
 * Test module for testing cycle performance (complex FFT functions)
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Filters and transformations API. */
#include "NatureDSP_Signal.h"
/* Measurement utilities */
#include "mips.h"

#define PROFILE_CFFT(fun,N,suffix,...)                 PROFILE_INVERTED(fun,(out1.suffix,inp0.suffix,inp1.suffix,1,N), \
                                                                 fout,"N=" #N,prf_ptscycle3,(N));
#define PROFILE_CFFT_INPL(fun,N,suffix, ...)           PROFILE_INVERTED(fun,(out0.suffix,out0.suffix,inp1.suffix,1,N), \
                                                                 fout,"N=" #N " [inplace]",prf_ptscycle3,(N));
#define PROFILE_CFFT_SCL(fun,N,suffix,scl_mtd)       { int bexp; \
                                                PROFILE_INVERTED(fun,(out1.suffix,inp0.suffix,inp1.suffix,1,N,&bexp,scl_mtd), \
                                                                 fout,"N=" #N " [scl=" #scl_mtd "]", prf_ptscycle3,(N)) };
#define PROFILE_CFFT_SCL_INPL(fun,N,suffix,scl_mtd)  { int bexp; \
                                                PROFILE_INVERTED(fun,(out0.suffix,out0.suffix,inp1.suffix,1,N,&bexp,scl_mtd), \
                                                                 fout,"N=" #N " [scl=" #scl_mtd " inplace]", prf_ptscycle3,(N)) };

#define PROFILE_BATCH( _PROFILER, fxn, suffix, ... ) {   \
              _PROFILER( fxn,     8,suffix, __VA_ARGS__ ); \
              _PROFILER( fxn,    16,suffix, __VA_ARGS__ ); \
              _PROFILER( fxn,    32,suffix, __VA_ARGS__ ); \
              _PROFILER( fxn,    64,suffix, __VA_ARGS__ ); \
              _PROFILER( fxn,   128,suffix, __VA_ARGS__ ); \
              _PROFILER( fxn,   256,suffix, __VA_ARGS__ ); \
              _PROFILER( fxn,   512,suffix, __VA_ARGS__ ); \
              _PROFILER( fxn,  1024,suffix, __VA_ARGS__ ); \
              _PROFILER( fxn,  2048,suffix, __VA_ARGS__ ); \
              _PROFILER( fxn,  4096,suffix, __VA_ARGS__ ); \
             }
#define PROFILE_INVERTED_FFT_cfft32x16_ie(N, twstep) PROFILE_INVERTED_FFT( fft_cplx32x16_ie,N,( out0.ci32, inp1.ci32, inp0.ci16, twstep, N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_cfft24x24_ie(N, twstep) PROFILE_INVERTED_FFT( fft_cplx24x24_ie,N,( out0.ci32, inp1.ci32, inp0.ci32, twstep, N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_cifft32x16_ie(N, twstep) PROFILE_INVERTED_FFT( ifft_cplx32x16_ie,N,( out0.ci32, inp1.ci32, inp0.ci16, twstep, N, 3),fout,prf_ptscycle3,N );
#define PROFILE_INVERTED_FFT_cifft24x24_ie(N, twstep) PROFILE_INVERTED_FFT( ifft_cplx24x24_ie,N,( out0.ci32, inp1.ci32, inp0.ci32, twstep, N, 3),fout,prf_ptscycle3,N );


void mips_cfft( int phaseNum, FILE * fout )
{
    perf_info( fout, "\nComplex FFT with memory improved usage:\n" );

    /*  Stage 1*/
    if ( phaseNum == 0 || phaseNum == 1 )
    {
        PROFILE_INVERTED_FFT_cfft32x16_ie(256, 1);
        PROFILE_INVERTED_FFT_cfft32x16_ie(512, 1);
        PROFILE_INVERTED_FFT_cfft32x16_ie(1024,1);
        PROFILE_INVERTED_FFT_cfft24x24_ie(256, 1);
        PROFILE_INVERTED_FFT_cfft24x24_ie(512, 1);
        PROFILE_INVERTED_FFT_cfft24x24_ie(1024,1);
        PROFILE_INVERTED_FFT_cifft32x16_ie(256, 1);
        PROFILE_INVERTED_FFT_cifft32x16_ie(512, 1);
        PROFILE_INVERTED_FFT_cifft32x16_ie(1024,1);
        PROFILE_INVERTED_FFT_cifft24x24_ie(256, 1);
        PROFILE_INVERTED_FFT_cifft24x24_ie(512, 1);
        PROFILE_INVERTED_FFT_cifft24x24_ie(1024,1);
    }
    /* Stage 2 */
    if ( phaseNum == 0 || phaseNum == 2 )
    {
        PROFILE_BATCH( PROFILE_CFFT         , fft_cplxf_ie , cf32    );
        PROFILE_BATCH( PROFILE_CFFT         ,ifft_cplxf_ie , cf32    );
    }

} /* mips_cfft() */
