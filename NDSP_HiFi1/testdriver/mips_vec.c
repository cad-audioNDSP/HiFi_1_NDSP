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
    test module for testing MCPS performance (vector operations)
*/

#include "NatureDSP_Signal.h"
#include "mips.h"

void mips_vec(int phaseNum, FILE* fout)
{
    perf_info(fout, "\nVector operations:\n");

    if ( phaseNum == 0 || phaseNum == 1 )
    {
        PROFILE_NORMALIZED(vec_add16x16     ,  (out0.i16,inp0.i16,inp1.i16,200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_add24x24,       (out0.i32,inp0.i32,inp1.i32,200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_add32x32     ,  (out0.i32,inp0.i32,inp1.i32,200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_add16x16_fast,  (out0.i16,inp0.i16,inp1.i16,200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_add24x24_fast,  (out0.i32,inp0.i32,inp1.i32, 200 ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_add32x32_fast,  (out0.i32,inp0.i32,inp1.i32,200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_power16x16,     (inp0.i16,24,200                 ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_power24x24,     (inp0.i32, 44, 200               ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_power32x32,     (inp0.i32,44,200                 ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_power16x16_fast,(inp0.i16,24,200                 ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_power24x24_fast,(inp0.i32, 44, 200               ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_power32x32_fast,(inp0.i32,44,200                 ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_scale16x16,     (out0.i16,inp0.i16,32767,200     ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_scale24x24,     (out0.i32, inp0.i32, 32767, 200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_scale32x24,     (out0.i32, inp0.i32, 32767, 200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_shift16x16,     (out0.i16,inp0.i16, 1,200        ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_shift24x24,     (out0.i32, inp0.i32, 1, 200      ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_shift32x32,     (out0.i32, inp0.i32, 1, 200      ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_scale16x16_fast,(out0.i16,inp0.i16,32767,200     ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_scale24x24_fast,(out0.i32, inp0.i32, 32767, 200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_scale32x24_fast,(out0.i32, inp0.i32, 32767, 200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_shift16x16_fast,(out0.i16,inp0.i16, 1,200        ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_shift24x24_fast,(out0.i32, inp0.i32, 1, 200      ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_shift32x32_fast,(out0.i32, inp0.i32, 1, 200      ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_dot16x16,       (inp0.i16,inp1.i16,200           ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_dot24x24,       (inp0.i32, inp1.i32 , 200        ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_dot32x16,       (inp0.i32,inp1.i16+1,200         ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_dot16x16_fast , (inp0.i16,inp1.i16,200           ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_dot24x24_fast,  (inp0.i32, inp1.i32, 200         ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_dot32x16_fast , (inp0.i32,inp1.i16,200           ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_max16x16,       (inp0.i16,200                    ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_min16x16,       (inp0.i16,200                    ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_max24x24,       (inp0.i32, 200                   ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_min24x24,       (inp0.i32, 200                   ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_max32x32,       (inp0.i32, 200                   ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_min32x32,       (inp0.i32, 200                   ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_max16x16_fast,  (inp0.i16, 200                   ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_min16x16_fast,  (inp0.i16, 200                   ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_max24x24_fast,  (inp0.i32, 200                   ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_min24x24_fast,  (inp0.i32, 200                   ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_max32x32_fast,  (inp0.i32, 200                   ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_min32x32_fast,  (inp0.i32, 200                   ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_poly4_24x24,    (out0.i32,inp0.i32,inp1.i32,0,200),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_poly8_24x24,    (out0.i32,inp0.i32,inp1.i32,0,200),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_poly4_32x32,    (out0.i32,inp0.i32,inp1.i32,0,200),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_poly8_32x32,    (out0.i32,inp0.i32,inp1.i32,0,200),fout,"N=200",prf_cyclespts,200);
    }
    if ( phaseNum == 0 || phaseNum == 2 )
    {
        PROFILE_NORMALIZED(vec_dotf,       (inp0.f32,inp1.f32,200           ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_addf,       (out0.f32,inp0.f32,inp1.f32,200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_powerf,     (inp0.f32,200                    ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_shiftf,     (out0.f32, inp0.f32, 1, 200      ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_scalef,     (out0.f32, inp0.f32, 1.   , 200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_scale_sf,   (out0.f32, inp0.f32, 32767 ,- 1000, 1000, 200), fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_minf,       (inp0.f32,200                    ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_maxf,       (inp0.f32, 200                   ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_poly4f,     (out0.f32,inp0.f32,inp1.f32,200  ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_poly8f,     (out0.f32,inp0.f32,inp1.f32,200  ),fout,"N=200",prf_cyclespts,200);
    }
    if ( phaseNum == 0 || phaseNum == 3 )
    {
        PROFILE_NORMALIZED(vec_dot32x32      , (inp0.i32,inp1.i32,200           ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_dot32x32_fast , (inp0.i32,inp1.i32,200           ),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_poly4_16x16,    (out0.i16,inp0.i16,inp1.i16,0,200),fout,"N=200",prf_cyclespts,200);
        PROFILE_NORMALIZED(vec_poly8_16x16,    (out0.i16,inp0.i16,inp1.i16,0,200),fout,"N=200",prf_cyclespts,200);
    }
}
