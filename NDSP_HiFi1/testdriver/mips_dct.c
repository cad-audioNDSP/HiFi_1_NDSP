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
    test module for testing MCPS performance (DCT)
*/

#include "NatureDSP_Signal.h"
#include "mips.h"

void mips_dct(int phaseNum, FILE* fout)
{
   perf_info(fout, "\nDCT:\n");
   if ( phaseNum == 0 || phaseNum == 1 )
   {
    PROFILE_SIMPLE(dct_16x16, (out0.i16, inp0.i16, 32, 3), fout, "N=32, scalingOpt=3", prf_cycle);
    PROFILE_SIMPLE(dct_24x24,(out0.i32, inp0.i32,32,3),fout,"N=32, scalingOpt=3",prf_cycle);
    PROFILE_SIMPLE(dct_32x16,(out0.i32, inp0.i32,32,3),fout,"N=32, scalingOpt=3",prf_cycle);
    PROFILE_SIMPLE(dct_32x32, (out0.i32, inp0.i32, 32, 3), fout, "N=32, scalingOpt=3", prf_cycle);
   }
   if ( phaseNum == 0 || phaseNum == 2 )
   {
    PROFILE_SIMPLE(dctf,(out0.f32, inp0.f32,32),fout,"N=32",prf_cycle);
    PROFILE_SIMPLE(dctf,(out0.f32, inp0.f32,64),fout,"N=64",prf_cycle);
   }
}
