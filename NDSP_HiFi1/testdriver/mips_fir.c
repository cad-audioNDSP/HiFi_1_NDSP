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
    test module for testing MCPS performance (FIR)
*/

#include "NatureDSP_Signal.h"
#include "mips.h"

#define BKFIR24X24_PROFILE(N,M)  OBJ_PROFILE_INVERTED(bkfir24x24, ( M),(objinstance_memory, M, inp2.i32),(bkfir24x24, out2.i32, inp2.i32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M);
#define BKFIR24X24P_PROFILE(N,M) OBJ_PROFILE_INVERTED(bkfir24x24p, ( M),(objinstance_memory, M, inp2.i32),(bkfir24x24p, out2.i32, inp2.i32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M);
#define BKFIR32X16_PROFILE(N,M)  OBJ_PROFILE_INVERTED(bkfir32x16, (M),(objinstance_memory, M, inp2.i16),(bkfir32x16, out2.i32, inp2.i32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M);
#define BKFIR32X32_PROFILE(N,M)  OBJ_PROFILE_INVERTED(bkfir32x32, (M),(objinstance_memory, M, inp2.i32),(bkfir32x32, out2.i32, inp2.i32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M);
#define BKFIRF_PROFILE(N,M)      OBJ_PROFILE_INVERTED(bkfirf,    ( M),(objinstance_memory, M, inp2.f32),(bkfirf    , out2.f32, inp2.f32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M);
#define BKFIR16X16_PROFILE(N,M)  OBJ_PROFILE_INVERTED(bkfir16x16, (M),(objinstance_memory, M, inp2.i16),(bkfir16x16, out2.i16, inp2.i16,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M);

#define BKFIRA32X16_PROFILE(N,M)  OBJ_PROFILE_INVERTED(bkfira32x16, (M),(objinstance_memory, M, inp2.i16),(bkfira32x16, out2.i32, inp2.i32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M);
#define BKFIRA16X16_PROFILE(N,M)  OBJ_PROFILE_INVERTED(bkfira16x16, (M),(objinstance_memory, M, inp2.i16),(bkfira16x16, out2.i16, inp2.i16,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M);
#define BKFIRA32X32_PROFILE(N,M)  OBJ_PROFILE_INVERTED(bkfira32x32, (M),(objinstance_memory, M, inp2.i32),(bkfira32x32, out2.i32, inp2.i32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M);
#define BKFIRA24X24_PROFILE(N,M)  OBJ_PROFILE_INVERTED(bkfira24x24, (M),(objinstance_memory, M, inp2.i32),(bkfira24x24, out2.i32, inp2.i32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M);
#define BKFIRAF_PROFILE(N,M)      OBJ_PROFILE_INVERTED(bkfiraf    , (M),(objinstance_memory, M, inp2.f32),(bkfiraf    , out2.f32, inp2.f32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M);


#define CXFIR16X16_PROFILE(N,M)  OBJ_PROFILE_INVERTED(cxfir16x16, (M),(objinstance_memory, M, inp2.ci16),(cxfir16x16, out2.ci16, inp2.ci16,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M*4);
#define CXFIR32X16_PROFILE(N,M)  OBJ_PROFILE_INVERTED(cxfir32x16, (M),(objinstance_memory, M, inp2.ci16),(cxfir32x16, out2.ci32, inp2.ci32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M*4);
#define CXFIR32X32_PROFILE(N,M)  OBJ_PROFILE_INVERTED(cxfir32x32, (M),(objinstance_memory, M, inp2.ci32),(cxfir32x32, out2.ci32, inp2.ci32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M*4);
#define CXFIR24X24_PROFILE(N,M)  OBJ_PROFILE_INVERTED(cxfir24x24, (M),(objinstance_memory, M, inp2.ci32),(cxfir24x24, out2.ci32, inp2.ci32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M*4);
#define CXFIRF_PROFILE(N,M)      OBJ_PROFILE_INVERTED(cxfirf,     (M),(objinstance_memory, M, inp2.cf32),(cxfirf    , out2.cf32, inp2.cf32,N),fout,"N=" #N "; M=" #M ,prf_maccycle,N*M*4);

#define FIRDEC16X16_PROFILE(D,N,M) OBJ_PROFILE_INVERTED(firdec16x16,(D,M),(objinstance_memory, D,M, inp2.i16),(firdec16x16,out2.i16, inp2.i16,N),fout,"N=" #N "; M=" #M "; D=" #D ,prf_maccycle,N*M);
#define FIRDEC32X16_PROFILE(D,N,M) OBJ_PROFILE_INVERTED(firdec32x16,(D,M),(objinstance_memory, D,M, inp2.i16),(firdec32x16,out2.i32, inp2.i32,N),fout,"N=" #N "; M=" #M "; D=" #D ,prf_maccycle,N*M);
#define FIRDEC32X32_PROFILE(D,N,M) OBJ_PROFILE_INVERTED(firdec32x32,(D,M),(objinstance_memory, D,M, inp2.i32),(firdec32x32,out2.i32, inp2.i32,N),fout,"N=" #N "; M=" #M "; D=" #D ,prf_maccycle,N*M);
#define FIRDEC24X24_PROFILE(D,N,M) OBJ_PROFILE_INVERTED(firdec24x24,(D,M),(objinstance_memory, D,M, inp2.i32),(firdec24x24,out2.i32, inp2.i32,N),fout,"N=" #N "; M=" #M "; D=" #D ,prf_maccycle,N*M);
#define FIRDECF_PROFILE(D,N,M)     OBJ_PROFILE_INVERTED(firdecf    ,(D,M),(objinstance_memory, D,M, inp2.f32),(firdecf    ,out2.f32, inp2.f32,N),fout,"N=" #N "; M=" #M "; D=" #D ,prf_maccycle,N*M);

#define CXFIRINTERP16X16_PROFILE(D,N,M) OBJ_PROFILE_INVERTED(cxfirinterp16x16,(D,M),(objinstance_memory, D,M, inp2.i16),(cxfirinterp16x16,out2.ci16, inp2.ci16, N),fout,"N=" #N "; M=" #M "; D=" #D ,prf_maccycle,2*N*M*D);
#define FIRINTERP16X16_PROFILE(D,N,M) OBJ_PROFILE_INVERTED(firinterp16x16,(D,M),(objinstance_memory, D,M, inp2.i16),(firinterp16x16,out2.i16, inp2.i16, N),fout,"N=" #N "; M=" #M "; D=" #D ,prf_maccycle,N*M*D);
#define FIRINTERP32X16_PROFILE(D,N,M) OBJ_PROFILE_INVERTED(firinterp32x16,(D,M),(objinstance_memory, D,M, inp2.i16),(firinterp32x16,out2.i32, inp2.i32, N),fout,"N=" #N "; M=" #M "; D=" #D ,prf_maccycle,N*M*D);
#define FIRINTERP32X32_PROFILE(D,N,M) OBJ_PROFILE_INVERTED(firinterp32x32,(D,M),(objinstance_memory, D,M, inp2.i32),(firinterp32x32,out2.i32, inp2.i32, N),fout,"N=" #N "; M=" #M "; D=" #D ,prf_maccycle,N*M*D);
#define FIRINTERP24X24_PROFILE(D,N,M) OBJ_PROFILE_INVERTED(firinterp24x24,(D,M),(objinstance_memory, D,M, inp2.i32),(firinterp24x24,out2.i32, inp2.i32, N),fout,"N=" #N "; M=" #M "; D=" #D ,prf_maccycle,N*M*D);
#define FIRINTERPF_PROFILE(D,N,M)     OBJ_PROFILE_INVERTED(firinterpf    ,(D,M),(objinstance_memory, D,M, inp2.f32),(firinterpf    ,out2.f32, inp2.f32, N),fout,"N=" #N "; M=" #M "; D=" #D ,prf_maccycle,N*M*D);

void mips_bkfir24x24(FILE* fout)
{
    BKFIR24X24_PROFILE( 80, 256)
    BKFIR24X24_PROFILE(2048,   8)
    BKFIR24X24_PROFILE( 160,   8)
    BKFIR24X24_PROFILE( 160,  16)
    BKFIR24X24_PROFILE(1024,  32)

    BKFIR24X24P_PROFILE(80, 256)
    BKFIR24X24P_PROFILE(2048, 8)
    BKFIR24X24P_PROFILE(160, 8)
    BKFIR24X24P_PROFILE(160, 16)
    BKFIR24X24P_PROFILE(1024, 32)

    BKFIRA24X24_PROFILE(80, 256)
    BKFIRA24X24_PROFILE(2048, 8)
    BKFIRA24X24_PROFILE(160, 8)
    BKFIRA24X24_PROFILE(160, 16)
    BKFIRA24X24_PROFILE(1024, 32)


}
void mips_bkfir32x32(FILE* fout)
{

    BKFIR32X16_PROFILE(80, 256)
    BKFIR32X16_PROFILE(2048, 8)
    BKFIR32X16_PROFILE(160, 8)
    BKFIR32X16_PROFILE(160, 16)
    BKFIR32X16_PROFILE(1024, 32)

    BKFIR32X32_PROFILE(80, 256)
    BKFIR32X32_PROFILE(2048, 8)
    BKFIR32X32_PROFILE(160, 8)
    BKFIR32X32_PROFILE(160, 16)
    BKFIR32X32_PROFILE(1024, 32)

    BKFIRA32X16_PROFILE( 80, 256)
    BKFIRA32X16_PROFILE(2048,   8)
    BKFIRA32X16_PROFILE( 160,   8)
    BKFIRA32X16_PROFILE( 160,  16)
    BKFIRA32X16_PROFILE(1024,  32)
}

void mips_cxfir32x16(FILE* fout)
{
    CXFIR32X16_PROFILE( 80, 128)
    CXFIR32X16_PROFILE(2048,   8)
    CXFIR32X16_PROFILE( 160,   8)
    CXFIR32X16_PROFILE( 160,  16)
    CXFIR32X16_PROFILE(1024,  32)

    CXFIR24X24_PROFILE( 80, 128)
    CXFIR24X24_PROFILE(2048,   8)
    CXFIR24X24_PROFILE( 160,   8)
    CXFIR24X24_PROFILE( 160,  16)
    CXFIR24X24_PROFILE(1024,  32)
}

void mips_firdec32x16(FILE* fout)
{
    FIRDEC32X16_PROFILE(2, 1024,  16)
    FIRDEC32X16_PROFILE(2, 1024, 256)
    FIRDEC32X16_PROFILE(2, 1024, 260)
    FIRDEC32X16_PROFILE(2, 1024, 261)
    FIRDEC32X16_PROFILE(2,   80, 256)
    FIRDEC32X16_PROFILE(3, 1024,  16)
    FIRDEC32X16_PROFILE(3, 1024, 256)
    FIRDEC32X16_PROFILE(3, 1024, 260)
    FIRDEC32X16_PROFILE(3, 1024, 261)
    FIRDEC32X16_PROFILE(4, 1024,  16)
    FIRDEC32X16_PROFILE(4, 1024, 256)
    FIRDEC32X16_PROFILE(4, 1024, 260)
    FIRDEC32X16_PROFILE(4, 1024, 261)
    FIRDEC32X16_PROFILE(5, 1024, 256)
    FIRDEC32X16_PROFILE(5, 1024, 260)
    FIRDEC32X16_PROFILE(7, 1024, 256)
    FIRDEC32X16_PROFILE(7, 1024, 260)
}
void mips_firdec24x24(FILE* fout)
{
    FIRDEC24X24_PROFILE(2, 1024,  16)
    FIRDEC24X24_PROFILE(2, 1024, 256)
    FIRDEC24X24_PROFILE(2, 1024, 260)
    FIRDEC24X24_PROFILE(2, 1024, 261)
    FIRDEC24X24_PROFILE(3, 1024,  16)
    FIRDEC24X24_PROFILE(3, 1024, 256)
    FIRDEC24X24_PROFILE(3, 1024, 260)
    FIRDEC24X24_PROFILE(3, 1024, 261)
    FIRDEC24X24_PROFILE(4, 1024,  16)
    FIRDEC24X24_PROFILE(4, 1024, 256)
    FIRDEC24X24_PROFILE(4, 1024, 260)
    FIRDEC24X24_PROFILE(4, 1024, 261)
    FIRDEC24X24_PROFILE(5, 1024, 256)
    FIRDEC24X24_PROFILE(5, 1024, 260)
    FIRDEC24X24_PROFILE(7, 1024, 256)
    FIRDEC24X24_PROFILE(7, 1024, 260)
    FIRDEC24X24_PROFILE(2,   80, 256)

}

void mips_firinterp32x16(FILE* fout)
{
    FIRINTERP32X16_PROFILE(2, 1024,  16)
    FIRINTERP32X16_PROFILE(2, 1024, 256)
    FIRINTERP32X16_PROFILE(2, 1024, 260)
    FIRINTERP32X16_PROFILE(3, 1024,  16)
    FIRINTERP32X16_PROFILE(3, 1024, 256)
    FIRINTERP32X16_PROFILE(3, 1024, 260)
    FIRINTERP32X16_PROFILE(4, 1024,  16)
    FIRINTERP32X16_PROFILE(4, 1024, 256)
    FIRINTERP32X16_PROFILE(4, 1024, 260)
    FIRINTERP32X16_PROFILE(5, 1024, 256)
    FIRINTERP32X16_PROFILE(5, 1024, 260)
    FIRINTERP32X16_PROFILE(7, 1024, 256)
    FIRINTERP32X16_PROFILE(7, 1024, 260)
    FIRINTERP32X16_PROFILE(2,   80, 204)
}

void mips_firinterp24x24(FILE* fout)
{
    FIRINTERP24X24_PROFILE(2, 1024,  16)
    FIRINTERP24X24_PROFILE(2, 1024, 256)
    FIRINTERP24X24_PROFILE(2, 1024, 260)
    FIRINTERP24X24_PROFILE(3, 1024,  16)
    FIRINTERP24X24_PROFILE(3, 1024, 256)
    FIRINTERP24X24_PROFILE(3, 1024, 260)
    FIRINTERP24X24_PROFILE(4, 1024,  16)
    FIRINTERP24X24_PROFILE(4, 1024, 256)
    FIRINTERP24X24_PROFILE(4, 1024, 260)
    FIRINTERP24X24_PROFILE(5, 1024, 256)
    FIRINTERP24X24_PROFILE(5, 1024, 260)
    FIRINTERP24X24_PROFILE(7, 1024, 256)
    FIRINTERP24X24_PROFILE(7, 1024, 260)
    FIRINTERP24X24_PROFILE(2,   80, 204)
}

void mips_convol(FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_convol32x16,(out0.i32, inp2.i32, inp1.i16+4,  80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_convol32x16,(out0.i32, inp2.i32, inp1.i16+4, 256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );
    PROFILE_INVERTED(fir_convol24x24,(out0.i32, inp2.i32, inp1.i32,    80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_convol24x24,(out0.i32, inp2.i32, inp1.i32,   256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );

    PROFILE_INVERTED(fir_convola32x16,(pScr, out0.i32, inp2.i32, inp1.i16+4,  80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_convola32x16,(pScr, out0.i32, inp2.i32, inp1.i16+4, 256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );
    PROFILE_INVERTED(fir_convola24x24,(pScr, out0.i32, inp2.i32, inp1.i32,    80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_convola24x24,(pScr, out0.i32, inp2.i32, inp1.i32,   256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );
}
void mips_cx_convol(FILE* fout)
{
  void* pScr = (void*)scratch_memory;
  PROFILE_INVERTED(cxfir_convol32x16, (out0.ci32, inp2.ci32, inp1.ci16 + 2, 80, 56), fout, "N=80; M=56",prf_maccycle, 4*80 * 56);
  PROFILE_INVERTED(cxfir_convol32x16, (out0.ci32, inp2.ci32, inp1.ci16 + 2, 256, 80), fout, "N=256; M=80",prf_maccycle, 4*80 * 256);
  PROFILE_INVERTED(cxfir_convola32x16, (pScr, out0.ci32, inp2.ci32, inp1.ci16 + 2, 80, 56), fout, "N=80; M=56",prf_maccycle, 4*80 * 56);
  PROFILE_INVERTED(cxfir_convola32x16, (pScr, out0.ci32, inp2.ci32, inp1.ci16 + 2, 256, 80), fout, "N=256; M=80",prf_maccycle, 4*80 * 256);
}
void mips_xcorr(FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_xcorr32x16,(out0.i32, inp2.i32, inp1.i16+0, 80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_xcorr32x16,(out0.i32, inp2.i32, inp1.i16+0, 256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );
    PROFILE_INVERTED(fir_xcorr24x24,(out0.i32, inp2.i32, inp1.i32, 80, 56),fout,"N=80; M=56",prf_maccycle,     80*56);
    PROFILE_INVERTED(fir_xcorr24x24,(out0.i32, inp2.i32, inp1.i32, 256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );

    PROFILE_INVERTED(fir_xcorra32x16,(pScr, out0.i32, inp2.i32, inp1.i16+0, 80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_xcorra32x16,(pScr, out0.i32, inp2.i32, inp1.i16+0, 256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );
    PROFILE_INVERTED(fir_xcorra24x24,(pScr, out0.i32, inp2.i32, inp1.i32, 80, 56),fout,"N=80; M=56",prf_maccycle, 80*56 );
    PROFILE_INVERTED(fir_xcorra24x24,(pScr, out0.i32, inp2.i32, inp1.i32, 256, 80),fout,"N=256; M=80",prf_maccycle, 80*256 );
}

void mips_blms(FILE* fout)
{
    PROFILE_INVERTED(fir_blms16x32,(out1.i32, out2.i32, inp1.i16, inp0.i16, 0x111111, 111, 80, 16),fout,"N=80; M=16",prf_maccycle,    80*16*2 );
    PROFILE_INVERTED(fir_blms16x32,(out1.i32, out2.i32, inp1.i16, inp0.i16, 0x111111, 111, 64, 16),fout,"N=64; M=16",prf_maccycle,    64*16*2 );
    PROFILE_INVERTED(fir_blms16x32,(out1.i32, out2.i32, inp1.i16, inp0.i16, 0x111111, 111, 64, 64),fout,"N=64; M=64",prf_maccycle,    64*64*2 );
    PROFILE_INVERTED(fir_blms16x32,(out1.i32, out2.i32, inp1.i16, inp0.i16, 0x111111, 111, 80, 64),fout,"N=80; M=64",prf_maccycle,    80*64*2 );
    PROFILE_INVERTED(fir_blms16x32,(out1.i32, out2.i32, inp2.i16, inp0.i16, 0x111111, 111, 80, 128),fout,"N=80; M=128",prf_maccycle, 80*128*2 );
    PROFILE_INVERTED(fir_blms16x32,(out1.i32, out2.i32, inp2.i16, inp0.i16, 0x111111, 111, 64, 128),fout,"N=64; M=128",prf_maccycle, 64*128*2 );

    PROFILE_INVERTED(fir_blms24x24,(out1.i32, out2.i32, inp1.i32, inp0.i32, 0x111111, 111, 80, 16),fout,"N=80; M=16",prf_maccycle,    80*16*2 );
    PROFILE_INVERTED(fir_blms24x24,(out1.i32, out2.i32, inp1.i32, inp0.i32, 0x111111, 111, 64, 16),fout,"N=64; M=16",prf_maccycle,    64*16*2 );
    PROFILE_INVERTED(fir_blms24x24,(out1.i32, out2.i32, inp1.i32, inp0.i32, 0x111111, 111, 64, 64),fout,"N=64; M=64",prf_maccycle,    64*64*2 );
    PROFILE_INVERTED(fir_blms24x24,(out1.i32, out2.i32, inp1.i32, inp0.i32, 0x111111, 111, 80, 64),fout,"N=80; M=64",prf_maccycle,    80*64*2 );
    PROFILE_INVERTED(fir_blms24x24,(out1.i32, out2.i32, inp2.i32, inp0.i32, 0x111111, 111, 80, 128),fout,"N=80; M=128",prf_maccycle, 80*128*2 );
    PROFILE_INVERTED(fir_blms24x24,(out1.i32, out2.i32, inp2.i32, inp0.i32, 0x111111, 111, 64, 128),fout,"N=64; M=128",prf_maccycle, 64*128*2 );
}

void mips_convolf(FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_convolf,(out0.f32, inp2.f32, inp1.f32,    80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_convolf,(out0.f32, inp2.f32, inp1.f32,   256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );

    PROFILE_INVERTED(fir_convolaf,(pScr, out0.f32, inp2.f32, inp1.f32,    80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_convolaf,(pScr, out0.f32, inp2.f32, inp1.f32,   256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );
}

void mips_acorr(FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_acorr24x24,(out0.i32, inp2.i32, 80),fout,"N=80",prf_maccycle,    80*80   );
    PROFILE_INVERTED(fir_acorr24x24,(out0.i32, inp2.i32, 256),fout,"N=256",prf_maccycle,  256*256 );

    PROFILE_INVERTED(fir_acorra24x24,(pScr, out0.i32, inp2.i32, 80),fout,"N=80",prf_maccycle,    80*80   );
    PROFILE_INVERTED(fir_acorra24x24,(pScr, out0.i32, inp2.i32, 256),fout,"N=256",prf_maccycle,  256*256 );
}

void mips_acorrf(FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_acorrf,(out0.f32, inp2.f32, 80),fout,"N=80",prf_maccycle,    80*80   );
    PROFILE_INVERTED(fir_acorrf,(out0.f32, inp2.f32, 256),fout,"N=256",prf_maccycle,  256*256 );
    PROFILE_INVERTED(fir_acorraf,(pScr, out0.f32, inp2.f32, 80),fout,"N=80",prf_maccycle,    80*80   );
    PROFILE_INVERTED(fir_acorraf,(pScr, out0.f32, inp2.f32, 256),fout,"N=256",prf_maccycle,  256*256 );
}

void mips_xcorrf(FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_xcorrf,  (out0.f32, inp2.f32, inp1.f32, 80, 56    ),fout,"N=80; M=56" ,prf_maccycle,    80*56);
    PROFILE_INVERTED(fir_xcorrf,  (out0.f32, inp2.f32, inp1.f32, 256, 80   ),fout,"N=256; M=80",prf_maccycle,   80*256);
    PROFILE_INVERTED(cxfir_xcorrf,(out0.cf32, inp2.cf32, inp1.cf32, 80, 56 ),fout,"N=80; M=56" ,prf_maccycle,  4*80*56);
    PROFILE_INVERTED(cxfir_xcorrf,(out0.cf32, inp2.cf32, inp1.cf32, 256, 80),fout,"N=256; M=80",prf_maccycle, 4*80*256);

    PROFILE_INVERTED(fir_xcorraf,  (pScr, out0.f32,  inp2.f32,  inp1.f32, 80, 56  ),fout,"N=80; M=56" ,prf_maccycle,    80*56);
    PROFILE_INVERTED(fir_xcorraf,  (pScr, out0.f32,  inp2.f32,  inp1.f32, 256, 80 ),fout,"N=256; M=80",prf_maccycle,   80*256);
    PROFILE_INVERTED(cxfir_xcorraf,(pScr, out0.cf32, inp2.cf32, inp1.cf32, 80, 56 ),fout,"N=80; M=56" ,prf_maccycle,  4*80*56);
    PROFILE_INVERTED(cxfir_xcorraf,(pScr, out0.cf32, inp2.cf32, inp1.cf32, 256, 80),fout,"N=256; M=80",prf_maccycle, 4*80*256);
}

void mips_blmsf(FILE* fout)
{
    PROFILE_INVERTED(fir_blmsf,(out1.f32, out2.f32, inp1.f32, inp0.f32, 0.1, 1.1, 80, 16),fout,"N=80; M=16",prf_maccycle,    80*16*2 );
    PROFILE_INVERTED(fir_blmsf,(out1.f32, out2.f32, inp1.f32, inp0.f32, 0.1, 1.1, 64, 16),fout,"N=64; M=16",prf_maccycle,    64*16*2 );
    PROFILE_INVERTED(fir_blmsf,(out1.f32, out2.f32, inp1.f32, inp0.f32, 0.1, 1.1, 64, 64),fout,"N=64; M=64",prf_maccycle,    64*64*2 );
    PROFILE_INVERTED(fir_blmsf,(out1.f32, out2.f32, inp1.f32, inp0.f32, 0.1, 1.1, 80, 64),fout,"N=80; M=64",prf_maccycle,    80*64*2 );
    PROFILE_INVERTED(fir_blmsf,(out1.f32, out2.f32, inp2.f32, inp0.f32, 0.1, 1.1, 80, 128),fout,"N=80; M=128",prf_maccycle, 80*128*2 );
    PROFILE_INVERTED(fir_blmsf,(out1.f32, out2.f32, inp2.f32, inp0.f32, 0.1, 1.1, 64, 128),fout,"N=64; M=128",prf_maccycle, 64*128*2 );
}

#define PROFILE_FIR(fun,N,M,firstate_type,suffix)                                                       \
{                                                                                                       \
    firstate_type state;                                                                                \
    fir_init((&state),inp2. suffix,inp1. suffix,M,1);                                                   \
    PROFILE_INVERTED(fun,(&state,out0. suffix,inp0. suffix,N),fout,"N=" #N ",M=" #M,prf_maccycle,(M*N));\
}

#define PROFILE_FIRf(fun,N,M) PROFILE_FIR(fun,N,M,fir_statef,f32)

#define PROFILE_CFIR(fun,N,M,firstate_type,suffix)                                                         \
{                                                                                                          \
    firstate_type state;                                                                                   \
    cfir_init(&state,inp2. suffix,inp1. suffix,M);                                                         \
    PROFILE_INVERTED(fun,(&state,out0. suffix,inp0. suffix,N),fout,"N=" #N ",M=" #M,prf_maccycle,(4*M*N)); \
}

#define PROFILE_CFIRf(fun,N,M)     PROFILE_CFIR(fun,N,M,cfir_statef,cf32)

#define PROFILE_DECIMA(fun,N,M,D,firstate_type,suffix)                                                              \
{                                                                                                                   \
    firstate_type state;                                                                                            \
    fir_init((&state),inp2. suffix,inp1. suffix,M,D);                                                               \
    PROFILE_INVERTED(fun,(&state,out0. suffix,inp0. suffix,N),fout,"N=" #N ",M=" #M",D=" #D,prf_maccycle,(M*(N/D)));\
}

#define PROFILE_DECIMAf(fun,N,M,D) PROFILE_DECIMA(fun,N,M,D,fir_statef,f32) 

#define PROFILE_INTERP(fun,N,M,D,firstate_type,suffix)                                                              \
{                                                                                                                   \
    firstate_type state;                                                                                            \
    fir_init((&state),inp2. suffix,inp1. suffix,M,D);                                                               \
    PROFILE_INVERTED(fun,(&state,out0. suffix,inp0. suffix,N),fout,"N=" #N ",M=" #M",D=" #D,prf_maccycle,(M*N*D));  \
}

#define PROFILE_INTERPf(fun,N,M,D)         PROFILE_INTERP(fun,N,M,D,fir_statef    ,f32)

/* phase3 measurements */
static void  mips_bkfir16x16(FILE* fout)
{
    BKFIR16X16_PROFILE(512, 256)
    BKFIR16X16_PROFILE(512, 32)
}

static void  mips_bkfira16x16(FILE* fout)
{
    BKFIRA16X16_PROFILE(512, 256)
    BKFIRA16X16_PROFILE(512, 32)
}

static void  mips_cxfir16x16(FILE* fout)
{
    CXFIR16X16_PROFILE(512, 256)
    CXFIR16X16_PROFILE(512, 32)
}

static void  mips_firdec16x16(FILE* fout)
{
    FIRDEC16X16_PROFILE(2, 1024,  16)
    FIRDEC16X16_PROFILE(2, 1024, 256)
    FIRDEC16X16_PROFILE(2, 1024, 260)
    FIRDEC16X16_PROFILE(2, 1024, 261)
    FIRDEC16X16_PROFILE(3, 1024,  16)
    FIRDEC16X16_PROFILE(3, 1024, 256)
    FIRDEC16X16_PROFILE(3, 1024, 260)
    FIRDEC16X16_PROFILE(3, 1024, 261)
    FIRDEC16X16_PROFILE(4, 1024,  16)
    FIRDEC16X16_PROFILE(4, 1024, 256)
    FIRDEC16X16_PROFILE(4, 1024, 260)
    FIRDEC16X16_PROFILE(4, 1024, 261)
    FIRDEC16X16_PROFILE(5, 1024, 256)
    FIRDEC16X16_PROFILE(5, 1024, 260)
    FIRDEC16X16_PROFILE(7, 1024, 256)
    FIRDEC16X16_PROFILE(7, 1024, 260)
    FIRDEC16X16_PROFILE(2,   80, 256)
}
static void  mips_firinterp16x16(FILE* fout)
{
    FIRINTERP16X16_PROFILE(2, 1024,  16)
    FIRINTERP16X16_PROFILE(2, 1024, 256)
    FIRINTERP16X16_PROFILE(2, 1024, 260)
    FIRINTERP16X16_PROFILE(3, 1024,  16)
    FIRINTERP16X16_PROFILE(3, 1024, 256)
    FIRINTERP16X16_PROFILE(3, 1024, 260)
    FIRINTERP16X16_PROFILE(4, 1024,  16)
    FIRINTERP16X16_PROFILE(4, 1024, 256)
    FIRINTERP16X16_PROFILE(4, 1024, 260)
    FIRINTERP16X16_PROFILE(5, 1024, 256)
    FIRINTERP16X16_PROFILE(5, 1024, 260)
    FIRINTERP16X16_PROFILE(7, 1024, 256)
    FIRINTERP16X16_PROFILE(7, 1024, 260)
    FIRINTERP16X16_PROFILE(2,   80, 204)
}

static void  mips_cxfirinterp16x16(FILE* fout)
{
    CXFIRINTERP16X16_PROFILE(2, 1024,  16)
    CXFIRINTERP16X16_PROFILE(2, 1024, 256)
    CXFIRINTERP16X16_PROFILE(2, 1024, 260)
    CXFIRINTERP16X16_PROFILE(3, 1024,  16)
    CXFIRINTERP16X16_PROFILE(3, 1024, 256)
    CXFIRINTERP16X16_PROFILE(3, 1024, 260)
    CXFIRINTERP16X16_PROFILE(4, 1024,  16)
    CXFIRINTERP16X16_PROFILE(4, 1024, 256)
    CXFIRINTERP16X16_PROFILE(4, 1024, 260)
    CXFIRINTERP16X16_PROFILE(5, 1024, 256)
    CXFIRINTERP16X16_PROFILE(5, 1024, 260)
    CXFIRINTERP16X16_PROFILE(7, 1024, 256)
    CXFIRINTERP16X16_PROFILE(7, 1024, 260)
    CXFIRINTERP16X16_PROFILE(2,   80, 204)
}

static void  mips_bkfira32x32(FILE* fout)
{
    BKFIRA32X32_PROFILE(512, 256)
    BKFIRA32X32_PROFILE(512, 32)
}
static void  mips_cxfir32x32(FILE* fout)
{
    CXFIR32X32_PROFILE(512, 256)
    CXFIR32X32_PROFILE(512, 32)
}

static void  mips_firdec32x32(FILE* fout)
{
    FIRDEC32X32_PROFILE(2, 1024,  16)
    FIRDEC32X32_PROFILE(2, 1024, 256)
    FIRDEC32X32_PROFILE(2, 1024, 260)
    FIRDEC32X32_PROFILE(2, 1024, 261)
    FIRDEC32X32_PROFILE(3, 1024,  16)
    FIRDEC32X32_PROFILE(3, 1024, 256)
    FIRDEC32X32_PROFILE(3, 1024, 260)
    FIRDEC32X32_PROFILE(3, 1024, 261)
    FIRDEC32X32_PROFILE(4, 1024,  16)
    FIRDEC32X32_PROFILE(4, 1024, 256)
    FIRDEC32X32_PROFILE(4, 1024, 260)
    FIRDEC32X32_PROFILE(4, 1024, 261)
    FIRDEC32X32_PROFILE(5, 1024, 256)
    FIRDEC32X32_PROFILE(5, 1024, 260)
    FIRDEC32X32_PROFILE(7, 1024, 256)
    FIRDEC32X32_PROFILE(7, 1024, 260)
    FIRDEC32X32_PROFILE(2,   80, 256)
}

static void  mips_firinterp32x32(FILE* fout)
{
    FIRINTERP32X32_PROFILE(2, 1024,  16)
    FIRINTERP32X32_PROFILE(2, 1024, 256)
    FIRINTERP32X32_PROFILE(2, 1024, 260)
    FIRINTERP32X32_PROFILE(3, 1024,  16)
    FIRINTERP32X32_PROFILE(3, 1024, 256)
    FIRINTERP32X32_PROFILE(3, 1024, 260)
    FIRINTERP32X32_PROFILE(4, 1024,  16)
    FIRINTERP32X32_PROFILE(4, 1024, 256)
    FIRINTERP32X32_PROFILE(4, 1024, 260)
    FIRINTERP32X32_PROFILE(5, 1024, 256)
    FIRINTERP32X32_PROFILE(5, 1024, 260)
    FIRINTERP32X32_PROFILE(7, 1024, 256)
    FIRINTERP32X32_PROFILE(7, 1024, 260)
    FIRINTERP32X32_PROFILE(2,   80, 204)
}

static void  mips_acorr16x16   (FILE* fout)
{
    PROFILE_INVERTED(fir_acorr16x16,(out0.i16, inp2.i16, 80),fout,"N=80",prf_maccycle,    80*80   );
    PROFILE_INVERTED(fir_acorr16x16,(out0.i16, inp2.i16, 256),fout,"N=256",prf_maccycle,  256*256 );
}
static void  mips_xcorr16x16   (FILE* fout)
{
    PROFILE_INVERTED(fir_xcorr16x16,  (out0.i16, inp2.i16, inp1.i16, 80, 56    ),fout,"N=80; M=56" ,prf_maccycle,    80*56);
    PROFILE_INVERTED(fir_xcorr16x16,  (out0.i16, inp2.i16, inp1.i16, 256, 80   ),fout,"N=256; M=80",prf_maccycle,   80*256);
}
static void  mips_convol16x16  (FILE* fout)
{
    PROFILE_INVERTED(fir_convol16x16,(out0.i16, inp2.i16, inp1.i16,    80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_convol16x16,(out0.i16, inp2.i16, inp1.i16,   256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );
}

static void  mips_acorra16x16  (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_acorra16x16,(pScr, out0.i16, inp2.i16, 80),fout,"N=80",prf_maccycle,    80*80   );
    PROFILE_INVERTED(fir_acorra16x16,(pScr, out0.i16, inp2.i16, 256),fout,"N=256",prf_maccycle,  256*256 );
}

static void  mips_xcorra16x16  (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_xcorra16x16,  (pScr, out0.i16,  inp2.i16,  inp1.i16, 80, 56  ),fout,"N=80; M=56" ,prf_maccycle,    80*56);
    PROFILE_INVERTED(fir_xcorra16x16,  (pScr, out0.i16,  inp2.i16,  inp1.i16, 256, 80 ),fout,"N=256; M=80",prf_maccycle,   80*256);
}

static void  mips_convola16x16 (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_convola16x16,(pScr, out0.i16, inp2.i16, inp1.i16,    80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_convola16x16,(pScr, out0.i16, inp2.i16, inp1.i16,   256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );
}

static void  mips_lacorra16x16 (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_lacorra16x16,(pScr, out0.i16, inp2.i16, 80),fout,"N=80",prf_maccycle,    80*80  /2 );
    PROFILE_INVERTED(fir_lacorra16x16,(pScr, out0.i16, inp2.i16, 256),fout,"N=256",prf_maccycle,  256*256/2 );
}

static void  mips_lxcorra16x16 (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_lxcorra16x16,  (pScr,out0.i16, inp2.i16, inp1.i16, 80, 56    ),fout,"N=80; M=56" ,prf_maccycle,   80  * 56);
    PROFILE_INVERTED(fir_lxcorra16x16,  (pScr,out0.i16, inp2.i16, inp1.i16, 256, 80   ),fout,"N=256; M=80",prf_maccycle,   256 * 80);
}

static void  mips_lconvola16x16(FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_lconvola16x16,(pScr, out0.i16, inp2.i16, inp1.i16,    80, 56),fout,"N=80; M=56",prf_maccycle,     80*56);
    PROFILE_INVERTED(fir_lconvola16x16,(pScr, out0.i16, inp2.i16, inp1.i16,   256, 80),fout,"N=256; M=80",prf_maccycle,   80*256);
}
static void  mips_lxcorra32x16 (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_lxcorra32x16,  (pScr, out0.i32,  inp2.i32,  inp1.i16, 80, 56  ),fout,"N=80; M=56" ,prf_maccycle,    80*56);
    PROFILE_INVERTED(fir_lxcorra32x16,  (pScr, out0.i32,  inp2.i32,  inp1.i16, 256, 80 ),fout,"N=256; M=80",prf_maccycle,   80*256);
}

static void  mips_lconvola32x16(FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_lconvola32x16,(pScr, out0.i32, inp2.i32, inp1.i16,    80, 56),fout,"N=80; M=56",prf_maccycle,     80*56);
    PROFILE_INVERTED(fir_lconvola32x16,(pScr, out0.i32, inp2.i32, inp1.i16,   256, 80),fout,"N=256; M=80",prf_maccycle,   80*256);
}
static void  mips_acorr32x32   (FILE* fout)
{
    PROFILE_INVERTED(fir_acorr32x32,(out0.i32, inp2.i32, 80),fout,"N=80",prf_maccycle,    80*80   );
    PROFILE_INVERTED(fir_acorr32x32,(out0.i32, inp2.i32, 256),fout,"N=256",prf_maccycle,  256*256 );
}
static void  mips_xcorr32x32   (FILE* fout)
{
    PROFILE_INVERTED(fir_xcorr32x32,  (out0.i32, inp2.i32, inp1.i32, 80, 56    ),fout,"N=80; M=56" ,prf_maccycle,    80*56);
    PROFILE_INVERTED(fir_xcorr32x32,  (out0.i32, inp2.i32, inp1.i32, 256, 80   ),fout,"N=256; M=80",prf_maccycle,   80*256);
}

static void  mips_convol32x32  (FILE* fout)
{
    PROFILE_INVERTED(fir_convol32x32,(out0.i32, inp2.i32, inp1.i32,    80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_convol32x32,(out0.i32, inp2.i32, inp1.i32,   256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );
}
static void  mips_acorra32x32  (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_acorra32x32,(pScr, out0.i32, inp2.i32, 80),fout,"N=80",prf_maccycle,    80*80   );
    PROFILE_INVERTED(fir_acorra32x32,(pScr, out0.i32, inp2.i32, 256),fout,"N=256",prf_maccycle,  256*256 );
}

static void  mips_xcorra32x32  (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_xcorra32x32,  (pScr, out0.i32,  inp2.i32,  inp1.i32, 80, 56  ),fout,"N=80; M=56" ,prf_maccycle,    80*56);
    PROFILE_INVERTED(fir_xcorra32x32,  (pScr, out0.i32,  inp2.i32,  inp1.i32, 256, 80 ),fout,"N=256; M=80",prf_maccycle,   80*256);
}

static void  mips_convola32x32 (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_convola32x32,(pScr, out0.i32, inp2.i32, inp1.i32,    80, 56),fout,"N=80; M=56",prf_maccycle,     80*56 );
    PROFILE_INVERTED(fir_convola32x32,(pScr, out0.i32, inp2.i32, inp1.i32,   256, 80),fout,"N=256; M=80",prf_maccycle,   80*256 );
}

static void  mips_lacorra32x32 (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_lacorra32x32,(pScr, out0.i32, inp2.i32, 80),fout,"N=80",prf_maccycle,    80*80   /2);
    PROFILE_INVERTED(fir_lacorra32x32,(pScr, out0.i32, inp2.i32, 256),fout,"N=256",prf_maccycle,  256*256 /2);
}

static void  mips_lxcorra32x32 (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_lxcorra32x32,  (pScr, out0.i32,  inp2.i32,  inp1.i32, 80, 56  ),fout,"N=80; M=56" ,prf_maccycle,    80*56);
    PROFILE_INVERTED(fir_lxcorra32x32,  (pScr, out0.i32,  inp2.i32,  inp1.i32, 256, 80 ),fout,"N=256; M=80",prf_maccycle,   80*256);
}

static void  mips_lconvola32x32(FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_lconvola32x32,(pScr, out0.i32, inp2.i32, inp1.i32,    80, 56),fout,"N=80; M=56",prf_maccycle,     80*56);
    PROFILE_INVERTED(fir_lconvola32x32,(pScr, out0.i32, inp2.i32, inp1.i32,   256, 80),fout,"N=256; M=80",prf_maccycle,   80*256);
}

static void  mips_lacorraf (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_lacorraf,(pScr, out0.f32, inp2.f32, 80),fout,"N=80",prf_maccycle,    80*80  /2);
    PROFILE_INVERTED(fir_lacorraf,(pScr, out0.f32, inp2.f32, 256),fout,"N=256",prf_maccycle,  256*256/2);
}
static void  mips_lxcorraf (FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_lxcorraf,  (pScr, out0.f32,  inp2.f32,  inp1.f32, 80, 56  ),fout,"N=80; M=56" ,prf_maccycle,    80*56);
    PROFILE_INVERTED(fir_lxcorraf,  (pScr, out0.f32,  inp2.f32,  inp1.f32, 256, 80 ),fout,"N=256; M=80",prf_maccycle,   80*256);
}

static void  mips_lconvolaf(FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    PROFILE_INVERTED(fir_lconvolaf, (pScr, out0.f32, inp2.f32, inp1.f32, 80, 56), fout, "N=80; M=56", prf_maccycle, 80 * 56);
    PROFILE_INVERTED(fir_lconvolaf, (pScr, out0.f32, inp2.f32, inp1.f32, 256, 80), fout, "N=256; M=80", prf_maccycle, 80 * 256);
}

void mips_blms16x16(FILE* fout)
{
    PROFILE_INVERTED(fir_blms16x16,(out1.i16, out2.i16, inp1.i16, inp0.i16, 0x111, 111, 80, 16),fout,"N=80; M=16",prf_maccycle,    80*16*2 );
    PROFILE_INVERTED(fir_blms16x16,(out1.i16, out2.i16, inp1.i16, inp0.i16, 0x111, 111, 64, 16),fout,"N=64; M=16",prf_maccycle,    64*16*2 );
    PROFILE_INVERTED(fir_blms16x16,(out1.i16, out2.i16, inp1.i16, inp0.i16, 0x111, 111, 64, 64),fout,"N=64; M=64",prf_maccycle,    64*64*2 );
    PROFILE_INVERTED(fir_blms16x16,(out1.i16, out2.i16, inp1.i16, inp0.i16, 0x111, 111, 80, 64),fout,"N=80; M=64",prf_maccycle,    80*64*2 );
    PROFILE_INVERTED(fir_blms16x16,(out1.i16, out2.i16, inp2.i16, inp0.i16, 0x111, 111, 80, 128),fout,"N=80; M=128",prf_maccycle, 80*128*2 );
    PROFILE_INVERTED(fir_blms16x16,(out1.i16, out2.i16, inp2.i16, inp0.i16, 0x111, 111, 64, 128),fout,"N=64; M=128",prf_maccycle, 64*128*2 );
}
void mips_blms32x32(FILE* fout)
{
    PROFILE_INVERTED(fir_blms32x32,(out1.i32, out2.i32, inp1.i32, inp0.i32, 0x111111, 111, 80, 16),fout,"N=80; M=16",prf_maccycle,    80*16*2 );
    PROFILE_INVERTED(fir_blms32x32,(out1.i32, out2.i32, inp1.i32, inp0.i32, 0x111111, 111, 64, 16),fout,"N=64; M=16",prf_maccycle,    64*16*2 );
    PROFILE_INVERTED(fir_blms32x32,(out1.i32, out2.i32, inp1.i32, inp0.i32, 0x111111, 111, 64, 64),fout,"N=64; M=64",prf_maccycle,    64*64*2 );
    PROFILE_INVERTED(fir_blms32x32,(out1.i32, out2.i32, inp1.i32, inp0.i32, 0x111111, 111, 80, 64),fout,"N=80; M=64",prf_maccycle,    80*64*2 );
    PROFILE_INVERTED(fir_blms32x32,(out1.i32, out2.i32, inp2.i32, inp0.i32, 0x111111, 111, 80, 128),fout,"N=80; M=128",prf_maccycle, 80*128*2 );
    PROFILE_INVERTED(fir_blms32x32,(out1.i32, out2.i32, inp2.i32, inp0.i32, 0x111111, 111, 64, 128),fout,"N=64; M=128",prf_maccycle, 64*128*2 );
}


void mips_fir(int phaseNum, FILE* fout)
{
    perf_info(fout, "\nFIR filters:\n");
    if ( phaseNum == 0 || phaseNum == 1 )
    {
        mips_bkfir24x24(fout);
        mips_bkfir32x32(fout);
        mips_cxfir32x16(fout);
        mips_firdec32x16(fout);
        mips_firdec24x24(fout);
        mips_firinterp32x16(fout);
        mips_firinterp24x24(fout);
        mips_convol(fout);
        mips_cx_convol(fout);
        mips_xcorr(fout);
        mips_acorr(fout);
        mips_blms(fout);
    }
    if ( phaseNum == 0 || phaseNum == 2 )
    {
        BKFIRAF_PROFILE(  512,  32);
        BKFIRAF_PROFILE( 1024,  32);
        BKFIRAF_PROFILE( 1024, 256);
        BKFIRAF_PROFILE( 1024, 512);
        BKFIRF_PROFILE(  512,  32);
        BKFIRF_PROFILE( 1024,  32);
        BKFIRF_PROFILE( 1024, 256);
        BKFIRF_PROFILE( 1024, 512);
        CXFIRF_PROFILE( 512,32);
        CXFIRF_PROFILE( 512,256);
        FIRDECF_PROFILE(2,  1024, 256);
        FIRDECF_PROFILE(2,  1024, 512);
        FIRDECF_PROFILE(3,  1024, 256);
        FIRDECF_PROFILE(3,  1024, 512);
        FIRDECF_PROFILE(4,  1024, 256);
        FIRDECF_PROFILE(4,  1024, 512);
        FIRDECF_PROFILE(8,  1024, 256);
        FIRDECF_PROFILE(8,  1024, 512);
        FIRDECF_PROFILE(11, 1024, 256 );
        FIRDECF_PROFILE(11, 1024, 512 );
        FIRDECF_PROFILE(23, 1024, 256 );
        FIRDECF_PROFILE(23, 1024, 512 );
        FIRINTERPF_PROFILE(2,1024,256);
        FIRINTERPF_PROFILE(2,1024,512);
        FIRINTERPF_PROFILE(3,1024,256);
        FIRINTERPF_PROFILE(3,1024,512);
        FIRINTERPF_PROFILE(4,1024,256);
        FIRINTERPF_PROFILE(4,1024,512);
        FIRINTERPF_PROFILE(8,1024,256);
        FIRINTERPF_PROFILE(8,1024,512);
        mips_convolf(fout);
        mips_xcorrf(fout);
        mips_acorrf(fout);
        mips_blmsf(fout);
    }
    if ( phaseNum == 0 || phaseNum == 3 )
    {
            mips_bkfir16x16(fout);
            mips_bkfira16x16(fout);
            mips_cxfir16x16(fout);
            mips_firdec16x16(fout);
            mips_firinterp16x16(fout);
            mips_cxfirinterp16x16(fout);
            mips_bkfira32x32(fout);
            mips_cxfir32x32(fout);
            mips_firdec32x32(fout);
            mips_firinterp32x32(fout);
            mips_acorr16x16   (fout);
            mips_xcorr16x16   (fout);
            mips_convol16x16  (fout);
            mips_acorra16x16  (fout);
            mips_xcorra16x16  (fout);
            mips_convola16x16 (fout);
            mips_lacorra16x16 (fout);
            mips_lxcorra16x16 (fout);
            mips_lconvola16x16(fout);
            mips_lxcorra32x16 (fout);
            mips_lconvola32x16(fout);
            mips_acorr32x32   (fout);
            mips_xcorr32x32   (fout);
            mips_convol32x32  (fout);
            mips_acorra32x32  (fout);
            mips_xcorra32x32  (fout);
            mips_convola32x32 (fout);
            mips_lacorra32x32 (fout);
            mips_lxcorra32x32 (fout);
            mips_lconvola32x32(fout);
            mips_lacorraf (fout);
            mips_lxcorraf (fout);
            mips_lconvolaf(fout);
            mips_blms16x16(fout);
            mips_blms32x32(fout);
    }
 }
