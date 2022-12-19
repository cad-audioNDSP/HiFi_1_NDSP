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

#include "common.h"
#include "NatureDSP_types.h"

/*-------------------------------------------------------------------------
  Vector Min/Max
  These routines find maximum/minimum value in a vector.
  Two versions of functions available: regular version (vec_min32x32, 
  vec_max32x32, vec_min24x24, vec_max24x24,vec_max16x16, vec_min16x16, 
  vec_maxf, vec_minf) with arbitrary arguments and faster version 
  (vec_min32x32_fast, vec_max32x32_fast, vec_min24x24_fast, 
  vec_max24x24_fast,vec_min16x16_fast, vec_min16x16_fast) that apply some 
  restrictions
  NOTE: functions return zero if N is less or equal to zero

  Precision: 
  32x32 32-bit data, 32-bit output
  24x24 24-bit data, 24-bit output
  16x16 16-bit data, 16-bit output
  f     single precision floating point
  
  Input:
  x[N]  input data
  N     length of vector
  Function return minimum or maximum value correspondingly

  Restriction:
  For regular routines:
  none
  For faster routines:
  x aligned on 8-byte boundary
  N   - multiple of 4
-------------------------------------------------------------------------*/
int16_t vec_max16x16_fast (const int16_t* restrict x, int N)
{
  int         n;
  const ae_int16x4 * restrict px = (const ae_int16x4 *)x;
  ae_int16x4  vxh, vmh;
    
  vmh = AE_MOVDA16(0x8000);
  NASSERT_ALIGN8(px);
  ASSERT((N & 3) == 0);
  if (N <= 0) return 0;
  for (n=0;n<N;n+=4)
  {
    AE_L16X4_IP(vxh, px, +8);
	vmh = AE_MAX16(vmh, vxh);
  }

  vxh = AE_SEL16_4321(vmh, vmh);
  vmh = AE_MAX16(vmh, vxh);
  vxh = AE_SEL16_5432(vmh, vmh);
  vmh = AE_MAX16(vmh, vxh);

  return (int16_t)AE_MOVAD16_0(vmh);
}
