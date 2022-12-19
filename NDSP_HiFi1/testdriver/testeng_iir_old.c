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
 * Test-engine add-on for old IIR categories 
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"
#include "testeng_iir_old.h"
#include <string.h>
#include <stdlib.h>

#define MIN(a,b)   ( (a)<(b) ? (a) : (b) )
#define MAX(a,b)   ( (a)>(b) ? (a) : (b) )

typedef struct
{
    int  M;           /* number of sections          */
    int  L;           /* number of streams           */
    void* objMem;     /* allocated memory for filter */
    void *handle;     /* filter handle               */
}
tIirOldContext;

/* function reads impulse response from file and creates IIR structure. returns 0 if failed */
int te_create_iir_old(tTestEngContext * context)
{
    int fmt;
    int res;
    size_t szObj;
    tVec coef,gain,scale;
    tIirOldDescr *api;
    int M, L, coefLen=0;
    tIirOldContext *iirContext;
    if (seqFileScanf(context->seqFile, "%d %d", &M, &L) != 2)
    {
        printf("bad SEQ-file format\n");
        return 0;
    }
    ASSERT(L==1 || L==3);
    iirContext = (tIirOldContext *)malloc(sizeof(tIirOldContext));
    context->target.handle = (void*)iirContext;
    if (iirContext==NULL) return 0;
    memset(iirContext, 0, sizeof(*iirContext));
    iirContext->M=M;
    iirContext->L=L;
    switch(context->desc->extraParam & (3<<8))
    {
        case TE_IIR_DF1:
        case TE_IIR_DF2:
        case TE_IIR_DF2T:
            coefLen=M*5; break;
        default: ASSERT(0);
    }
    api=(tIirOldDescr *)context->target.fut;
    fmt = context->desc->extraParam & 0x0f;
    if(!NatureDSP_Signal_isPresent(api->init) ||
       !NatureDSP_Signal_isPresent(api->alloc) ||
       !NatureDSP_Signal_isPresent(api->process))
    {
        return -1;  // FUT is not defined
    }
    if (context->desc->extraParam & TE_IIR_FLOAT)
    {
        if (!vecAlloc(&coef , coefLen , 0, fmt, NULL)) return 0;
        if (!vecAlloc(&gain ,0        , 0, FMT_FRACT16, NULL)) return 0;
        if (!vecAlloc(&scale, 1       , 0, FMT_FLOAT32, NULL)) return 0;
        res = seqFileReadVec(context->seqFile, &coef) &
              seqFileReadVec(context->seqFile, &scale);
    }
    else
    {
        if (!vecAlloc(&coef , coefLen , 0, fmt, NULL)) return 0;
        if (!vecAlloc(&gain , M       , 0, FMT_FRACT16, NULL)) return 0;
        if (!vecAlloc(&scale, 1       , 0, FMT_FRACT16, NULL)) return 0;
        res = seqFileReadVec(context->seqFile, &coef) &
              seqFileReadVec(context->seqFile, &gain) & 
              seqFileReadVec(context->seqFile, &scale);
    }

    switch(context->desc->extraParam & (3<<8))
    {
        case TE_IIR_DF1:
        case TE_IIR_DF2:
        case TE_IIR_DF2T:
            szObj=api->alloc(M);
            iirContext->objMem=malloc(szObj);
            if (context->desc->extraParam & TE_IIR_FLOAT)
            {
                iirContext->handle=((tIirOldFxnInitFloat*)api->init)(iirContext->objMem,M,vecGetElem(&coef,0),(int16_t)*vecGetElem_fl32(&scale,0));
            }
            else
            {
                iirContext->handle=api->init(iirContext->objMem,M,vecGetElem(&coef,0),vecGetElem_fr16(&gain,0),*vecGetElem_fr16(&scale,0));
            }
            break;
    }
    vecsFree(&coef,&gain,&scale,NULL);
    return res;
}

/* function destroys IIR structure, returns 0 if failed */
int te_destroy_iir_old(tTestEngContext * context)
{
    tIirOldContext *iirContext;
    iirContext = (tIirOldContext *)context->target.handle;
    if (iirContext)
    {
        free(iirContext->objMem);
        free(iirContext);
    }
    return 1;
}

/* 
   Allocate vectors and load the data set for IIR:
*  vector X (in), vector Z (out) */
int te_loadFxn_iir_old(tTestEngContext * context)
{
    int type;
    int M, N, L;
    int nElemIn,nElemOut, res;
    tIirOldContext *iirContext;
    iirContext = (tIirOldContext *)context->target.handle;

    ASSERT(context && context->seqFile);

    M = context->args.M;
    N = context->args.N;
    L = iirContext->L;
    type=context->desc->fmt;

    nElemIn = MAX(0, M*N*L);
    nElemIn = nElemOut = MAX(0, M*N*L);

    memset(&context->dataSet, 0, sizeof(context->dataSet));

    /* Allocate data vectors memory. */
    res = (4 == vecsAlloc(context->desc->isAligned, type,
    &context->dataSet.X, nElemIn,
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
        printf("te_loadFxn_iir_old(): failed to read vectors data; "
            "fmt = 0x%02x, nElemIn = %d, nElemOut = %d\n",
            (unsigned)context->desc->fmt, nElemIn, nElemOut);
    }
    }
    else
    {
    printf("te_loadFxn_iir_old(): failed to allocate vectors; "
        "fmt = 0x%02x, nElemIn = %d, nElemOut = %d\n",
        (unsigned)context->desc->fmt, nElemIn, nElemOut);
    }

    /* Free vectors data if failed. */
    if (!res) freeVectors(context);
    return (res);

} /* te_loadFxn_iir_old() */


/* Apply IIR function to the test case data set.
*  vector X (in), vector Z (out) */
void te_processFxn_iir_old(tTestEngContext * context)
{
    //typedef void tFxn(const void * X, void * Z, int N, void* handle);
    void *X, *Z;
    tIirOldContext* iirContext;
    tIirOldDescr *api;
    int N;

    ASSERT(context && context->target.fut);

    iirContext = (tIirOldContext *)context->target.handle;
    api=(tIirOldDescr *)context->target.fut;
    X = vecGetElem(&context->dataSet.X, 0);
    Z = vecGetElem(&context->dataSet.Z, 0);

    N = context->args.N;
    /* call IIR function */
    if (context->desc->extraParam & TE_IIR_FLOAT)
    {
        ((tIirOldFxnProcessFloat*)api->process)(iirContext->handle,Z,X,N);
    }
    else
    {
        tAllocPtr scratch;
        void* pScr;
        /* allocate scratch */
        pScr=mallocAlign(&scratch,api->getScratch(N,iirContext->M),8);
        api->process(iirContext->handle,pScr,Z,X,N);
        /* free scratch */
        freeAlign(&scratch);
    }
} /* te_processFxn_iir_old() */
