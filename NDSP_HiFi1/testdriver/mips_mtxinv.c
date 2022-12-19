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
 * test module for testing cycle performance (matrix inversion and related functions)
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Matrix functions API. */
#include "NatureDSP_Signal.h"
/* measurement utilities */
#include "mips.h"

#define PROFILE_MATINV(fun,N,suffix)    PROFILE_NORMALIZED(fun,(inp0.suffix),fout,"",prf_cyclesmtx,1);

void mips_mtxinv( int phaseNum, FILE* fout)
{
    int i;
    perf_info(fout, "\nMatrix inversions:\n");
    if ( phaseNum == 0 || phaseNum == 2 )
    {
        /* fill with random floating point data */
        Rand_reset(12,273);
        for (i=0; i<16; i++)
        {
            inp0.f32[i]=((int16_t)Rand())*(1.f/32768.f);
        }
        PROFILE_MATINV(mtx_inv2x2f,2,f32);
        PROFILE_MATINV(mtx_inv3x3f,3,f32);
        PROFILE_MATINV(mtx_inv4x4f,4,f32);
    }
    if ( phaseNum == 0 || phaseNum == 3 )
    {
        /* fill with random floating point data */
        Rand_reset(12,273);
        for (i=0; i<32; i++)
        {
            inp0.f32[i]=((int16_t)Rand())*(1.f/32768.f);
        }
        PROFILE_MATINV(cmtx_inv2x2f,2,cf32);
        PROFILE_MATINV(cmtx_inv3x3f,3,cf32);
        PROFILE_MATINV(cmtx_inv4x4f,4,cf32);
    }
}
