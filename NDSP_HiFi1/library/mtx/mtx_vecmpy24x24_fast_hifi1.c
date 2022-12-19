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

void mtx_vecmpy24x24_fast (  f24* restrict z,
               const f24* restrict x,
               const f24* restrict y,
               int M, int N, int lsh)

{
  const ae_f32x2  *  restrict px = (const ae_f32x2 *)x;
  const ae_f32x2  *  restrict py = (const ae_f32x2 *)y;
        ae_f32    *  restrict pz = (ae_f32 *)z;
  const ae_f32x2  *  restrict px2 = (const ae_f32x2 *)x;
  
  int m,n;
  ae_f32x2 vxl,val,vyl;

#ifdef COMPILER_XTENSA
    #pragma concurrent
#endif
  NASSERT_ALIGN8(px);
  NASSERT_ALIGN8(py);
  ASSERT((M&1)==0);
  ASSERT((N&1)==0);

  for(m=M; m>0; m-=2)
  {
    ae_f64 vah,vbh;
    ae_f32x2  vy1;
    
    vah = AE_ZERO64();
    vbh = AE_ZERO64();
    px = px2;
    px2 = px + (N>>1);
    py = (const ae_f32x2 *)y;
    for(n=(N>>1); n>0; n--)
    {
      AE_L32X2_IP(vxl, px, 8);
      AE_L32X2_IP(vyl, px2, 8);
      AE_L32X2_IP(val, py, 8);
      AE_MULAF32R_LL(vah, vxl, val);
      AE_MULAF32R_HH(vah, vxl, val);
      AE_MULAF32R_LL(vbh, vyl, val);
      AE_MULAF32R_HH(vbh, vyl, val);
    }

    vah = AE_SLAA64(vah, lsh);
    vbh = AE_SLAA64(vbh, lsh);
    vy1 = AE_ROUND24X2F48SASYM(vah, vah);
    vy1 = AE_SLAI32(vy1, 8);
    AE_S32_L_IP(vy1, pz, 4);
    vy1 = AE_ROUND24X2F48SASYM(vbh, vbh);
    vy1 = AE_SLAI32(vy1, 8);
    AE_S32_L_IP(vy1, pz, 4);
  }
} /* mtx_vecmpy24x24_fast() */

