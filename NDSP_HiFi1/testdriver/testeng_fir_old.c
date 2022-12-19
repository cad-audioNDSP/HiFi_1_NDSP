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
 * Test-engine add-on for older FIR API
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"
#include "testeng.h"
#include "testeng_fir.h"
#include "testeng_fir_old.h"
#include "vectools.h"
#include <string.h>
#include <stdlib.h>

#define MIN(a,b)   ( (a)<(b) ? (a) : (b) )
#define MAX(a,b)   ( (a)>(b) ? (a) : (b) )


typedef struct
{
    tVec ir;          /* impulse response */
    int  L;           /* number of streams */
    int  D;           /* interpolation/decimation ratio */
    void *firMem;     /* memory allocated for FIR */ 
    void *handle;     /* filter handle */
}
tFirOldContext;

/* function reads impulse response from file and creates FIR structure. returns 0 if failed */
int te_create_fir_old(tTestEngContext * context)
{
    int res;
    size_t szObj;
    int M, L, D, irLen=0;
    tFirOldContext *firContext;
    if (seqFileScanf(context->seqFile, "%d %d %d ", &M, &L, &D) != 3)
    {
        printf("bad SEQ-file format\n");
        return 0;
    }
    firContext = (tFirOldContext *)malloc(sizeof(tFirOldContext));
    context->target.handle = (void*)firContext;
    if (firContext==NULL) return 0;
    memset(firContext, 0, sizeof(*firContext));
    firContext->D=D;
    firContext->L=L;
    switch(context->desc->extraParam & (3<<8))
    {
        case TE_FIR_FIR: irLen=M;  break;
        case TE_FIR_DN:  irLen=M;  break;
        case TE_FIR_UP:  irLen=M*D;break;
        default: ASSERT("wrong extraParam");
    }
    if (!vecAlloc(&firContext->ir, irLen, context->desc->isAligned, context->desc->extraParam & 0x1F, NULL)) return 0;
    if (!NatureDSP_Signal_isPresent(((tFirOldDescr*)context->target.fut)->alloc) ||
        !NatureDSP_Signal_isPresent((((tFirOldDescr*)context->target.fut)->init)))
    {
        firContext->firMem=malloc(1);    // FUT is not defined
        return -1; 
    }
    if (D!=1) szObj=((tFirOldFxnAllocD*)((tFirOldDescr*)context->target.fut)->alloc)(D,M);
    else      szObj=((tFirOldDescr*)context->target.fut)->alloc(M);
    firContext->firMem=malloc(szObj);
    res=seqFileReadVec(context->seqFile, &firContext->ir);
    if (D!=1)  firContext->handle=((tFirOldFxnInitD*)((tFirOldDescr*)context->target.fut)->init)(firContext->firMem,D,M,vecGetElem(&firContext->ir, 0));
    else       firContext->handle=((tFirOldDescr*)context->target.fut)->init(firContext->firMem,M,vecGetElem(&firContext->ir, 0));

    return res;
}
/*Similar to te_create_fir_old() but with lower 8 bits of each impulse response buffer element zeroed*/
int te_create_fir_old_24(tTestEngContext * context)
{
    int res;
    size_t szObj;
    int M, L, D, irLen=0, i;
    tFirOldContext *firContext;
    if (seqFileScanf(context->seqFile, "%d %d %d ", &M, &L, &D) != 3)
    {
        printf("bad SEQ-file format\n");
        return 0;
    }
    firContext = (tFirOldContext *)malloc(sizeof(tFirOldContext));
    context->target.handle = (void*)firContext;
    if (firContext==NULL) return 0;
    memset(firContext, 0, sizeof(*firContext));
    firContext->D=D;
    firContext->L=L;
    switch(context->desc->extraParam & (3<<8))
    {
        case TE_FIR_FIR: irLen=M;  break;
        case TE_FIR_DN:  irLen=M;  break;
        case TE_FIR_UP:  irLen=M*D;break;
        default: ASSERT("wrong extraParam");
    }
    if (!vecAlloc(&firContext->ir, irLen, context->desc->isAligned, context->desc->extraParam & 0x1F, NULL)) return 0;
    if (!NatureDSP_Signal_isPresent(((tFirOldDescr*)context->target.fut)->alloc) ||
        !NatureDSP_Signal_isPresent((((tFirOldDescr*)context->target.fut)->init)))
    {
        firContext->firMem=malloc(1);    // FUT is not defined
        return -1; 
    }
    if (D!=1) szObj=((tFirOldFxnAllocD*)((tFirOldDescr*)context->target.fut)->alloc)(D,M);
    else      szObj=((tFirOldDescr*)context->target.fut)->alloc(M);
    firContext->firMem=malloc(szObj);
    res=seqFileReadVec(context->seqFile, &firContext->ir);
	for(i=0;i<(firContext->ir.nElem);i++) ((int32_t *)((uint8_t*)firContext->ir.ptr.aligned + (firContext->ir.offset )*firContext->ir.szElem))[i]&=0xFFFFFF00;
    if (D!=1)  firContext->handle=((tFirOldFxnInitD*)((tFirOldDescr*)context->target.fut)->init)(firContext->firMem,D,M,vecGetElem(&firContext->ir, 0));
    else       firContext->handle=((tFirOldDescr*)context->target.fut)->init(firContext->firMem,M,vecGetElem(&firContext->ir, 0));
    return res;
}
/* function destroys FIR structure, returns 0 if failed */
int te_destroy_fir_old(tTestEngContext * context)
{
    tFirOldContext *firContext;
    firContext = (tFirOldContext *)context->target.handle;
    if (firContext)
    {
        ASSERT(firContext->firMem);
        free(firContext->firMem);
        free(firContext);
    }
    return 1;
}

/* 
   Allocate vectors and load the data set for FIR:
*  vector X (in), vector Z (out) */
int te_loadFxn_fir_old(tTestEngContext * context)
{
    int fmtX;
    int M, N, L,D;
    int nElemIn=0,nElemOut=0, res;
    tFirOldContext *firContext;
    firContext = (tFirOldContext *)context->target.handle;

    ASSERT(context && context->seqFile);

    M = context->args.M;
    N = context->args.N;
    L = firContext->L;
    D = firContext->D;

    nElemIn = MAX(0, M*N*L);
    switch(context->desc->extraParam & (3<<8))
    {
        case TE_FIR_FIR: nElemIn = nElemOut = MAX(0, M*N*L)          ; break;
        case TE_FIR_DN:  nElemIn = MAX(0, M*N*L); nElemOut =nElemIn/D; break;
        case TE_FIR_UP:  nElemIn = MAX(0, M*N*L); nElemOut =nElemIn*D; break;
        default: ASSERT("wrong extraParam");
    }

    memset(&context->dataSet, 0, sizeof(context->dataSet));
    fmtX=(context->desc->fmt);
    /* Allocate data vectors memory. */
    res = (1 == vecsAlloc(context->desc->isAligned, fmtX,
                            &context->dataSet.X, nElemIn, 0));
    res &= (3 == vecsAlloc(0, fmtX,
                            &context->dataSet.Z, nElemOut,
                            &context->dataSet.Zlo, nElemOut,
                            &context->dataSet.Zhi, nElemOut, 0));
    if (res)
    {
    /* Load vectors data from the SEQ-file. */
    if (!(res = seqFileReadVecs(context->seqFile,
            &context->dataSet.X,
            &context->dataSet.Zlo,
            &context->dataSet.Zhi, 0)))
    {
        printf("te_loadFxn_fir_old(): failed to read vectors data; "
                "fmt = 0x%02x, nElemIn = %d, nElemOut = %d\n",
                (unsigned)context->desc->fmt, nElemIn, nElemOut);
    }
    }
    else
    {
    printf("te_loadFxn_fir_old(): failed to allocate vectors; "
            "fmt = 0x%02x, nElemIn = %d, nElemOut = %d\n",
            (unsigned)context->desc->fmt, nElemIn, nElemOut);
    }

    /* Free vectors data if failed. */
    if (!res) freeVectors(context);
    return (res);

} /* te_loadFxn_fir_old() */
/*Function similar to te_loadFxn_fir_old() but with lower 8 bits in each input buffer element is zeroed*/
int te_loadFxn_fir_old_24(tTestEngContext * context)
{
    int fmtX;
    int M, N, L,D, i;
    int nElemIn=0,nElemOut=0, res;
    tFirOldContext *firContext;
    firContext = (tFirOldContext *)context->target.handle;

    ASSERT(context && context->seqFile);

    M = context->args.M;
    N = context->args.N;
    L = firContext->L;
    D = firContext->D;

    nElemIn = MAX(0, M*N*L);
    switch(context->desc->extraParam & (3<<8))
    {
        case TE_FIR_FIR: nElemIn = nElemOut = MAX(0, M*N*L)          ; break;
        case TE_FIR_DN:  nElemIn = MAX(0, M*N*L); nElemOut =nElemIn/D; break;
        case TE_FIR_UP:  nElemIn = MAX(0, M*N*L); nElemOut =nElemIn*D; break;
        default: ASSERT("wrong extraParam");
    }

    memset(&context->dataSet, 0, sizeof(context->dataSet));
    fmtX=(context->desc->fmt);
    /* Allocate data vectors memory. */
    res = (1 == vecsAlloc(context->desc->isAligned, fmtX,
                            &context->dataSet.X, nElemIn, 0));
    res &= (3 == vecsAlloc(0, fmtX,
                            &context->dataSet.Z, nElemOut,
                            &context->dataSet.Zlo, nElemOut,
                            &context->dataSet.Zhi, nElemOut, 0));
    if (res)
    {
    /* Load vectors data from the SEQ-file. */
    if (!(res = seqFileReadVecs(context->seqFile,
            &context->dataSet.X,
            &context->dataSet.Zlo,
            &context->dataSet.Zhi, 0)))
    {
        printf("te_loadFxn_fir_old(): failed to read vectors data; "
                "fmt = 0x%02x, nElemIn = %d, nElemOut = %d\n",
                (unsigned)context->desc->fmt, nElemIn, nElemOut);
    }
	else
	{
		for(i=0;i<(context->dataSet.X.nElem);i++) ((int32_t *)((uint8_t*)context->dataSet.X.ptr.aligned + (context->dataSet.X.offset )*context->dataSet.X.szElem))[i]&=0xFFFFFF00;
	}
    }
    else
    {
    printf("te_loadFxn_fir_old(): failed to allocate vectors; "
            "fmt = 0x%02x, nElemIn = %d, nElemOut = %d\n",
            (unsigned)context->desc->fmt, nElemIn, nElemOut);
    }

    /* Free vectors data if failed. */
    if (!res) freeVectors(context);
    return (res);

}

/* Apply FIR function to the test case data set.
*  vector X (in), vector Z (out) */
void te_processFxn_fir_old(tTestEngContext * context)
{
    tFirOldFxnProcess *fxn;
    void *X, *Z;
    tFirOldContext* firContext;
    int D,N;

    ASSERT(context && context->target.fut);

    firContext = (tFirOldContext *)context->target.handle;
    X = vecGetElem(&context->dataSet.X, 0);
    Z = vecGetElem(&context->dataSet.Z, 0);

    N = context->args.N;
    int t=N;
    D = firContext->D;
    switch(context->desc->extraParam & (3<<8))
    {
        case TE_FIR_FIR: N=t;  break;
        case TE_FIR_DN:  N=t/D; break;
        case TE_FIR_UP:  N=t;   break;
        default: ASSERT("wrong extraParam");
    }

    fxn = ((tFirOldDescr*)context->target.fut)->process;
    fxn(firContext->handle, Z,X,N);
} /* te_processFxn_fir_old() */
