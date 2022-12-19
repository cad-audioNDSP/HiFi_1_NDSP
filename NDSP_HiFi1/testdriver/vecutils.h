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

#ifndef VECUTILS_H__
#define VECUTILS_H__
/*  
	Helper functions for reading data vectors from .seq files
	Integrit, 2006-2015
*/

#include "NatureDSP_types.h"
#include "malloc16.h"
#include <stdio.h>


typedef struct
{
    tAllocPtr   ptr;
    int offset; // offset of first element
    int N;
    int fmt;    // 1 if double precision
}
tVecDescr;

#define FMT_SNGL  0
#define FMT_DBL   1 
#define FMT_QUAD  2 


// allocate vector and initialize from raw data. Return zero if failed.
int vector_allocate(tVecDescr* pVec, int N, int isAligned, int fmt, const void* initData);
void vector_free    (tVecDescr* pVec);
// the same but with a list of vectors, ended with NULL
void vectors_free (tVecDescr* pVec,...);
// allocate vectors
void vectors_allocate(int isAligned,int fmt, tVecDescr* pVec, int N, ... );

/*    get element of vector with given index */
int16_t* vector_get (const tVecDescr *pVec, int n);
int32_t* vector_getL(const tVecDescr *pVec, int n);
int64_t* vector_getLL(const tVecDescr *pVec, int n);
/*
    compare two vectors: 
    returns max diff

    vector_diffph_ returns difference in phases (not including 2*pi cycles)
*/
int32_t vector_diff(const void*x,const void*y,int N, int fmt);
int32_t vector_diff_(const tVecDescr* x,const tVecDescr* y);
int32_t vector_diff_sign_(const tVecDescr* x,const tVecDescr* y);
int32_t vector_diff_rel_(const tVecDescr* x,const tVecDescr* y);
int32_t vector_diffph_(const tVecDescr* x,const tVecDescr* y);
int32_t vector_diff_mantissa_(const tVecDescr* mant_x,const tVecDescr* exp_x,const tVecDescr* mant_y,const tVecDescr* exp_y);

/*
    function reads mtx seq files
*/
typedef struct
{
    FILE* f;
}
tSeqRd;


void SeqRdInit(tSeqRd* pRd,const char* fname);
void SeqRdClose(tSeqRd* pRd);
void SeqRdOmitComments(tSeqRd* pRd);
// raw vector reads: returns 1 if ok, 0 if file is ended
int SeqRdGet(tSeqRd* pRd,void* x, int N, int fmt);

// read vectors from seq. file
int vectors_read(tSeqRd* reader, tVecDescr* pVec, ...);

#endif
