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
DISCARD_FUN(  void, vec_atanf, ( float32_t * restrict z, 
              const float32_t * restrict x, int N ))

#else
/* Tables */
#include "pif_tbl.h"
#include "atanf_tbl.h"
/* +/-Infinity, single precision */
#include "inff_tbl.h"

#define sz_f32    (int)sizeof(float32_t)
/*===========================================================================
  Vector matematics:
  vec_tan          Arctangent        
===========================================================================*/
/*-------------------------------------------------------------------------
  Arctangent 
  Functions calculate arctangent of number. Fixed point functions scale 
  output by pi which corresponds to the real phases +pi/4 and represent 
  input and output in Q15 or Q31
  NOTE:
  1.  Scalar floating point function is compatible with standard ANSI C
      routines and sets errno and exception flags accordingly

  Precision: 
  16x16  16-bit inputs, 16-bit output. Accuracy: 2 LSB
  24x24  24-bit inputs, 24-bit output. Accuracy: 74000 (3.4e-5)
  32x32  32-bit inputs, 32-bit output. Accuracy: 42    (2.0e-8)
  f      floating point. Accuracy: 2 ULP


  Input:
  x[N]   input data, Q15, Q31 or floating point
  N      length of vectors
  Output:
  z[N]   result, Q15, Q31 or floating point

  Restriction:
  x,z should not overlap

  Scalar versions:
  ----------------
  return result, Q15, Q31 or floating point
-------------------------------------------------------------------------*/

void vec_atanf (  float32_t * restrict z, 
                  const float32_t * restrict x, 
                  int N )
{
  /*
    float32_t y;
    int sx,big;
    const union ufloat32uint32* p;
     range reduction 
    sx = x<0;
    x = sx ? -x : x;
    big = x>1.0f;
    if (big) x = 1.0f / x;
    p = (x<0.5f) ? atanftbl1 : atanftbl2;
     approximate atan(x)/x-1 
    y = p[0].f;
    y = x*y + p[1].f;
    y = x*y + p[2].f;
    y = x*y + p[3].f;
    y = x*y + p[4].f;
    y = x*y + p[5].f;
    y = x*y + p[6].f;
    y = x*y + p[7].f;
     convert result to true atan(x)  
    y = x*y + x;

    if (big) y = pi2f.f - y;
    y = sx ? -y : y; apply original sign 
    return y;
  */

  const xtfloatx2 *          X;
        xtfloatx2 * restrict Z;
  const xtfloatx2 *          S_rd;
        xtfloatx2 * restrict S_wr;
  const xtfloat   *          T1;
  const xtfloat   *          T2;
  ae_valign X_va, Z_va;

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

  blkNum = (N + blkSize-1)/blkSize;

  for ( blkIx=0; blkIx<blkNum; blkIx++ )
  {
    blkLen = XT_MIN( N - blkIx*blkSize, blkSize );

    /*
     * Part I, range reduction. Reference C code:
     *
     *   {
     *     float32_t x0, y0;
     *   
     *     for ( n=0; n<blkLen; n++ )
     *     {
     *       x0 = fabsf( x[blkIx*blkSize+n] );
     *       y0 = ( x0>1.f ? 1.f/x0 : x0 );
     *       scr[n] = y0;
     *     }
     *   }
     */

    {
      /* Input value; reducted value. */
      xtfloatx2 x0, y0;
      /* Is greater than one; is a +/-infinity  */
      xtbool2 b_gt1, b_inf;

      X    = (xtfloatx2*)( (uintptr_t)x + blkIx*blkSize*sz_f32 );
      S_wr = (xtfloatx2*)scr;

      X_va = XT_LASX2PP( X );

      __Pragma( "loop_count min=1" );
      for ( n=0; n<(blkLen+1)/2; n++ )
      {
        XT_LASX2IP( x0, X_va, X );

        x0 = XT_ABS_SX2( x0 );
        b_inf = XT_OEQ_SX2( plusInff.f, x0 );
        b_gt1 = XT_OLT_SX2( (xtfloatx2)1.0f, x0 );

        /* y <- 1.f/x */
        y0 = XT_RECIP_SX2( x0 );

        /* Fast reciprocal refinement produces NaN for an infinity on input! */
        XT_MOVT_SX2( y0, (xtfloatx2)0.0f, b_inf );
        /* Select reciprocal for x>1.f */
        XT_MOVF_SX2( y0, x0, b_gt1 );

        XT_SSX2IP( y0, S_wr, +2*sz_f32 );
      }
    }

    __Pragma( "no_reorder" );

    /*
     * Part II, polynomial approximation. Reference C code:
     *
     *   {
     *     const union ufloat32uint32 * ptbl;
     *     float32_t x0, y0, z0;
     *   
     *     for ( n=0; n<blkLen; n++ )
     *     {
     *       x0 = x[blkIx*blkSize+n];
     *       y0 = scr[n];
     *       
     *       ptbl = ( y0<0.5f ? atanftbl1 : atanftbl2 );
     *   
     *       // Approximate atan(x)/x-1
     *       z0 = ptbl[0].f;
     *       z0 = ptbl[1].f + y0*z0;
     *       z0 = ptbl[2].f + y0*z0;
     *       z0 = ptbl[3].f + y0*z0;
     *       z0 = ptbl[4].f + y0*z0;
     *       z0 = ptbl[5].f + y0*z0;
     *       z0 = ptbl[6].f + y0*z0;
     *       z0 = ptbl[7].f + y0*z0;
     *       z0 =        y0 + y0*z0;
     *   
     *       if ( fabsf(x0)>1.f ) z0 = pi2f.f - z0;
     *   
     *       // Restore the input sign.
     *       z0 = setsignf( z0, takesignf(x0) );
     *   
     *       z[blkIx*blkSize+n] = z0;
     *     }
     *   }
     */

    {
      /* Input value; reducted input value; output value. */
      xtfloatx2 x0, y0, /*y1, */z0, z1;
      /* Polynomial coeffs for 0.f<=y<0.5f (#1) and 0.5f<=y<=1.f (#2). */
      xtfloatx2 cf1_0, cf1_1, cf1_2, cf1_3, cf1_4, cf1_5, cf1_6, cf1_7;
      xtfloatx2 cf2_0, cf2_1, cf2_2, cf2_3, cf2_4, cf2_5, cf2_6, cf2_7;
      /* Selected polynomial coeffs. */
      xtfloatx2 cf0, cf1, cf2, cf3, cf4, cf5, cf6, cf7;
      /* Input sign; integer representation of output value. */
      ae_int32x2 sx, z0_i;
      /* Is greater than one; is less than 0.5f */
      xtbool2 b_gt1, b_lt05;
      xtfloat tmp;
      X    = (xtfloatx2*)( (uintptr_t)x + blkIx*blkSize*sz_f32 );
      Z    = (xtfloatx2*)( (uintptr_t)z + blkIx*blkSize*sz_f32 );
      S_rd = (xtfloatx2*)scr;

      X_va = XT_LASX2PP( X );
      Z_va = AE_ZALIGN64();

      T1 = (xtfloat  *)atanftbl1;
      T2 = (xtfloat  *)atanftbl2;

      /* Pre-load polynomial coeff set #2. */
      tmp = XT_LSI(T2, +0 * sz_f32); cf2_0 = tmp;
      tmp = XT_LSI(T2, +1 * sz_f32); cf2_1 = tmp;
      tmp = XT_LSI(T2, +2 * sz_f32); cf2_2 = tmp;
      tmp = XT_LSI(T2, +3 * sz_f32); cf2_3 = tmp;
      tmp = XT_LSI(T2, +4 * sz_f32); cf2_4 = tmp;
      tmp = XT_LSI(T2, +5 * sz_f32); cf2_5 = tmp;
      tmp = XT_LSI(T2, +6 * sz_f32); cf2_6 = tmp;
      tmp = XT_LSI(T2, +7 * sz_f32); cf2_7 = tmp;
      for ( n=0; n<blkLen/2; n++ )
      {
        XT_LASX2IP( x0, X_va, X );

        /* Extract the sign bit and take absolute value. */
        sx = XT_AE_MOVINT32X2_FROMXTFLOATX2( x0 );
        sx = AE_SRLI32( sx, 31 );
        sx = AE_SLLI32( sx, 31 );
        x0 = XT_ABS_SX2( x0 );

        XT_LSX2IP( y0, S_rd, +2*sz_f32 );

        b_lt05 = XT_OLT_SX2( y0, (xtfloatx2)0.5f );

        /* Reload coeff set #1 on each iteration. */
        tmp = XT_LSI(T1, +0 * sz_f32); cf1_0 = tmp;
        tmp = XT_LSI(T1, +1 * sz_f32); cf1_1 = tmp;
        tmp = XT_LSI(T1, +2 * sz_f32); cf1_2 = tmp;
        tmp = XT_LSI(T1, +3 * sz_f32); cf1_3 = tmp;
        tmp = XT_LSI(T1, +4 * sz_f32); cf1_4 = tmp;
        tmp = XT_LSI(T1, +5 * sz_f32); cf1_5 = tmp;
        tmp = XT_LSI(T1, +6 * sz_f32); cf1_6 = tmp;
        tmp = XT_LSI(T1, +7 * sz_f32); cf1_7 = tmp;

        /* Select coeffs from sets #1, #2 by reducted value's magnitude. */
        cf0 = cf1_0; XT_MOVF_SX2( cf0, cf2_0, b_lt05 );
        cf1 = cf1_1; XT_MOVF_SX2( cf1, cf2_1, b_lt05 );
        cf2 = cf1_2; XT_MOVF_SX2( cf2, cf2_2, b_lt05 );
        cf3 = cf1_3; XT_MOVF_SX2( cf3, cf2_3, b_lt05 );
        cf4 = cf1_4; XT_MOVF_SX2( cf4, cf2_4, b_lt05 );
        cf5 = cf1_5; XT_MOVF_SX2( cf5, cf2_5, b_lt05 );
        cf6 = cf1_6; XT_MOVF_SX2( cf6, cf2_6, b_lt05 );
        cf7 = cf1_7; XT_MOVF_SX2( cf7, cf2_7, b_lt05 );

        /* Compute the approximation to z(y) = tan(y)/y-1. */

        {
            /* The polynomial is evaluated by Horner's method */
                                        z0 = cf0;
            XT_MADD_SX2( cf1, y0, z0 ); z0 = cf1;
            XT_MADD_SX2( cf2, y0, z0 ); z0 = cf2;
            XT_MADD_SX2( cf3, y0, z0 ); z0 = cf3;
            XT_MADD_SX2( cf4, y0, z0 ); z0 = cf4;
            XT_MADD_SX2( cf5, y0, z0 ); z0 = cf5;
            XT_MADD_SX2( cf6, y0, z0 ); z0 = cf6;
            XT_MADD_SX2( cf7, y0, z0 ); z0 = cf7;
            XT_MADD_SX2(  y0, y0, z0 ); z0 = y0;
        }
        /* Account for the range reduction. */
        b_gt1 = XT_OLT_SX2( (xtfloatx2)1.0f, x0 );
        z1 = XT_SUB_SX2( pi2f.f, z0 );
        XT_MOVT_SX2( z0, z1, b_gt1 );

        /* Propagate the input sign. */
        z0_i = XT_AE_MOVINT32X2_FROMXTFLOATX2( z0 );
        z0_i = AE_OR32( z0_i, sx );
        z0 = XT_AE_MOVXTFLOATX2_FROMINT32X2( z0_i );

        XT_SASX2IP( z0, Z_va, Z );
      }

      XT_SASX2POSFP( Z_va, Z );

      /* Deliberately process the last input value if it's even-numbered. */
      if ( blkLen & 1 )
      {
        x0 = XT_LSI( (xtfloat*)X, 0 );

        /* Extract the sign bit and take absolute value. */
        sx = XT_AE_MOVINT32X2_FROMXTFLOATX2( x0 );
        sx = AE_SRLI32( sx, 31 );
        sx = AE_SLLI32( sx, 31 );
        x0 = XT_ABS_SX2( x0 );

        y0 = XT_LSI( (xtfloat*)S_rd, 0 );

        b_lt05 = XT_OLT_SX2( y0, (xtfloatx2)0.5f );

        /* Load coeff set #1. */
        tmp = XT_LSI(T1, +0 * sz_f32); cf1_0 = tmp;
        tmp = XT_LSI(T1, +1 * sz_f32); cf1_1 = tmp;
        tmp = XT_LSI(T1, +2 * sz_f32); cf1_2 = tmp;
        tmp = XT_LSI(T1, +3 * sz_f32); cf1_3 = tmp;
        tmp = XT_LSI(T1, +4 * sz_f32); cf1_4 = tmp;
        tmp = XT_LSI(T1, +5 * sz_f32); cf1_5 = tmp;
        tmp = XT_LSI(T1, +6 * sz_f32); cf1_6 = tmp;
        tmp = XT_LSI(T1, +7 * sz_f32); cf1_7 = tmp;
        /* Select coeffs from sets #1, #2 by reducted value's magnitude. */
        cf0 = cf1_0; XT_MOVF_SX2( cf0, cf2_0, b_lt05 );
        cf1 = cf1_1; XT_MOVF_SX2( cf1, cf2_1, b_lt05 );
        cf2 = cf1_2; XT_MOVF_SX2( cf2, cf2_2, b_lt05 );
        cf3 = cf1_3; XT_MOVF_SX2( cf3, cf2_3, b_lt05 );
        cf4 = cf1_4; XT_MOVF_SX2( cf4, cf2_4, b_lt05 );
        cf5 = cf1_5; XT_MOVF_SX2( cf5, cf2_5, b_lt05 );
        cf6 = cf1_6; XT_MOVF_SX2( cf6, cf2_6, b_lt05 );
        cf7 = cf1_7; XT_MOVF_SX2( cf7, cf2_7, b_lt05 );

        {
            /* The polynomial is evaluated by Horner's method */
                                        z0 = cf0;
            XT_MADD_SX2( cf1, y0, z0 ); z0 = cf1;
            XT_MADD_SX2( cf2, y0, z0 ); z0 = cf2;
            XT_MADD_SX2( cf3, y0, z0 ); z0 = cf3;
            XT_MADD_SX2( cf4, y0, z0 ); z0 = cf4;
            XT_MADD_SX2( cf5, y0, z0 ); z0 = cf5;
            XT_MADD_SX2( cf6, y0, z0 ); z0 = cf6;
            XT_MADD_SX2( cf7, y0, z0 ); z0 = cf7;
            XT_MADD_SX2(  y0, y0, z0 ); z0 = y0;
        }
        /* Account for the range reduction. */
        b_gt1 = XT_OLT_SX2( (xtfloatx2)1.0f, x0 );
        z1 = XT_SUB_SX2( pi2f.f, z0 );
        XT_MOVT_SX2( z0, z1, b_gt1 );

        /* Propagate the input sign. */
        z0_i = XT_AE_MOVINT32X2_FROMXTFLOATX2( z0 );
        z0_i = AE_OR32( z0_i, sx );
        z0 = XT_AE_MOVXTFLOATX2_FROMINT32X2( z0_i );

        XT_SSI( z0, (xtfloat*)Z, 0 );
      }
    }

  } /* for ( blkIx=0; blkIx<blkNum; blkIx++ ) */

} /* vec_atanf() */

#endif /* #if HAVE_VFPU */

