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
#include "polyatan16x16q23_tbl.h"
#include "common.h"

/*-------------------------------------------------------------------------
Floating-Point Full-Quadrant Arc Tangent
The functions compute the full quadrant arc tangent of the ratio y/x. 
Floating point functions output is in radians. Fixed point functions 
scale its output by pi.

NOTE:
1. Scalar function is compatible with standard ANSI C routines and set 
   errno and exception flags accordingly
2. Scalar function assigns EDOM to errno whenever y==0 and x==0.

Special cases:
     y    |   x   |  result   |  extra conditions    
  --------|-------|-----------|---------------------
   +/-0   | -0    | +/-pi     |
   +/-0   | +0    | +/-0      |
   +/-0   |  x    | +/-pi     | x<0
   +/-0   |  x    | +/-0      | x>0
   y      | +/-0  | -pi/2     | y<0
   y      | +/-0  |  pi/2     | y>0
   +/-y   | -inf  | +/-pi     | finite y>0
   +/-y   | +inf  | +/-0      | finite y>0
   +/-inf | x     | +/-pi/2   | finite x
   +/-inf | -inf  | +/-3*pi/4 | 
   +/-inf | +inf  | +/-pi/4   |

Input:
  y[N]  input data, Q15 or floating point
  x[N]  input data, Q15 or floating point
  N     length of vectors
Output:
  z[N]  result, Q15 or floating point
  
Restrictions:
x, y, z should not overlap
---------------------------------------------------------------------------*/
int16_t   scl_atan2_16x16 (int16_t y, int16_t x)
{
    const ae_int32* restrict ptbl;
    ae_int16x4 z0;
    ae_int32x2 X0,Y0,Z0,E0,t,X0_tmp;
    ae_f32x2 f0,X20;
    xtbool2 sx0,sy0,small0;
    int ey0l;
    ptbl =(const ae_int32*)polyatan16x16q23;

    /* load, select quadrant */
    X0=AE_MOVDA32X2(x,x);
    Y0=AE_MOVDA32X2(y,y);
    sx0=AE_LT32(X0,0);
    sy0=AE_LT32(Y0,0);
    X0=AE_ABS32(X0); Y0=AE_ABS32(Y0); 
    small0=AE_LT32(X0,Y0);
    Z0=X0; X0=AE_MIN32(X0,Y0); Y0=AE_MAX32(Z0,Y0);
    ey0l=AE_NSAZ32_L(Y0)-8; 
        
    X0=AE_SLAA32(X0,ey0l);
    Y0=AE_SLAA32(Y0,ey0l);
    /* divide X/Y in Q23 via reciprocal */
    t=AE_L32_I(ptbl,4*sizeof(int32_t));
    Z0=AE_SUB32(t,Y0);
    t=AE_L32_I(ptbl,5*sizeof(int32_t));
    f0=t;
	Y0 = AE_SLAI32(Y0,8);
	X0 = AE_SLAI32(X0,8);
    AE_MULSFP32X2RAS_L(f0,Z0,Y0); E0=f0;
    E0=AE_SLAI32(E0,9);
    f0=Z0; AE_MULAFP32X2RAS_L(f0,Z0,E0); Z0=f0;
    f0=t;
    AE_MULSFP32X2RAS_L(f0,Z0,Y0); E0=f0;
    E0=AE_SLAI32(E0,9);
    f0=Z0; AE_MULAFP32X2RAS_L(f0,Z0,E0); Z0=f0;
    Z0=AE_MULFP32X2RAS_L(Z0,X0);
    X0=AE_SLAI32(Z0,1);
    X0_tmp=AE_SLAI32(Z0,9);

    /* compute atan via polynomial */
    X20=AE_SAT24S(AE_MULFP32X2RAS_L(X0,X0_tmp));
    X20=AE_SLAI32(X20,8);
    Y0=AE_L32_I(ptbl,0*sizeof(int32_t));
    f0=AE_L32_I(ptbl,1*sizeof(int32_t));
    AE_MULAFP32X2RAS_L(f0,Y0,X20); Y0=f0;
    f0=AE_L32_I(ptbl,2*sizeof(int32_t)); 
    AE_MULAFP32X2RAS_L(f0,Y0,X20); Y0=f0;
    f0=AE_L32_I(ptbl,3*sizeof(int32_t)); 
    AE_MULAFP32X2RAS_L(f0,Y0,X20); Y0=f0;
    Y0=AE_MULFP32X2RAS_L(Y0,X0_tmp); 

    /* move to the right octant */
    t=AE_L32_I(ptbl,5*sizeof(int32_t));
    AE_MOVT32X2(Y0,AE_SUB32(t,Y0),small0);
    t=AE_L32_I(ptbl,6*sizeof(int32_t));
    AE_MOVT32X2(Y0,AE_SUB32(t,Y0),sx0);
    AE_MOVT32X2(Y0,AE_NEG32(Y0),sy0);
    Y0=AE_SLAI32S(Y0,8);
    z0=AE_ROUND16X4F32SASYM(Y0,Y0);
    /* save */
    return AE_MOVAD16_0(z0);
}

