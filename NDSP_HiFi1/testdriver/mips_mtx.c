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
    test module for testing MCPS performance (matrix operations)
*/

#include "NatureDSP_Signal.h"
#include "mips.h"

#define PROFILE_mpy16x16(M,N,P) PROFILE_INVERTED(mtx_mpy16x16,(pScr,(int16_t**)z, inp0.i16, (const int16_t**)y, M, N, P,0),fout,#M"x"#N" x "#N"x"#P,prf_maccycle, (N*M*P));
#define PROFILE_mpy16x16_fast(M,N,P) PROFILE_INVERTED(mtx_mpy16x16_fast,(pScr,(int16_t**)z, inp0.i16, (const int16_t**)y, M, N, P,0),fout,#M"x"#N" x "#N"x"#P,prf_maccycle, (N*M*P));
#define PROFILE_mpy24x24(M,N,P) PROFILE_INVERTED(mtx_mpy24x24,(pScr,(int32_t**)z, inp0.i32, (const int32_t**)y, M, N, P,0),fout,#M"x"#N" x "#N"x"#P,prf_maccycle, (N*M*P));
#define PROFILE_mpy24x24_fast(M,N,P) PROFILE_INVERTED(mtx_mpy24x24_fast,((int32_t**)z, inp0.i32, (const int32_t**)y, M, N, P,0),fout,#M"x"#N" x "#N"x"#P,prf_maccycle, (N*M*P));
#define PROFILE_mpy32x32(M,N,P) PROFILE_INVERTED(mtx_mpy32x32,(pScr,(int32_t**)z, inp0.i32, (const int32_t**)y, M, N, P,0),fout,#M"x"#N" x "#N"x"#P,prf_maccycle, (N*M*P));
#define PROFILE_mpy32x32_fast(M,N,P) PROFILE_INVERTED(mtx_mpy32x32_fast,((int32_t**)z, inp0.i32, (const int32_t**)y, M, N, P,0),fout,#M"x"#N" x "#N"x"#P,prf_maccycle, (N*M*P));
#define PROFILE_mpyf(M,N,P) PROFILE_INVERTED(mtx_mpyf,(out0.f32, inp0.f32, inp1.f32, M, N, P),fout,#M"x"#N" x "#N"x"#P,prf_maccycle, (N*M*P));
#define PROFILE_mpyf_fast(M,N,P) PROFILE_INVERTED(mtx_mpyf_fast,(out0.f32, inp0.f32, inp1.f32, M, N, P),fout,#M"x"#N" x "#N"x"#P,prf_maccycle, (N*M*P));

#define PROFILE_mtx_vecmpy16x16(fun,M,N) PROFILE_INVERTED(fun,(out0.i16, inp0.i16, inp1.i16, M, N,0),fout,#M"x"#N" x "#N"x1",prf_maccycle, (N*M));
#define PROFILE_mtx_vecmpy24x24(fun,M,N) PROFILE_INVERTED(fun,(out0.i32, inp0.i32, inp1.i32, M, N,0),fout,#M"x"#N" x "#N"x1",prf_maccycle, (N*M));
#define PROFILE_mtx_vecmpy32x32(fun,M,N) PROFILE_INVERTED(fun,(out0.i32, inp0.i32, inp1.i32, M, N,0),fout,#M"x"#N" x "#N"x1",prf_maccycle, (N*M));
#define PROFILE_mtx_vecmpyf(fun,M,N)     PROFILE_INVERTED(fun,(out0.f32, inp0.f32, inp1.f32, M, N),fout,#M"x"#N" x "#N"x1",prf_maccycle, (N*M));

#define PROFILE_mtx_op(fun,x,N,L)      PROFILE_NORMALIZED(fun,(out0.x,inp0.x,inp1.x,L),fout,#N"x"#N", L="#L,prf_cyclesmtx,L)
#define PROFILE_mtx_op1(fun,x,rsh,N,L) PROFILE_NORMALIZED(fun,(out0.x,inp0.x,inp1.x,rsh,L),fout,#N"x"#N", L="#L,prf_cyclesmtx,L)
#define PROFILE_mtx_op2(fun,x,N,L)     PROFILE_NORMALIZED(fun,(out0.x,inp0.x,L),fout,#N"x"#N", L="#L,prf_cyclesmtx,L)
#define PROFILE_mtx_op3(fun,x,rsh,N,L) PROFILE_NORMALIZED(fun,(out0.x,inp0.x,rsh,L),fout,#N"x"#N", L="#L,prf_cyclesmtx,L)
#define PROFILE_q2rot(fun,x,L)         PROFILE_NORMALIZED(fun,(out0.x,inp0.x,L),fout,"L="#L,prf_cyclesmtx,L)


#define MAX_M 40
#define MAX_N 100
#define MAX_P 8

void mips_mtx(int phaseNum, FILE* fout)
{
    void* pScr=(void*)scratch_memory;
    perf_info(fout, "\nMatrix operations:\n");

    if ( phaseNum == 0 || phaseNum == 1 )
    {
        void* z[MAX_M*MAX_P];
        void* y[MAX_N*MAX_P];
        // prepare multiplies
        {
            int k;
            for (k=0; k<MAX_M*MAX_P; k++) z[k]=out0.i16;
            for (k=0; k<MAX_N*MAX_P; k++) y[k]=inp1.i16;
        }
        PROFILE_mpy16x16(40,80,8);
        PROFILE_mpy16x16(40,81,8);
        PROFILE_mpy16x16(40,82,8);
        PROFILE_mpy16x16(40,83,8);
        PROFILE_mpy16x16(2,100,8);
        PROFILE_mpy16x16(8,80,2);
        PROFILE_mpy16x16(8,4,2);
        PROFILE_mpy16x16(8,16,2);
        PROFILE_mpy16x16(8,32,2);
        PROFILE_mpy16x16_fast(8,80,2);
        PROFILE_mpy16x16_fast(8,84,2);
        PROFILE_mpy16x16_fast(8,4,2);
        PROFILE_mpy16x16_fast(8,16,2);
        PROFILE_mpy16x16_fast(8,32,2);

        PROFILE_mpy24x24(40,80,8);
        PROFILE_mpy24x24(40,81,8);
        PROFILE_mpy24x24(40,82,8);
        PROFILE_mpy24x24(40,83,8);
        PROFILE_mpy24x24(2,100,8);
        PROFILE_mpy24x24(8,80,2);
        PROFILE_mpy24x24(8,4,2);
        PROFILE_mpy24x24(8,16,2);
        PROFILE_mpy24x24(8,32,2);
        PROFILE_mpy24x24_fast(8,80,2);
        PROFILE_mpy24x24_fast(8,84,2);
        PROFILE_mpy24x24_fast(8,4,2);
        PROFILE_mpy24x24_fast(8,16,2);
        PROFILE_mpy24x24_fast(8,32,2);

        PROFILE_mtx_vecmpy16x16(mtx_vecmpy16x16,     16,100);
        PROFILE_mtx_vecmpy16x16(mtx_vecmpy16x16,     16,104);
        PROFILE_mtx_vecmpy16x16(mtx_vecmpy16x16,     40,40);
        PROFILE_mtx_vecmpy16x16(mtx_vecmpy16x16_fast,16,100);
        PROFILE_mtx_vecmpy16x16(mtx_vecmpy16x16_fast,16,104);
        PROFILE_mtx_vecmpy16x16(mtx_vecmpy16x16_fast,40,40);

        PROFILE_mtx_vecmpy24x24(mtx_vecmpy24x24,     16,100);
        PROFILE_mtx_vecmpy24x24(mtx_vecmpy24x24,     16,101);
        PROFILE_mtx_vecmpy24x24(mtx_vecmpy24x24,     16,102);
        PROFILE_mtx_vecmpy24x24(mtx_vecmpy24x24,     16,103);
        PROFILE_mtx_vecmpy24x24(mtx_vecmpy24x24,     16,104);
        PROFILE_mtx_vecmpy24x24(mtx_vecmpy24x24,     40,40);
        PROFILE_mtx_vecmpy24x24(mtx_vecmpy24x24_fast,16,100);
        PROFILE_mtx_vecmpy24x24(mtx_vecmpy24x24_fast,16,104);
        PROFILE_mtx_vecmpy24x24(mtx_vecmpy24x24_fast,40,40);
    }
    if ( phaseNum == 0 || phaseNum == 2 )
    {
        PROFILE_mpyf(40,80,8);
        PROFILE_mpyf(40,81,8);
        PROFILE_mpyf(40,82,8);
        PROFILE_mpyf(40,83,8);
        PROFILE_mpyf(2,100,8);
        PROFILE_mpyf(8,80,2);
        PROFILE_mpyf(8,4,2);
        PROFILE_mpyf(8,16,2);
        PROFILE_mpyf(8,32,2);
        PROFILE_mpyf_fast(8,80,2);
        PROFILE_mpyf_fast(8,84,2);
        PROFILE_mpyf_fast(8,4,2);
        PROFILE_mpyf_fast(8,16,2);
        PROFILE_mpyf_fast(8,32,2);
        PROFILE_mtx_vecmpyf(mtx_vecmpyf,     16,100);
        PROFILE_mtx_vecmpyf(mtx_vecmpyf,     16,101);
        PROFILE_mtx_vecmpyf(mtx_vecmpyf,     16,102);
        PROFILE_mtx_vecmpyf(mtx_vecmpyf,     16,103);
        PROFILE_mtx_vecmpyf(mtx_vecmpyf,     16,104);
        PROFILE_mtx_vecmpyf(mtx_vecmpyf,     40,40);
        PROFILE_mtx_vecmpyf(mtx_vecmpyf_fast,16,100);
        PROFILE_mtx_vecmpyf(mtx_vecmpyf_fast,16,104);
        PROFILE_mtx_vecmpyf(mtx_vecmpyf_fast,40,40);
    }
    if ( phaseNum == 0 || phaseNum == 3 )
    {
        void* z[MAX_M*MAX_P];
        void* y[MAX_N*MAX_P];
        // prepare multiplies
        {
            int k;
            for (k=0; k<MAX_M*MAX_P; k++) z[k]=out0.i16;
            for (k=0; k<MAX_N*MAX_P; k++) y[k]=inp1.i16;
        }
        PROFILE_mpy32x32(40,80,8);
        PROFILE_mpy32x32(40,81,8);
        PROFILE_mpy32x32(40,82,8);
        PROFILE_mpy32x32(40,83,8);
        PROFILE_mpy32x32(2,100,8);
        PROFILE_mpy32x32(8,80,2);
        PROFILE_mpy32x32(8,4,2);
        PROFILE_mpy32x32(8,16,2);
        PROFILE_mpy32x32(8,32,2);
        PROFILE_mpy32x32_fast(8,80,2);
        PROFILE_mpy32x32_fast(8,84,2);
        PROFILE_mpy32x32_fast(8,4,2);
        PROFILE_mpy32x32_fast(8,16,2);
        PROFILE_mpy32x32_fast(8,32,2);
        PROFILE_mtx_vecmpy32x32(mtx_vecmpy32x32,     16,100);
        PROFILE_mtx_vecmpy32x32(mtx_vecmpy32x32,     16,101);
        PROFILE_mtx_vecmpy32x32(mtx_vecmpy32x32,     16,102);
        PROFILE_mtx_vecmpy32x32(mtx_vecmpy32x32,     16,103);
        PROFILE_mtx_vecmpy32x32(mtx_vecmpy32x32,     16,104);
        PROFILE_mtx_vecmpy32x32(mtx_vecmpy32x32,     40,40);
        PROFILE_mtx_vecmpy32x32(mtx_vecmpy32x32_fast,16,100);
        PROFILE_mtx_vecmpy32x32(mtx_vecmpy32x32_fast,16,104);
        PROFILE_mtx_vecmpy32x32(mtx_vecmpy32x32_fast,40,40);

        PROFILE_mtx_op(mtx_add2x2_16x16,i16,2,256);
        PROFILE_mtx_op(mtx_add3x3_16x16,i16,3,256);
        PROFILE_mtx_op(mtx_add4x4_16x16,i16,4,256);
        PROFILE_mtx_op(mtx_add2x2_32x32,i32,2,256);
        PROFILE_mtx_op(mtx_add3x3_32x32,i32,3,256);
        PROFILE_mtx_op(mtx_add4x4_32x32,i32,4,256);
        PROFILE_mtx_op(mtx_add2x2f     ,f32,2,256);
        PROFILE_mtx_op(mtx_add3x3f     ,f32,3,256);
        PROFILE_mtx_op(mtx_add4x4f     ,f32,4,256);

        PROFILE_mtx_op(cmtx_add2x2_16x16,ci16,2,256);
        PROFILE_mtx_op(cmtx_add3x3_16x16,ci16,3,256);
        PROFILE_mtx_op(cmtx_add4x4_16x16,ci16,4,256);
        PROFILE_mtx_op(cmtx_add2x2_32x32,ci32,2,256);
        PROFILE_mtx_op(cmtx_add3x3_32x32,ci32,3,256);
        PROFILE_mtx_op(cmtx_add4x4_32x32,ci32,4,256);
        PROFILE_mtx_op(cmtx_add2x2f     ,cf32,2,256);
        PROFILE_mtx_op(cmtx_add3x3f     ,cf32,3,256);
        PROFILE_mtx_op(cmtx_add4x4f     ,cf32,4,256);

        PROFILE_mtx_op(mtx_sub2x2_16x16,i16,2,256);
        PROFILE_mtx_op(mtx_sub3x3_16x16,i16,3,256);
        PROFILE_mtx_op(mtx_sub4x4_16x16,i16,4,256);
        PROFILE_mtx_op(mtx_sub2x2_32x32,i32,2,256);
        PROFILE_mtx_op(mtx_sub3x3_32x32,i32,3,256);
        PROFILE_mtx_op(mtx_sub4x4_32x32,i32,4,256);
        PROFILE_mtx_op(mtx_sub2x2f     ,f32,2,256);
        PROFILE_mtx_op(mtx_sub3x3f     ,f32,3,256);
        PROFILE_mtx_op(mtx_sub4x4f     ,f32,4,256);

        PROFILE_mtx_op(cmtx_sub2x2_16x16,ci16,2,256);
        PROFILE_mtx_op(cmtx_sub3x3_16x16,ci16,3,256);
        PROFILE_mtx_op(cmtx_sub4x4_16x16,ci16,4,256);
        PROFILE_mtx_op(cmtx_sub2x2_32x32,ci32,2,256);
        PROFILE_mtx_op(cmtx_sub3x3_32x32,ci32,3,256);
        PROFILE_mtx_op(cmtx_sub4x4_32x32,ci32,4,256);
        PROFILE_mtx_op(cmtx_sub2x2f     ,cf32,2,256);
        PROFILE_mtx_op(cmtx_sub3x3f     ,cf32,3,256);
        PROFILE_mtx_op(cmtx_sub4x4f     ,cf32,4,256);

        PROFILE_mtx_op1(mtx_mul2x2_16x16,i16,0,2,256);
        PROFILE_mtx_op1(mtx_mul3x3_16x16,i16,0,3,256);
        PROFILE_mtx_op1(mtx_mul4x4_16x16,i16,0,4,256);
        PROFILE_mtx_op1(mtx_mul2x2_32x32,i32,0,2,256);
        PROFILE_mtx_op1(mtx_mul3x3_32x32,i32,0,3,256);
        PROFILE_mtx_op1(mtx_mul4x4_32x32,i32,0,4,256);
        PROFILE_mtx_op(mtx_mul2x2f     ,f32,2,256);
        PROFILE_mtx_op(mtx_mul3x3f     ,f32,3,256);
        PROFILE_mtx_op(mtx_mul4x4f     ,f32,4,256);

        PROFILE_mtx_op1(cmtx_mul2x2_16x16,ci16,0,2,256);
        PROFILE_mtx_op1(cmtx_mul3x3_16x16,ci16,0,3,256);
        PROFILE_mtx_op1(cmtx_mul4x4_16x16,ci16,0,4,256);
        PROFILE_mtx_op1(cmtx_mul2x2_32x32,ci32,0,2,256);
        PROFILE_mtx_op1(cmtx_mul3x3_32x32,ci32,0,3,256);
        PROFILE_mtx_op1(cmtx_mul4x4_32x32,ci32,0,4,256);
        PROFILE_mtx_op(cmtx_mul2x2f     ,cf32,2,256);
        PROFILE_mtx_op(cmtx_mul3x3f     ,cf32,3,256);
        PROFILE_mtx_op(cmtx_mul4x4f     ,cf32,4,256);

        PROFILE_mtx_op2(mtx_tran2x2_16x16,i16,2,256);
        PROFILE_mtx_op2(mtx_tran3x3_16x16,i16,3,256);
        PROFILE_mtx_op2(mtx_tran4x4_16x16,i16,4,256);
        PROFILE_mtx_op2(mtx_tran2x2_32x32,i32,2,256);
        PROFILE_mtx_op2(mtx_tran3x3_32x32,i32,3,256);
        PROFILE_mtx_op2(mtx_tran4x4_32x32,i32,4,256);
        PROFILE_mtx_op2(mtx_tran2x2f     ,f32,2,256);
        PROFILE_mtx_op2(mtx_tran3x3f     ,f32,3,256);
        PROFILE_mtx_op2(mtx_tran4x4f     ,f32,4,256);

        PROFILE_mtx_op2(cmtx_tran2x2_16x16,ci16,2,256);
        PROFILE_mtx_op2(cmtx_tran3x3_16x16,ci16,3,256);
        PROFILE_mtx_op2(cmtx_tran4x4_16x16,ci16,4,256);
        PROFILE_mtx_op2(cmtx_tran2x2_32x32,ci32,2,256);
        PROFILE_mtx_op2(cmtx_tran3x3_32x32,ci32,3,256);
        PROFILE_mtx_op2(cmtx_tran4x4_32x32,ci32,4,256);
        PROFILE_mtx_op2(cmtx_tran2x2f     ,cf32,2,256);
        PROFILE_mtx_op2(cmtx_tran3x3f     ,cf32,3,256);
        PROFILE_mtx_op2(cmtx_tran4x4f     ,cf32,4,256);

        PROFILE_mtx_op3(mtx_det2x2_16x16,i16,0,2,256);
        PROFILE_mtx_op3(mtx_det3x3_16x16,i16,0,3,256);
        PROFILE_mtx_op3(mtx_det4x4_16x16,i16,0,4,256);
        PROFILE_mtx_op3(mtx_det2x2_32x32,i32,0,2,256);
        PROFILE_mtx_op3(mtx_det3x3_32x32,i32,0,3,256);
        PROFILE_mtx_op3(mtx_det4x4_32x32,i32,0,4,256);
        PROFILE_mtx_op2(mtx_det2x2f     ,f32,2,256);
        PROFILE_mtx_op2(mtx_det3x3f     ,f32,3,256);
        PROFILE_mtx_op2(mtx_det4x4f     ,f32,4,256);

        PROFILE_mtx_op3(cmtx_det2x2_16x16,ci16,0,2,256);
        PROFILE_mtx_op3(cmtx_det3x3_16x16,ci16,0,3,256);
        PROFILE_mtx_op3(cmtx_det4x4_16x16,ci16,0,4,256);
        PROFILE_mtx_op3(cmtx_det2x2_32x32,ci32,0,2,256);
        PROFILE_mtx_op3(cmtx_det3x3_32x32,ci32,0,3,256);
        PROFILE_mtx_op3(cmtx_det4x4_32x32,ci32,0,4,256);
        PROFILE_mtx_op2(cmtx_det2x2f     ,cf32,2,256);
        PROFILE_mtx_op2(cmtx_det3x3f     ,cf32,3,256);
        PROFILE_mtx_op2(cmtx_det4x4f     ,cf32,4,256);

        PROFILE_q2rot(q2rot_16x16,i16,256);
        PROFILE_q2rot(q2rot_32x32,i32,256);
        PROFILE_q2rot(q2rotf,f32,256);

    }
}
