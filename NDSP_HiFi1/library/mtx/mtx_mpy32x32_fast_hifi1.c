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
void mtx_mpy32x32_fast(int32_t** restrict z,
                    const int32_t*  restrict x,
                    const int32_t** restrict y,
                    int M, int N, int P, int lsh)
{
  ae_int32x2  *  restrict pz  = (      ae_int32x2       *)z;
  const ae_int32x2      *  restrict px;
  const ae_int32x2      *  restrict px2;
  const ae_int32x2    *  restrict py;

  int n,m;
  int offa, offb, offc, offd;
  ae_int32x2  vpw;
  ae_int64    zz;
  ae_int32x2    vxl, val, vbl, vyl;

  ae_int32x2 * restrict T1;
  ae_int32x2 * restrict T2;
  ae_int32x2 * restrict Y1;
  ae_int32x2 * restrict Y2;

  ae_f64      vah,vbh,vch,vdh;
  ae_int64    vha,vhb,vhc,vhd;
  ae_int32x2  vy1, vy2;
  ae_valign      y_align, z_align;
  NASSERT_ALIGN8(x);
  ASSERT((N&1)==0);
  ASSERT(M==8);
  ASSERT(P==2);
  zz = AE_ZERO64();
  z_align = AE_LA64_PP(pz);
  __Pragma("concurrent")

    for (m=0; m<4; m++)
    {
      vah = (AE_ZERO64());
      vbh = (AE_ZERO64());
      vch = (AE_ZERO64());
      vdh = (AE_ZERO64());
      py  = (const ae_int32x2       *)y;
      y_align = AE_LA64_PP(py);

      px = (const ae_int32x2 *)x + m*N;
      px2 = (ae_int32x2 *)((const ae_int32 *)px + N);
      for (n = 0; n<(N >> 1); n++)
      {
        AE_LA32X2_IP(vpw, y_align, py);
        offc = AE_MOVAD32_H(vpw);
        offd = AE_MOVAD32_L(vpw);
        T1 = (ae_int32x2 *)(offc);
        T2 = (ae_int32x2 *)(offd);
        NASSERT_ALIGN8(T1);
        NASSERT_ALIGN8(T2);
        AE_L32X2_IP(vxl, px, 8);
        AE_L32X2_IP(vyl, px2, 8);
        AE_L32X2_IP(val, T1, 0);
        AE_L32X2_IP(vbl, T2, 0);

        AE_MULAF32R_HH(vah, val, vxl);
        AE_MULAF32R_HL(vah, vbl, vxl);
        AE_MULAF32R_HH(vch, val, vyl);
        AE_MULAF32R_HL(vch, vbl, vyl);
        AE_MULAF32R_LH(vbh, val, vxl);
        AE_MULAF32R_LL(vbh, vbl, vxl);
        AE_MULAF32R_LH(vdh, val, vyl);
        AE_MULAF32R_LL(vdh, vbl, vyl);
      }
      vha = AE_SLAA64(vah, lsh);
      vhb = AE_SLAA64(vbh, lsh);
      vhc = AE_SLAA64(vch, lsh);
      vhd = AE_SLAA64(vdh, lsh);
      vy1 = AE_ROUND32X2F48SASYM(vha, vhb);
      vy2 = AE_ROUND32X2F48SASYM(vhc, vhd);
      AE_LA32X2_IP(vpw, z_align, pz);
      offa = AE_MOVAD32_H(vpw);
      offb = AE_MOVAD32_L(vpw);
      Y1 = (ae_int32x2 *)(offa);
      Y2 = (ae_int32x2 *)(offb);
      NASSERT_ALIGN8(Y1);
      NASSERT_ALIGN8(Y2);
      AE_S32X2_IP(vy1, Y1, 0);
      AE_S32X2_IP(vy2, Y2, 0);
  }
} /* mtx_mpy32x32_fast() */

