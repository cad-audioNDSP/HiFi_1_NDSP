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
    test module for testing MCPS performance (IIR)
*/

#include "NatureDSP_Signal.h"
#include "mips.h"


/* IIR performance measurement tests */
//----bqrirr---
static const int16_t coef_sos_16[5*8] =
{ // b0      b1      b2      a1     a2
   16384,  31383,  16384,  12682, 14622,
   16384, -31383,  16384, -12682, 14622,
   16384,  32215,  16384,   7625, 11691,
   16384, -32215,  16384,  -7625, 11691,
   16384,      0, -16384,      0, 10377,
   16384,  31383,  16384,  12682, 14622,
   16384, -31383,  16384, -12682, 14622,
   16384,  32215,  16384,   7625, 11691,
};
static const int32_t coef_sos_32[5*8] =
{ //  b0          b1         b2         a1         a2
  1073741824, 2056704919, 1073741824, 831104644,958261518,
  1073741824,-2056704919, 1073741824,-831104644,958261518,
  1073741824, 2111239901, 1073741824, 499713750,766176384,
  1073741824,-2111239901, 1073741824,-499713750,766176384,
  1073741824,          0,-1073741824,         0,680063938,
  1073741824, 2056704919, 1073741824, 831104644,958261518,
  1073741824,-2056704919, 1073741824,-831104644,958261518,
  1073741824, 2111239901, 1073741824, 499713750,766176384,
};
static const float32_t coef_sos_f[5*16] =
{ //  b0          b1         b2         a1         a2
1.0000000, 1.9154557, 1.0000000, 0.7740265, 0.8924506,
1.0000000,-1.9154557, 1.0000000,-0.7740265, 0.8924506,
1.0000000, 1.9662454, 1.0000000, 0.4653947, 0.7135574,
1.0000000,-1.9662454, 1.0000000,-0.4653947, 0.7135574,
1.0000000, 0.0000000,-1.0000000, 0.0000000, 0.6333589,
1.0000000, 1.9154557, 1.0000000, 0.7740265, 0.8924506,
1.0000000,-1.9154557, 1.0000000,-0.7740265, 0.8924506,
1.0000000, 1.9662454, 1.0000000, 0.4653947, 0.7135574,
1.0000000, 1.9154557, 1.0000000, 0.7740265, 0.8924506,
1.0000000,-1.9154557, 1.0000000,-0.7740265, 0.8924506,
1.0000000, 1.9662454, 1.0000000, 0.4653947, 0.7135574,
1.0000000,-1.9662454, 1.0000000,-0.4653947, 0.7135574,
1.0000000, 0.0000000,-1.0000000, 0.0000000, 0.6333589,
1.0000000, 1.9154557, 1.0000000, 0.7740265, 0.8924506,
1.0000000,-1.9154557, 1.0000000,-0.7740265, 0.8924506,
1.0000000, 1.9662454, 1.0000000, 0.4653947, 0.7135574
};

static const int16_t coef_g[8] =
{
  2460,19682,9107,9107,22170,2460,19682,9107
};
//----latr--- 
static const int16_t scale_q15 = 7815;                
static const int32_t scale_q31 = 512117760;
static const int16_t refl_q15[9] = 
{
  -19932,21123,-20868,19136,-14564,7276,-19932,21123,-20868
};
static const int32_t refl_q31[9] =
{
  -1306313216,1384367872,-1367562496,1254099968,-954675200,476997120,-1306313216,1384367872,-1367562496
};

static const float32_t reflf[9] =
{
  -0.60830f,0.64465f,-0.63682f,0.58399f,-0.44456f,0.22212f,-0.60830f,0.64465f,-0.63682f
};
static const float32_t scalef = 0.23847f;


#define OBJ_PROFILE_NORMALIZED_IIR(_objname, _a_arg, _i_arg, _p_arg, _file, _info_,_fmt, _norm)  \
{                                                                                     \
  int isPresent;                                                                      \
  isPresent =NatureDSP_Signal_isPresent(_objname##_alloc);                            \
  isPresent|=NatureDSP_Signal_isPresent(_objname##_init);                             \
  isPresent |= NatureDSP_Signal_isPresent(_objname);                                  \
  if ( (isPresent)&&(_objname##_alloc _a_arg <= sizeof(objinstance_memory)))           \
  {                                                                                   \
        _objname##_handle_t handle;                                                   \
        handle = _objname##_init _i_arg;                                              \
        if (handle == NULL) isPresent = 0;                                            \
        if (isPresent)                                                                \
        {                                                                             \
               PROFILE_NORMALIZED(_objname, _p_arg, _file, _info_, _fmt, _norm)       \
        }                                                                             \
                                                                                   \
  }                                                                               \
  if (!isPresent)                                                                 \
  {                                                                              \
  perf_info(_file, #_objname "\tNOT TESTED\n");                                \
  }                                                                                    \
}

#define OBJ_PROFILE_NORMALIZED_LAT(_objname, _a_arg, _i_arg, _p_arg, _file, _info_,_fmt, _norm)  \
{                                                                                     \
  int isPresent;                                                                      \
  isPresent =NatureDSP_Signal_isPresent(_objname##_alloc);                            \
  isPresent|=NatureDSP_Signal_isPresent(_objname##_init);                             \
  isPresent |= NatureDSP_Signal_isPresent(_objname##_process);                        \
  if (isPresent && (_objname##_alloc _a_arg <= sizeof(objinstance_memory)))           \
  {                                                                                   \
        _objname##_handle_t handle;                                                   \
        handle = _objname##_init _i_arg;                                              \
        if (handle == NULL) isPresent = 0;                                            \
        if (isPresent)                                                                \
        {                                                                             \
           PROFILE_NORMALIZED(_objname##_process, _p_arg, _file, _info_, _fmt, _norm) \
        }                                                                             \
       if(!isPresent)                                                                 \
       {                                                                              \
         perf_info(_file, #_objname "\tNOT TESTED\n");                                \
       }                                                                              \
  }                                                                                   \
}

/* IIR performance measurement tests */
#define PROFILE_BQRIIR16X16_DF1(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bqriir16x16_df1,(M),(objinstance_memory,M,coef_sos_16, coef_g, gain),(handle, scratch_memory, out0.i16, inp1.i16, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M);
#define PROFILE_BQRIIR16X16_DF2(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bqriir16x16_df2,(M),(objinstance_memory,M,coef_sos_16, coef_g, gain),(handle, scratch_memory, out0.i16, inp1.i16, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M);
#define PROFILE_BQ3IIR16X16_DF1(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bq3iir16x16_df1,(M),(objinstance_memory,M,coef_sos_16, coef_g, gain),(handle, scratch_memory, out0.i16, inp1.i16, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M*3);
#define PROFILE_BQ3IIR16X16_DF2(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bq3iir16x16_df2,(M),(objinstance_memory,M,coef_sos_16, coef_g, gain),(handle, scratch_memory, out0.i16, inp1.i16, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M*3);
#define PROFILE_BQRIIR32X16_DF1(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bqriir32x16_df1,(M),(objinstance_memory,M,coef_sos_16, coef_g, gain),(handle, scratch_memory, out0.i32, inp1.i32, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M);
#define PROFILE_BQRIIR32X16_DF2(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bqriir32x16_df2,(M),(objinstance_memory,M,coef_sos_16, coef_g, gain),(handle, scratch_memory, out0.i32, inp1.i32, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M);
#define PROFILE_BQ3IIR32X16_DF1(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bq3iir32x16_df1,(M),(objinstance_memory,M,coef_sos_16, coef_g, gain),(handle, scratch_memory, out0.i32, inp1.i32, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M*3);
#define PROFILE_BQ3IIR32X16_DF2(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bq3iir32x16_df2,(M),(objinstance_memory,M,coef_sos_16, coef_g, gain),(handle, scratch_memory, out0.i32, inp1.i32, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M*3);
#define PROFILE_BQRIIR24x24_DF1(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bqriir24x24_df1,(M),(objinstance_memory,M,coef_sos_32, coef_g, gain),(handle, scratch_memory, out0.i32, inp1.i32, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M);
#define PROFILE_BQRIIR24x24_DF2(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bqriir24x24_df2,(M),(objinstance_memory,M,coef_sos_32, coef_g, gain),(handle, scratch_memory, out0.i32, inp1.i32, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M);
#define PROFILE_BQRIIR32X32_DF1(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bqriir32x32_df1,(M),(objinstance_memory,M,coef_sos_32, coef_g, gain),(handle, scratch_memory, out0.i32, inp1.i32, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M);
#define PROFILE_BQRIIR32X32_DF2(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bqriir32x32_df2,(M),(objinstance_memory,M,coef_sos_32, coef_g, gain),(handle, scratch_memory, out0.i32, inp1.i32, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M);
#define PROFILE_BQ3IIR32X32_DF1(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bq3iir32x32_df1,(M),(objinstance_memory,M,coef_sos_32, coef_g, gain),(handle, scratch_memory, out0.i32, inp1.i32, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M*3);
#define PROFILE_BQ3IIR32X32_DF2(N,M,gain) OBJ_PROFILE_NORMALIZED_IIR(bq3iir32x32_df2,(M),(objinstance_memory,M,coef_sos_32, coef_g, gain),(handle, scratch_memory, out0.i32, inp1.i32, N),fout,"N=" #N ", M=" #M ", gain=" #gain ,prf_cyclesbqd,N*M*3);
#define PROFILE_BQRIIRF_DF1(N,M)  OBJ_PROFILE_NORMALIZED_IIR(bqriirf_df1 ,(M),(objinstance_memory,M,coef_sos_f,1),(handle, out0.f32, inp1.f32, N),fout,"N=" #N ", M=" #M ,prf_cyclesbqd,N*M);
#define PROFILE_BQRIIRF_DF2(N,M)  OBJ_PROFILE_NORMALIZED_IIR(bqriirf_df2 ,(M),(objinstance_memory,M,coef_sos_f,1),(handle, out0.f32, inp1.f32, N),fout,"N=" #N ", M=" #M ,prf_cyclesbqd,N*M);
#define PROFILE_BQRIIRF_DF2T(N,M) OBJ_PROFILE_NORMALIZED_IIR(bqriirf_df2t,(M),(objinstance_memory,M,coef_sos_f,1),(handle, out0.f32, inp1.f32, N),fout,"N=" #N ", M=" #M ,prf_cyclesbqd,N*M);
#define PROFILE_BQ3IIRF_DF1(N,M)  OBJ_PROFILE_NORMALIZED_IIR(bq3iirf_df1 ,(M),(objinstance_memory,M,coef_sos_f,1),(handle, out0.f32, inp1.f32, N),fout,"N=" #N ", M=" #M ,prf_cyclesbqd,N*M*3);
#define PROFILE_BQ3IIRF_DF2(N,M)  OBJ_PROFILE_NORMALIZED_IIR(bq3iirf_df2 ,(M),(objinstance_memory,M,coef_sos_f,1),(handle, out0.f32, inp1.f32, N),fout,"N=" #N ", M=" #M ,prf_cyclesbqd,N*M*3);
#define PROFILE_BQCIIRF_DF1(N,M)  OBJ_PROFILE_NORMALIZED_IIR(bqciirf_df1 ,(M),(objinstance_memory,M,coef_sos_f,1),(handle, out0.cf32, inp1.cf32, N),fout,"N=" #N ", M=" #M ,prf_cyclesbqd,N*M);

const char* prf_cyclessampleM =" %d (%.1f cycles/(sample*M)";

#define PROFILE_LATR16X16(N,M) OBJ_PROFILE_NORMALIZED_LAT(latr16x16,(M),(objinstance_memory,M,refl_q15,scale_q15),(handle, out0.i16, inp0.i16, N),fout,"N="#N", M="#M,prf_cyclessampleM,N*M);
#define PROFILE_LATR32X32(N,M) OBJ_PROFILE_NORMALIZED_LAT(latr32x32,(M),(objinstance_memory,M,refl_q31,scale_q31),(handle, out0.i32, inp0.i32, N),fout,"N="#N", M="#M,prf_cyclessampleM,N*M);
#define PROFILE_LATR32X16(N,M) OBJ_PROFILE_NORMALIZED_LAT(latr32x16,(M),(objinstance_memory,M,refl_q15,scale_q15),(handle, out0.i32, inp0.i32, N),fout,"N="#N", M="#M,prf_cyclessampleM,N*M);
#define PROFILE_LATR24x24(N,M) OBJ_PROFILE_NORMALIZED_LAT(latr24x24,(M),(objinstance_memory,M,refl_q31,scale_q31),(handle, out0.i32, inp0.i32, N),fout,"N="#N", M="#M,prf_cyclessampleM,N*M);
#define PROFILE_LATRF(N,M)     OBJ_PROFILE_NORMALIZED_LAT(latrf,(M),(objinstance_memory,M,reflf,scalef),(handle, out0.f32, inp0.f32, N),fout,"N="#N", M="#M,prf_cyclessampleM,N*M);

#define PROFILE_NEWIIR(fun,N,M,iirstate_type,suffixc,suffixd)                                           \
{                                                                                                       \
    iirstate_type state;                                                                                \
    iir_init((&state),inp2.suffixc,inp1.suffixd,M);                                                     \
    PROFILE_NORMALIZED(fun,(&state,out0.suffixd,inp0.suffixd,N),fout,"N=" #N ",M=" #M,prf_cyclesbqd,(N*M)); \
}

#define PROFILE_IIRDF1f(fun,N,M)     PROFILE_NEWIIR(fun,N,M,iir_statef,f32)

void mips_iir(int phaseNum, FILE* fout)
{
    perf_info(fout, "\nIIR filters:\n");
    if ( phaseNum == 0 || phaseNum == 1 )
    {
        PROFILE_BQRIIR32X16_DF1(256, 1, 0)
        PROFILE_BQRIIR32X16_DF1(256, 2, 1)
        PROFILE_BQRIIR32X16_DF1(256, 3, 0)
        PROFILE_BQRIIR32X16_DF1(256, 4, 1)
        PROFILE_BQRIIR32X16_DF1(256, 5, 0)
        PROFILE_BQRIIR32X16_DF1(256, 6, 1)
        PROFILE_BQRIIR32X16_DF1(256, 7, 0)
        PROFILE_BQRIIR32X16_DF1(256, 8, 1)
        PROFILE_BQRIIR32X16_DF1(80, 5, 0) 
        PROFILE_BQRIIR32X16_DF1(80, 5, 1)
   

        PROFILE_BQRIIR32X16_DF2(256, 1, 0) 
        PROFILE_BQRIIR32X16_DF2(256, 2, 1) 
        PROFILE_BQRIIR32X16_DF2(256, 3, 0) 
        PROFILE_BQRIIR32X16_DF2(256, 4, 1) 
        PROFILE_BQRIIR32X16_DF2(256, 5, 0) 
        PROFILE_BQRIIR32X16_DF2(256, 6, 1) 
        PROFILE_BQRIIR32X16_DF2(256, 7, 0) 
        PROFILE_BQRIIR32X16_DF2(256, 8, 1) 
        PROFILE_BQRIIR32X16_DF2(80, 5, 0) 
        PROFILE_BQRIIR32X16_DF2(80, 5, 1)

        PROFILE_BQRIIR24x24_DF1(256, 1, 0) 
        PROFILE_BQRIIR24x24_DF1(256, 2, 1) 
        PROFILE_BQRIIR24x24_DF1(256, 3, 0) 
        PROFILE_BQRIIR24x24_DF1(256, 4, 1) 
        PROFILE_BQRIIR24x24_DF1(256, 5, 0) 
        PROFILE_BQRIIR24x24_DF1(256, 6, 1) 
        PROFILE_BQRIIR24x24_DF1(256, 7, 0) 
        PROFILE_BQRIIR24x24_DF1(256, 8, 1) 
        PROFILE_BQRIIR24x24_DF1(80, 5, 0) 
        PROFILE_BQRIIR24x24_DF1(80, 5, 1)
                                     
        PROFILE_BQRIIR24x24_DF2(256, 1, 0) 
        PROFILE_BQRIIR24x24_DF2(256, 2, 1) 
        PROFILE_BQRIIR24x24_DF2(256, 3, 0) 
        PROFILE_BQRIIR24x24_DF2(256, 4, 1) 
        PROFILE_BQRIIR24x24_DF2(256, 5, 0) 
        PROFILE_BQRIIR24x24_DF2(256, 6, 1) 
        PROFILE_BQRIIR24x24_DF2(256, 7, 0) 
        PROFILE_BQRIIR24x24_DF2(256, 8, 1) 
        PROFILE_BQRIIR24x24_DF2(80, 5, 0) 
        PROFILE_BQRIIR24x24_DF2(80, 5, 1)
    
        PROFILE_BQRIIR32X32_DF2(256, 1, 0) 
        PROFILE_BQRIIR32X32_DF2(256, 2, 1) 
        PROFILE_BQRIIR32X32_DF2(256, 3, 0) 
        PROFILE_BQRIIR32X32_DF2(256, 4, 1) 
        PROFILE_BQRIIR32X32_DF2(256, 5, 0) 
        PROFILE_BQRIIR32X32_DF2(256, 6, 1) 
        PROFILE_BQRIIR32X32_DF2(256, 7, 0) 
        PROFILE_BQRIIR32X32_DF2(256, 8, 1) 
        PROFILE_BQRIIR32X32_DF2(80, 5, 0) 
        PROFILE_BQRIIR32X32_DF2(80, 5, 1)

        PROFILE_LATR32X16(256, 1)
        PROFILE_LATR32X16(256, 2)
        PROFILE_LATR32X16(256, 3)
        PROFILE_LATR32X16(256, 4)
        PROFILE_LATR32X16(256, 5)
        PROFILE_LATR32X16(256, 6)
        PROFILE_LATR32X16(256, 7)
        PROFILE_LATR32X16(256, 8)
        PROFILE_LATR32X16(256, 9)
        PROFILE_LATR32X16(80, 6)

        PROFILE_LATR24x24(256, 1)
        PROFILE_LATR24x24(256, 2)
        PROFILE_LATR24x24(256, 3)
        PROFILE_LATR24x24(256, 4)
        PROFILE_LATR24x24(256, 5)
        PROFILE_LATR24x24(256, 6)
        PROFILE_LATR24x24(256, 7)
        PROFILE_LATR24x24(256, 8)
        PROFILE_LATR24x24(256, 9)
        PROFILE_LATR24x24(80, 6)
    }

    if ( phaseNum == 0 || phaseNum == 2 )
    {
        PROFILE_BQRIIRF_DF2T(512, 1) 
        PROFILE_BQRIIRF_DF2T(512, 2) 
        PROFILE_BQRIIRF_DF2T(512, 3) 
        PROFILE_BQRIIRF_DF2T(512, 4) 
        PROFILE_BQRIIRF_DF2T(512, 8) 
        PROFILE_BQRIIRF_DF2T(512,12) 
        PROFILE_BQRIIRF_DF2T(512,16) 

        PROFILE_BQRIIRF_DF1(512, 1) 
        PROFILE_BQRIIRF_DF1(512, 2) 
        PROFILE_BQRIIRF_DF1(512, 3) 
        PROFILE_BQRIIRF_DF1(512, 4) 
        PROFILE_BQRIIRF_DF1(512, 8) 
        PROFILE_BQRIIRF_DF1(512,12) 
        PROFILE_BQRIIRF_DF1(512,16) 

        PROFILE_BQRIIRF_DF2(512, 1) 
        PROFILE_BQRIIRF_DF2(512, 2) 
        PROFILE_BQRIIRF_DF2(512, 3) 
        PROFILE_BQRIIRF_DF2(512, 4) 
        PROFILE_BQRIIRF_DF2(512, 8) 
        PROFILE_BQRIIRF_DF2(512,12) 
        PROFILE_BQRIIRF_DF2(512,16) 

        PROFILE_BQCIIRF_DF1(512, 1) 
        PROFILE_BQCIIRF_DF1(512, 2) 
        PROFILE_BQCIIRF_DF1(512, 3) 
        PROFILE_BQCIIRF_DF1(512, 4) 
        PROFILE_BQCIIRF_DF1(512, 8) 
        PROFILE_BQCIIRF_DF1(512,12) 
        PROFILE_BQCIIRF_DF1(512,16) 

        PROFILE_LATRF(256, 1)
        PROFILE_LATRF(256, 2)
        PROFILE_LATRF(256, 3)
        PROFILE_LATRF(256, 4)
        PROFILE_LATRF(256, 5)
        PROFILE_LATRF(256, 6)
        PROFILE_LATRF(256, 7)
        PROFILE_LATRF(256, 8)
        PROFILE_LATRF(256, 9)
        PROFILE_LATRF(80, 6)
    }
    if ( phaseNum == 0 || phaseNum == 3 )
    {
        PROFILE_BQRIIR16X16_DF1(256, 1, 0) 
        PROFILE_BQRIIR16X16_DF1(256, 2, 1) 
        PROFILE_BQRIIR16X16_DF1(256, 3, 0) 
        PROFILE_BQRIIR16X16_DF1(256, 4, 1) 
        PROFILE_BQRIIR16X16_DF1(256, 5, 0) 
        PROFILE_BQRIIR16X16_DF1(256, 6, 1) 
        PROFILE_BQRIIR16X16_DF1(256, 7, 0) 
        PROFILE_BQRIIR16X16_DF1(256, 8, 1) 
        PROFILE_BQRIIR16X16_DF1(80, 5, 0) 
        PROFILE_BQRIIR16X16_DF1(80, 5, 1)
        
        PROFILE_BQRIIR16X16_DF2(256, 1, 0) 
        PROFILE_BQRIIR16X16_DF2(256, 2, 1) 
        PROFILE_BQRIIR16X16_DF2(256, 3, 0) 
        PROFILE_BQRIIR16X16_DF2(256, 4, 1) 
        PROFILE_BQRIIR16X16_DF2(256, 5, 0) 
        PROFILE_BQRIIR16X16_DF2(256, 6, 1) 
        PROFILE_BQRIIR16X16_DF2(256, 7, 0) 
        PROFILE_BQRIIR16X16_DF2(256, 8, 1) 
        PROFILE_BQRIIR16X16_DF2(80, 5, 0) 
        PROFILE_BQRIIR16X16_DF2(80, 5, 1)
        
        PROFILE_BQRIIR32X32_DF1(256, 1, 0) 
        PROFILE_BQRIIR32X32_DF1(256, 2, 1) 
        PROFILE_BQRIIR32X32_DF1(256, 3, 0) 
        PROFILE_BQRIIR32X32_DF1(256, 4, 1) 
        PROFILE_BQRIIR32X32_DF1(256, 5, 0) 
        PROFILE_BQRIIR32X32_DF1(256, 6, 1) 
        PROFILE_BQRIIR32X32_DF1(256, 7, 0) 
        PROFILE_BQRIIR32X32_DF1(256, 8, 1) 
        PROFILE_BQRIIR32X32_DF1(80, 5, 0) 
        PROFILE_BQRIIR32X32_DF1(80, 5, 1)
        
        PROFILE_BQ3IIR16X16_DF1(256, 1, 0) 
        PROFILE_BQ3IIR16X16_DF1(256, 2, 1) 
        PROFILE_BQ3IIR16X16_DF1(256, 3, 0) 
        PROFILE_BQ3IIR16X16_DF1(256, 4, 1) 
        PROFILE_BQ3IIR16X16_DF1(256, 5, 0) 
        PROFILE_BQ3IIR16X16_DF1(256, 6, 1) 
        PROFILE_BQ3IIR16X16_DF1(256, 7, 0) 
        PROFILE_BQ3IIR16X16_DF1(256, 8, 1) 
        PROFILE_BQ3IIR16X16_DF1(80, 5, 0) 
        PROFILE_BQ3IIR16X16_DF1(80, 5, 1)
        
        PROFILE_BQ3IIR16X16_DF2(256, 1, 0) 
        PROFILE_BQ3IIR16X16_DF2(256, 2, 1) 
        PROFILE_BQ3IIR16X16_DF2(256, 3, 0) 
        PROFILE_BQ3IIR16X16_DF2(256, 4, 1) 
        PROFILE_BQ3IIR16X16_DF2(256, 5, 0) 
        PROFILE_BQ3IIR16X16_DF2(256, 6, 1) 
        PROFILE_BQ3IIR16X16_DF2(256, 7, 0) 
        PROFILE_BQ3IIR16X16_DF2(256, 8, 1) 
        PROFILE_BQ3IIR16X16_DF2(80, 5, 0) 
        PROFILE_BQ3IIR16X16_DF2(80, 5, 1)
        
        PROFILE_BQ3IIR32X16_DF1(256, 1, 0)
        PROFILE_BQ3IIR32X16_DF1(256, 2, 1)
        PROFILE_BQ3IIR32X16_DF1(256, 3, 0)
        PROFILE_BQ3IIR32X16_DF1(256, 4, 1)
        PROFILE_BQ3IIR32X16_DF1(256, 5, 0)
        PROFILE_BQ3IIR32X16_DF1(256, 6, 1)
        PROFILE_BQ3IIR32X16_DF1(256, 7, 0)
        PROFILE_BQ3IIR32X16_DF1(256, 8, 1)
        PROFILE_BQ3IIR32X16_DF1(80, 5, 0) 
        PROFILE_BQ3IIR32X16_DF1(80, 5, 1)
        
        PROFILE_BQ3IIR32X16_DF2(256, 1, 0) 
        PROFILE_BQ3IIR32X16_DF2(256, 2, 1) 
        PROFILE_BQ3IIR32X16_DF2(256, 3, 0) 
        PROFILE_BQ3IIR32X16_DF2(256, 4, 1) 
        PROFILE_BQ3IIR32X16_DF2(256, 5, 0) 
        PROFILE_BQ3IIR32X16_DF2(256, 6, 1) 
        PROFILE_BQ3IIR32X16_DF2(256, 7, 0) 
        PROFILE_BQ3IIR32X16_DF2(256, 8, 1) 
        PROFILE_BQ3IIR32X16_DF2(80, 5, 0) 
        PROFILE_BQ3IIR32X16_DF2(80, 5, 1)
        
        PROFILE_BQ3IIR32X32_DF1(256, 1, 0) 
        PROFILE_BQ3IIR32X32_DF1(256, 2, 1) 
        PROFILE_BQ3IIR32X32_DF1(256, 3, 0) 
        PROFILE_BQ3IIR32X32_DF1(256, 4, 1) 
        PROFILE_BQ3IIR32X32_DF1(256, 5, 0) 
        PROFILE_BQ3IIR32X32_DF1(256, 6, 1) 
        PROFILE_BQ3IIR32X32_DF1(256, 7, 0) 
        PROFILE_BQ3IIR32X32_DF1(256, 8, 1) 
        PROFILE_BQ3IIR32X32_DF1(80, 5, 0) 
        PROFILE_BQ3IIR32X32_DF1(80, 5, 1)
        
        PROFILE_BQ3IIR32X32_DF2(256, 1, 0) 
        PROFILE_BQ3IIR32X32_DF2(256, 2, 1) 
        PROFILE_BQ3IIR32X32_DF2(256, 3, 0) 
        PROFILE_BQ3IIR32X32_DF2(256, 4, 1) 
        PROFILE_BQ3IIR32X32_DF2(256, 5, 0) 
        PROFILE_BQ3IIR32X32_DF2(256, 6, 1) 
        PROFILE_BQ3IIR32X32_DF2(256, 7, 0) 
        PROFILE_BQ3IIR32X32_DF2(256, 8, 1) 
        PROFILE_BQ3IIR32X32_DF2(80, 5, 0) 
        PROFILE_BQ3IIR32X32_DF2(80, 5, 1)

        PROFILE_BQ3IIRF_DF1(512, 1) 
        PROFILE_BQ3IIRF_DF1(512, 2) 
        PROFILE_BQ3IIRF_DF1(512, 3) 
        PROFILE_BQ3IIRF_DF1(512, 4) 
        PROFILE_BQ3IIRF_DF1(512, 8) 
        PROFILE_BQ3IIRF_DF1(512,12) 
        PROFILE_BQ3IIRF_DF1(512,16) 

        PROFILE_BQ3IIRF_DF2(512, 1) 
        PROFILE_BQ3IIRF_DF2(512, 2) 
        PROFILE_BQ3IIRF_DF2(512, 3) 
        PROFILE_BQ3IIRF_DF2(512, 4) 
        PROFILE_BQ3IIRF_DF2(512, 8) 
        PROFILE_BQ3IIRF_DF2(512,12) 
        PROFILE_BQ3IIRF_DF2(512,16) 

        PROFILE_LATR16X16(256, 1)
        PROFILE_LATR16X16(256, 2)
        PROFILE_LATR16X16(256, 3)
        PROFILE_LATR16X16(256, 4)
        PROFILE_LATR16X16(256, 5)
        PROFILE_LATR16X16(256, 6)
        PROFILE_LATR16X16(256, 7)
        PROFILE_LATR16X16(256, 8)
        PROFILE_LATR16X16(256, 9)
        PROFILE_LATR16X16(80, 6)
       
        PROFILE_LATR32X32(256, 1)
        PROFILE_LATR32X32(256, 2)
        PROFILE_LATR32X32(256, 3)
        PROFILE_LATR32X32(256, 4)
        PROFILE_LATR32X32(256, 5)
        PROFILE_LATR32X32(256, 6)
        PROFILE_LATR32X32(256, 7)
        PROFILE_LATR32X32(256, 8)
        PROFILE_LATR32X32(256, 9)
        PROFILE_LATR32X32(80, 6)
    }
}
