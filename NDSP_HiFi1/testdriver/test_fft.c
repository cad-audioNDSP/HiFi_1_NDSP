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
    test module for testing FFT part of NatureDSP_Signal library
*/
#include "config.h"
#include "NatureDSP_Signal.h"
#include "utils.h"
#include "vecutils.h"
#include "baseop.h"

#ifndef TE_SANITY_VECTOR_DIR
#define TE_SANITY_VECTOR_DIR   "./vectors_sanity/"
#endif

int32_t maxdiff_cplx_16x16 [4][11] =    {{0,0,0,0,0,0,0,0,0,0,0 },
                                        {0,0,0,0,0,0,0,0,0,0,0 },
                                        {0,0,0,0,1,1,1,1,0,0,0 },
                                        {0,0,0,0,15,15,15,15,20,20,0 }};
int32_t maxdiff_real_16x16 [4][11] =    {{0,0,0,0,0,0,0,0,0,0,0 },
                                        {0,0,0,0,0,0,0,0,0,0,0 },
                                        {0,0,0,0,0,2,1,2,1,1,0},
                                        {0,0,0,0,0,8,6,7,7,8,8}};
int32_t maxdiff_cplx_32x16 [4][11] =    {{0,0,0,0,0,0,0,0,0,0,0 },
                                        {0,0,0,0,0,0,0,0,0,0,0 },
                                        {0,0,0,0,0,0,0,0,0,0,0 },
                                        {0,0,0,0,9000,7000,4000,3000,1500,1000,0 }};
int32_t maxdiff_real_32x16 [4][11] =    {{0,0,0,0,0,0,0,0,0,0,0 },
                                        {0,0,0,0,0,0,0,0,0,0,0 },
                                        {0,0,0,0,0,0,0,0,0,0,0 },
                                        {0,0,0,0,0,5000,5000,2000,2000,1000,1000}};
int32_t maxdiff_cplx_24x24 [4][11] =    {{0,0,0,0,1100,1300,1300,1700,1700,2000,0 },
                                        {0,0,0,0,1700,1800,1700,1700,1700,1700,0 },
                                        {0,0,0,0,1700,1800,1700,1700,1700,1700,0 },
                                        {0,0,0,0,6000,17000,8000,20000,9000,23000,0 }};
int32_t maxdiff_real_24x24 [4][11] =    {{0,0,0,0,0,800,900,900,1100,1100,2000 },
                                        {0,0,0,0,0,2864,2804,2708,2575,1628,2000 },
                                        {0,0,0,0,0,2864,2804,2708,2575,1628,2000 },
                                        {0,0,0,0,0,45000,60000,60000,71000,71000,82350 }};

typedef int (*fnfft)(void* y,void* x,fft_handle_t,int scalingOption);
typedef void (*fnshift) (void * y,void * x, int t,int N);
typedef int32_t (*maxdiff_tbl)[11];

typedef struct
{
    const char* name;
    int     fmt;
    fnfft   fft;
    fnfft   ifft;
    fnfft   rfft;
    fnfft   rifft;
    fnshift shift;
    maxdiff_tbl     maxdiff_cplx;
    maxdiff_tbl     maxdiff_real;
}
tDescr;

tDescr descr24x24=
{
    "24x24",
    FMT_DBL,
    (fnfft)fft_cplx24x24,
    (fnfft)ifft_cplx24x24,
    (fnfft)fft_real24x24,
    (fnfft)ifft_real24x24,
    (fnshift)vec_shift24x24,
    maxdiff_cplx_24x24,
    maxdiff_real_24x24
};

tDescr descr32x16=
{
    "32x16",
    FMT_DBL,
    (fnfft)fft_cplx32x16,
    (fnfft)ifft_cplx32x16,
    (fnfft)fft_real32x16,
    (fnfft)ifft_real32x16,
    (fnshift)vec_shift32x32,
    maxdiff_cplx_32x16,
    maxdiff_real_32x16
};

tDescr descr16x16=
{
    "16x16",
    FMT_SNGL,
    (fnfft)fft_cplx16x16,
    (fnfft)ifft_cplx16x16,
    (fnfft)fft_real16x16,
    (fnfft)ifft_real16x16,
    (fnshift)vec_shift16x16,
    maxdiff_cplx_16x16,
    maxdiff_real_16x16
};

// type==16: 16x16, type==24: 24x24, type==32: 32x16
int test_fft_cplx(const char *pNameIn, const char *pNameOut, int log2FFTsize, int scalingOpt, int type)
{
    int res = 1;
    int testCase=0;
    tDescr *descr;
    int32_t diff,maxdiff=0;
    int N = 1<<log2FFTsize;
    int hr = TEST_OK;
    FILE *pFileIn = NULL, *pFileOut = NULL;
    unsigned long lHz = 8000;
    tVecDescr vInp0,vInpRef,vOut0;
    int32_t *inp0,*inpRef,*out0;
    fft_handle_t h_fft=NULL,h_ifft=NULL;

    switch(type)
    {
    case 16:    descr=&descr16x16; 
                switch(N)
                {
                case 16 : h_fft=cfft16x16_16 ; h_ifft=cifft16x16_16 ; break;
                case 32 : h_fft=cfft16x16_32 ; h_ifft=cifft16x16_32 ; break;
                case 64 : h_fft=cfft16x16_64 ; h_ifft=cifft16x16_64 ; break;
                case 128: h_fft=cfft16x16_128; h_ifft=cifft16x16_128; break;
                case 256: h_fft=cfft16x16_256; h_ifft=cifft16x16_256; break;
                case 512: h_fft=cfft16x16_512; h_ifft=cifft16x16_512; break;
                default:
                    ASSERT(0);  // fft handles not found
                }
                break;
    case 24:    descr=&descr24x24; 
                switch(N)
                {
                case 16 : h_fft=cfft24_16 ; h_ifft=cifft24_16 ; break;
                case 32 : h_fft=cfft24_32 ; h_ifft=cifft24_32 ; break;
                case 64 : h_fft=cfft24_64 ; h_ifft=cifft24_64 ; break;
                case 128: h_fft=cfft24_128; h_ifft=cifft24_128; break;
                case 256: h_fft=cfft24_256; h_ifft=cifft24_256; break;
                case 512: h_fft=cfft24_512; h_ifft=cifft24_512; break;
                default:
                    ASSERT(0);  // fft handles not found
                }
                break;
    case 32:    descr=&descr32x16; 
                switch(N)
                {
                case 16 : h_fft=cfft16_16 ; h_ifft=cifft16_16 ; break;
                case 32 : h_fft=cfft16_32 ; h_ifft=cifft16_32 ; break;
                case 64 : h_fft=cfft16_64 ; h_ifft=cifft16_64 ; break;
                case 128: h_fft=cfft16_128; h_ifft=cifft16_128; break;
                case 256: h_fft=cfft16_256; h_ifft=cifft16_256; break;
                case 512: h_fft=cfft16_512; h_ifft=cifft16_512; break;
                default:
                    ASSERT(0);  // fft handles not found
                }
                break;
    default:    descr=0; assert(!"Unsupported FFT type"); abort();
    }

    vectors_allocate(1,descr->fmt, &vInp0, 2*N, &vInpRef, 2*N, &vOut0, 2*N,NULL);
    inp0  =vector_getL(&vInp0,0);
    inpRef=vector_getL(&vInpRef,0);
    out0  =vector_getL(&vOut0,0);

    printf("Complex FFT/IFFT %s N=%d, scaling option %d: ",descr->name,1<<log2FFTsize,scalingOpt);

    if (!NatureDSP_Signal_isPresent(descr->fft) || !NatureDSP_Signal_isPresent(descr->ifft))
    {
      res = -1;
    }
    if (res>0)
    {
        int cnt = 0;
        int fft_sh, ifft_sh;

        WAVREADER_OPEN_24_STEREO(pFileIn,  pNameIn,  lHz)
        WAVWRITER_OPEN          (pFileOut, pNameOut);

        while(1) 
        {
            size_t szElem=descr->fmt==FMT_DBL ? sizeof(int32_t):sizeof(int16_t);
            testCase++;
            if (descr->fmt==FMT_DBL)
            {
                if(2*N != readpcm24((int32_t *)inp0, 2*N, pFileIn)) { break;}
            }
            else
            {
                int k;
                int16_t *inp=(int16_t *)inp0;
                int err=0;
                for (k=0; k<2*N; k++)
                {
                    int32_t t;
                    err |= readpcm24(&t, 1, pFileIn)!=1;
                    inp[k]=S_extract_l(t);
                }
                if (err) break;
            }
            memcpy(inpRef,inp0,2*N*szElem);
            fft_sh=ifft_sh=0;
            if (scalingOpt)
            {
//                vec_shift(inp0, inp0, -(log2FFTsize+1), 2*N);
#if 0
{
    static const int32_t a16[]={983040,0,-65536,-329472,-65536,-158218,-65536,-98082,-65536,-65536,-65536,-43790,-65536,-27146,-65536,-13036,-65536,0,-65536,13036,-65536,27146,-65536,43790,-65536,65536,-65536,98082,-65536,158218,-65536,329472};
    static const int32_t a32[]={8126464,0,-262144,-2661592,-262144,-1317888,-262144,-864176,-262144,-632872,-262144,-490440,-262144,-392328,-262144,-319424,-262144,-262144,-262144,-215136,-262144,-175160,-262144,-140120,-262144,-108584,-262144,-79520,-262144,-52144,-262144,-25816,-262144,0,-262144,25816,-262144,52144,-262144,79520,-262144,108584,-262144,140120,-262144,175160,-262144,215136,-262144,262144,-262144,319424,-262144,392328,-262144,490440,-262144,632872,-262144,864176,-262144,1317888,-262144,2661592};
    static const int32_t a64[]={16515072,0,-262144,-5336064,-262144,-2661592,-262144,-1767232,-262144,-1317888,-262144,-1046536,-262144,-864176,-262144,-732640,-262144,-632872,-262144,-554256,-262144,-490440,-262144,-437360,-262144,-392328,-262144,-353464,-262144,-319424,-262144,-289232,-262144,-262144,-262144,-237592,-262144,-215136,-262144,-194416,-262144,-175160,-262144,-157120,-262144,-140120,-262144,-123984,-262144,-108584,-262144,-93800,-262144,-79520,-262144,-65664,-262144,-52144,-262144,-38888,-262144,-25816,-262144,-12880,-262144,0,-262144,12880,-262144,25816,-262144,38888,-262144,52144,-262144,65664,-262144,79520,-262144,93800,-262144,108584,-262144,123984,-262144,140120,-262144,157120,-262144,175160,-262144,194416,-262144,215136,-262144,237592,-262144,262144,-262144,289232,-262144,319424,-262144,353464,-262144,392328,-262144,437360,-262144,490440,-262144,554256,-262144,632872,-262144,732640,-262144,864176,-262144,1046536,-262144,1317888,-262144,1767232,-262144,2661592,-262144,5336064};
    static const int32_t a128[]={133169152,0,-1048576,-42714240,-1048576,-21344256,-1048576,-14215200,-1048576,-10646368,-1048576,-8501632,-1048576,-7068928,-1048576,-6043104,-1048576,-5271552,-1048576,-4669536,-1048576,-4186144,-1048576,-3789056,-1048576,-3456704,-1048576,-3174080,-1048576,-2930560,-1048576,-2718336,-1048576,-2531488,-1048576,-2365536,-1048576,-2217024,-1048576,-2083168,-1048576,-1961760,-1048576,-1851008,-1048576,-1749440,-1048576,-1655872,-1048576,-1569312,-1048576,-1488864,-1048576,-1413856,-1048576,-1343616,-1048576,-1277696,-1048576,-1215584,-1048576,-1156928,-1048576,-1101344,-1048576,-1048576,-1048576,-998336,-1048576,-950368,-1048576,-904512,-1048576,-860544,-1048576,-818304,-1048576,-777664,-1048576,-738496,-1048576,-700640,-1048576,-664000,-1048576,-628480,-1048576,-594016,-1048576,-560480,-1048576,-527808,-1048576,-495936,-1048576,-464800,-1048576,-434336,-1048576,-404480,-1048576,-375200,-1048576,-346400,-1048576,-318080,-1048576,-290176,-1048576,-262656,-1048576,-235456,-1048576,-208576,-1048576,-181952,-1048576,-155552,-1048576,-129344,-1048576,-103264,-1048576,-77344,-1048576,-51520,-1048576,-25728,-1048576,0,-1048576,25728,-1048576,51520,-1048576,77344,-1048576,103264,-1048576,129344,-1048576,155552,-1048576,181952,-1048576,208576,-1048576,235456,-1048576,262656,-1048576,290176,-1048576,318080,-1048576,346400,-1048576,375200,-1048576,404480,-1048576,434336,-1048576,464800,-1048576,495936,-1048576,527808,-1048576,560480,-1048576,594016,-1048576,628480,-1048576,664000,-1048576,700640,-1048576,738496,-1048576,777664,-1048576,818304,-1048576,860544,-1048576,904512,-1048576,950368,-1048576,998336,-1048576,1048576,-1048576,1101344,-1048576,1156928,-1048576,1215584,-1048576,1277696,-1048576,1343616,-1048576,1413856,-1048576,1488864,-1048576,1569312,-1048576,1655872,-1048576,1749440,-1048576,1851008,-1048576,1961760,-1048576,2083168,-1048576,2217024,-1048576,2365536,-1048576,2531488,-1048576,2718336,-1048576,2930560,-1048576,3174080,-1048576,3456704,-1048576,3789056,-1048576,4186144,-1048576,4669536,-1048576,5271552,-1048576,6043104,-1048576,7068928,-1048576,8501632,-1048576,10646368,-1048576,14215200,-1048576,21344256,-1048576,42714240};
    static const int32_t a256[]={267386880,0,-1048576,-85441376,-1048576,-42714240,-1048576,-28469024,-1048576,-21344256,-1048576,-17067680,-1048576,-14215200,-1048576,-12176480,-1048576,-10646368,-1048576,-9455328,-1048576,-8501632,-1048576,-7720544,-1048576,-7068928,-1048576,-6516896,-1048576,-6043104,-1048576,-5631904,-1048576,-5271552,-1048576,-4953088,-1048576,-4669536,-1048576,-4415360,-1048576,-4186144,-1048576,-3978368,-1048576,-3789056,-1048576,-3615840,-1048576,-3456704,-1048576,-3309920,-1048576,-3174080,-1048576,-3048000,-1048576,-2930560,-1048576,-2820960,-1048576,-2718336,-1048576,-2622048,-1048576,-2531488,-1048576,-2446144,-1048576,-2365536,-1048576,-2289312,-1048576,-2217024,-1048576,-2148416,-1048576,-2083168,-1048576,-2021024,-1048576,-1961760,-1048576,-1905152,-1048576,-1851008,-1048576,-1799136,-1048576,-1749440,-1048576,-1701728,-1048576,-1655872,-1048576,-1611776,-1048576,-1569312,-1048576,-1528352,-1048576,-1488864,-1048576,-1450720,-1048576,-1413856,-1048576,-1378176,-1048576,-1343616,-1048576,-1310144,-1048576,-1277696,-1048576,-1246176,-1048576,-1215584,-1048576,-1185856,-1048576,-1156928,-1048576,-1128768,-1048576,-1101344,-1048576,-1074624,-1048576,-1048576,-1048576,-1023136,-1048576,-998336,-1048576,-974080,-1048576,-950368,-1048576,-927200,-1048576,-904512,-1048576,-882304,-1048576,-860544,-1048576,-839232,-1048576,-818304,-1048576,-797792,-1048576,-777664,-1048576,-757920,-1048576,-738496,-1048576,-719392,-1048576,-700640,-1048576,-682176,-1048576,-664000,-1048576,-646112,-1048576,-628480,-1048576,-611136,-1048576,-594016,-1048576,-577120,-1048576,-560480,-1048576,-544032,-1048576,-527808,-1048576,-511776,-1048576,-495936,-1048576,-480288,-1048576,-464800,-1048576,-449472,-1048576,-434336,-1048576,-419328,-1048576,-404480,-1048576,-389760,-1048576,-375200,-1048576,-360736,-1048576,-346400,-1048576,-332192,-1048576,-318080,-1048576,-304096,-1048576,-290176,-1048576,-276384,-1048576,-262656,-1048576,-249024,-1048576,-235456,-1048576,-221984,-1048576,-208576,-1048576,-195232,-1048576,-181952,-1048576,-168704,-1048576,-155552,-1048576,-142400,-1048576,-129344,-1048576,-116288,-1048576,-103264,-1048576,-90304,-1048576,-77344,-1048576,-64416,-1048576,-51520,-1048576,-38624,-1048576,-25728,-1048576,-12864,-1048576,0,-1048576,12864,-1048576,25728,-1048576,38624,-1048576,51520,-1048576,64416,-1048576,77344,-1048576,90304,-1048576,103264,-1048576,116288,-1048576,129344,-1048576,142400,-1048576,155552,-1048576,168704,-1048576,181952,-1048576,195232,-1048576,208576,-1048576,221984,-1048576,235456,-1048576,249024,-1048576,262656,-1048576,276384,-1048576,290176,-1048576,304096,-1048576,318080,-1048576,332192,-1048576,346400,-1048576,360736,-1048576,375200,-1048576,389760,-1048576,404480,-1048576,419328,-1048576,434336,-1048576,449472,-1048576,464800,-1048576,480288,-1048576,495936,-1048576,511776,-1048576,527808,-1048576,544032,-1048576,560480,-1048576,577120,-1048576,594016,-1048576,611136,-1048576,628480,-1048576,646112,-1048576,664000,-1048576,682176,-1048576,700640,-1048576,719392,-1048576,738496,-1048576,757920,-1048576,777664,-1048576,797792,-1048576,818304,-1048576,839232,-1048576,860544,-1048576,882304,-1048576,904512,-1048576,927200,-1048576,950368,-1048576,974080,-1048576,998336,-1048576,1023136,-1048576,1048576,-1048576,1074624,-1048576,1101344,-1048576,1128768,-1048576,1156928,-1048576,1185856,-1048576,1215584,-1048576,1246176,-1048576,1277696,-1048576,1310144,-1048576,1343616,-1048576,1378176,-1048576,1413856,-1048576,1450720,-1048576,1488864,-1048576,1528352,-1048576,1569312,-1048576,1611776,-1048576,1655872,-1048576,1701728,-1048576,1749440,-1048576,1799136,-1048576,1851008,-1048576,1905152,-1048576,1961760,-1048576,2021024,-1048576,2083168,-1048576,2148416,-1048576,2217024,-1048576,2289312,-1048576,2365536,-1048576,2446144,-1048576,2531488,-1048576,2622048,-1048576,2718336,-1048576,2820960,-1048576,2930560,-1048576,3048000,-1048576,3174080,-1048576,3309920,-1048576,3456704,-1048576,3615840,-1048576,3789056,-1048576,3978368,-1048576,4186144,-1048576,4415360,-1048576,4669536,-1048576,4953088,-1048576,5271552,-1048576,5631904,-1048576,6043104,-1048576,6516896,-1048576,7068928,-1048576,7720544,-1048576,8501632,-1048576,9455328,-1048576,10646368,-1048576,12176480,-1048576,14215200,-1048576,17067680,-1048576,21344256,-1048576,28469024,-1048576,42714240,-1048576,85441376};
    switch(N)
    {
    case 16:    memcpy(inp0,a16,N*2*sizeof(int32_t)); break;
    case 32:    memcpy(inp0,a32,N*2*sizeof(int32_t)); break;
    case 64:    memcpy(inp0,a64,N*2*sizeof(int32_t)); break;
    case 128:   memcpy(inp0,a128,N*2*sizeof(int32_t)); break;
    case 256:   memcpy(inp0,a256,N*2*sizeof(int32_t)); break;
    }
}
                fft_sh = descr->fft (out0, inp0, N,scalingOpt);
    {
        int k;
        for (k=0;k<N;k++)
        {
            printf("%d ",(out0[2*k+0]+0x400)>>12);
        }
        printf("\n");
        break;
    }
#endif
                fft_sh = descr->fft (out0, inp0, h_fft,scalingOpt);
                ifft_sh= descr->ifft(inp0, out0, h_ifft,scalingOpt);
                descr->shift(inp0, inp0, ifft_sh+fft_sh-log2FFTsize, 2*N);
            }
            else
            {
                ASSERT(descr->fmt==FMT_DBL);
                fft_sh  = log2FFTsize+1;
                ifft_sh = 0;
                descr->shift(inp0, inp0, -fft_sh, 2*N);
                descr->fft (out0, inp0, h_fft, scalingOpt);
                descr->ifft(inp0, out0, h_ifft,scalingOpt);
                descr->shift(inp0, inp0, ifft_sh+fft_sh-log2FFTsize, 2*N);
            }
            if (descr->fmt==FMT_DBL)
            {
                writepcm24((int32_t *)inp0, 2*N, pFileOut);
            }
            else
            {
                int k;
                const int16_t* inp=(const int16_t*)inp0;
                for (k=0; k<2*N; k++)
                {
                    int32_t t;
                    t=L_deposit_s(inp[k]);
                    writepcm24(&t, 1, pFileOut);
                }
            }
            // compare 
            diff=vector_diff(inpRef,inp0,2*N,descr->fmt);
            if (diff>maxdiff) maxdiff=diff;

            cnt += N;
            if(cnt > MAX_SAMPLES_PROCESS && MAX_SAMPLES_PROCESS > 0) { break;}
        }
    }
    maxdiff>>=log2FFTsize;
    //descr->maxdiff_cplx[scalingOpt][log2FFTsize]=maxdiff;
    if (res == -1) printf("NOT TESTED\n");
    else  { 
      if ((res>0) && (maxdiff > descr->maxdiff_cplx[scalingOpt][log2FFTsize])) printf("FAILED %d\n", (int)maxdiff);
      else if (res>0) printf("OK\n");
    }
   // if (maxdiff > descr->maxdiff_cplx[scalingOpt][log2FFTsize]) printf("FAILED\n");
   // else printf("OK\n");

exit:
    WAVREADER_CLOSE(pFileIn)
    WAVWRITER_CLOSE(pFileOut, lHz, 2, 24);
    vectors_free(&vInp0, &vInpRef, &vOut0,NULL);
    
    return hr;
}

int test_fft_real(const char *pNameIn, const char *pNameOut, int log2FFTsize, int scalingOpt, int type)
{
    int res = 1;
    int testCase=0;
    tDescr *descr;
    int32_t maxdiff=0,diff;
    int hr = TEST_OK;
    FILE *pFileIn = NULL, *pFileOut = NULL;
    unsigned long lHz = 8000;
    int N = 1<<log2FFTsize;
    int fft_sh, ifft_sh;
    tVecDescr vInp0,vInpRef,vOut0,vInp1,vOut1;
    int32_t *inp0,*inpRef,*out0,*out1,*inp1;
    fft_handle_t h_fft=NULL,h_ifft=NULL;
    switch(type)
    {
    case 16:    descr=&descr16x16; 
                switch(N)
                {
                case 32  : h_fft=rfft16x16_32  ; h_ifft=rifft16x16_32  ; break;
                case 64  : h_fft=rfft16x16_64  ; h_ifft=rifft16x16_64  ; break;
                case 128 : h_fft=rfft16x16_128 ; h_ifft=rifft16x16_128 ; break;
                case 256 : h_fft=rfft16x16_256 ; h_ifft=rifft16x16_256 ; break;
                case 512 : h_fft=rfft16x16_512 ; h_ifft=rifft16x16_512 ; break;
                case 1024: h_fft=rfft16x16_1024; h_ifft=rifft16x16_1024; break;
                default:
                    assert(!"fft handles not found");
                }
                break;
    case 24:    descr=&descr24x24; 
                switch(N)
                {
                case 32  : h_fft=rfft24_32  ; h_ifft=rifft24_32  ; break;
                case 64  : h_fft=rfft24_64  ; h_ifft=rifft24_64  ; break;
                case 128 : h_fft=rfft24_128 ; h_ifft=rifft24_128 ; break;
                case 256 : h_fft=rfft24_256 ; h_ifft=rifft24_256 ; break;
                case 512 : h_fft=rfft24_512 ; h_ifft=rifft24_512 ; break;
                case 1024: h_fft=rfft24_1024; h_ifft=rifft24_1024; break;
                default:
                    assert(!"fft handles not found");
                }
                break;
    case 32:    descr=&descr32x16; 
                switch(N)
                {
                case 32  : h_fft=rfft16_32  ; h_ifft=rifft16_32  ; break;
                case 64  : h_fft=rfft16_64  ; h_ifft=rifft16_64  ; break;
                case 128 : h_fft=rfft16_128 ; h_ifft=rifft16_128 ; break;
                case 256 : h_fft=rfft16_256 ; h_ifft=rifft16_256 ; break;
                case 512 : h_fft=rfft16_512 ; h_ifft=rifft16_512 ; break;
                case 1024: h_fft=rfft16_1024; h_ifft=rifft16_1024; break;
                default:
                    assert(!"fft handles not found");
                }
                break;
    default:    descr=0; assert(!"Unsupported FFT type"); abort();
    }

    vectors_allocate(1,descr->fmt, &vInp0, 2*N, &vInp1, N, &vInpRef, 2*N, &vOut0, N+2, &vOut1, 2*N,NULL);
    inp0  =vector_getL(&vInp0,0);
    inp1  =vector_getL(&vInp1,0);
    inpRef=vector_getL(&vInpRef,0);
    out0  =vector_getL(&vOut0,0);
    out1  =vector_getL(&vOut1,0);

    printf("Real FFT/IFFT %s N=%d, scaling option %d: ",descr->name,1<<log2FFTsize,scalingOpt);
    if (!NatureDSP_Signal_isPresent(descr->rfft) || !NatureDSP_Signal_isPresent(descr->rifft))
    {
      res = -1;
    }
    if (res>0)
    {
        int cnt = 0;

        WAVREADER_OPEN_24_STEREO(pFileIn,  pNameIn,  lHz)
        WAVWRITER_OPEN          (pFileOut, pNameOut);

        while(1) 
        {
            int i;
            size_t szElem=descr->fmt==FMT_DBL ? sizeof(int32_t):sizeof(int16_t);
            testCase++;
            if (descr->fmt==FMT_DBL)
            {
                if(2*N != readpcm24((int32_t *)inp0, 2*N, pFileIn)) { break;}
            }
            else
            {
                int k;
                int16_t *inp=(int16_t *)inp0;
                int err=0;
                for (k=0; k<2*N; k++)
                {
                    int32_t t;
                    err |= readpcm24(&t, 1, pFileIn)!=1;
                    inp[k]=S_extract_l(t);
                }
                if (err) break;
            }
            memcpy(inpRef,inp0,2*N*szElem);

            if (scalingOpt)
            {
                if (descr->fmt==FMT_DBL) 
                {
                    for(i = 0; i < N; i++) inp1[i] = inp0[2*i+0];
                }
                else 
                { 
                    int16_t *sinp0=(int16_t*)inp0,
                            *sinp1=(int16_t*)inp1;
                    for(i = 0; i < N; i++) sinp1[i] = sinp0[2*i+0];
                }
                fft_sh =descr->rfft (out0, inp1, h_fft,scalingOpt);
                ifft_sh=descr->rifft(inp1, out0, h_ifft,scalingOpt);
                descr->shift(inp1, inp1, ifft_sh+fft_sh-log2FFTsize+1, N);
                if (descr->fmt==FMT_DBL) 
                {
                    for(i = 0; i < N; i++) { out1[2*i+0] = inp1[i]; inp1[i] = inp0[2*i+1]; }
                }
                else
                {
                    int16_t *sinp0=(int16_t*)inp0,
                            *sinp1=(int16_t*)inp1,
                            *sout1=(int16_t*)out1;
                    for(i = 0; i < N; i++) { sout1[2*i+0] = sinp1[i]; sinp1[i] = sinp0[2*i+1]; }
                }
                fft_sh =descr->rfft (out0, inp1, h_fft,scalingOpt);
                ifft_sh=descr->rifft(inp1, out0, h_ifft,scalingOpt);
                descr->shift(inp1, inp1, ifft_sh+fft_sh-log2FFTsize+1, N);
                if (descr->fmt==FMT_DBL) 
                {
                    for(i = 0; i < N; i++) out1[2*i+1] = inp1[i];
                }
                else
                {
                    int16_t *sinp1=(int16_t*)inp1,
                            *sout1=(int16_t*)out1;
                    for(i = 0; i < N; i++) sout1[2*i+1] = sinp1[i];
                }
            }
            else
            {
                ASSERT(descr->fmt==FMT_DBL);
                descr->shift(inp0, inp0, -(log2FFTsize), 2*N);
                for(i = 0; i < N; i++) inp1[i] = inp0[2*i+0];
                descr->rfft (out0, inp1, h_fft,0);
                descr->rifft(inp1, out0, h_ifft,0);
                for(i = 0; i < N; i++) { out1[2*i+0] = inp1[i]; inp1[i] = inp0[2*i+1]; }
                descr->rfft (out0, inp1, h_fft,0);
                descr->rifft(inp1, out0, h_ifft,0);
                for(i = 0; i < N; i++) out1[2*i+1] = inp1[i];
                descr->shift(out1, out1, 1, 2*N);
            }

            if (descr->fmt==FMT_DBL)
            {
                writepcm24(out1, 2*N, pFileOut);
            }
            else
            {
                int k;
                const int16_t* inp=(const int16_t*)out1;
                for (k=0; k<2*N; k++)
                {
                    int32_t t;
                    t=L_deposit_s(inp[k]);
                    writepcm24(&t, 1, pFileOut);
                }
            }
            // compare 
            diff=vector_diff(inpRef,out1,2*N,descr->fmt);
            if (diff>maxdiff) maxdiff=diff;

            cnt += N;
            if(cnt > MAX_SAMPLES_PROCESS && MAX_SAMPLES_PROCESS > 0) { break; }
        }
    }

    maxdiff>>=log2FFTsize;
    if (res == -1) printf("NOT TESTED\n");
    else  {
      if ((res>0) && (maxdiff > descr->maxdiff_real[scalingOpt][log2FFTsize])) printf(" FAILED %d\n ",(int)maxdiff);
      else if (res>0) printf("OK\n ");
    }

exit:
    WAVREADER_CLOSE(pFileIn)
    WAVWRITER_CLOSE(pFileOut, lHz, 2, 24);

    vector_free(&vInp0);
    vector_free(&vInp1);
    vector_free(&vInpRef);
    vector_free(&vOut0);
    vector_free(&vOut1);

    return hr;
}


int main_fft( int phaseNum, int isFull, int isVerbose, int breakOnError )
{
    (void)isFull;
    (void)isVerbose;
    (void)breakOnError;

    if(isFull == 2) // if SANITY
    {
    	test_fft_cplx(TE_SANITY_VECTOR_DIR"/sine5sanity.wav",   "fft_cplx_16x16.0016s2.wav",    4,2,16);
    	test_fft_real(TE_SANITY_VECTOR_DIR"/sine5sanity.wav",   "fft_real_16x16.0032s2.wav",    5,2,16);
    	test_fft_cplx(TE_SANITY_VECTOR_DIR"/sine5sanity.wav",   "fft_cplx_32x16.0016s3.wav",    4,3,32);
    	test_fft_real(TE_SANITY_VECTOR_DIR"/sine5sanity.wav",   "fft_real_32x16.0032s3.wav",    5,3,32);
    	test_fft_cplx(TE_SANITY_VECTOR_DIR"/sine5sanity.wav",   "fft_cplx_24x24.0016s0.wav",    4,0,24);
    	test_fft_real(TE_SANITY_VECTOR_DIR"/sine5sanity.wav",   "fft_real_24x24.0032s0.wav",    5,0,24);
    }

    else if(phaseNum==0 || phaseNum==1)
    {
#if 1
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0016s2.wav",          4,2,16);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0032s2.wav",          5,2,16);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0064s2.wav",          6,2,16);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0128s2.wav",          7,2,16);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0256s2.wav",          8,2,16);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0512s2.wav",          9,2,16);

    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.0032s2.wav",          5,2,16);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.0064s2.wav",          6,2,16);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.0128s2.wav",          7,2,16);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.0256s2.wav",          8,2,16);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.0512s2.wav",          9,2,16);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.1024s2.wav",         10,2,16);
    
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0016s3.wav",          4,3,16);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0032s3.wav",          5,3,16);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0064s3.wav",          6,3,16);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0128s3.wav",          7,3,16);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0256s3.wav",          8,3,16);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_16x16.0512s3.wav",          9,3,16);

    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.0032s3.wav",          5,3,16);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.0064s3.wav",          6,3,16);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.0128s3.wav",          7,3,16);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.0256s3.wav",          8,3,16);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.0512s3.wav",          9,3,16);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_16x16.1024s3.wav",         10,3,16);
#endif
#if 1
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_32x16.0016s3.wav",          4,3,32);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_32x16.0032s3.wav",          5,3,32);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_32x16.0064s3.wav",          6,3,32);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_32x16.0128s3.wav",          7,3,32);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_32x16.0256s3.wav",          8,3,32);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_32x16.0512s3.wav",          9,3,32);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_32x16.0032s3.wav",          5,3,32);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_32x16.0064s3.wav",          6,3,32);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_32x16.0128s3.wav",          7,3,32);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_32x16.0256s3.wav",          8,3,32);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_32x16.0512s3.wav",          9,3,32);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_32x16.1024s3.wav",         10,3,32);
#endif

//------------------------------------------ 24x24 ------------------------------------------
#if 1
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0016s0.wav",          4,0,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0032s0.wav",          5,0,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0064s0.wav",          6,0,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0128s0.wav",          7,0,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0256s0.wav",          8,0,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0512s0.wav",          9,0,24);
#endif
#if 1
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0016s1.wav",          4,1,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0032s1.wav",          5,1,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0064s1.wav",          6,1,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0128s1.wav",          7,1,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0256s1.wav",          8,1,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0512s1.wav",          9,1,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0016s2.wav",          4,2,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0032s2.wav",          5,2,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0064s2.wav",          6,2,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0128s2.wav",          7,2,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0256s2.wav",          8,2,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0512s2.wav",          9,2,24);
#endif
#if 1
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0016s3.wav",          4,3,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0032s3.wav",          5,3,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0064s3.wav",          6,3,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0128s3.wav",          7,3,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0256s3.wav",          8,3,24);
    test_fft_cplx(TESTDIR"/sine5.wav",   "fft_cplx_24x24.0512s3.wav",          9,3,24);
#endif

#if 1
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0032s0.wav",          5,0,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0064s0.wav",          6,0,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0128s0.wav",          7,0,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0256s0.wav",          8,0,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0512s0.wav",          9,0,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.1024s0.wav",         10,0,24);
#endif
#if 1
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0032s1.wav",          5,1,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0064s1.wav",          6,1,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0128s1.wav",          7,1,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0256s1.wav",          8,1,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0512s1.wav",          9,1,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.1024s1.wav",         10,1,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0032s2.wav",          5,2,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0064s2.wav",          6,2,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0128s2.wav",          7,2,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0256s2.wav",          8,2,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0512s2.wav",          9,2,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.1024s2.wav",         10,2,24);
#endif

#if 1
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0032s3.wav",          5,3,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0064s3.wav",          6,3,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0128s3.wav",          7,3,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0256s3.wav",          8,3,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.0512s3.wav",          9,3,24);
    test_fft_real(TESTDIR"/sine5.wav",   "fft_real_24x24.1024s3.wav",         10,3,24);
#endif

    }
    return 1;
}
