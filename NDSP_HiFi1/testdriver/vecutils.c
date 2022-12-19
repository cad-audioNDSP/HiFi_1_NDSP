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

#include "NatureDSP_types.h"
#include "vecutils.h"
#include "malloc16.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

/*  
	Helper functions for reading data vectors from .seq files
	Integrit, 2006-2015
*/



void SeqRdOmitComments(tSeqRd* pRd)
{
    int isComment;
    long pos;
    if (pRd->f==NULL) return;
    do
    {
        char s[256],*p;
        pos=ftell(pRd->f);
        if (fgets(s,255,pRd->f)==NULL) break;
        s[strcspn(s,";\n\r")]=0; // remove trailing line terminators and comments
        p=s+strspn(s," \t");     // remove leading spaces
        isComment = p[0]==0;
    }
    while(isComment);
    fseek(pRd->f,pos,SEEK_SET);
}

// raw vector reads: returns 1 if ok, 0 if file is ended
int SeqRdGet(tSeqRd* pRd,void* x, int N, int fmt)
{
    int16_t* sx=(int16_t*)x;
    int32_t* dx=(int32_t*)x;
    int64_t* wx=(int64_t*)x;
    int k,m,n;
    int32_t* submtx;
    int64_t* submtxw;
    ASSERT(pRd->f);
    submtx = (int32_t*)calloc(N,sizeof(int32_t));
    submtxw = (int64_t*)calloc(N,sizeof(int64_t));
    SeqRdOmitComments(pRd);
    if(feof(pRd->f)) return 0;
        for (m=0; m<N; m++) 
        {
            int64_t lval;
            k=fscanf(pRd->f,"%lld",&lval);
            if  (k<1) 
            {
                printf("\nmatrix read error\n");
                ASSERT (0);
            }
            submtx[m]=(int32_t)lval;
            submtxw[m]=lval;
            if(((fmt & FMT_DBL )==0)&&((fmt & FMT_QUAD )==0))
            {
                if (lval>MAX_INT16 || lval <MIN_INT16)
                {
                    printf("\nmatrix read error (single precision value %lld is out of range)\n",lval);
                    ASSERT (0);
                }
            }
        }
        switch(fmt)
        {
        case 0:
            // block order, real data
            for (n=0; n<N; n++) sx[n]=(int16_t)submtx[n];
            break;
        case 0+FMT_DBL:
            // block order, real data
            for (n=0; n<N; n++) dx[n]=submtx[n];
            break;
        case 0+FMT_QUAD:
            // block order, real data
            for (n=0; n<N; n++) wx[n]=submtxw[n];
            break;
        default:
            ASSERT(0); //not implemented yet
        }
    free(submtx);
    free(submtxw);
    return 1;
}

void SeqRdInit(tSeqRd* pRd,const char* fname)
{
    pRd->f=fopen(fname,"rb");
    SeqRdOmitComments(pRd);
}

void SeqRdClose(tSeqRd* pRd)
{
    if (pRd->f) fclose(pRd->f);
}


// allocate matrix. Return zero if failed.
int vector_allocate(tVecDescr* pVec, int N, int isAligned, int fmt, const void* initData)
{
    static int alignOffset=0;
    int szElem=sizeof(int16_t);
    int S=N,sz;
    if (!isAligned)
    {
        alignOffset=(alignOffset+1);
        if(alignOffset>5) alignOffset=1;
        S=S+alignOffset;
    }
    if (fmt & FMT_DBL ) szElem*=2;
    if (fmt & FMT_QUAD ) szElem*=4;
    sz=S*szElem;
    if ( !mallocAlign(&pVec->ptr,sz,16) ) return 0;
    pVec->N=N;
    pVec->offset=isAligned?0:alignOffset;
    pVec->fmt=fmt;
    if (initData==NULL) return 1;
    {
        const uint8_t *src=(const uint8_t *)initData;
        int n;
        for (n=0; n<N; n++)
        {
            memcpy(vector_get(pVec,n),src,szElem);
            src+=szElem;
        }
    }
    return 1;
}

void vector_free    (tVecDescr* pVec)
{
    freeAlign(&pVec->ptr);
    memset(pVec,0,sizeof(*pVec));
}

// the same but with a list of vectors, ended with NULL
void vectors_free (tVecDescr* pVec,...)
{
    va_list list;
    va_start(list,pVec);
    do 
    {
        vector_free(pVec);
        pVec=va_arg(list,tVecDescr*);
    }
    while(pVec!=NULL);
    va_end(list);
}

// allocate vectors
void vectors_allocate(int isAligned,int fmt, tVecDescr* pVec, int N, ... )
{
    va_list list;
    va_start(list,N);
    do 
    {
        vector_allocate(pVec, N, isAligned, fmt, NULL);
        pVec=va_arg(list,tVecDescr*);
        if (pVec==NULL) break;
        N   =va_arg(list,int);
    }
    while(pVec!=NULL);
    va_end(list);
}
// read vectors from seq. file
int vectors_read(tSeqRd* reader, tVecDescr* pVec, ...)
{
    int ok=1;
    va_list list;
    va_start(list,pVec);
    do 
    {
        SeqRdOmitComments(reader);
        ok &= SeqRdGet(reader, vector_get(pVec,0), pVec->N, pVec->fmt);
        pVec=va_arg(list,tVecDescr*);
    }
    while(pVec!=NULL && ok);
    va_end(list);
    return ok;
}

int16_t* vector_get (const tVecDescr *pVec, int n)
{
    int16_t *x=(int16_t *)pVec->ptr.aligned; 
    NASSERT(n<=pVec->N);
    n+=pVec->offset;
    if (pVec->fmt==FMT_DBL) n<<=1;
    if (pVec->fmt==FMT_QUAD) n<<=2;
    return x+n;
}

int32_t* vector_getL (const tVecDescr *pVec, int n)
{
    return (int32_t*)vector_get(pVec,n);
}

int64_t* vector_getLL (const tVecDescr *pVec, int n)
{
    return (int64_t*)vector_get(pVec,n);
}

/*
    compare two vectors: 
    returns max diff
*/
int32_t vector_diff(const void*x,const void*y,int N, int fmt)
{
    int n;
    int32_t d,maxdiff=0;
    const int32_t *lx=(const int32_t *)x,*ly=(const int32_t *)y;
    const int16_t *sx=(const int16_t *)x,*sy=(const int16_t *)y;

    if (fmt & FMT_DBL)
    {
        for (n=0;n<N;n++) 
        {
            d=labs((int32_t)lx[n]-ly[n]);
            maxdiff=MAX(d,maxdiff);
        }
    }
    else
    {
        for (n=0;n<N;n++) 
        {
            d=labs((int32_t)sx[n]-sy[n]);
            maxdiff=MAX(d,maxdiff);
        }
    }
    return maxdiff;
}

int32_t vector_diff_(const tVecDescr* x,const tVecDescr* y)
{
    int N=MIN(x->N,y->N);
    ASSERT(x->fmt==y->fmt);
    return vector_diff(vector_get(x,0),vector_get(y,0),N, x->fmt);
}

int32_t vector_diff_sign(const void*x,const void*y,int N, int fmt)
{
    int n;
    int32_t d,maxdiff=0;
    const int32_t *lx=(const int32_t *)x,*ly=(const int32_t *)y;
    const int16_t *sx=(const int16_t *)x,*sy=(const int16_t *)y;

    if (fmt & FMT_DBL)
    {
        for (n=0;n<N;n++) 
        {
            d=((uint32_t)(lx[n]^ly[n]))>>31;
            d&=1;
            maxdiff=MAX(d,maxdiff);
        }
    }
    else
    {
        for (n=0;n<N;n++) 
        {
            d=((uint32_t)(sx[n]^sy[n]))>>15;
            d&=1;
            maxdiff=MAX(d,maxdiff);
        }
    }
    return maxdiff;
}

int32_t vector_diff_sign_(const tVecDescr* x,const tVecDescr* y)
{
    int N=MIN(x->N,y->N);
    ASSERT(x->fmt==y->fmt);
    return vector_diff_sign(vector_get(x,0),vector_get(y,0),N, x->fmt);
}

int32_t vector_diff_rel(const void*x,const void*y,int N, int fmt)
{
    int n,k;
    int32_t d,maxdiff=0;
    const int32_t *lx=(const int32_t *)x,*ly=(const int32_t *)y;
    const int16_t *sx=(const int16_t *)x,*sy=(const int16_t *)y;

    if (fmt & FMT_DBL)
    {
        for (n=0;n<N;n++) 
        {
            int32_t a,b,dd;
            a=lx[n]; b=ly[n];
            dd=(a<b)? b-a : a-b;
            // shift left to fit mantissas into 31 bit
            for(k=0;k<31;k++)
            {
                if((((a^(a<<1))>>31)&1) ||
                   (((b^(b<<1))>>31)&1)) break;
                a<<=1;
                b<<=1;
            }
            d = (a<b)? b-a : a-b;
            if (dd<2) d=MIN(d,dd);
            maxdiff=MAX(d,maxdiff);
        }
    }
    else
    {
        for (n=0;n<N;n++) 
        {
            int32_t a,b;
            a=sx[n]; b=sy[n];
            // shift left to fit mantissas into 31 bit
            for(k=0;k<31;k++)
            {
                if((((a^(a<<1))>>31)&1) ||
                   (((b^(b<<1))>>31)&1)) break;
                a<<=1;
                b<<=1;
            }
            d=a-b;
            maxdiff=MAX(d,maxdiff);
        }
    }
    return maxdiff;
}

int32_t vector_diff_rel_(const tVecDescr* x,const tVecDescr* y)
{
    int N=MIN(x->N,y->N);
    ASSERT(x->fmt==y->fmt);
    return vector_diff_rel(vector_get(x,0),vector_get(y,0),N, x->fmt);
}


static int32_t vector_diffph(const void*x,const void*y,int N, int fmt)
{
    int n;
    int32_t d,maxdiff=0;
    const int32_t *lx=(const int32_t *)x,*ly=(const int32_t *)y;
    const int16_t *sx=(const int16_t *)x,*sy=(const int16_t *)y;

    if (fmt & FMT_DBL)
    {
        for (n=0;n<N;n++) 
        {
            d=labs((int32_t)lx[n]-ly[n]);
            maxdiff=MAX(d,maxdiff);
        }
    }
    else
    {
        for (n=0;n<N;n++) 
        {
            d=labs((int16_t)((int16_t)sx[n]-sy[n]));
            maxdiff=MAX(d,maxdiff);
        }
    }
    return maxdiff;
}

int32_t vector_diffph_(const tVecDescr* x,const tVecDescr* y)
{
    int N=MIN(x->N,y->N);
    ASSERT(x->fmt==y->fmt);
    return vector_diffph(vector_get(x,0),vector_get(y,0),N, x->fmt);
}

/*
    compare mantissa of 2 vectors: 
    returns max diff
*/
static int32_t mant_diff(int32_t x,int32_t ex,int32_t y,int32_t ey)
{
    int e;
    int64_t a,b;
    e=MIN(ex,ey);
    ex=ex-e;
    ey=ey-e;
    a=((int64_t)x)<<ex;
    b=((int64_t)y)<<ey;
    a=a-b;
    if (a<0) a=-a;
    if (a>MAX_INT32) a=MAX_INT32;
    if (a<MIN_INT32) a=MIN_INT32;
    return (int32_t)a;
}

static int32_t vector_diff_mantissa(const void*mant_x,const int16_t* exp_x,const void*mant_y,const int16_t* exp_y,int N, int fmt)
{
    int n;
    int32_t d,maxdiff=0;
    const int32_t *lx=(const int32_t *)mant_x,*ly=(const int32_t *)mant_y;
    const int16_t *sx=(const int16_t *)mant_x,*sy=(const int16_t *)mant_y;

    if (fmt & FMT_DBL)
    {
        for (n=0;n<N;n++) 
        {
            d=mant_diff(lx[n],exp_x[n],ly[n],exp_y[n]);
            maxdiff=MAX(d,maxdiff);
        }
    }
    else
    {
        for (n=0;n<N;n++) 
        {
            d=mant_diff(sx[n],exp_x[n],sy[n],exp_y[n]);
            maxdiff=MAX(d,maxdiff);
        }
    }
    return maxdiff;
}

int32_t vector_diff_mantissa_(const tVecDescr* mant_x,const tVecDescr* exp_x,const tVecDescr* mant_y,const tVecDescr* exp_y)
{
    int N=MIN(mant_x->N,mant_y->N);
    ASSERT(mant_x->fmt==mant_y->fmt);
    ASSERT(exp_x->fmt==FMT_SNGL);
    ASSERT(exp_y->fmt==FMT_SNGL);
    return vector_diff_mantissa(vector_get(mant_x,0),vector_get(exp_x,0),vector_get(mant_y,0),vector_get(exp_y,0),N, mant_y->fmt);
}
