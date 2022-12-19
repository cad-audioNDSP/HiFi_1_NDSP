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
   Matrix multiply
 * Optimized code for HiFi1
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
/* Common helper macros. */
#include "common.h"
#if !(HAVE_VFPU)
DISCARD_FUN(void, mtx_mpyf, (float32_t * z, const float32_t * x, const float32_t * y, int M, int N, int P))

#else
/*-------------------------------------------------------------------------
  Matrix Multiply
  These functions compute the expression z = 2^lsh * x * y for the matrices 
  x and y. The columnar dimension of x must match the row dimension of y. 
  The resulting matrix has the same number of rows as x and the same number 
  of columns as y.
  NOTE: lsh factor is not relevant for floating point routines.

  Functions require scratch memory for storing intermediate data. This 
  scratch memory area should be aligned on 8 byte boundary and its size is 
  calculated by macros SCRATCH_MTX_MPY24X24(M,N,P), 
  SCRATCH_MTX_MPY32X32(M,N,P), SCRATCH_MTX_MPY16X16(M,N,P).

  Two versions of functions available: regular version (mtx_mpy32x32,
  mtx_mpy24x24, mtx_mpy16x16, mtx_mpyf) with arbitrary arguments and faster
  version mtx_mpy32x32_fast, mtx_mpy24x24_fast, mtx_mpy16x16_fast, 
  mtx_mpyf_fast) that apply some restrictions.

  Precision:
  32x32 32-bit inputs, 32-bit output 
  24x24 24-bit inputs, 24-bit output
  16x16 16-bit inputs, 16-bit output
  f     floating point

  Input:
  x[M*N]      input matrix x, Q15, Q31 or floating point
  y[N*P]      input matrix y, Q15, Q31 or floating point
  M           number of rows in matrix x and z
  N           number of columns in matrix x and number of rows in matrix y
  P           number of columns in matrices y and z
  lsh         left shift applied to the result (applied to the fixed-
              point functions only) 
  Output:
  z[M*P]      output matrix z, Q15, Q31 or floating point 
  Scratch:
  pScr        size in bytes defined by macros SCRATCH_MTX_MPY32X32,
              SCRATCH_MTX_MPY24X24, SCRATCH_MTX_MPY16X16

  Restrictions:
  For regular routines (mtx_mpy32x32, mtx_mpy24x24, mtx_mpy16x16, mtx_mpyf):
  x,y,z   should not overlap

  For faster routines (mtx_mpy32x32_fast, mtx_mpy24x24_fast, 
                       mtx_mpy16x16_fast, mtx_mpyf_fast):
  x,y,z   should not overlap
  x,y,z   aligned on 8-byte boundary
  M,N,P   multiplies of 4
  lsh     should be in range:
          -31...31 for mtx_mpy32x32, mtx_mpy32x32_fast,
                   mtx_mpy24x24, mtx_mpy24x24_fast
          -15...15 for mtx_mpy16x16, mtx_mpy16x16_fast  

-------------------------------------------------------------------------*/
#define SZ_F32 (sizeof(float32_t))

/* Load scalar and replicate */
#define XT_LSRX2IP(d, a, off)\
{\
  ae_int32x2 t;\
  AE_L32_IP(t, castxcc(const ae_int32,a), off);\
  d = XT_AE_MOVXTFLOATX2_FROMINT32X2(t);\
}

void mtx_mpyf( float32_t * z, const float32_t * x,  const float32_t * y, int M, int N, int P )
{
  const xtfloatx2 *restrict pX0;
  const xtfloatx2 *restrict pX1;
  const xtfloatx2 *restrict pX2;
  const xtfloatx2 *restrict pX3;
  const xtfloatx2 *restrict pY0;
        xtfloatx2 *restrict pZ0;
        xtfloatx2 *restrict pZ1;
        xtfloatx2 *restrict pZ2;
        xtfloatx2 *restrict pZ3;
  xtfloatx2 y0, y1;
  xtfloatx2 x0, x1, x2, x3;
  xtfloatx2 z00, z01, z10, z11, z20, z21, z30, z31;
  xtfloat rx0, rx1, ry0, ry1, rz00, rz01, rz10, rz11;
  ae_valign v_y0, v_z0, v_z1, v_z2, v_z3;
  int m, n, p;
  int _P = P & (~3);
  int _M = M & (~3);

  NASSERT(x);
  NASSERT(y);
  NASSERT(z);
  NASSERT((z != x) && (z != y));

  if (M<=0 || P<=0) return;
  /* If N<=0 then clear output matrix and return */
  if (N<=0)
  {
    int MP = M*P;
    pZ0 = (xtfloatx2 *)z;
    v_z0 = AE_ZALIGN64();
    z00 = (xtfloatx2)0.0f;
    for (n = 0; n < (MP>>1); n++)
    {
      XT_SASX2IP(z00, v_z0, pZ0);
    }
    XT_SASX2POSFP(v_z0, pZ0);
    if (MP&1) XT_SSI(z00, (xtfloat *)pZ0, 0);
    return;
  }

  /* Compute output matrix by 4x4 squares */
  for ( m = 0; m < (M>>2); m++ )
  {
    int m4 = m<<2;

    pZ0 = (xtfloatx2 *)(z+m4*P);
    pZ1 = (xtfloatx2 *)((float32_t *)pZ0+P);
    pZ2 = (xtfloatx2 *)((float32_t *)pZ1+P);
    pZ3 = (xtfloatx2 *)((float32_t *)pZ2+P);

    v_z0 = v_z1 = v_z2 = v_z3 = AE_ZALIGN64();

    for ( p = 0; p < (P>>2); p++ )
    {
      int p4 = p<<2;

      pX0 = (const xtfloatx2 *)(x+m4*N);
      pX1 = (const xtfloatx2 *)((float32_t *)pX0+N);
      pX2 = (const xtfloatx2 *)((float32_t *)pX1+N);
      pX3 = (const xtfloatx2 *)((float32_t *)pX2+N);
      
      pY0  = (const xtfloatx2 *)(y+p4);

      z00 = z01 = z10 = z11 = (xtfloatx2)XT_CONST_S(0);
      z20 = z21 = z30 = z31 = (xtfloatx2)XT_CONST_S(0);

      __Pragma("loop_count min=1")
      for ( n = 0; n < N; n++ )
      {
        XT_LSRX2IP(x0, pX0, SZ_F32);
        XT_LSRX2IP(x1, pX1, SZ_F32);
        XT_LSRX2IP(x2, pX2, SZ_F32);
        XT_LSRX2IP(x3, pX3, SZ_F32);

        v_y0 = XT_LASX2PP(pY0);
        XT_LASX2IP(y0, v_y0, pY0);
        XT_LASX2IP(y1, v_y0, pY0);
        pY0 = (const xtfloatx2 *)XT_ADDX4(P-4, (intptr_t)pY0);

        XT_MADD_SX2(z00, x0, y0);
        XT_MADD_SX2(z01, x0, y1);
        XT_MADD_SX2(z10, x1, y0);
        XT_MADD_SX2(z11, x1, y1);
        XT_MADD_SX2(z20, x2, y0);
        XT_MADD_SX2(z21, x2, y1);
        XT_MADD_SX2(z30, x3, y0);
        XT_MADD_SX2(z31, x3, y1);
      }
      XT_SASX2IP(z00, v_z0, pZ0);
      XT_SASX2IP(z01, v_z0, pZ0);
      XT_SASX2IP(z10, v_z1, pZ1);
      XT_SASX2IP(z11, v_z1, pZ1);
      XT_SASX2IP(z20, v_z2, pZ2);
      XT_SASX2IP(z21, v_z2, pZ2);
      XT_SASX2IP(z30, v_z3, pZ3);
      XT_SASX2IP(z31, v_z3, pZ3);
    }
    XT_SASX2POSFP(v_z0, pZ0);
    XT_SASX2POSFP(v_z1, pZ1);
    XT_SASX2POSFP(v_z2, pZ2);
    XT_SASX2POSFP(v_z3, pZ3);
  }
  /* Compute remaining values: */
  
  /* compute last P%2 columns */
  if (P&2)
  {
    pZ0 = (xtfloatx2 *)(z+_P);
    pZ1 = (xtfloatx2 *)((float32_t *)pZ0+1);

    for (m = 0; m < (M>>1); m++)
    {
      int m2 = m<<1;
    
      pX0 = (const xtfloatx2 *)(x+m2*N);
      pX1 = (const xtfloatx2 *)((float32_t *)pX0+N);
      pY0 = (const xtfloatx2 *)(y+_P);
    
      rz00 = rz01 = rz10 = rz11 = XT_CONST_S(0);
      
      __Pragma("loop_count min=1")
      for ( n = 0; n < N; n++ )
      {
        XT_LSIP(rx0, castxcc(const xtfloat,pX0), SZ_F32);
        XT_LSIP(rx1, castxcc(const xtfloat,pX1), SZ_F32);
        XT_LSIP(ry0, castxcc(const xtfloat,pY0), SZ_F32);
        XT_LSXP(ry1, castxcc(const xtfloat,pY0), SZ_F32*(P-1));
        XT_MADD_S(rz00, rx0, ry0);
        XT_MADD_S(rz01, rx0, ry1);
        XT_MADD_S(rz10, rx1, ry0);
        XT_MADD_S(rz11, rx1, ry1);
      }
      XT_SSXP(rz00, castxcc(xtfloat,pZ0), P*SZ_F32);
      XT_SSXP(rz01, castxcc(xtfloat,pZ1), P*SZ_F32);
      XT_SSXP(rz10, castxcc(xtfloat,pZ0), P*SZ_F32);
      XT_SSXP(rz11, castxcc(xtfloat,pZ1), P*SZ_F32);
    }
  }
  /* compute last odd column */
  if (P&1)
  {
    pZ0 = (xtfloatx2 *)(z+P-1);

    for (m = 0; m < (M>>1); m++)
    {
      int m2 = m<<1;
    
      pX0 = (const xtfloatx2 *)(x+m2*N);
      pX1 = (const xtfloatx2 *)((float32_t *)pX0+N);
      pY0 = (const xtfloatx2 *)(y+P-1);
    
      rz00 = rz01 = XT_CONST_S(0);
      
      __Pragma("loop_count min=1")
      for ( n = 0; n < N; n++ )
      {
        XT_LSIP(rx0, castxcc(const xtfloat,pX0), SZ_F32);
        XT_LSIP(rx1, castxcc(const xtfloat,pX1), SZ_F32);
        XT_LSXP(ry0, castxcc(const xtfloat,pY0), SZ_F32*P);
        XT_MADD_S(rz00, rx0, ry0);
        XT_MADD_S(rz01, rx1, ry0);
      }
      XT_SSXP(rz00, castxcc(xtfloat,pZ0), P*SZ_F32);
      XT_SSXP(rz01, castxcc(xtfloat,pZ0), P*SZ_F32);
    }
  }

  /* compute last M%2 rows */
  if (M&2)
  {
    pZ0 = (xtfloatx2 *)(z + _M*P);
    pZ1 = (xtfloatx2 *)((float32_t *)pZ0 + P);

    for (p = 0; p < (_P>>1); p++)
    {
      int p2 = p<<1;
      pX0 = (const xtfloatx2 *)(x + _M*N);
      pX1 = (const xtfloatx2 *)((float32_t *)pX0 + N);
      pY0 = (const xtfloatx2 *)(y + p2);

      rz00 = rz01 = rz10 = rz11 = XT_CONST_S(0);
      
      __Pragma("loop_count min=1")
      for (n = 0; n < N; n++)
      {
        XT_LSIP(rx0, castxcc(const xtfloat,pX0), SZ_F32);
        XT_LSIP(rx1, castxcc(const xtfloat,pX1), SZ_F32);
        XT_LSIP(ry0, castxcc(const xtfloat,pY0), SZ_F32);
        XT_LSXP(ry1, castxcc(const xtfloat,pY0), SZ_F32*(P-1));
        XT_MADD_S(rz00, rx0, ry0);
        XT_MADD_S(rz01, rx0, ry1);
        XT_MADD_S(rz10, rx1, ry0);
        XT_MADD_S(rz11, rx1, ry1);
      }
      XT_SSIP(rz00, castxcc(xtfloat,pZ0), SZ_F32);
      XT_SSIP(rz01, castxcc(xtfloat,pZ0), SZ_F32);
      XT_SSIP(rz10, castxcc(xtfloat,pZ1), SZ_F32);
      XT_SSIP(rz11, castxcc(xtfloat,pZ1), SZ_F32);
    }
  }
  /* compute last odd row */
  if (M&1)
  {
    pZ0 = (xtfloatx2 *)(z + (M-1)*P);
    for (p = 0; p < P; p++)
    {
      pX0 = (const xtfloatx2 *)(x + (M-1)*N);
      pY0 = (const xtfloatx2 *)(y + p);

      rz00 = XT_CONST_S(0);
      
      __Pragma("loop_count min=1")
      for (n = 0; n < N; n++)
      {
        XT_LSIP(rx0,  castxcc(const xtfloat,pX0), SZ_F32);
        XT_LSXP(ry0,  castxcc(const xtfloat,pY0), SZ_F32*P);
        XT_MADD_S(rz00, rx0, ry0);
      }
      XT_SSIP(rz00, castxcc(xtfloat,pZ0), SZ_F32);
    }
  }
} /* mtx_mpyf() */

#endif  /* #if  HAVE_VFPU */
