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
  These functions compute the expression z = 2^lsh * x * y for the matrices 
  x and y. The columnar dimension of x must match the row dimension of y. 
  The resulting matrix has the same number of rows as x and the same number 
  of columns as y.

  NOTES:
  In the fixed-point routines, rows of matrices z and y may be stored in 
  non consecutive manner. Matrix x will have all the elements in contiguous 
  memory locations.

  Functions require scratch memory for storing intermediate data. This 
  scratch memory area should be aligned on 8 byte boundary and its size 
  is calculated by macros SCRATCH_MTX_MPY32X32(M,N,P), 
  SCRATCH_MTX_MPY24X24(M,N,P), SCRATCH_MTX_MPY16X16(M,N,P)

  Two versions of functions available: regular version (mtx_mpy32x32, 
  mtx_mpy24x24, mtx_mpy16x16, mtx_mpyf) with arbitrary arguments and 
  faster version (mtx_mpy32x32_fast, mtx_mpy24x24_fast, 
  mtx_mpy16x16_fast, mtx_mpyf_fast) that apply some restrictions.

  Precision: 
  32x32 32-bit inputs, 32-bit output
  24x24 24-bit inputs, 24-bit output
  16x16 16-bit inputs, 16-bit output
  f     floating point

  Input:
  x[M*N]      input matrix,Q31, Q15 or floating point
  y[N][P]     input matrix y. For fixed point routines, these are N 
              vectors of size P,Q31 or Q15. For floating point, this 
              is just a matrix of size NxP.
  M           number of rows in matrix x and z
  N           number of columns in matrix x and number of rows in matrix y
  P           number of columns in matrices y and z
  lsh         additional left shift
  Output:
  z[M][P]     output matrix z. For fixed point routines, these are M 
              vectors of size P Q31 or Q15. For floating point, this 
              is single matrix of size MxP
  Scratch:
  pScr        Scratch memory area with size in bytes defined by macros 
              SCRATCH_MTX_MPY32X32, SCRATCH_MTX_MPY24X24, 
              SCRATCH_MTX_MPY16X16


  Restrictions:
  For regular routines (mtx_mpy32x32,mtx_mpy24x24, mtx_mpy16x16, mtx_mpyf):
  x,y,z should not overlap

  For faster routines (mtx_mpy32x32_fast, mtx_mpy24x24_fast, 
  mtx_mpy16x16_fast, mtx_mpyf_fast):
  x,y,z should not overlap
  x - aligned on 8-byte boundary
  all rows which addresses are written to y[] - aligned on 8-byte boundary
  N is a multiple of 4,M=8,P=2  

-------------------------------------------------------------------------*/
void mtx_mpy16x16_fast ( void* pScr, 
                     int16_t** restrict z,
               const int16_t*  restrict x,
               const int16_t** restrict y,
               int M, int N, int P, int lsh )
{
    /* code with Quad MAC option */
    int m,n;
    const ae_int16x4  * restrict py;
    const ae_int32x2  * restrict pz;
    const ae_int16x4  * restrict px;
    const ae_int16    * restrict T0;
    const ae_int16    * restrict T1;
    ae_int16x4 t0,t1;
    ae_valign az;
    int16_t * restrict scr;


    NASSERT(N%4==0);
    NASSERT(M==8);
    NASSERT(P==2);

    if(N<=0)
    {
        for (m=0; m<8; m++) z[m][0]=z[m][1]=0; 
        return;
    }

    scr=(int16_t *)pScr;
    NASSERT_ALIGN8(scr);
    for(n=0; n<N; n++)
    {
        scr[n  ]=y[n][0];
        scr[n+N]=y[n][1];
    }
    __Pragma("no_reorder")
    pz=(const ae_int32x2  *)z;
    az = AE_LA64_PP(pz);
    for (m=0; m<8; m+=2)
    {
        ae_int32x2 a0,a1,addr;
        ae_int64 B0,B1,B2,B3;
        py=(const ae_int16x4  *)scr;
        px=(const ae_int16x4  *)x;

        B0=B1=B2=B3=AE_ZERO64();
        for (n=0; n<N; n+=4)
        {
            ae_int16x4 y0,y1,x0,x1;
            x1=AE_L16X4_X(px,N*sizeof(int16_t));
            y1=AE_L16X4_X(py,N*sizeof(int16_t));
            AE_L16X4_IP(x0,px,sizeof(x0));
            AE_L16X4_IP(y0,py,sizeof(y0));
            AE_MULAAAAQ16(B0,x0,y0);
            AE_MULAAAAQ16(B1,x0,y1);
            AE_MULAAAAQ16(B2,x1,y0);
            AE_MULAAAAQ16(B3,x1,y1);
        }
        a0=AE_TRUNCA32X2F64S(B0,B1,lsh+33);
        a1=AE_TRUNCA32X2F64S(B2,B3,lsh+33);
        t0=AE_ROUND16X4F32SASYM(a0,a0);
        t1=AE_ROUND16X4F32SASYM(a1,a1);
        AE_LA32X2_IP(addr, az, pz); 
        T0 = (ae_int16*) AE_MOVAD32_H(addr);
        T1 = (ae_int16*) AE_MOVAD32_L(addr);
        AE_S16_0_I(AE_SHORTSWAP(t0),castxcc(ae_int16,T0),0);
        AE_S16_0_I(             t0 ,castxcc(ae_int16,T0),2);
        AE_S16_0_I(AE_SHORTSWAP(t1),castxcc(ae_int16,T1),0);
        AE_S16_0_I(             t1 ,castxcc(ae_int16,T1),2);
        x+=N*2;
    }
} /* mtx_mpy16x16_fast() */
