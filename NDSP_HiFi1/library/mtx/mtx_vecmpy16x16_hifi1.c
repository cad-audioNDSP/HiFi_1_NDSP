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
#include "NatureDSP_Signal.h"
#include "NatureDSP_types.h"
#include "common.h"

/*===========================================================================
  Matrix Operations:
  mtx_mpy              Matrix Multiply
  mtx_vecmpy           Matrix by Vector Multiply
===========================================================================*/
/*-------------------------------------------------------------------------
  Matrix by Vector Multiply
  These functions compute the expression z = 2^lsh * x * y for the matrices 
  x and vector y. 

  Two versions of functions available: regular version (mtx_vecmpy32x32, 
  mtx_vecmpy24x24, mtx_vecmpy16x16,mtx_vecmpyf) with arbitrary arguments 
  and faster version (mtx_vecmpy32x32_fast, mtx_vecmpy24x24_fast, 
  mtx_vecmpy16x16_fast, mtx_vecmpyf_fast) that apply some restrictions.

  Precision: 
  32x32 32-bit inputs, 32-bit output
  24x24 24-bit inputs, 24-bit output
  16x16 16-bit inputs, 16-bit output
  f     floating point

  Input:
  x[M*N] input matrix,Q31,Q15 or floating point
  y[N]   input vector,Q31,Q15 or floating point
  M      number of rows in matrix x
  N      number of columns in matrix x
  lsh    additional left shift
  Output:
  z[M]   output vector,Q31,Q15 or floating point

  Restriction:
  For regular routines (mtx_vecmpy24x24, mtx_vecmpy16x16, mtx_vecmpyf)
  x,y,z should not overlap

  For faster routines (mtx_vecmpy24x24_fast, mtx_vecmpy16x16_fast,
  mtx_vecmpyf_fast)
  x,y,z should not overlap
  x,y   aligned on 8-byte boundary
  N and M are multiples of 4
-------------------------------------------------------------------------*/
void mtx_vecmpy16x16 (  int16_t* restrict z,
               const int16_t* restrict x,
               const int16_t* restrict y,
               int M, int N, int lsh)
{
    /* code with Quad MAC option */
    int m,n;
    xtbool4 mask;
    const ae_int16x4      *  restrict px0;
    const ae_int16x4      *  restrict px1;
    const ae_int16x4      *  restrict py;
          ae_int16        *  restrict pz = (      ae_int16       *)z;
    ae_int32x2 a0;
    ae_int16x4 t,x0,x1,y0;
    ae_int64 B0,B1;
    ae_valign ax0,ax1,ay;

    if (N<=0 || M<=0)    /* exceptional situation */
    {
        for (m=0; m<M; m++) z[m]=0;
        return ;
    }
    /* mask for last 1...3 values from y */
    mask=(1<<(4-(N&3)))-1;
    /* process by pair of rows */
    for(m=0; m<(M&~1); m+=2)
    {
        B0=B1=AE_ZERO64();
        px0=(const ae_int16x4 *)x;
        px1=(const ae_int16x4 *)(x+N);
        py =(const ae_int16x4 *)y;
        ax0=AE_LA64_PP(px0);
        ax1=AE_LA64_PP(px1);
        ay =AE_LA64_PP(py);
        for (n=0; n<(N&~3); n+=4)
        {
            AE_LA16X4_IP(x0,ax0,px0);
            AE_LA16X4_IP(x1,ax1,px1);
            AE_LA16X4_IP(y0,ay ,py );
            AE_MULAAAAQ16(B0,x0,y0);
            AE_MULAAAAQ16(B1,x1,y0);
        }
        if(N&3)
        {
            AE_LA16X4_IP(x0,ax0,px0);
            AE_LA16X4_IP(x1,ax1,px1);
            AE_LA16X4_IP(y0,ay ,py );
            AE_MOVT16X4(y0,AE_ZERO16(),mask);
            AE_MULAAAAQ16(B0,x0,y0);
            AE_MULAAAAQ16(B1,x1,y0);
        }
        a0=AE_TRUNCA32X2F64S(B0,B1,lsh+33);
        t=AE_ROUND16X4F32SASYM(a0,a0);
        AE_S16_0_IP(AE_SHORTSWAP(t),pz,2);
        AE_S16_0_IP(t,pz,2);
        x+=N*2; /* next 2 rows */
    }
    if(M&1)
    {
        B0=AE_ZERO64();
        px0=(const ae_int16x4 *)x;
        py =(const ae_int16x4 *)y;
        ax0=AE_LA64_PP(px0);
        ay =AE_LA64_PP(py);
        for (n=0; n<(N&~3); n+=4)
        {
            AE_LA16X4_IP(x0,ax0,px0);
            AE_LA16X4_IP(y0,ay ,py );
            AE_MULAAAAQ16(B0,x0,y0);
        }
        if(N&3)
        {
            AE_LA16X4_IP(x0,ax0,px0);
            AE_LA16X4_IP(y0,ay ,py );
            AE_MOVT16X4(y0,AE_ZERO16(),mask);
            AE_MULAAAAQ16(B0,x0,y0);
        }
        a0=AE_TRUNCA32X2F64S(B0,B0,lsh+33);
        t=AE_ROUND16X4F32SASYM(a0,a0);
        AE_S16_0_IP(t,pz,2);
    }
} /* mtx_vecmpy16x16() */
