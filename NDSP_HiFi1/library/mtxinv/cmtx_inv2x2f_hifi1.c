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
 * Complex Matrix Inversion
 * Optimized code for Fusion
 */
  
/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Common helper macros. */
#include "common.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, cmtx_inv2x2f, (complex_float* x))

#else

/*-------------------------------------------------------------------------
  These functions implement in-place matrix inversion by Gauss elimination 
  with full pivoting

  Precision: 
  f   floating point (real and complex)

  Input:
  x[N*N]      input matrix
  Output:
  x[N*N]      result
  N is 2,3 or 4

  Restrictions:
  none
-------------------------------------------------------------------------*/
void cmtx_inv2x2f(complex_float* x)
{
  xtfloatx2 *pX = (xtfloatx2 *) x;
  xtfloatx2 a, b, c, d;

  xtfloatx2 r,detm,detmr,det,det_r,rc,ari,bri,cri,dri,rn;

  a = XT_LSX2I(pX, 0*sizeof(complex_float));
  b = XT_LSX2I(pX, 1*sizeof(complex_float));
  c = XT_LSX2I(pX, 2*sizeof(complex_float));
  d = XT_LSX2I(pX, 3*sizeof(complex_float));
  /* Find the determinant and its reciprocal */
  /* r = det(x) = a*d-b*c */
  r=XT_MULC_S(a,d);
  XT_MSUBC_S(r,b,c);
  
  /* r = 1.0/det(x) */
  detm=XT_MUL_SX2(r,r);
  detmr=XT_SEL32_LH_SX2(detm,detm);
  det=XT_ADD_SX2(detm,detmr);
  det_r=XT_RECIP_SX2(det);
  rc=XT_CONJC_S(r);
  r=XT_MUL_SX2(det_r,rc);

  /* Calculate matrix inversion */
  /* a = a*r */
  ari=XT_MULC_S(a,r);
  //  /* b = b*(-r) */
  rn = XT_NEG_SX2(r);
  bri=XT_MULC_S(b,rn);
  //  /* c = c*(-r) */
  cri=XT_MULC_S(c,rn);
  //  /* d = d*r */
  dri=XT_MULC_S(d,r);

  /* Store computed values */
  XT_SSX2IP(dri,pX,sizeof(xtfloatx2));
  XT_SSX2IP(bri,pX,sizeof(xtfloatx2));
  XT_SSX2IP(cri,pX,sizeof(xtfloatx2));
  XT_SSX2IP(ari,pX,sizeof(xtfloatx2));

}/* cmtx_inv2x2f() */

#endif  /* #if HAVE_VFPU*/
