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
    helper for correlation/convolution
    C code optimized for HiFi1
*/
/* Portable data types. */
#include "NatureDSP_types.h"
/* Common utility and macros declarations. */
#include "common.h"

#if !(HAVE_VFPU)
DISCARD_FUN(void, raw_lcorrf, (float32_t * restrict r,
            const float32_t * restrict x,
            const float32_t * restrict y,
            int N, int M))
            
#else
#include "raw_lcorrf.h"

/*-----------------------------------------------------
    raw linear correlation:
    Input:
    x[N] 
    y[M]
    Output:
    r[N+M-1]
    restriction:
    M should be >0
-----------------------------------------------------*/
void raw_lcorrf(float32_t  * restrict r,
          const float32_t  * restrict x,
          const float32_t  * restrict y,
                int N, int M)
{
    int m, n;

    int n_iter, m_iter;

    xtfloat A0, A1, A2, A3, X0, X1, X2, X3, Y0, Y1,Y0T;

    xtfloatx2 A10, A32, A01,A_TMP, X01, X23,X21, Y01,X12,X34,Y01R,A23;
    xtfloatx2 _01={0,1};
    ae_valign x_align, y_align,z_align;

    const xtfloat * restrict pX;
    const xtfloat * restrict pY;
    const xtfloat * restrict pS;
          xtfloat * restrict pR;
       

    pR = (xtfloat*)r;
  

    const xtfloatx2 * restrict pXt;
    const xtfloatx2 * restrict pYt;

    /*
    * Compute first M-1 entries.
    */
    pS = (const xtfloat*)(y + M - 4);
    n_iter = (M - 1);
    pXt = (const xtfloatx2*)(x);
    pYt = (const xtfloatx2*)(pS);
    z_align = AE_ZALIGN64();
    for (n = 0; n<(n_iter&~3); n += 4, pS -= 4)
    {
        pXt = (const xtfloatx2*)(x);
        pYt = (const xtfloatx2*)(pS);
        x_align = AE_LA64_PP(pXt);
        y_align = AE_LA64_PP(pYt);

        XT_LASX2IP(X01, x_align, pXt);
        XT_LASX2IP(X23, x_align, pXt);
        XT_LASX2IP(Y01, y_align, pYt);

        A32=XT_MULMUX_S(X01,Y01,0);
        A_TMP=XT_MUL_SX2(X01,Y01);
        XT_MADDMUX_S(A32,A_TMP,_01,5);


        XT_LASX2IP(Y01, y_align, pYt);


        X21 = XT_SEL32_HL_SX2(X23,X01);

        A10=XT_MULMUX_S(X01,Y01,0);
        XT_MADDMUX_S(A32,Y01,X23,5);
        XT_MADDMUX_S(A32,Y01,X21,0);
        A01 = XT_SEL32_LH_SX2(A10, A10);
        A_TMP=XT_MUL_SX2(X01, Y01);
        XT_MADD_SX2(A01,_01,A_TMP);
        A23 = XT_SEL32_LH_SX2(A32, A32);


        X01 = XT_SEL32_LH_SX2(X01, X23);
        X12 = XT_SEL32_LL_SX2(X01, X23);


        for (m = 0; m < (n >> 1); m++)
        {

            XT_LASX2IP(X34, x_align, pXt);
            XT_LASX2IP(Y01, y_align, pYt);
            X23 = XT_SEL32_LH_SX2(X12, X34);
            Y01R = XT_SEL32_LH_SX2(Y01, Y01);


            XT_MADDMUX_S(A01,Y01,X01,0);
            XT_MADDMUX_S(A23,Y01,X23,0);
            XT_MADDMUX_S(A23,Y01R,X34,0);
            XT_MADDMUX_S(A01,Y01R,X12,0);

            X01 = XT_SEL32_HL_SX2(X23, X23);
            X12 = XT_SEL32_LL_SX2(X23, X34);


        }
        if (n & 1)
        {
            XT_LASX2IP(X34, x_align, pXt);
            XT_LSIP(Y0T, castxcc(const xtfloat, pYt), 4);

            X23 = XT_SEL32_LH_SX2(X12, X34);


            XT_MADDMUX_S(A01,Y0T,X01,0);
            XT_MADDMUX_S(A23,Y0T,X23,0);
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
        pX = (const xtfloat*)(x);
        pY = (pS);
        XT_LSIP(X0, pX, 4);
        XT_LSIP(X1, pX, 4);
        XT_LSIP(X2, pX, 4);

        XT_LSIP(Y0, pY, 4);
        XT_LSIP(Y1, pY, 4);

        XT_MADD_S(A2, X0, Y1);

        XT_LSIP(Y0, pY, 4);
        XT_LSIP(Y1, pY, 4);

        XT_MADD_S(A2, X1, Y0);
        XT_MADD_S(A2, X2, Y1);

        XT_MADD_S(A1, X0, Y0);
        XT_MADD_S(A1, X1, Y1);

        XT_MADD_S(A0, X0, Y1);

        X0 = X1;
        X1 = X2;

        for (m = 0; m < (n >> 1); m++)
        {
            XT_LSIP(X2, pX, 4);
            XT_LSIP(X3, pX, 4);

            XT_LSIP(Y0, pY, 4);
            XT_LSIP(Y1, pY, 4);
            XT_MADD_S(A0, X0, Y0);
            XT_MADD_S(A1, X1, Y0);
            XT_MADD_S(A2, X2, Y0);
            XT_MADD_S(A0, X1, Y1);
            XT_MADD_S(A1, X2, Y1);
            XT_MADD_S(A2, X3, Y1);
            X0 = X2;
            X1 = X3;
        }
        if (n & 1)
        {
            XT_LSIP(Y0, pY, 4);
            XT_MADD_S(A0, X0, Y0);
            XT_MADD_S(A1, X1, Y0);
            XT_MADD_S(A2, X2, Y0);
        }
        XT_SSIP(A0, pR, 4);
        if (n_iter>1) XT_SSIP(A1, pR, 4);
        if (n_iter>2) XT_SSIP(A2, pR, 4);
    }
//scope
    /*
    * Compute r[M]...r[N] entries.
    */
    z_align = AE_ZALIGN64();
    n_iter = (N - M + 1);
    pX = (const xtfloat*)(x);
    for (n = 0; n<(n_iter&~3); n += 4)
    {
        A01=A23=XT_CONST_S(0);

        pXt = (const xtfloatx2*)(x + n+3);
        pYt = (const xtfloatx2*)(y);
        x_align = AE_LA64_PP(pXt);
        y_align = AE_LA64_PP(pYt);

        XT_LSIP(X0, pX, 4);
        XT_LSIP(X1, pX, 4);
        XT_LSIP(X2, pX, 8);

        X01=XT_SEL32_LL_SX2(X0,X1);
        X12=XT_SEL32_LL_SX2(X1,X2);
    

        for (m = 0; m<(M >> 1); m++)
        {

            XT_LASX2IP(X34, x_align, pXt);
            XT_LASX2IP(Y01, y_align, pYt);
            X23 = XT_SEL32_LH_SX2(X12, X34);
            Y01R = XT_SEL32_LH_SX2(Y01, Y01);



            XT_MADDMUX_S(A01,Y01,X01,0);
            XT_MADDMUX_S(A23,Y01,X23,0);
            XT_MADDMUX_S(A23,Y01R,X34,0);
            XT_MADDMUX_S(A01,Y01R,X12,0);

            X01 = XT_SEL32_HL_SX2(X23, X23);
            X12 = XT_SEL32_LL_SX2(X23, X34);


        }

        if (M & 1)
        {

            XT_LASX2IP(X34, x_align, pXt);

            XT_LSIP(Y0T, castxcc(const xtfloat, pYt), 4);
            X23 = XT_SEL32_LH_SX2(X12, X34);


            XT_MADDMUX_S(A01,Y0T,X01,0);
            XT_MADDMUX_S(A23,Y0T,X23,0);
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
        pX = (const xtfloat*)(x + ((N - M + 1)&~3));
        pY = (const xtfloat*)y;
        XT_LSIP(X0, pX, 4);
        XT_LSIP(X1, pX, 4);
        for (m = 0; m<(M >> 1); m++)
        {
            XT_LSIP(X2, pX, 4);
            XT_LSIP(X3, pX, 4);

            XT_LSIP(Y0, pY, 4);
            XT_LSIP(Y1, pY, 4);
            XT_MADD_S(A0, X0, Y0);
            XT_MADD_S(A1, X1, Y0);
            XT_MADD_S(A2, X2, Y0);
            XT_MADD_S(A0, X1, Y1);
            XT_MADD_S(A1, X2, Y1);
            XT_MADD_S(A2, X3, Y1);
            X0 = X2;
            X1 = X3;
        }
        if (M & 1)
        {
            XT_LSIP(X2, pX, 4);
            XT_LSIP(Y0, pY, 4);
            XT_MADD_S(A0, X0, Y0);
            XT_MADD_S(A1, X1, Y0);
            XT_MADD_S(A2, X2, Y0);
        }
        XT_SSIP(A0, pR, 4);
        if (n_iter>1) XT_SSIP(A1, pR, 4);
        if (n_iter>2) XT_SSIP(A2, pR, 4);
    }
//scope
    /*
    * Compute r[N+1]....r[N+M-1] entries.
    */
    z_align = AE_ZALIGN64();
    n_iter = M - 1;
    m_iter = n_iter - 4;    
    pS = (const xtfloat*)(x + (N - M) + 1);
    for (n = 0; n<(n_iter&~3); n += 4, m_iter -= 4)
    {
        xtfloat Y3T,X0T;
        xtfloatx2 Y23, Y34, Y12,Y10,Y32;

        A0 = A1 = A2 = A3 = XT_CONST_S(0);

        pX = pS + n;
        pY = (const xtfloat*)y;

        pXt = (const xtfloatx2*)(pS + n);
        pYt = (const xtfloatx2*)(y);
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

            XT_LSIP(Y3T, castxcc(const xtfloat, pYt), 4);
            XT_LSIP(X0T, castxcc(const xtfloat, pXt), 4);

            Y32 = XT_SEL32_LL_SX2(Y3T, Y12);

            XT_MADDMUX_S(A01,X0T,Y32,0);
            XT_MADDMUX_S(A23,X0T,Y10,0);
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
        pY = (const xtfloat*)y;

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
}
#endif

