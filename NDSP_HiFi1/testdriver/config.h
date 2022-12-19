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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <time.h>


#define IS_OK(hr) ((hr) == TEST_OK)
#define TEST_OK                 0
#define TEST_FAILED_ACCURACY    1
#define TEST_FAILED_ALIGN       2
#define TEST_FAILED_SCALAR      3
#define TEST_FAILED_FILEOPEN    4
#define TEST_FAILED_MEMORY      5
#define TEST_FAILED_PARAMS      6
#define TEST_NOT_TESTED        -1

#if __SMALL_TEST__ == 0
    #define MAX_SAMPLES_PROCESS      0
    #define STEP_MASK_LOG         0xff // full range test step
    #define STEP_MASK_SINE        0xff // set wider step mask to speed up test
    #define STEP_MASK_TAN         0xff
    #define STEP_MASK_ATAN        0xff
#else
    #define MAX_SAMPLES_PROCESS  16000
    #define STEP_MASK_LOG        0xfff
    #define STEP_MASK_SINE       0xfff
    #define STEP_MASK_TAN        0xfff
    #define STEP_MASK_ATAN       0xfff
#endif

#endif//__CONFIG_H__
