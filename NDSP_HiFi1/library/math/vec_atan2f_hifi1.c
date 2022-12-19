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

#if !(HAVE_VFPU)
DISCARD_FUN(void, vec_atan2f, (float32_t * z, const float32_t * y, const float32_t * x, int N))

#else
/* Tables */
#include "pif_tbl.h"
#include "atanf_tbl.h"
/* +/-Infinity, single precision */
#include "inff_tbl.h"
/* sNaN/qNaN, single precision. */
#include "nanf_tbl.h"

#define sz_f32    (int)sizeof(float32_t)
/*===========================================================================
  Scalar matematics:
  scl_atan2          full quadrant Arctangent        
===========================================================================*/
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

#include <float.h>
void vec_atan2f( float32_t * z, const float32_t * y, const float32_t * x,  int N )
{
  /*
    const union ufloat32uint32* p;
    int sx,sy,big;
    sx=takesignf(x);
    sy=takesignf(y);
    x=fabs(x);
    y=fabs(y);
    if(x==0.f && y==0.f)
    {
      // The actual result depends on input signs.
      x = 1.f;
      y = 0.f;
    }

    big=x>y;
    if(big)
    {
        x=y/x;
    }
    else
    {
      // compare x==y is necessary to support (+/-Inf, +/-Inf) cases 
      x = (x == y) ? 1.0f : x / y;
    }
    p = (x<0.5f) ? atanftbl1 : atanftbl2;
    // approximate atan(x)/x-1 
    y = p[0].f;
    y = x*y + p[1].f;
    y = x*y + p[2].f;
    y = x*y + p[3].f;
    y = x*y + p[4].f;
    y = x*y + p[5].f;
    y = x*y + p[6].f;
    y = x*y + p[7].f;
    // convert result to true atan(x) 
    y = x*y + x;

    if (!big) y = pi2f.f - y;
    if (sx)   y = pif.f - y;
    if (sy)   y = -y;
    return   y;
  */

  const xtfloatx2 *          X;
  const xtfloatx2 *          Y;
        xtfloatx2 * restrict Z;
  const xtfloatx2 *          S_rd;
        xtfloatx2 * restrict S_wr;
  const xtfloat   *          T;
  const xtfloat   *          T1;
  const xtfloat   *          T2;
  ae_valign X_va, Y_va, Z_va;

  /* Current block index; overall number of blocks; number of values in the current block */
  int blkIx, blkNum, blkLen;
  /* Block size, blkLen <= blkSize */
  const int blkSize = MAX_ALLOCA_SZ/sz_f32;
  /* Allocate a fixed-size scratch area on the stack. */
  float32_t ALIGN(8) scr[blkSize];

  int n;

  if ( N<=0 ) return;

  NASSERT_ALIGN8( scr );

  /*
   * Data are processed in blocks of scratch area size. Further, the algorithm
   * implementation is splitted in order to feed the optimizing compiler with a
   * few loops of managable size.
   */

  blkNum = ( N + blkSize-1 )/blkSize;

  for ( blkIx=0; blkIx<blkNum; blkIx++ )
  {
    blkLen = XT_MIN( N - blkIx*blkSize, blkSize );

    /*
     * Part I, reduction to [0,pi/4]. Reference C code:
     *
     *   {
     *     float32_t x0, y0, p0;
     *   
     *     for ( n=0; n<blkLen; n++ )
     *     {
     *       y0 = fabsf( y[blkIx*blkSize+n] );
     *       x0 = fabsf( x[blkIx*blkSize+n] );
     *   
     *       // The actual result depends on input signs.
     *       if ( x0==0.f && y0==0.f ) { x0 = 1.f; y0 = 0.f; };
     *   
     *       if ( x0>y0 ) p0 = y0/x0;
     *       // Special case of x==y is necessary to support (+/-Inf, +/-Inf) cases.
     *       else p0 = ( x0==y0 ? 1.f : x0/y0 );
     *   
     *       scr[n] = p0;
     *     }
     *   }
     */

    {
      /* Input values */
      xtfloatx2 x0, y0;
      /* Numerator; denominator; reciprocal; quotient */
      xtfloatx2 num, den, rcp, quo;
      /* Auxiliary vars */
      xtfloatx2 s, eps;
      /* Is NaN; Inf/Inf; x/Inf; 0/0; x and y are subnormal */
      xtbool2 b_nan, b_num_inf, b_den_inf, b_eqz, b_subn;

      X    = (xtfloatx2*)( (uintptr_t)x + blkIx*blkSize*sz_f32 );
      Y    = (xtfloatx2*)( (uintptr_t)y + blkIx*blkSize*sz_f32 );
      S_wr = (xtfloatx2*)scr;

      X_va = XT_LASX2PP( X );
      Y_va = XT_LASX2PP( Y );

      __Pragma( "loop_count min=1" );
      for ( n=0; n<(blkLen+1)/2; n++ )
      {
        XT_LASX2IP( x0, X_va, X );
        XT_LASX2IP( y0, Y_va, Y );

        /* Reproduce NaN in both x and y to ensure NaN propagation. */
        b_nan = XT_UN_SX2( x0, y0 );
        XT_MOVT_SX2( x0, qNaNf.f, b_nan );
        XT_MOVT_SX2( y0, qNaNf.f, b_nan );

        x0 = XT_ABS_SX2( x0 );
        y0 = XT_ABS_SX2( y0 );

        /* num <= den */
        num = XT_MIN_SX2( x0, y0 );
        den = XT_MAX_SX2( y0, x0 );

        /* Check for a special case. */
        b_num_inf = XT_OEQ_SX2( num, plusInff.f ); /* Inf/Inf */

        /* Scale up numerator and denominator if the former is subnormal. This
         * avoids two potential issues:
         *   - overflow of 1/den
         *   - loss of precision in num/den when num is subnormal. */
        b_subn = XT_OLT_SX2(num, FLT_MIN);
        s = XT_MUL_SX2( num, 8388608.f ); XT_MOVT_SX2( num, s, b_subn );
        s = XT_MUL_SX2( den, 8388608.f ); XT_MOVT_SX2( den, s, b_subn );

        /* Check for a special case. */
        b_den_inf = XT_OEQ_SX2( den, plusInff.f );           /* x/Inf */
        b_eqz = XT_OEQ_SX2( den, (xtfloatx2)XT_CONST_S(0) ); /* 0/0   */

        /* Initial appromimation for 1/den. */
        rcp = XT_RECIP0_SX2( den );
        /* Newton-Raphson iteration for 1/den. */
        eps = (xtfloatx2)1.0f; /* Here the literal constant provides a better loop schedule. */
        XT_MSUB_SX2( eps, rcp, den );
        XT_MADD_SX2( rcp, rcp, eps );
        /* Approximation for the quotient num/den. */
        quo = XT_MUL_SX2( num, rcp );
        /* Refine the quotient by a modified Newton-Raphson iteration. */
        eps = num;
        XT_MSUB_SX2( eps, quo, den );
        XT_MADD_SX2( quo, rcp, eps );

        XT_MOVT_SX2( quo, (xtfloatx2)XT_CONST_S(0), b_eqz );     /*     0/0 -> 0 */
        XT_MOVT_SX2( quo, (xtfloatx2)XT_CONST_S(0), b_den_inf ); /*   x/Inf -> 0 */
        XT_MOVT_SX2( quo, (xtfloatx2)XT_CONST_S(1), b_num_inf ); /* Inf/Inf -> 1 */

        XT_SSX2IP( quo, S_wr, +2*sz_f32 );
      }
    }

    __Pragma( "no_reorder" );

    /*
     * Part II, polynomial approximation and full quadrant restoration.
     * Reference C code:
     *
     *   {
     *     const union ufloat32uint32 * ptbl;
     *     float32_t x0, y0, z0, p0;
     *     int sx, sy;
     *   
     *     for ( n=0; n<blkLen; n++ )
     *     {
     *       y0 = y[blkIx*blkSize+n];
     *       x0 = x[blkIx*blkSize+n];
     *       p0 = scr[n];
     *   
     *       sy = takesignf( y0 ); y0 = fabsf( y0 );
     *       sx = takesignf( x0 ); x0 = fabsf( x0 );
     *   
     *       ptbl = ( p0<0.5f ? atanftbl1 : atanftbl2 );
     *    
     *       // Approximate atan(p)/p-1
     *       z0 = ptbl[0].f;
     *       z0 = ptbl[1].f + p0*z0;
     *       z0 = ptbl[2].f + p0*z0;
     *       z0 = ptbl[3].f + p0*z0;
     *       z0 = ptbl[4].f + p0*z0;
     *       z0 = ptbl[5].f + p0*z0;
     *       z0 = ptbl[6].f + p0*z0;
     *       z0 = ptbl[7].f + p0*z0;
     *       z0 =        p0 + p0*z0;
     *   
     *       if ( x0<y0 ) z0 = pi2f.f - z0;
     *       if ( sx    ) z0 = pif.f - z0;
     *       if ( sy    ) z0 = -z0;
     *   
     *       z[blkIx*blkSize+n] = z0;
     *     }
     *   }
     */

    {
      /* Input values; output value; reducted input value */
      xtfloatx2 x0, y0, z0, z1, p0;
      /* Temporary; input values' sign */
      ae_int32x2 t, sx, sy;
      /* Polynomial coeffs for 0.f<=p<0.5f (#1) and 0.5f<=p<=1.f (#2). */
      xtfloatx2 cf1_0, cf1_1, cf1_2, cf1_3, cf1_4, cf1_5, cf1_6, cf1_7;
      xtfloatx2 cf2_0, cf2_1, cf2_2, cf2_3, cf2_4, cf2_5, cf2_6, cf2_7;
      /* Selected polynomial coeffs. */
      xtfloatx2 cf0, cf1, cf2, cf3, cf4, cf5, cf6, cf7;
      /* x less than y; x is negative; num/den is less than 0.5f. */
      xtbool2 b_xlty, b_sx, b_lt05;
      xtfloat tmp;
      static const union ufloat32uint32 c[2] = {
        { 0x3fc90fdb }, { 0x40490fdb } /* pi2f, pif*/
      };
      X = (xtfloatx2*)( (uintptr_t)x + blkIx*blkSize*sz_f32 );
      Y = (xtfloatx2*)( (uintptr_t)y + blkIx*blkSize*sz_f32 );
      Z = (xtfloatx2*)( (uintptr_t)z + blkIx*blkSize*sz_f32 );

      S_rd = (xtfloatx2*)scr;

      X_va = XT_LASX2PP( X );
      Y_va = XT_LASX2PP( Y );
      Z_va = AE_ZALIGN64();
      T = (xtfloat  *)c;
      T1 = (xtfloat  *)atanftbl1;
      T2 = (xtfloat  *)atanftbl2;

      for ( n=0; n<blkLen/2; n++ )
      {
        XT_LASX2IP( x0, X_va, X );
        XT_LASX2IP( y0, Y_va, Y );

        /* Keep sign of x as a boolean. */
        sx = XT_AE_MOVINT32X2_FROMXTFLOATX2( x0 );
        b_sx = AE_LT32( sx, AE_ZERO32() );

        /* Keep y sign as a binary value. */
        sy = XT_AE_MOVINT32X2_FROMXTFLOATX2( y0 );
        sy = AE_AND32( sy, 0x80000000 );

        x0 = XT_ABS_SX2( x0 );
        y0 = XT_ABS_SX2( y0 );
        b_xlty = XT_OLT_SX2( x0, y0 );

        XT_LSX2IP( p0, S_rd, +2*sz_f32 );
        b_lt05 = XT_OLT_SX2( p0, (xtfloatx2)0.5f );


        /* Reload coeff sets on each iteration. */
        tmp = XT_LSI(T1, +0 * sz_f32); cf1_0 = tmp;
        tmp = XT_LSI(T1, +1 * sz_f32); cf1_1 = tmp;
        tmp = XT_LSI(T1, +2 * sz_f32); cf1_2 = tmp;
        tmp = XT_LSI(T1, +3 * sz_f32); cf1_3 = tmp;
        tmp = XT_LSI(T1, +4 * sz_f32); cf1_4 = tmp;
        tmp = XT_LSI(T1, +5 * sz_f32); cf1_5 = tmp;
        tmp = XT_LSI(T1, +6 * sz_f32); cf1_6 = tmp;
        tmp = XT_LSI(T1, +7 * sz_f32); cf1_7 = tmp;

        tmp = XT_LSI(T2, +0 * sz_f32); cf2_0 = tmp;
        tmp = XT_LSI(T2, +1 * sz_f32); cf2_1 = tmp;
        tmp = XT_LSI(T2, +2 * sz_f32); cf2_2 = tmp;
        tmp = XT_LSI(T2, +3 * sz_f32); cf2_3 = tmp;
        tmp = XT_LSI(T2, +4 * sz_f32); cf2_4 = tmp;
        tmp = XT_LSI(T2, +5 * sz_f32); cf2_5 = tmp;
        tmp = XT_LSI(T2, +6 * sz_f32); cf2_6 = tmp;
        tmp = XT_LSI(T2, +7 * sz_f32); cf2_7 = tmp;

        /* Select coeffs from sets #1, #2 by reducted input value. */
        cf0 = cf1_0; XT_MOVF_SX2( cf0, cf2_0, b_lt05 );
        cf1 = cf1_1; XT_MOVF_SX2( cf1, cf2_1, b_lt05 );
        cf2 = cf1_2; XT_MOVF_SX2( cf2, cf2_2, b_lt05 );
        cf3 = cf1_3; XT_MOVF_SX2( cf3, cf2_3, b_lt05 );
        cf4 = cf1_4; XT_MOVF_SX2( cf4, cf2_4, b_lt05 );
        cf5 = cf1_5; XT_MOVF_SX2( cf5, cf2_5, b_lt05 );
        cf6 = cf1_6; XT_MOVF_SX2( cf6, cf2_6, b_lt05 );
        cf7 = cf1_7; XT_MOVF_SX2( cf7, cf2_7, b_lt05 );

  /*
  * Approximate p(x) = atan(x)/x-1.
  */
  {
      /* The polynomial is evaluated by Horner's method */
                                  y0 = cf0;
      XT_MADD_SX2( cf1, p0, y0 ); y0 = cf1;
      XT_MADD_SX2( cf2, p0, y0 ); y0 = cf2;
      XT_MADD_SX2( cf3, p0, y0 ); y0 = cf3;
      XT_MADD_SX2( cf4, p0, y0 ); y0 = cf4;
      XT_MADD_SX2( cf5, p0, y0 ); y0 = cf5;
      XT_MADD_SX2( cf6, p0, y0 ); y0 = cf6;
      XT_MADD_SX2( cf7, p0, y0 ); y0 = cf7;
      /* atan(x) = p*x+x */
      XT_MADD_SX2(p0, y0, p0); z0 = p0;
  }
        /* if ( x0<y0 ) z0 = pi2f.f - z0; */
        tmp = XT_LSI(T, +0 * sz_f32); p0 = tmp;
        z1 = XT_SUB_SX2(p0, z0); XT_MOVT_SX2(z0, z1, b_xlty);
        /* if ( sx ) z0 = pif.f - z0; */
        tmp = XT_LSI(T, +1 * sz_f32); p0 = tmp;
        z1 = XT_SUB_SX2(p0, z0); XT_MOVT_SX2(z0, z1, b_sx);
        /* if ( sy ) z0 = -z0;*/
        t = XT_AE_MOVINT32X2_FROMXTFLOATX2( z0 );
        t = AE_XOR32( t, sy );
        z0 = XT_AE_MOVXTFLOATX2_FROMINT32X2( t );

        XT_SASX2IP( z0, Z_va, Z );
      }

      XT_SASX2POSFP( Z_va, Z );

      /* Deliberately process the last input value if it's even-numbered. */
      if ( blkLen & 1 )
      {
        x0 = XT_LSI( (xtfloat*)X, 0 );
        y0 = XT_LSI( (xtfloat*)Y, 0 );

        /* Keep sign of x as a boolean. */
        sx = XT_AE_MOVINT32X2_FROMXTFLOATX2( x0 );
        b_sx = AE_LT32( sx, AE_ZERO32() );

        /* Keep y sign as a binary value. */
        sy = XT_AE_MOVINT32X2_FROMXTFLOATX2( y0 );
        sy = AE_AND32( sy, 0x80000000 );

        x0 = XT_ABS_SX2( x0 );
        y0 = XT_ABS_SX2( y0 );
        b_xlty = XT_OLT_SX2( x0, y0 );

        p0 = XT_LSI( (xtfloat*)S_rd, 0 );

        b_lt05 = XT_OLT_SX2( p0, (xtfloatx2)0.5f );

        tmp = XT_LSI(T2, +0 * sz_f32); cf2_0 = tmp;
        tmp = XT_LSI(T2, +1 * sz_f32); cf2_1 = tmp;
        tmp = XT_LSI(T2, +2 * sz_f32); cf2_2 = tmp;
        tmp = XT_LSI(T2, +3 * sz_f32); cf2_3 = tmp;
        tmp = XT_LSI(T2, +4 * sz_f32); cf2_4 = tmp;
        tmp = XT_LSI(T2, +5 * sz_f32); cf2_5 = tmp;
        tmp = XT_LSI(T2, +6 * sz_f32); cf2_6 = tmp;
        tmp = XT_LSI(T2, +7 * sz_f32); cf2_7 = tmp;

        tmp = XT_LSI(T1, +0 * sz_f32); cf1_0 = tmp;
        tmp = XT_LSI(T1, +1 * sz_f32); cf1_1 = tmp;
        tmp = XT_LSI(T1, +2 * sz_f32); cf1_2 = tmp;
        tmp = XT_LSI(T1, +3 * sz_f32); cf1_3 = tmp;
        tmp = XT_LSI(T1, +4 * sz_f32); cf1_4 = tmp;
        tmp = XT_LSI(T1, +5 * sz_f32); cf1_5 = tmp;
        tmp = XT_LSI(T1, +6 * sz_f32); cf1_6 = tmp;
        tmp = XT_LSI(T1, +7 * sz_f32); cf1_7 = tmp;

        /* Select coeffs from sets #1, #2 by reducted input value. */
        cf0 = cf1_0; XT_MOVF_SX2( cf0, cf2_0, b_lt05 );
        cf1 = cf1_1; XT_MOVF_SX2( cf1, cf2_1, b_lt05 );
        cf2 = cf1_2; XT_MOVF_SX2( cf2, cf2_2, b_lt05 );
        cf3 = cf1_3; XT_MOVF_SX2( cf3, cf2_3, b_lt05 );
        cf4 = cf1_4; XT_MOVF_SX2( cf4, cf2_4, b_lt05 );
        cf5 = cf1_5; XT_MOVF_SX2( cf5, cf2_5, b_lt05 );
        cf6 = cf1_6; XT_MOVF_SX2( cf6, cf2_6, b_lt05 );
        cf7 = cf1_7; XT_MOVF_SX2( cf7, cf2_7, b_lt05 );

        /*
        * Approximate p(x) = atan(x)/x-1.
        */
        {
            /* The polynomial is evaluated by Horner's method */
                                        y0 = cf0;
            XT_MADD_SX2( cf1, p0, y0 ); y0 = cf1;
            XT_MADD_SX2( cf2, p0, y0 ); y0 = cf2;
            XT_MADD_SX2( cf3, p0, y0 ); y0 = cf3;
            XT_MADD_SX2( cf4, p0, y0 ); y0 = cf4;
            XT_MADD_SX2( cf5, p0, y0 ); y0 = cf5;
            XT_MADD_SX2( cf6, p0, y0 ); y0 = cf6;
            XT_MADD_SX2( cf7, p0, y0 ); y0 = cf7;
            /* atan(x) = p*x+x */
            XT_MADD_SX2(p0, y0, p0); z0 = p0;
        }

        /* if ( x0<y0 ) z0 = pi2f.f - z0; */
        z1 = XT_SUB_SX2( pi2f.f, z0 ); XT_MOVT_SX2( z0, z1, b_xlty );
        /* if ( sx ) z0 = pif.f - z0; */
        z1 = XT_SUB_SX2( pif.f, z0 ); XT_MOVT_SX2( z0, z1, b_sx );
        /* if ( sy ) z0 = -z0; */
        t = XT_AE_MOVINT32X2_FROMXTFLOATX2( z0 );
        t = AE_XOR32( t, sy );
        z0 = XT_AE_MOVXTFLOATX2_FROMINT32X2( t );

        XT_SSI( z0, (xtfloat*)Z, 0 );
      }
    }
     
  } /* for ( blkIx=0; blkIx<blkNum; blkIx++ ) */

} /* vec_atan2f() */

#endif /* #if HAVE_VFPU */

