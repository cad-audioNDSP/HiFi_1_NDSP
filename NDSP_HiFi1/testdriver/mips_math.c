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
    test module for testing MCPS performance (vector mathematics)
*/

#include "NatureDSP_Signal.h"
#include "mips.h"

void mips_math(int phaseNum, FILE* fout)
{
    perf_info(fout, "\nVector mathematics:\n");
    if ( phaseNum == 0 || phaseNum == 1 )
    {
        PROFILE_NORMALIZED(vec_recip16x16,           (out0.i16,out1.i16,inp0.i16,256              ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_recip24x24,           (out0.i32,out1.i16,inp0.i32,256              ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_recip32x32,           (out0.i32, out1.i16, inp0.i32, 256           ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_divide16x16,          (out0.i16,out1.i16,inp0.i16,inp1.i16,256     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_divide24x24,          (out0.i32, out1.i16, inp0.i32, inp1.i32, 256 ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_divide32x32  ,        (out0.i32,out1.i16,inp0.i32,inp1.i32,256     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_divide16x16_fast,     (out0.i16,out1.i16,inp0.i16,inp1.i16,256     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_divide24x24_fast,     (out0.i32, out1.i16, inp0.i32, inp1.i32, 256 ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_divide32x32_fast,     (out0.i32,out1.i16,inp0.i32,inp1.i32,256     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_bexp16,               (inp0.i16,256                                ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_bexp24,               (inp0.i32,256                                ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_bexp32,               (inp0.i32,256                                ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_bexp16_fast,          (inp0.i16,256                                ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_bexp24_fast,          (inp0.i32,256                                ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_bexp32_fast,          (inp0.i32,256                                ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_log2_32x32,           (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_logn_32x32,           (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_log10_32x32,          (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_log2_24x24,           (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_logn_24x24,           (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_log10_24x24,          (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilog2_24x24,       (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilogn_24x24,       (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilog10_24x24,      (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilog2_32x32,       (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilogn_32x32,       (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilog10_32x32,      (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sine32x32,            (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_cosine32x32,          (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sine24x24,            (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_cosine24x24,          (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sine32x32_fast,       (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_cosine32x32_fast,     (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sine24x24_fast,       (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_cosine24x24_fast,     (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_tan32x32,             (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_atan32x32,            (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_tan24x24,             (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_atan24x24,            (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sqrt24x24,            (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sqrt24x24_fast,       (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sqrt32x32,            (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sqrt32x32_fast,       (out0.i32, inp0.i32, 256                     ),fout,"N=256",prf_cyclespts,256);

        PROFILE_SIMPLE(scl_recip16x16,  (7002 ),                    fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_recip32x32,  (7002 ),                    fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_recip24x24,  (-1966),                    fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_divide16x16, (-17621, -29508),           fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_divide32x32, (-1154751292, -1933789767), fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_divide24x24, (1154751292, -1933789767),  fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_log2_32x32, (496366179),                 fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_logn_32x32, (496366179),                 fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_log10_32x32, (496366179),                fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_log2_24x24, (496366179),                 fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_logn_24x24, (496366179),                 fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_log10_24x24, (496366179),                fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_antilog2_32x32, (-1010430329),           fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_antilogn_32x32, (-1010430329),           fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_antilog10_32x32, (-1010430329),          fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_antilog2_24x24, (-1010430329),           fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_antilogn_24x24, (-1010430329),           fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_antilog10_24x24, (-1010430329),          fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_sqrt32x32, (-1154751292),                fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_sqrt24x24, (-1154751292),                fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_sine32x32, (-1154751292),                fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_cosine32x32, (-1154751292),              fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_sine24x24, (-1154751292),                fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_cosine24x24, (-1154751292),              fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_tan32x32, (2147483640),                  fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_atan32x32, (-1154751292),                fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_tan24x24, (2147483640),                  fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_atan24x24, (-1154751292),                fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_bexp16,(3917),                           fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_bexp24, (3917),                          fout,"",prf_cycle);
        PROFILE_SIMPLE(scl_bexp32,(9621325),                        fout,"",prf_cycle);
    }
    if ( phaseNum == 0 || phaseNum == 2 )
    {
        typedef union {struct { float32_t re, im;}s; complex_float z;} tcomplex_float;
        static const tcomplex_float  cone={{1.f,0.f}};
        PROFILE_NORMALIZED(vec_bexpf,               (inp0.f32,256                                ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_int2float,           (out0.f32, inp0.i32, 1,256                   ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_float2int,           (out0.i32, inp0.f32, 1,256                   ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_complex2mag,         (out0.f32, inp0.cf32, 256                    ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_complex2invmag,      (out0.f32, inp0.cf32, 256                    ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sinef     ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_cosinef   ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_tanf      ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_log2f     ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_log10f    ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_lognf     ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilog2f ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilognf ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilog10f,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_atanf     ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_atan2f    ,          (out0.f32, inp0.f32,inp1.f32, 256            ),fout,"N=256",prf_cyclespts,256);

        PROFILE_SIMPLE(scl_bexpf,(9621325.f),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_int2float,(9621325  ,1),fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_float2int,(9621325.f,1),fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_complex2mag,(cone.z),   fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_complex2invmag,(cone.z),fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_sinef     ,(1.2f),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_cosinef   ,(1.2f),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_tanf      ,(0.4f),      fout,"x=0.4",prf_cycle);
        PROFILE_SIMPLE(scl_tanf      ,(1.2f),      fout,"x=1.2",prf_cycle);
        PROFILE_SIMPLE(scl_log2f     ,(1.2f),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_log10f    ,(1.2f),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_lognf     ,(1.2f),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_antilog2f ,(1.2f),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_antilog10f,(1.2f),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_antilognf ,(1.2f),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_atanf     ,(0.7f),      fout,"x=0.7",prf_cycle);
        PROFILE_SIMPLE(scl_atanf     ,(1.3f),      fout,"x=1.3",prf_cycle);
        PROFILE_SIMPLE(scl_atan2f,    (1.2f,2.f),  fout,""     ,prf_cycle);
    }
    if ( phaseNum == 0 || phaseNum == 3 )
    {
        PROFILE_NORMALIZED(vec_atan16x16        ,          (out0.i16, inp0.i16, 200                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_atan2_16x16      ,          (out0.i16, inp0.i16,inp1.i16, 256            ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_log2_16x16       ,          (out0.i16, inp0.i16, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_logn_16x16       ,          (out0.i16, inp0.i16, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_log10_16x16      ,          (out0.i16, inp0.i16, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilog2_16x16   ,          (out0.i16, inp0.i16, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilogn_16x16   ,          (out0.i16, inp0.i16, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_antilog10_16x16  ,          (out0.i16, inp0.i16, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sine16x16        ,          (out0.i16, inp0.i16, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_cosine16x16      ,          (out0.i16, inp0.i16, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_tan16x16         ,          (out0.i16, inp0.i16, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sqrt16x16        ,          (out0.i16, inp0.i16, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_dividef          ,          (out0.f32, inp0.f32, inp1.f32, 256           ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_recipf           ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_asinf            ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_acosf            ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_sqrtf            ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_rsqrtf           ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_float2floor      ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);
        PROFILE_NORMALIZED(vec_float2ceil       ,          (out0.f32, inp0.f32, 256                     ),fout,"N=256",prf_cyclespts,256);

        PROFILE_SIMPLE(scl_atan16x16       ,(0x300),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_atan2_16x16     ,(0x300,0x200),fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_log2_16x16      ,(0x300),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_logn_16x16      ,(0x300),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_log10_16x16     ,(0x300),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_antilog2_16x16  ,(0x300),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_antilogn_16x16  ,(0x300),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_antilog10_16x16 ,(0x300),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_sine16x16       ,(0x300),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_cosine16x16     ,(0x300),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_tan16x16        ,(0x300),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_sqrt16x16       ,(0x300),      fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_dividef         ,(2.f,3.f),    fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_recipf          ,(3.f),        fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_asinf           ,(0.25f),       fout,"x=0.25"     ,prf_cycle);
        PROFILE_SIMPLE(scl_asinf           ,(0.75f),       fout,"x=0.75"     ,prf_cycle);
        PROFILE_SIMPLE(scl_acosf           ,(0.25f),       fout,"x=0.25"     ,prf_cycle);
        PROFILE_SIMPLE(scl_acosf           ,(0.75f),       fout,"x=0.75"     ,prf_cycle);
        PROFILE_SIMPLE(scl_sqrtf           ,(3.f),        fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_rsqrtf          ,(3.f),        fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_float2floor     ,(3.2f),       fout,""     ,prf_cycle);
        PROFILE_SIMPLE(scl_float2ceil      ,(3.2f),       fout,""     ,prf_cycle);
    }
}
