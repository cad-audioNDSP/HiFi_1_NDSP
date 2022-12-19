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
  Common Exponent 
  These functions determine the number of redundant sign bits for each value 
  (as if it was loaded in a 32-bit register) and returns the minimum number 
  over the whole vector. This may be useful for a FFT implementation to 
  normalize data.  
  NOTES:
  Faster version of functions make the same task but in a different manner - 
  they compute exponent of maximum absolute value in the array. It allows 
  faster computations but not bitexact results - if minimum value in the 
  array will be -2^n , fast function returns max(0,30-n) while non-fast 
  function returns (31-n).
  Floating point function returns 0-floor(log2(max(abs(x)))). Returned 
  result will be always in range [-129...146]. 
  Special cases
  x       | result
  --------+-------
  0       |    0
  +/-Inf  | -129
  NaN     |    0

  If dimension N<=0 functions return 0

  Precision: 
  32 32-bit inputs 
  24 24-bit inputs 
  16 16-bit inputs 
  f  single precision floating point

  Input:
  x[N] input data
  N    length of vector
  Returned value: minimum exponent

  Restriction:
  Regular versions (vec_bexp16, vec_bexp24, vec_bexp32, vec_bexpf):
  none

  Faster versions (vec_bexp16_fast, vec_bexp24_fast, vec_bexp32_fast):
  x,y - aligned on 8-byte boundary
  N   - a multiple of 4
-------------------------------------------------------------------------*/
int vec_bexp16_fast (const int16_t * restrict x, int N)
{
  ae_int16x4         vA0_16, vB0_16;
  int                         k, b;
  const ae_int16x4 * restrict  px   = (const ae_int16x4 *)(x);
  ae_valign a1 = AE_LA64_PP(px);


  vB0_16 = AE_MOVI16X4(0);
  NASSERT_ALIGN8(x);
  ASSERT((N&1)==0);
  if (N<=0) return 0;
  

  for (k=0; k<(N>>2); k++)
  {
    AE_LA16X4_IP(vA0_16,a1,px);
    vA0_16 = AE_ABS16S(vA0_16);
    vB0_16 = AE_MAX16(vB0_16,vA0_16);
  }
  
   vB0_16 = AE_MAX16(vB0_16, AE_SEL16_5432(vB0_16, vB0_16));
   vB0_16 = AE_MAX16(vB0_16, AE_SEL16_4321(vB0_16, vB0_16));

  b = AE_NSA16_0(vB0_16);
  b += 16;
  return b;
}
