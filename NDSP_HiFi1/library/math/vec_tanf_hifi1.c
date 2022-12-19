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
/* DSP Library API */
#include "NatureDSP_Signal.h"
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, vec_tanf, ( float32_t *restrict z, const float32_t *restrict x, int N))

#else
/* Value of 2/pi, 4/pi, etc. */
#include "inv2pif_tbl.h"
/* Angular argument reduction constants. */
#include "sinf_tbl.h"
/* tan/cotan approximation polynomial coeffs. */
#include "tanf_tbl.h"
/* sNaN/qNaN, single precision. */
#include "nanf_tbl.h"

#define sz_f32    (int)sizeof(float32_t)
/*===========================================================================
  Vector matematics:
  vec_tan             Tangent    
===========================================================================*/

/*-------------------------------------------------------------------------
  Tangent 
  Fixed point functions calculate tan(pi*x) for number written in Q15 or 
  Q31. Floating point functions compute tan(x)
  
  Precision:
  16x16  16-bit inputs (Q15), 16-bit outputs (Q8.7). Accuracy: 1 LSB
  24x24  24-bit inputs, 32-bit outputs. Accuracy: (1.3e-4*y+1LSB) 
                                        if abs(y)<=464873(14.19 in Q15) 
                                        or abs(x)<pi*0.4776
  32x32  32-bit inputs, 32-bit outputs. Accuracy: (1.3e-4*y+1LSB)
                                        if abs(y)<=464873(14.19 in Q15) 
                                        or abs(x)<pi*0.4776
  f      floating point.                Accuracy: 2 ULP

  NOTE:
  1.  Scalar floating point function is compatible with standard ANSI C routines 
      and set errno and exception flags accordingly
  2.  Floating point functions limit the range of allowable input values: [-9099, 9099]
      Whenever the input value does not belong to this range, the result is set to NaN.

  Input:
  x[N]   input data, Q15, Q31 or floating point
  N      length of vectors
  Output:
  y[N]   result, Q8.7 (16-bit function), Q16.15 (24 or 32-bit functions) 
         or floating point

  Restriction:
  x,y - should not overlap

  Scalar versions:
  ----------------
  Return result, Q8.7 (16-bit function), Q16.15 (24 or 32-bit functions) 
  or floating point
-------------------------------------------------------------------------*/

void vec_tanf( float32_t * restrict z, 
         const float32_t * restrict x,
         int N)
{
  /*
    float32_t x2,y,yt,yc;
    int sx,n,j,st;
    sx=takesignf(x);
    x=sx?-x:x;
    if(x>tanf_maxval) return qNaN.f;
     argument reduction
       process reduces x by integral multiple of pi/4.
       The output is deduced to the sum of two single precision 
       floating point values x+dx. 
    
    n = (int)STDLIB_MATH(ceilf)(x*inv4pif.f);
    j = n&~1;

    {
      float32_t dx, t, y = x, jj = (float32_t)j;
      const union ufloat32uint32 c[6] = {
        { 0x3f4a0000 },
        { 0xbb700000 },
        { 0xb6160000 },
        { 0x32080000 },
        { 0x2e060000 },
        { 0xa9b9ee5a } };
      dx = 0.f;
      y -= c[0].f*jj;
      y -= c[1].f*jj;
      y -= c[2].f*jj;
      t = y; y -= c[3].f*jj; t = (t - y); t -= c[3].f*jj; dx = t;
      t = y; y -= c[4].f*jj; t = (t - y); t -= c[4].f*jj; dx = (dx + t);
      t = y; y -= c[5].f*jj; t = (t - y); t -= c[5].f*jj; dx = (dx + t);
      y = (y + dx);
      x = y;
    }

     compute tan via minmax polynomial 
    x2 = x*x;
    yt = polytanf_tbl[0].f;
    yt = yt*x2 + polytanf_tbl[1].f;
    yt = yt*x2 + polytanf_tbl[2].f;
    yt = yt*x2 + polytanf_tbl[3].f;
    yt = yt*x2 + polytanf_tbl[4].f;
    yt = yt*x2 + polytanf_tbl[5].f;
    yt = yt*x2 + polytanf_tbl[6].f;
    yt = yt*x2;
    
    dx is small enough (<3e-8) and wiil be used to modify
    tangent value computed by polynomial using derivation
    tg(x+dx)=tg(x)+dx/cos(x)^2
    for 2 ULP operation it is enough to suppose 1/cos(x)^2 ~ 1
    for 1 ULP operation, it should be computed accurately

    resulting value is decomposed as follows
    tag(x+dx)~(P*x+dx)+x
    The order of summation is crucial!
    
    yt = (yt*x) + x;
    yc = 1.f / yt;

    adjust sign 
    n = (n >> 1) & 1;
    st = sx ^ n;
     select tan/cotan 
    y = n ? yc : yt;
     apply the sign 
    y = changesignf(y, st);
    return y;
  */

  const xtfloatx2 *          X;
        xtfloatx2 * restrict Z;
  const xtfloatx2 *          S_rd;
        xtfloatx2 * restrict S_wr;
  const xtfloat   *          T;

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
     *     float32_t xn, p, dp, t;
     *     int ji;
     *     float32_t jf;
     *   
     *     static const union ufloat32uint32 c[6] = {
     *       { 0x3f4a0000 }, { 0xbb700000 },
     *       { 0xb6160000 }, { 0x32080000 },
     *       { 0x2e060000 }, { 0xa9b9ee5a }
     *     };
     *   
     *     for ( n=0; n<blkLen; n++ )
     *     {
     *       xn = fabsf( x[blkIx*blkSize+n] );
     *   
     *       // Determine the pi/2-wide segment the input value belongs to.
     *       ji = (int)ceilf( xn*inv4pif.f );
     *       jf = (float32_t)( ji & ~1 );
     *   
     *       // Calculate the difference between the segment midpoint and input value.
     *       p = xn;
     *       p -= c[0].f*jf;
     *       p -= c[1].f*jf;
     *       p -= c[2].f*jf;
     *       t = p; p -= c[3].f*jf; t = t - p; t -= c[3].f*jf; dp = t;
     *       t = p; p -= c[4].f*jf; t = t - p; t -= c[4].f*jf; dp += t;
     *       t = p; p -= c[5].f*jf; t = t - p; t -= c[5].f*jf; dp += t;
     *       p += dp;
     *   
     *       scr[n] = p;
     *     }
     *   }
     */

    {
      /* Input value; reducted input value; correction term. */
      xtfloatx2 xn, p, dp;
      /* Auxiliary floating-point vars. */
      xtfloatx2 t, r;
      /* Input value segment number. */
      ae_int32x2 ji;
      xtfloatx2 jf;
      /* pi/4 splitted into 7-bit chunks. */
      xtfloatx2 c0, c1, c2, c3, c4, c5;
      /* Scalar auxiliary var.  */
      xtfloat cs;

      static const union ufloat32uint32 c[6] = {
        { 0x3f4a0000 }, { 0xbb700000 },
        { 0xb6160000 }, { 0x32080000 },
        { 0x2e060000 }, { 0xa9b9ee5a }
      };

      X    = (xtfloatx2*)( (uintptr_t)x + blkIx*blkSize*sz_f32 );
      S_wr = (xtfloatx2*)scr;
      T    = (xtfloat  *)c;

      X_va = XT_LASX2PP( X );

      __Pragma( "loop_count min=1" );
      for ( n=0; n<(blkLen+1)/2; n++ )
      {
        XT_LASX2IP( xn, X_va, X );

        /*
         * Determine the pi/2-wide segment the input value belongs to.
         */
        
        xn = XT_ABS_SX2( xn );
        t = XT_MUL_SX2( xn, inv4pif.f );
        t = XT_FICEIL_SX2( t );
        ji = XT_TRUNC_SX2( t, 0 );
        ji = AE_AND32( ji, AE_NOT32( AE_MOVI(1) ) );
        jf = XT_FLOAT_SX2( ji, 0 );

        /*
         * Calculate the difference between the segment midpoint and input value.
         */

        XT_LSIP( cs, T, +1*sz_f32 ); c0 = cs;
        XT_LSIP( cs, T, +1*sz_f32 ); c1 = cs;
        XT_LSIP( cs, T, +1*sz_f32 ); c2 = cs;
        XT_LSIP( cs, T, +1*sz_f32 ); c3 = cs;
        XT_LSIP( cs, T, +1*sz_f32 ); c4 = cs;
        XT_LSIP( cs, T, -5*sz_f32 ); c5 = cs;

        p = xn;
        XT_MSUB_SX2( p, jf, c0 );
        XT_MSUB_SX2( p, jf, c1 );
        XT_MSUB_SX2( p, jf, c2 );

        r = XT_MUL_SX2(jf,c3); t = p; p = XT_SUB_SX2(p,r); t = XT_SUB_SX2(t,p); t = XT_SUB_SX2(t,r); dp = t;
        r = XT_MUL_SX2(jf,c4); t = p; p = XT_SUB_SX2(p,r); t = XT_SUB_SX2(t,p); t = XT_SUB_SX2(t,r); dp = XT_ADD_SX2(t,dp);
        r = XT_MUL_SX2(jf,c5); t = p; p = XT_SUB_SX2(p,r); t = XT_SUB_SX2(t,p); t = XT_SUB_SX2(t,r); dp = XT_ADD_SX2(t,dp);

        p = XT_ADD_SX2( p, dp );

        XT_SSX2IP( p, S_wr, +2*sz_f32 );
      }
    }

    __Pragma( "no_reorder" );

    /*
     * Part II, tangent approximation via minmax polynomial. Reference C code:
     *
     *   {
     *     float32_t yt, p, p2;
     *   
     *     for ( n=0; n<blkLen; n++ )
     *     {
     *       p = scr[n];
     *       p2 = p*p;
     *   
     *       yt = polytanf_tbl[0].f;
     *       yt = polytanf_tbl[1].f + yt*p2;
     *       yt = polytanf_tbl[2].f + yt*p2;
     *       yt = polytanf_tbl[3].f + yt*p2;
     *       yt = polytanf_tbl[4].f + yt*p2;
     *       yt = polytanf_tbl[5].f + yt*p2;
     *       yt = polytanf_tbl[6].f + yt*p2;
     *       yt =                     yt*p2;
     *   
     *       scr[n] = yt*p + p;
     *     }
     *   }
     */

    {
      /* Reducted input value and its 2nd power; tangent approximation. */
      xtfloatx2 p, yt;
      /* Polynomial coeffs. */
      xtfloatx2 cf0, cf1, cf2, cf3, cf4, cf5, cf6;

      S_rd = (xtfloatx2*)scr;
      S_wr = (xtfloatx2*)scr;
      T    = (xtfloat  *)polytanf_tbl;

      __Pragma( "loop_count min=1" );
      for ( n=0; n<(blkLen+1)/2; n++ )
      {
        XT_LSX2IP( p, S_rd, +2*sz_f32 );

        cf0 = XT_LSI( T, 0*sz_f32 );
        cf1 = XT_LSI( T, 1*sz_f32 );
        cf2 = XT_LSI( T, 2*sz_f32 );
        cf3 = XT_LSI( T, 3*sz_f32 );
        cf4 = XT_LSI( T, 4*sz_f32 );
        cf5 = XT_LSI( T, 5*sz_f32 );
        cf6 = XT_LSI( T, 6*sz_f32 );
        {
          xtfloatx2 p2;
          p2 = XT_MUL_SX2( p, p );
                                      yt = cf0;
          XT_MADD_SX2( cf1, p2, yt ); yt = cf1;
          XT_MADD_SX2( cf2, p2, yt ); yt = cf2;
          XT_MADD_SX2( cf3, p2, yt ); yt = cf3;
          XT_MADD_SX2( cf4, p2, yt ); yt = cf4;
          XT_MADD_SX2( cf5, p2, yt ); yt = cf5;
          XT_MADD_SX2( cf6, p2, yt ); yt = cf6;
          yt = XT_MUL_SX2 ( p2, yt );

          XT_MADD_SX2( p, p, yt ); yt = p;
        }
        XT_SSX2IP( yt, S_wr, +2*sz_f32 );
      }
    }

    __Pragma( "no_reorder" );

    /*
     * Part III, estimation of cotangent and finalization of results. Reference C code:
     *
     *   {
     *     float32_t xn, xm, yt, yc, zn;
     *     int ji, sx, sz;
     *   
     *     for ( n=0; n<blkLen; n++ )
     *     {
     *       xn = x[blkIx*blkSize+n];
     *       yt = scr[n];
     *   
     *       xm = fabsf(xn);
     *       ji = (int)ceilf( xm*inv4pif.f );
     *       ji = (ji>>1) & 1;
     *       sx = takesignf( xn );
     *       sz = sx ^ ji;
     *       yc = 1.f/yt;
     *       zn = ( ji ? yc : yt );
     *       zn = changesignf( zn, sz );
     *   
     *       z[blkIx*blkSize+n] = ( xm<=tanf_maxval ? zn : qNaNf.f );
     *     }
     *   }
     */

    {
      /* Input value and its magnitude; resulting value. */
      xtfloatx2 xn, xm, zn;
      /* Tangent and cotangent */
      xtfloatx2 yt, yc;
      /* Input value segment number. */
      ae_int32x2 ji;
      /* Auxiliary floating-point var. */
      xtfloatx2 t;
      /* Input sign; result sign; auxiliary var.  */
      ae_int32x2 sx, sz, s;
      /* Cotangent/tangent selection flag; input data validation flag. */
      xtbool2 b_cot, b_dom;

      X = (xtfloatx2*)( (uintptr_t)x + blkIx*blkSize*sz_f32 );
      Z = (xtfloatx2*)( (uintptr_t)z + blkIx*blkSize*sz_f32 );

      S_rd = (xtfloatx2*)scr;

      X_va = XT_LASX2PP( X );
      Z_va = AE_ZALIGN64();

      for ( n=0; n<blkLen/2; n++ )
      {
        XT_LASX2IP( xn, X_va, X );

        /* Re-calculate the pi/2-wide segment number. */
        xm = XT_ABS_SX2( xn );
        t = XT_MUL_SX2( xm, inv4pif.f );
        t = XT_FICEIL_SX2( t );
        ji = XT_TRUNC_SX2( t, 0 );
        ji = AE_SLLI32( ji, 30 );

        /* Compute the sign adjustment term. */
        sx = XT_AE_MOVINT32X2_FROMXTFLOATX2( xn );
        sz = AE_XOR32( sx, ji );
        sz = AE_AND32( sz, 0x80000000 );

        /* Load tangent approximation from the scratch. */
        XT_LSX2IP( yt, S_rd, +2*sz_f32 );

        /* Compute the cotangent for odd-numbered segments. */
        yc = XT_RECIP_SX2( yt );
        b_cot = AE_LT32( ji, AE_ZERO32() );
        zn = yt; XT_MOVT_SX2( zn, yc, b_cot );

        /* Adjust the sign. */
        s = XT_AE_MOVINT32X2_FROMXTFLOATX2( zn );
        s = AE_XOR32( s, sz );
        zn = XT_AE_MOVXTFLOATX2_FROMINT32X2( s );

        /* Set result to quiet NaN for an out-of-domain input value. */
        b_dom = XT_OLE_SX2( xm, tanf_maxval );
        XT_MOVF_SX2( zn, qNaNf.f, b_dom );

        XT_SASX2IP( zn, Z_va, Z );
      }

      XT_SASX2POSFP( Z_va, Z );

      /* Deliberately process the last input value if it's even-numbered. */
      if ( blkLen & 1 )
      {
        xn = XT_LSI( (xtfloat*)X, 0 );

        /* Re-calculate the pi/2-wide segment number. */
        xm = XT_ABS_SX2( xn );
        t = XT_MUL_SX2( xm, inv4pif.f );
        t = XT_FICEIL_SX2( t );
        ji = XT_TRUNC_SX2( t, 0 );
        ji = AE_SLLI32( ji, 30 );

        /* Compute the sign adjustment term. */
        sx = XT_AE_MOVINT32X2_FROMXTFLOATX2( xn );
        sz = AE_XOR32( sx, ji );
        sz = AE_AND32( sz, 0x80000000 );

        /* Load tangent approximation from the scratch. */
        yt = XT_LSI( (xtfloat*)S_rd, 0 );

        /* Compute the cotangent for odd-numbered segments. */
        yc = XT_RECIP_SX2( yt );
        b_cot = AE_LT32( ji, AE_ZERO32() );
        zn = yt; XT_MOVT_SX2( zn, yc, b_cot );

        /* Adjust the sign. */
        s = XT_AE_MOVINT32X2_FROMXTFLOATX2( zn );
        s = AE_XOR32( s, sz );
        zn = XT_AE_MOVXTFLOATX2_FROMINT32X2( s );

        /* Set result to quiet NaN for an out-of-domain input value. */
        b_dom = XT_OLE_SX2( xm, tanf_maxval );
        XT_MOVF_SX2( zn, qNaNf.f, b_dom );

        XT_SSI( zn, (xtfloat*)Z, 0 );
      }
    }

  } /* for ( blkIx=0; blkIx<blkNum; blkIx++ ) */

} /* vec_tanf() */

#endif /* #if HAVE_VFPU */

