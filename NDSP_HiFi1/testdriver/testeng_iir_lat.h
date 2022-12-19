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
 * Test-engine add-on for lattice IIR categories 
 */
#ifndef TESTENG_IIR_LAT_H__
#define TESTENG_IIR_LAT_H__

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"
/* Test data vectors tools and SEQ-file reader. */
#include "vectools.h"
#include "testeng.h"
#include "testeng_iir.h"

typedef size_t tIirLatFxnAlloc (int);
typedef void * tIirLatFxnInit (void * objmem, int M, const void * k,int16_t gain);
typedef void   tIirLatFxnProcess (void *, void * ,const void *, int );

typedef struct
{
    tIirLatFxnAlloc      *alloc;
    tIirLatFxnInit       *init;
    tIirLatFxnProcess    *process;
}
tIirLatDescr;


/* function reads coefficients from file and creates IIR structure. returns 0 if failed */
int te_create_iir_lat(tTestEngContext * context);

/* function destroys IIR structure, returns 0 if failed */
int te_destroy_iir_lat(tTestEngContext * context);

/* 
   Allocate vectors and load the data set for IIR:
*  vector X (in), vector Z (out) */
int te_loadFxn_iir_lat(tTestEngContext * context);

/* Apply IIR function to the test case data set.
*  vector X (in), vector Z (out) */
void te_processFxn_iir_lat(tTestEngContext * context);

#endif
