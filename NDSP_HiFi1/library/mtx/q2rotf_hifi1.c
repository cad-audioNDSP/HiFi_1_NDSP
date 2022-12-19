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

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
/* Common helper macros. */
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, q2rotf, (float32_t *r, const float32_t *q, int L))

#else
/*-------------------------------------------------------------------------
  Quaternion to Rotation Matrix Conversion
  These functions convert sequence of unit quaternions to corresponding 
  rotation matrices

  Precision: 
  16x16  16-bit input, 16-bit output
  32x32  32-bit input, 32-bit output
  f      floating point 

  Input:
  q[L][4]    L quaternions
  L          number of matrices
  Output:
  r[L][3*3]  L rotation matrices

  Restrictions:
  q,r  should not overlap
-------------------------------------------------------------------------*/
void q2rotf(float32_t *r, const float32_t *q, int L)
{
		xtfloatx2    *  pr = (xtfloatx2      *)r;
        xtfloat    *  pr_ = (xtfloat      *)(r+8);
  const xtfloatx2    *  restrict pq = (const xtfloatx2      *)(q);
  int l;
  xtfloat B8;
  xtfloatx2 q0, q1;
  xtfloatx2 q1_r,q0_r,q012,B57,B57_T,B04,B04_R,B04_T,B04_TR,B8D,B13,B13_T,B62,B62_T,B_T;

  xtfloatx2 B01,B23,B45,B67;
  xtfloatx2 T1_N1={1,-1};
  ae_valign qa,z_align;
  qa = AE_LA64_PP(pq);
  z_align = AE_ZALIGN64();

  for (l = 0; l<(L); l++)
  {

    pr = (xtfloatx2    *)(r + 9 *l );
    XT_LASX2IP(q0, qa, pq);
    XT_LASX2IP(q1, qa, pq);

    q1_r=XT_SEL32_LH_SX2(q1,q1);
    B04=XT_MUL_SX2(q0,q0);
    B_T=XT_MUL_SX2(q1_r,q1_r);
    B04_T=B04;
	
    B04=XT_SUB_SX2(B04,B_T);
    B04_R=XT_SEL32_LH_SX2(B04,B04);
    XT_MADD_SX2(B04_R,B04,T1_N1);
    B04_T=XT_ADD_SX2(B04_T,B_T);
    B04_TR=XT_SEL32_LH_SX2(B04_T,B04_T);
    B8D=XT_SUB_SX2(B04_T,B04_TR);

    q012=XT_ADD_SX2(q0,q0);
    B13=XT_MUL_SX2(q012,q1_r);
    B62=XT_MUL_SX2(q012,q1);
    B13_T=XT_SEL32_LH_SX2(B13,B13);
    XT_MADD_SX2(B13,B13_T,T1_N1);
    B62_T=XT_SEL32_LH_SX2(B62,B62);
    XT_MADD_SX2(B62,B62_T,T1_N1);

    B57 = XT_MUL_SX2(q1, q1_r); //q[2] * q[3]
    q0_r=XT_SEL32_LH_SX2(q0,q0);
    B57_T = XT_MUL_SX2(q0,q0_r);

    XT_MADD_SX2(B57,B57_T,T1_N1);
    B57=XT_ADD_SX2(B57,B57);

    B01=XT_SEL32_HH_SX2(B04_R,B13);
    B23=XT_SEL32_LL_SX2(B62,B13);
    B45=XT_SEL32_LH_SX2(B04_R,B57);
    B67=XT_SEL32_HL_SX2(B62,B57);
    B8=XT_HIGH_S(B8D);

    XT_SASX2IP (B01,z_align,pr);
    XT_SASX2IP (B23,z_align,pr);
    XT_SASX2IP (B45,z_align,pr);
    XT_SASX2IP (B67,z_align,pr);
  	XT_SSIP(B8, pr_, 4*9);

  	AE_SA64POS_FP(z_align, pr);

  }
} /* q2rotf() */

#endif  /* #if  HAVE_VFPU */ 

