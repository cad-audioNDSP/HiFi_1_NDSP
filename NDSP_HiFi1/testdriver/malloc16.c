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
    simplest aligned allocator. 
    Helps in allocation data aligned by 16-byte boundary
*/
#include "NatureDSP_types.h"
#include "malloc16.h"
#include <stdlib.h>

/*
    function returns aligned (by align bytes) memory block 
*/
void* mallocAlign(tAllocPtr* ptr, size_t sz, size_t align)
{
    uintptr_t a;
    if ( !( ptr->allocated=malloc(sz+align+2) ) ) return 0;
    ptr->sz=sz;
    ptr->align=align;
    align-=1;
    a = (uintptr_t)ptr->allocated;
    a = (a+(align))&(~(align));
    ptr->aligned = (void*)a;
    ASSERT(a);
    {
        char *beef;
        beef=(char *)(ptr->allocated);
        beef+=ptr->sz+ptr->align;
        beef[0]=(char)0xBE; beef[1]=(char)0xEF;
    }
    return ptr->aligned;
}

void freeAlign   (tAllocPtr* ptr)
{
    ASSERT(ptr->allocated);
    ASSERT(ptr->aligned);
    {
        const char *beef;
        beef=(char *)(ptr->allocated);
        beef+=ptr->sz+ptr->align;
        ASSERT(beef[0]==(char)0xBE);
        ASSERT(beef[1]==(char)0xEF);
    }
    free(ptr->allocated);
}
