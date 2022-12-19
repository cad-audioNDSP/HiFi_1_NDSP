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
#include "common.h"

/*===========================================================================
  Vector matematics:
  vec_add              Vector Sum
===========================================================================*/

/*-------------------------------------------------------------------------
  Vector Sum
  This routine makes pair wise saturated summation of vectors.
  Two versions of routines are available: regular versions (vec_add32x32, 
  vec_add24x24, vec_add16x16, vec_addf) work with arbitrary arguments, 
  faster versions (vec_add32x32_fast, vec_add24x24_fast, vec_add16x16_fast) 
  apply some restrictions.

  Precision: 
  32x32 32-bit inputs, 32-bit output
  24x24 24-bit inputs, 24-bit output
  16x16 16-bit inputs, 16-bit output
  f     single precision floating point

  Input:
  x[N]   input data
  y[N]   input data
  N      length of vectors
  Output:
  z[N]   output data

  Restriction:
  Regular versions (vec_add32x32, vec_add24x24, vec_add16x16, vec_addf):
  x,y,z - should not be overlapped
  Faster versions (vec_add32x32_fast, vec_add24x24_fast, vec_add16x16_fast):
  z,x,y - aligned on 8-byte boundary
  N   - multiple of 4
-------------------------------------------------------------------------*/
void vec_add24x24_fast
(
  f24 *       restrict z,
  const f24 * restrict x,
  const f24 * restrict y,
  int N
)
{
  int n;

  ae_int32x2  vxw, vyw, vzw;
  
  const ae_int32x2 * restrict px = (const ae_int32x2 *)x;
  const ae_int32x2 * restrict py = (const ae_int32x2 *)y;
        ae_int32x2 * restrict pz = (      ae_int32x2 *)z;
  
  NASSERT_ALIGN8(px);
  NASSERT_ALIGN8(py);
  NASSERT_ALIGN8(pz);
  NASSERT((N&1)==0);
  
  ae_int32x2 F24 = AE_MOVDA32(0XFFFFFF00);  

  AE_L32X2_IP(vxw, px, sizeof(*px));
  AE_L32X2_IP(vyw, py, sizeof(*py));
  vxw = AE_AND32(vxw, F24);
  vyw = AE_AND32(vyw, F24);

  for (n=0; n<N; n+=2)
  {
	vzw = AE_ADD32S(vxw, vyw);
    AE_L32X2_IP(vxw, px, sizeof(*px));
    AE_L32X2_IP(vyw, py, sizeof(*py));
    vxw = AE_AND32(vxw, F24);
    vyw = AE_AND32(vyw, F24);
    AE_S32X2_IP(vzw, pz, sizeof(*pz));
  }
}
