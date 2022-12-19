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
    NatureDSP Signal Processing Library. FIR part
    Real data linear auto-correlation, floating point, no requirements on vectors
    length and alignment.
*/

/*-------------------------------------------------------------------------
  Linear Autocorrelation 
  Functions estimate the linear auto-correlation of vector x. Returns 
  autocorrelation of length N.

  Precision: 
  16x16   16-bit data, 16-bit outputs
  32x32   32-bit data, 32-bit outputs
  f       floating point

  Input:
  s[]       scratch area of
            FIR_LACORRA16X16_SCRATCH_SIZE( N )
            FIR_LACORRA32X32_SCRATCH_SIZE( N )
            FIR_LACORRAF_SCRATCH_SIZE( N )    
            bytes
  x[N]      input data Q31, Q15 or floating point
  N         length of x
  Output:
  r[N]      output data, Q31, Q15 or floating point

  Restrictions:
  x,r,s should not overlap
  N >0
  s   - aligned on an 8-bytes boundary
-------------------------------------------------------------------------*/
/* Portable data types. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, fir_lacorraf, (void * restrict s,
            float32_t  * restrict r,
            const float32_t  * restrict x, int N))

#else
/* NON OPTIMIZED REFERENCE CODE: to be use it for educational purposes only */
/*
void fir_lacorraf(    void             * restrict s,
                      float32_t        * restrict r, 
                      const float32_t  * restrict x, int N)
{
    float32_t acc;
    int n,k;
    for (k=0; k<N; k++)
    {
        acc=0.f;
        for (n=0; n<N-k; n++)
        {
            acc+= x[n+k] * x[n] ;
        }
        r[k]=acc;
    }
}
*/ 

void fir_lacorraf( void       * restrict s,
                   float32_t  * restrict r,
             const float32_t  * restrict x, int N)
{
    int m, n;

    int n_iter, m_iter;

    xtfloat A0, A1, A2, X0, X1, X2, Y0, Y1;
    xtfloatx2 X01,X23,A_TMP;
    xtfloatx2 A01={0,0};
    xtfloatx2 A23={0,0};
    xtfloatx2 _10={1,0};
    xtfloatx2 _01={0,1};

    const xtfloat * restrict pX;
    const xtfloat * restrict pY;
    const xtfloat * restrict pS;
          xtfloat * restrict pR;

          ae_valign x_align,z_align,y_align;
    const xtfloatx2 * restrict pXt;
    const xtfloatx2 * restrict pYt;

    NASSERT(r && x && N > 0);

    pR = (xtfloat*)r;
    /*
    * Compute r[0] entries.
    */
    {
        pXt = (const xtfloatx2*)(x);
        x_align = AE_LA64_PP(pXt);
        for (m = 0; m<(N>>2); m++)
        {

            XT_LASX2IP(X01, x_align, pXt);
            XT_MADD_SX2(A01,X01,X01);

            XT_LASX2IP(X23, x_align, pXt);
            XT_MADD_SX2(A23,X23,X23);
        }
        if(N&2)
        {
            XT_LASX2IP(X01, x_align, pXt);
            XT_MADD_SX2(A01,X01,X01);
        }
        if(N&1)
        {
        	XT_LSIP(X0, castxcc(const xtfloat, pXt), 4);
        	A_TMP=XT_MUL_SX2(X0,X0);
        	XT_MADD_SX2(A01,_10,A_TMP);
        }
        A01=XT_ADD_SX2(A01,A23);
        A23 = XT_SEL32_LH_SX2(A01, A01);
        A01=XT_ADD_SX2(A01,A23);
        XT_SSIP(A01, pR, 4);
    }

    /*
    * Compute r[N+1]....r[N+M-1] entries.
    */
    z_align = AE_ZALIGN64();
    n_iter = N - 1;
    m_iter = n_iter - 4;
    pS = (const xtfloat*)(x + 1);
    for (n = 0; n<(n_iter&~3); n += 4, m_iter -= 4)
    {
        xtfloat Y3,X0;
        xtfloatx2 Y23, Y34, Y12,Y10,Y32,X21,Y01;

        pXt = (const xtfloatx2*)(pS + n);
        pYt = (const xtfloatx2*)(x);
        x_align = AE_LA64_PP(pXt);
        y_align = AE_LA64_PP(pYt);

        XT_LASX2IP(X01, x_align, pXt);
        XT_LASX2IP(X23, x_align, pXt);
        XT_LASX2IP(Y01, y_align, pYt);

        X21=XT_SEL32_HL_SX2(X23,X01);

        A01=XT_MULMUX_S(Y01,X01,0);
        A23=XT_MULMUX_S(Y01,X23,0);
        XT_MADDMUX_S(A01, Y01, X21,5);
        A_TMP=XT_MUL_SX2(Y01, X23);
        XT_MADDMUX_S(A23,A_TMP,_01,5);
        XT_LASX2IP(Y23, y_align, pYt);

        XT_MADDMUX_S(A01, Y23, X23,0);
        A_TMP=XT_MUL_SX2(Y23, X23);
        XT_MADDMUX_S(A01,A_TMP,_01,5);

        Y10 = XT_SEL32_HL_SX2(Y23, Y01);
        Y12 = XT_SEL32_HL_SX2(Y10, Y23);

        for (m = 0; m<(m_iter >> 1); m++)
        {

            XT_LASX2IP(X01, x_align, pXt);
            XT_LASX2IP(Y34, y_align, pYt);

            Y32 = XT_SEL32_HL_SX2(Y34, Y12);


            XT_MADDMUX_S(A01,X01,Y32,0);
            XT_MADDMUX_S(A23,X01,Y10,0);
            XT_MADDMUX_S(A01,X01,Y34,5);
            XT_MADDMUX_S(A23,X01,Y12,5);

            Y10 = XT_SEL32_HL_SX2(Y32, Y32);
            Y12 = XT_SEL32_HL_SX2(Y34, Y34);

        }

        if (m_iter & 1)
        {

            XT_LSIP(Y3, castxcc(const xtfloat, pYt), 4);
            XT_LSIP(X0, castxcc(const xtfloat, pXt), 4);

            Y32 = XT_SEL32_LL_SX2(Y3, Y12);


            XT_MADDMUX_S(A01,X0,Y32,0);
            XT_MADDMUX_S(A23,X0,Y10,0);
        }

        XT_SASX2IP(A01,z_align,castxcc(xtfloatx2, pR));
        XT_SASX2IP(A23,z_align,castxcc(xtfloatx2, pR));
    }
    AE_SA64POS_FP(z_align, castxcc(xtfloatx2, pR));
    /* last 1...3 iterations */
    n_iter &= 3;
    if (n_iter)
    {
        A0 = A1 = A2 = XT_CONST_S(0);

        pX = (const xtfloat*)(x + N - 3);
        pY = (const xtfloat*)x;

        XT_LSIP(X0, pX, 4);
        XT_LSIP(X1, pX, 4);
        XT_LSIP(X2, pX, 4);

        XT_LSIP(Y0, pY, 4);
        XT_LSIP(Y1, pY, 4);

        XT_MADD_S(A0, X0, Y0);
        XT_MADD_S(A0, X1, Y1);

        XT_MADD_S(A1, X1, Y0);
        XT_MADD_S(A1, X2, Y1);

        XT_MADD_S(A2, X2, Y0);

        XT_LSIP(Y0, pY, 4);
        XT_MADD_S(A0, X2, Y0);

        if (n_iter>2) XT_SSIP(A0, pR, 4);
        if (n_iter>1) XT_SSIP(A1, pR, 4);
        XT_SSIP(A2, pR, 4);
    }     
} // fir_lacorraf

#endif /*HAVE_VFPU*/


