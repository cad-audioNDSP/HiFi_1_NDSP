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
    WAV Writer
*/
#ifndef __WAVWRITER_H__
#define __WAVWRITER_H__
#include "NatureDSP_types.h"
#include <stdio.h>
/**
*   Opens file for writing, and appends WAV header template.
*   Overwrites existing files.
*   Return opened FILE handle.
* 
*   Example:
*
*   FILE * pWavFile = WAV_writerOpen("MyFile.wav");
*/
FILE * WAV_writerOpen(
    const char * pFileName      //!< File name to create.
    );

/**
*   Writes WAV header and close the file. Assumes file was opened with 
*   WAV_writerOpen().
*   @return 1 in case of success, or 0 in case of write error or invalid params.
*
*   \warning If RTL ftell() badly implemented, use WAV_writeHeaderEx() 
* 
*   Example:
*
*   WAV_writerClose(pWavFile, 44100, 2, 16); // CD-format audio
*/
int WAV_writerClose(
    FILE * pF,                  //!< FILE handle
    unsigned long hz,           //!< Sample rate, Hz (44100, ...)
    unsigned int  ch,           //!< Number of channels (1,2 ...)
    unsigned int  bips          //!< Bits per sample (8,16 ...)
    );



#endif //__WAVWRITER_H__
