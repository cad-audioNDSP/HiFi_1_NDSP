# makefile for building IntegrIT(R) NatureDSP_Signal(tm) library
# Version 0.32 tuned for Cadence HiFi cores
# ---------------------------------------------------
# Target platform: gcc/xcc
#
#  make invocation:
#  make clean   - clean up all temporary files
#  make depends - refresh dependencies (makedeps.mk)
#  make all     - make depends and release
#
#  System variables XTENSA_CORE,XTENSA_SYSTEM should 
#  be properly configured for correct procedure.
#  if XTENSA_SYSTEM is undefined, g++ build procedure
#  will use cstub files and headers from local
#  directory, not from system path
#
#  Other variables:
#  LANG=C++    build under g++, otherwise xcc used
#  SAVETEMPS=1 save temporary files
#-----------------------------------------------------

SRCDIRS		=	../../../../library/ \
				../../../../library/dct \
				../../../../library/fft \
				../../../../library/fir \
				../../../../library/iir \
				../../../../library/math \
				../../../../library/mtx \
				../../../../library/mtxinv \
				../../../../library/tables \
				../../../../library/twiddles \
				../../../../library/vec \
				../../../../library_reference # !!!! Temporary !!!!
OUTDIR		=	../../../bin
INCDIRS		=	../../../../library/include	\
			../../../../library/include_private	

TARGET				= NatureDSP_Signal
TARGET_SYMFILE			= symbols.txt
BUILD_TYPE			= LIBRARY
INTEGRIT_OLD_SUFFIXES		= 1
COMPILE_CSTUB			= 0

# check if c++ compilation required
ifeq (C++,$(LANG))
CPP	= 1
HAVE_XTENSA = 0
else
CPP	= 0
HAVE_XTENSA = 1
endif


ifeq (0,$(HAVE_XTENSA))
USE_PATCHES=1  # use patched cstubs for gcc, otherwise use REFERENCE variable
else
ifeq (1,$(REFERENCE))
USE_PATCHES=0
REF_SUFFIX=_reference
else
USE_PATCHES=1
REF_SUFFIX=
endif
endif


MODULES =  \
		alog10f_tbl \
		alog2f_tbl \
    asinf_tbl \
    atanf_tbl \
    alog10f_tbl \
    bkfir16x16_hifi1 \
    bkfir24x24_hifi1 \
    bkfir24x24p_hifi1 \
    bkfir32x16_hifi1 \
    bkfir32x32_hifi1 \
    bkfira16x16_hifi1 \
    bkfira24x24_hifi1 \
    bkfira32x16_hifi1 \
    bkfira32x32_hifi1 \
    bkfiraf_hifi1 \
    bkfiraf_process_hifi1 \
    bkfirf_hifi1 \
    bkfirf_process_hifi1 \
    cxfir_convol32x16_hifi1 \
    cxfir_convola32x16_hifi1 \
    cxfir_xcorraf_hifi1 \
    cxfir_xcorrf_hifi1 \
    cxfir16x16_hifi1 \
    cxfir24x24_hifi1 \
    cxfir32x16_hifi1 \
    cxfir32x32_hifi1 \
    cxfirf_hifi1 \
    cxfirf_process_hifi1 \
    cxfirinterp16x16_hifi1 \
    cxfirinterp16x16_D2_hifi1 \
    cxfirinterp16x16_D3_hifi1 \
    cxfirinterp16x16_D4_hifi1 \
    cxfirinterp16x16_DX_hifi1 \
    bq3iir16x16_df1_hifi1 \
    bq3iir16x16_df2_hifi1 \
    bq3iir32x16_df1_hifi1 \
    bq3iir32x16_df2_hifi1 \
    bq3iir32x32_df1_hifi1 \
    bq3iir32x32_df2_hifi1 \
    bq3iirf_df1_hifi1 \
    bq3iirf_df1_process_hifi1 \
    bq3iirf_df2_hifi1 \
    bq3iirf_df2_process_hifi1 \
    bqciirf_df1_hifi1 \
    bqciirf_df1_process_hifi1 \
    bqriir16x16_df1_hifi1 \
    bqriir16x16_df2_hifi1 \
    bqriir24x24_df1_hifi1 \
    bqriir24x24_df2_hifi1 \
    bqriir32x16_df1_hifi1 \
    bqriir32x16_df2_hifi1 \
    bqriir32x32_df1_hifi1 \
    bqriir32x32_df2_hifi1 \
    bqriirf_df1_hifi1 \
    bqriirf_df1_process_hifi1 \
    bqriirf_df2_hifi1 \
    bqriirf_df2_process_hifi1 \
    bqriirf_df2t_hifi1 \
    bqriirf_df2t_process_hifi1 \
    cmtx_add2x2_16x16_hifi1 \
    cmtx_add2x2_32x32_hifi1 \
    cmtx_add2x2f_hifi1 \
    cmtx_add3x3_16x16_hifi1 \
    cmtx_add3x3_32x32_hifi1 \
    cmtx_add3x3f_hifi1 \
    cmtx_add4x4_16x16_hifi1 \
    cmtx_add4x4_32x32_hifi1 \
		cmtx_add4x4f_hifi1 \
		cmtx_det2x2_16x16_hifi1 \
		cmtx_det2x2_32x32_hifi1 \
		cmtx_det2x2f_hifi1 \
		cmtx_det3x3_16x16_hifi1 \
		cmtx_det3x3_32x32_hifi1 \
		cmtx_det3x3f_hifi1 \
		cmtx_det4x4_16x16_hifi1 \
		cmtx_det4x4_32x32_hifi1 \
		cmtx_det4x4f_hifi1 \
		cmtx_inv2x2f_hifi1 \
		cmtx_inv3x3f_hifi1 \
		cmtx_inv4x4f_hifi1 \
		cmtx_mul2x2_16x16_hifi1 \
		cmtx_mul2x2_32x32_hifi1 \
		cmtx_mul2x2f_hifi1 \
		cmtx_mul3x3_16x16_hifi1 \
		cmtx_mul3x3_32x32_hifi1 \
		cmtx_mul3x3f_hifi1 \
		cmtx_mul4x4_16x16_hifi1 \
		cmtx_mul4x4_32x32_hifi1 \
		cmtx_mul4x4f_hifi1 \
		cmtx_sub2x2_16x16_hifi1 \
		cmtx_sub2x2_32x32_hifi1 \
		cmtx_sub2x2f_hifi1 \
		cmtx_sub3x3_16x16_hifi1 \
		cmtx_sub3x3_32x32_hifi1 \
		cmtx_sub3x3f_hifi1 \
		cmtx_sub4x4_16x16_hifi1 \
		cmtx_sub4x4_32x32_hifi1 \
		cmtx_sub4x4f_hifi1 \
		cmtx_tran2x2_16x16_hifi1 \
		cmtx_tran2x2_32x32_hifi1 \
		cmtx_tran2x2f_hifi1 \
		cmtx_tran3x3_16x16_hifi1 \
		cmtx_tran3x3_32x32_hifi1 \
		cmtx_tran3x3f_hifi1 \
		cmtx_tran4x4_16x16_hifi1 \
		cmtx_tran4x4_32x32_hifi1 \
		cmtx_tran4x4f_hifi1 \
		cxfir_convol32x16_hifi1 \
		cxfir_convola32x16_hifi1 \
		cxfir_xcorraf_hifi1 \
		cxfir_xcorrf_hifi1 \
		cxfir16x16_hifi1 \
		cxfir24x24_hifi1 \
		cxfir32x16_hifi1 \
		cxfir32x32_hifi1 \
		cxfirf_hifi1 \
		cxfirf_process_hifi1 \
		cxfirinterp16x16_hifi1 \
		dct_24x24_hifi1 \
		dct_32x16_hifi1 \
		dct_16x16_hifi1 \
		dct_32x32_hifi1 \
		dctf_hifi1 \
		expf_tbl \
		fft_cplx16x16_hifi1 \
		fft_cplx24x24_hifi1 \
		fft_cplx24x24_ie_hifi1 \
		fft_cplx_24x24_s1_ie_hifi1 \
		fft_cplx32x16_hifi1 \
		fft_cplx32x16_ie_hifi1 \
		fft_cplx_inc1024_hifi1 \
		fft_cplx_inc128_hifi1 \
		fft_cplx_inc2048_hifi1 \
		fft_cplx_inc256_hifi1 \
		fft_cplx_inc4096_hifi1 \
		fft_cplx_inc512_hifi1 \
		fft_cplx_inc64_hifi1 \
		fft_cplx_twd1024_24x24_hifi1 \
		fft_cplx_twd1024_hifi1 \
		fft_cplx_twd128_24x24_hifi1 \
		fft_cplx_twd128_hifi1 \
		fft_cplx_twd16_24x24_hifi1 \
		fft_cplx_twd16_hifi1 \
		fft_cplx_twd2048_24x24_hifi1 \
		fft_cplx_twd2048_hifi1 \
		fft_cplx_twd256_24x24_hifi1 \
		fft_cplx_twd256_hifi1 \
		fft_cplx_twd32_24x24_hifi1 \
		fft_cplx_twd32_hifi1 \
		fft_cplx_twd4096_24x24_hifi1 \
		fft_cplx_twd4096_hifi1 \
		fft_cplx_twd512_24x24_hifi1 \
		fft_cplx_twd512_hifi1 \
		fft_cplx_twd64_24x24_hifi1 \
		fft_cplx_twd64_hifi1 \
		fft_cplx_twiddles_24x24 \
		fft_cplx_twd8_16x16_hifi1 \
		fft_cplxf_ie_hifi1 \
		fft_pack24_ie_hifi1 \
		fft_real16x16_hifi1 \
		fft_real24x24_hifi1 \
		fft_real24x24_ie_24p_hifi1 \
		fft_real24x24_ie_hifi1 \
		fft_real32x16_hifi1 \
		fft_real32x16_ie_24p_hifi1 \
		fft_real32x16_ie_hifi1 \
		fft_real_twiddles \
		fft_real_twiddles_24x24 \
		fft_realf_ie_hifi1 \
		fft_revorder_ie_hifi1 \
		fft_stage_last_ie_hifi1 \
		fft_unpack24to32_s1_ie_hifi1 \
		fft_cplx32x32_hifi1 \
		fft_real32x32_hifi1 \
		fft_cplx_twd1024_32x32_hifi1 \
		fft_cplx_twd128_32x32_hifi1 \
		fft_cplx_twd16_32x32_hifi1 \
		fft_cplx_twd2048_32x32_hifi1 \
		fft_cplx_twd256_32x32_hifi1 \
		fft_cplx_twd32_32x32_hifi1 \
		fft_cplx_twd4096_32x32_hifi1 \
		fft_cplx_twd512_32x32_hifi1 \
		fft_cplx_twd64_32x32_hifi1 \
		fft_real_twd32_hifi1 \
		fft_cplx_twd1024_16x16_hifi1 \
		fft_cplx_twd128_16x16_hifi1 \
		fft_cplx_twd16_16x16_hifi1 \
		fft_cplx_twd2048_16x16_hifi1 \
		fft_cplx_twd256_16x16_hifi1 \
		fft_cplx_twd32_16x16_hifi1 \
		fft_cplx_twd4096_16x16_hifi1 \
		fft_cplx_twd512_16x16_hifi1 \
		fft_cplx_twd64_16x16_hifi1 \
		fir_acorr16x16_hifi1 \
		fir_acorr24x24_hifi1 \
		fir_acorr32x32_hifi1 \
		fir_acorra16x16_hifi1 \
		fir_acorra24x24_hifi1 \
		fir_acorra32x32_hifi1 \
		fir_acorraf_hifi1 \
		fir_acorrf_hifi1 \
		fir_blms16x16_hifi1 \
		fir_blms16x32_hifi1 \
		fir_blms24x24_hifi1 \
		fir_blms32x32_hifi1 \
		fir_blmsf_hifi1 \
		fir_convol16x16_hifi1 \
		fir_convol24x24_hifi1 \
		fir_convol32x16_hifi1 \
		fir_convol32x32_hifi1 \
		fir_convola16x16_hifi1 \
		fir_convola24x24_hifi1 \
		fir_convola32x16_hifi1 \
		fir_convola32x32_hifi1 \
		fir_convolaf_hifi1 \
		fir_convolf_hifi1 \
		fir_decimaf_2x_hifi1 \
		fir_decimaf_3x_hifi1 \
		fir_decimaf_4x_hifi1 \
		fir_decimaf_Dx_hifi1 \
		fir_interpf_2x_hifi1 \
		fir_interpf_3x_hifi1 \
		fir_interpf_4x_hifi1 \
		fir_interpf_Dx_hifi1 \
		fir_lacorra16x16_hifi1 \
		fir_lacorra32x32_hifi1 \
		fir_lacorraf_hifi1 \
		fir_lconvola16x16_hifi1 \
		fir_lconvola32x16_hifi1 \
		fir_lconvola32x32_hifi1 \
		fir_lconvolaf_hifi1 \
		fir_lxcorra16x16_hifi1 \
		fir_lxcorra32x16_hifi1 \
		fir_lxcorra32x32_hifi1 \
		fir_lxcorraf_hifi1 \
		fir_xcorr16x16_hifi1 \
		fir_xcorr24x24_hifi1 \
		fir_xcorr32x16_hifi1 \
		fir_xcorr32x32_hifi1 \
		fir_xcorra16x16_hifi1 \
		fir_xcorra24x24_hifi1 \
		fir_xcorra32x16_hifi1 \
		fir_xcorra32x32_hifi1 \
		fir_xcorraf_hifi1 \
		fir_xcorrf_hifi1 \
		firdec16x16_hifi1 \
		firdec16x16_D2_hifi1 \
		firdec16x16_D3_hifi1 \
		firdec16x16_D4_hifi1 \
		firdec16x16_DX_hifi1 \
		firdec24x24_hifi1 \
		firdec32x16_hifi1 \
		firdec32x32_D2_hifi1 \
		firdec32x32_D3_hifi1 \
		firdec32x32_D4_hifi1 \
		firdec32x32_DX_hifi1 \
		firdec32x32_hifi1 \
		firdecf_hifi1 \
		firinterp16x16_hifi1 \
		firinterp16x16_D2_hifi1 \
		firinterp16x16_D3_hifi1 \
		firinterp16x16_D4_hifi1 \
		firinterp16x16_DX_hifi1 \
		firinterp24x24_hifi1 \
		firinterp32x16_hifi1 \
		firinterp32x32_D2_hifi1 \
		firinterp32x32_D3_hifi1 \
		firinterp32x32_D4_hifi1 \
		firinterp32x32_DX_hifi1 \
		firinterp32x32_hifi1 \
		firinterpf_hifi1 \
		ifft_cplx16x16_hifi1 \
		ifft_cplx24x24_hifi1 \
		ifft_cplx24x24_ie_hifi1 \
		ifft_cplx32x16_hifi1 \
		ifft_cplx32x16_ie_hifi1 \
		ifft_cplx_twd1024_24x24_hifi1 \
		ifft_cplx_twd1024_hifi1 \
		ifft_cplx_twd128_24x24_hifi1 \
		ifft_cplx_twd128_hifi1 \
		ifft_cplx_twd16_24x24_hifi1 \
		ifft_cplx_twd16_hifi1 \
		ifft_cplx_twd2048_24x24_hifi1 \
		ifft_cplx_twd2048_hifi1 \
		ifft_cplx_twd256_24x24_hifi1 \
		ifft_cplx_twd256_hifi1 \
		ifft_cplx_twd32_24x24_hifi1 \
		ifft_cplx_twd32_hifi1 \
		ifft_cplx_twd4096_24x24_hifi1 \
		ifft_cplx_twd4096_hifi1 \
		ifft_cplx_twd512_24x24_hifi1 \
		ifft_cplx_twd512_hifi1 \
		ifft_cplx_twd64_24x24_hifi1 \
		ifft_cplx_twd64_hifi1 \
		ifft_cplxf_ie_hifi1 \
		ifft_real16x16_hifi1 \
		ifft_real24x24_hifi1 \
		ifft_real24x24_ie_24p_hifi1 \
		ifft_real24x24_ie_hifi1 \
		ifft_real32x16_hifi1 \
		ifft_real32x16_ie_24p_hifi1 \
		ifft_real32x16_ie_hifi1 \
		ifft_realf_ie_hifi1 \
		ifft_cplx32x32_hifi1 \
		ifft_real32x32_hifi1 \
		ifft_cplx_twd1024_16x16_hifi1 \
		ifft_cplx_twd128_16x16_hifi1 \
		ifft_cplx_twd16_16x16_hifi1 \
		ifft_cplx_twd2048_16x16_hifi1 \
		ifft_cplx_twd256_16x16_hifi1 \
		ifft_cplx_twd32_16x16_hifi1 \
		ifft_cplx_twd4096_16x16_hifi1 \
		ifft_cplx_twd512_16x16_hifi1 \
		ifft_cplx_twd64_16x16_hifi1 \
		ifft_cplx_twd8_16x16_hifi1 \
		inff_tbl \
		inv2pif_tbl \
		raw_lcorr16x16_hifi1 \
		latr16x16_hifi1 \
		latr24x24_hifi1 \
		latr32x16_hifi1 \
		latr32x32_hifi1 \
		latrf_hifi1 \
		latrf1_hifi1 \
		latrf2_hifi1 \
		latrf3_hifi1 \
		latrf4_hifi1 \
		latrf5_hifi1 \
		latrf6_hifi1 \
		latrf7_hifi1 \
		latrf8_hifi1 \
		latrfX_hifi1 \
		log10f_tbl \
		log2f_tbl \
		lognf_tbl \
		logq23_tbl \
		mtx_add2x2_16x16_hifi1 \
		mtx_add2x2_32x32_hifi1 \
		mtx_add2x2f_hifi1 \
		mtx_add3x3_16x16_hifi1 \
		mtx_add3x3_32x32_hifi1 \
		mtx_add3x3f_hifi1 \
		mtx_add4x4_16x16_hifi1 \
		mtx_add4x4_32x32_hifi1 \
		mtx_add4x4f_hifi1 \
		mtx_det2x2_16x16_hifi1 \
		mtx_det2x2_32x32_hifi1 \
		mtx_det2x2f_hifi1 \
		mtx_det3x3_16x16_hifi1 \
		mtx_det3x3_32x32_hifi1 \
		mtx_det3x3f_hifi1 \
		mtx_det4x4_16x16_hifi1 \
		mtx_det4x4_32x32_hifi1 \
		mtx_det4x4f_hifi1 \
		mtx_inv2x2f_hifi1 \
		mtx_inv3x3f_hifi1 \
		mtx_inv4x4f_hifi1 \
		mtx_mpy16x16_hifi1 \
		mtx_mpy16x16_m8p2_hifi1 \
		mtx_mpy24x24_hifi1 \
		mtx_mpy24x24_m8p2_hifi1 \
		mtx_mpy32x32_fast_hifi1 \
		mtx_mpy32x32_hifi1 \
		mtx_mpyf_fast_hifi1 \
		mtx_mpyf_hifi1 \
		mtx_mul2x2_16x16_hifi1 \
		mtx_mul2x2_32x32_hifi1 \
		mtx_mul2x2f_hifi1 \
		mtx_mul3x3_16x16_hifi1 \
		mtx_mul3x3_32x32_hifi1 \
		mtx_mul3x3f_hifi1 \
		mtx_mul4x4_16x16_hifi1 \
		mtx_mul4x4_32x32_hifi1 \
		mtx_mul4x4f_hifi1 \
		mtx_sub2x2_16x16_hifi1 \
		mtx_sub2x2_32x32_hifi1 \
		mtx_sub2x2f_hifi1 \
		mtx_sub3x3_16x16_hifi1 \
		mtx_sub3x3_32x32_hifi1 \
		mtx_sub3x3f_hifi1 \
		mtx_sub4x4_16x16_hifi1 \
		mtx_sub4x4_32x32_hifi1 \
		mtx_sub4x4f_hifi1 \
		mtx_tran2x2_16x16_hifi1 \
		mtx_tran2x2_32x32_hifi1 \
		mtx_tran2x2f_hifi1 \
		mtx_tran3x3_16x16_hifi1 \
		mtx_tran3x3_32x32_hifi1 \
		mtx_tran3x3f_hifi1 \
		mtx_tran4x4_16x16_hifi1 \
		mtx_tran4x4_32x32_hifi1 \
		mtx_tran4x4f_hifi1 \
                mtx_vecmpy16x16_fast_hifi1 \
		mtx_vecmpy16x16_hifi1 \
		mtx_vecmpy24x24_fast_hifi1 \
		mtx_vecmpy24x24_hifi1 \
		mtx_vecmpy32x32_fast_hifi1 \
		mtx_vecmpy32x32_hifi1 \
		mtx_vecmpyf_fast_hifi1 \
		mtx_vecmpyf_hifi1 \
		nan_tbl \
		nanf_tbl \
		pif_tbl \
		polyatan16x16q23_tbl \
		polypow2q23_tbl \
		polyrsqrtq23_tbl \
		q2rot_16x16_hifi1 \
		q2rot_32x32_hifi1 \
		q2rotf_hifi1 \
		raw_corr16x16_hifi1 \
		raw_corr24x24_hifi1 \
		raw_corr32x16_hifi1 \
		raw_corr32x32_hifi1 \
		raw_corrf_hifi1 \
		raw_lcorr16x16_hifi1 \
		raw_lcorr32x16_hifi1 \
		raw_lcorr32x32_hifi1 \
		raw_lcorrf \
		scl_acosf_hifi1 \
		scl_antilog10_24x24_hifi1 \
		scl_antilog10_32x32_hifi1 \
		scl_antilog2_24x24_hifi1 \
		scl_antilog2_32x32_hifi1 \
		scl_antilogn_24x24_hifi1 \
		scl_antilogn_32x32_hifi1 \
		scl_antilog10_16x16_hifi1 \
		scl_antilog10f_hifi1 \
		scl_antilog2_16x16_hifi1 \
		scl_antilog2f_hifi1 \
		scl_antilogn_16x16_hifi1 \
		scl_antilognf_hifi1 \
		scl_asinf_hifi1 \
		scl_atan16x16_hifi1 \
		scl_atan24x24_hifi1 \
		scl_atan32x32_hifi1 \
		scl_atan_table \
		scl_atan_table16 \
                scl_atan2_16x16_hifi1 \
		scl_atan2f_hifi1 \
		scl_atanf_hifi1 \
		scl_bexp16_hifi1 \
		scl_bexp24_hifi1 \
		scl_bexp32_hifi1 \
		scl_bexpf_hifi1 \
		scl_complex2invmag_hifi1 \
		scl_complex2mag_hifi1 \
                scl_cosine16x16_hifi1 \
		scl_cosine24x24_hifi1 \
		scl_cosine32x32_hifi1 \
		scl_cosinef_hifi1 \
		scl_divide16x16_hifi1 \
		scl_divide24x24_hifi1 \
		scl_divide32x32_hifi1 \
                scl_dividef_hifi1 \
                scl_float2ceil_hifi1 \
                scl_float2floor_hifi1 \
		scl_float2int_hifi1 \
		scl_int2float_hifi1 \
                scl_log10_16x16_hifi1 \
		scl_log10_24x24_hifi1 \
		scl_log10_32x32_hifi1 \
		scl_log10f_hifi1 \
		scl_log2_16x16_hifi1 \
		scl_log2_24x24_hifi1 \
		scl_log2_32x32_hifi1 \
		scl_log2f_hifi1 \
		scl_logn_16x16_hifi1 \
		scl_logn_24x24_hifi1 \
		scl_logn_32x32_hifi1 \
		scl_lognf_hifi1 \
		scl_recip16x16_hifi1 \
		scl_recip24x24_hifi1 \
		scl_recip32x32_hifi1 \
		scl_recipf_hifi1 \
		scl_rsqrtf_hifi1 \
		scl_sine16x16_hifi1 \
		scl_sine24x24_hifi1 \
		scl_sine32x32_hifi1 \
		scl_sine_table16 \
		scl_sine_table32 \
		scl_sinef_hifi1 \
		scl_sqrt16x16_hifi1 \
		scl_sqrt24x24_hifi1 \
		scl_sqrt32x32_hifi1 \
		scl_sqrt_table \
		scl_sqrtf_hifi1 \
		scl_tan16x16_hifi1 \
		scl_tan24x24_hifi1 \
		scl_tan32x32_hifi1 \
		scl_tanf_hifi1 \
		sinf_tbl \
		sqrt2f_tbl \
		tan16x16_tbl \
		tanf_tbl \
		vec_acosf_hifi1 \
		vec_add16x16_hifi1 \
		vec_add16x16_hifi1_fast \
		vec_add24x24_hifi1 \
		vec_add24x24_hifi1_fast \
		vec_add32x32_hifi1 \
		vec_add32x32_hifi1_fast \
		vec_addf_hifi1 \
		vec_alog_table \
		vec_antilog10_24x24_hifi1 \
		vec_antilog10_32x32_hifi1 \
		vec_antilog10f_hifi1 \
		vec_antilog2_24x24_hifi1 \
		vec_antilog2_32x32_hifi1 \
		vec_antilog2f_hifi1 \
		vec_antilogn_24x24_hifi1 \
		vec_antilogn_32x32_hifi1 \
		vec_antilognf_hifi1 \
		vec_antilog10_16x16_hifi1 \
		vec_antilog2_16x16_hifi1 \
		vec_antilogn_16x16_hifi1 \
		vec_asinf_hifi1 \
		vec_atan16x16_hifi1 \
		vec_atan24x24_hifi1 \
		vec_atan32x32_hifi1 \
		vec_atan2_16x16_hifi1 \
		vec_atan2f_hifi1 \
		vec_atanf_hifi1 \
		vec_bexp16_fast_hifi1 \
		vec_bexp16_hifi1 \
		vec_bexp24_fast_hifi1 \
		vec_bexp24_hifi1 \
		vec_bexp32_fast_hifi1 \
		vec_bexp32_hifi1 \
		vec_bexpf_hifi1 \
		vec_complex2invmag_hifi1 \
		vec_complex2mag_hifi1 \
		vec_cosine16x16_hifi1 \
		vec_cosine24x24_fast_hifi1 \
		vec_cosine24x24_hifi1 \
		vec_cosine32x32_fast_hifi1 \
		vec_cosine32x32_hifi1 \
		vec_cosinef_hifi1 \
		vec_divide16x16_fast_hifi1 \
		vec_divide16x16_hifi1 \
		vec_divide24x24_fast_hifi1 \
		vec_divide24x24_hifi1 \
		vec_divide32x32_fast_hifi1 \
		vec_divide32x32_hifi1 \
		vec_dividef_hifi1 \
		vec_dot16x16_fast_hifi1 \
		vec_dot16x16_hifi1 \
		vec_dot24x24_fast_hifi1 \
		vec_dot24x24_hifi1 \
		vec_dot32x16_fast_hifi1 \
		vec_dot32x16_hifi1 \
		vec_dot32x32_fast_hifi1 \
		vec_dot32x32_hifi1 \
		vec_dotf_hifi1 \
		vec_float2ceil_hifi1 \
		vec_float2floor_hifi1 \
		vec_float2int_hifi1 \
		vec_int2float_hifi1 \
		vec_log_table \
		vec_log10_16x16_hifi1 \
		vec_log10_24x24_hifi1 \
		vec_log10_32x32_hifi1 \
		vec_log10f_hifi1 \
		vec_log2_16x16_hifi1 \
		vec_log2_24x24_hifi1 \
		vec_log2_32x32_hifi1 \
		vec_log2f_hifi1 \
		vec_logn_16x16_hifi1 \
		vec_logn_24x24_hifi1 \
		vec_logn_32x32_hifi1 \
		vec_lognf_hifi1 \
		vec_max16x16_fast_hifi1 \
		vec_max16x16_hifi1 \
		vec_max24x24_fast_hifi1 \
		vec_max24x24_hifi1 \
		vec_max32x32_fast_hifi1 \
		vec_max32x32_hifi1 \
		vec_maxf_hifi1 \
		vec_min16x16_fast_hifi1 \
		vec_min16x16_hifi1 \
		vec_min24x24_fast_hifi1 \
		vec_min24x24_hifi1 \
		vec_min32x32_fast_hifi1 \
		vec_min32x32_hifi1 \
		vec_minf_hifi1 \
		vec_poly4_16x16_hifi1 \
		vec_poly4_24x24_hifi1 \
		vec_poly4_32x32_hifi1 \
		vec_poly4f_hifi1 \
		vec_poly8_16x16_hifi1 \
		vec_poly8_24x24_hifi1 \
		vec_poly8_32x32_hifi1 \
		vec_poly8f_hifi1 \
		vec_power16x16_fast_hifi1 \
		vec_power16x16_hifi1 \
		vec_power24x24_fast_hifi1 \
		vec_power24x24_hifi1 \
		vec_power32x32_fast_hifi1 \
		vec_power32x32_hifi1 \
		vec_powerf_hifi1 \
		vec_recip_table \
		vec_recip16x16_hifi1 \
		vec_recip24x24_hifi1 \
		vec_recip32x32_hifi1 \
		vec_recipf_hifi1 \
		vec_rsqrtf_hifi1 \
		vec_scale_sf_hifi1 \
		vec_scale16x16_fast_hifi1 \
		vec_scale16x16_hifi1 \
		vec_scale24x24_fast_hifi1 \
		vec_scale24x24_hifi1 \
		vec_scale32x24_fast_hifi1 \
		vec_scale32x24_hifi1 \
		vec_scalef_hifi1 \
		vec_shift16x16_fast_hifi1 \
		vec_shift16x16_hifi1 \
		vec_shift24x24_fast_hifi1 \
		vec_shift24x24_hifi1 \
		vec_shift32x32_fast_hifi1 \
		vec_shift32x32_hifi1 \
		vec_shiftf_hifi1 \
		vec_sine16x16_hifi1 \
		vec_sine24x24_fast_hifi1 \
		vec_sine24x24_hifi1 \
		vec_sine32x32_fast_hifi1 \
		vec_sine32x32_hifi1 \
		vec_sinef_hifi1 \
		vec_sqrt16x16_hifi1 \
		vec_sqrt24x24_fast_hifi1 \
		vec_sqrt24x24_hifi1 \
		vec_sqrt32x32_fast_hifi1 \
		vec_sqrt32x32_hifi1 \
		vec_sqrtf_hifi1 \
		vec_sub16x16_hifi1 \
		vec_sub32x32_hifi1 \
		vec_subf_hifi1 \
		vec_tan16x16_hifi1 \
		vec_tan24x24_hifi1 \
		vec_tan32x32_hifi1 \
		vec_tanf_hifi1 \
		feature \
		version

ifneq (0, $(HAVE_XTENSA))
#CFLAGS_FILE_module_name   += some compiler switches
endif

			
default:	all

.PHONY:	all clean cleanest cleanlib debug ferret default profile release
.PHONY: $(foreach proj,$(EXTERNAL_PROJECTS), $(proj))
.PHONY: $(foreach proj,$(EXTERNAL_PROJECTS), $(proj)/clean)
.PHONY: $(foreach proj,$(EXTERNAL_PROJECTS), $(proj)/cleanest)

debug:		all
ferret:		all
profile:	all
release:	all 
 

ifneq (1, $(HAVE_XTENSA)) ############################
CORE			:= cstub
else #################################################
ifneq (,$(XTENSA_TOOLS))
XTPATH=$(XTENSA_TOOLS)/bin/
else
ifneq (,$(XTTOOLS))
XTPATH=$(XTENSA_TOOLS)/bin/
endif
endif
endif ################################################

ifeq ("cstub", "$(CORE)") ############################
ifeq ("","$(PLATFORM)")
ifeq (1, $(CPP))
PLATFORM		:= $(XTENSA_CORE)-C++
else
PLATFORM		:= $(XTENSA_CORE)
endif
endif
else #################################################
ifeq ("", "$(CORE)")
ifeq (1,$(HAVE_XTENSA))
PLATFORM		:= $(XTENSA_CORE)$(REF_SUFFIX)
else
PLATFORM		:= XRC_B64_PM
endif
else
PLATFORM		:= $(CORE)
endif
endif ################################################

ifneq (, $(findstring cstub, $(PLATFORM))) ###########
SOLID_LIBRARY		:= 1
CCFLAGS_DIR_fft_files	+= -Wno-uninitialized -Wno-strict-aliasing
CCFLAGS_DIR_cstub 	+= -Wno-uninitialized -Wno-strict-aliasing
CCFLAGS_DIR_library	+= -Wno-strict-aliasing
CCFLAGS_DIR_P2_files	+= -Wno-strict-aliasing
else #################################################
SOLID_LIBRARY		:= 0
CCFLAGS_DIR_fft_files += -OPT:alias=restrict
CCFLAGS_DIR_P2_files += -fvectorize
endif ################################################
###############################################################
##                      Host OS detection                    ##
###############################################################

WINDIR+=$(windir)

ifeq (,$(WINDIR))
# Unix-like system
ULIS=1
override HOSTOS=$(shell uname)
else
ULIS=0
override HOSTOS=win32
SHELL=cmd
endif

###############################################################
##              Platform-specific configuration              ##
###############################################################
HIFI_CSTUB_MODULES += cstub-Xm_$(XTENSA_CORE)

ifeq (,$(XTENSA_SYSTEM))
HIFI_CSTUB_INCDIRS	+= ../../../../cstub ../../../../cstub/xtensa/tie ../../../../cstub/xtensa/config
HIFI_CSTUB_SRCDIRS	+= ../../../../cstub
CFLAGS_FILE_$(subst -,_,$(HIFI_CSTUB_MODULES)) = -Dasm=__asm__ -O0
else
HIFI_CSTUB_INCDIRS += $(XTENSA_SYSTEM)/../src/cstub $(XTENSA_SYSTEM)/../src/cstub/xtensa/tie $(XTENSA_SYSTEM)/../xtensa-elf/arch/include/xtensa/config
HIFI_CSTUB_SRCDIRS += $(XTENSA_SYSTEM)/../src/cstub
endif


ifneq (0, $(HAVE_XTENSA)) ############################
#############################################################
VARPREFIX.$(PLATFORM)		     = $(PLATFORM)
CC.$(PLATFORM)      		     = xt-clang
LD.$(PLATFORM)      		     = xt-clang
AR.$(PLATFORM)      		     = xt-ar
#MAKE.$(PLATFORM)      		     = xt-make
OBJCOPY.$(PLATFORM)          	     = xt-objcopy
RUN.$(PLATFORM)     		     = xt-run
PROF.$(PLATFORM)      		     = xt-prof
CFLAGS.$(PLATFORM) 		    += -pipe -DCONFIG_$(PLATFORM)
CFLAGS.$(PLATFORM)     		+= -DPROC_$(PLATFORM) -mlongcalls -fno-builtin
CFLAGS.profile.$(PLATFORM)      += -O3 -fvectorize
CFLAGS.release.$(PLATFORM)	+= -O3 -fvectorize -fno-unsafe-math-optimizations
CFLAGS.debug.$(PLATFORM)	+= -O0 -g -D_DEBUG -mcoproc -fno-unsafe-math-optimizations 
CFLAGS.ferret.$(PLATFORM)	+= -O3 -g -fvectorize 
RUNFLAGS.debug.$(PLATFORM)	 = --turbo
RUNFLAGS.ferret.$(PLATFORM)	 = 
RUNFLAGS.release.$(PLATFORM)     = --turbo
TARGETOS.$(PLATFORM)		     = Xtensa
else
VARPREFIX.$(PLATFORM)		 = $(PLATFORM)
ifeq (1, $(CPP))
CC.$(PLATFORM)      		 = g++
LD.$(PLATFORM)      		 = g++
else
VARPREFIX.$(PLATFORM)		 = $(PLATFORM)
CC.$(PLATFORM)      		 = gcc
LD.$(PLATFORM)      		 = gcc
endif
AR.$(PLATFORM)      		 = ar
#MAKE.$(PLATFORM)      		 = make
OBJCOPY.$(PLATFORM)              = objcopy
RUN.$(PLATFORM)     		 = 
PROF.$(PLATFORM)      		 = 
CFLAGS.$(PLATFORM)         	 += -m32 -pipe
CFLAGS.$(PLATFORM) 		 += -DXTENSA_CORE=Xm_$(XTENSA_CORE)
ifeq (XRC_B64_LP,$(XTENSA_CORE))
CFLAGS.$(PLATFORM)         	 += -DB64_LP
endif
INCDIRS			 	 += $(HIFI_CSTUB_INCDIRS)
#SRCDIRS			 += $(HIFI_CSTUB_SRCDIRS)
ifeq (1, $(COMPILE_CSTUB))
MODULES				 += $(HIFI_CSTUB_MODULES)
endif
CFLAGS.profile.$(PLATFORM)       += -O3 
CFLAGS.release.$(PLATFORM)	 += -O3 
RUNFLAGS.debug.$(PLATFORM)	 = 
RUNFLAGS.release.$(PLATFORM)	 = 
RUNFLAGS.ferret.$(PLATFORM)	 = 
TARGETOS.$(PLATFORM)		 = Linux
LFLAGS.$(PLATFORM) 		 += -m32
endif

CFLAGS.release.$(PLATFORM)	 += -DUSE_ISA_PATCHES=$(USE_PATCHES)

###############################################################
##                     Well-known suffixes                   ##
###############################################################
#POSTFIX.$(PLATFORM)-Xtensa-profile.1=_$(PLATFORM)
#POSTFIX.$(PLATFORM)-Xtensa-release.1=_$(PLATFORM)
#POSTFIX.$(PLATFORM)-Xtensa-debug.1=_$(PLATFORM)
#POSTFIX.$(PLATFORM)-Xtensa-ferret.1=_$(PLATFORM)
#POSTFIX.$(PLATFORM)-Xtensa-profile.1=_$(PLATFORM)

###############################################################
##                 Host-specific configuration               ##
###############################################################

ifeq (1,$(ULIS)) ##############################################
ECHO=echo
override RM=rm -f
CP=cp
PSEP=/
NULLDEV=/dev/null
MKDIR=mkdir
JOIN=&&
else ##########################################################
ECHO=echo
override RM=del
CP=copy
PSEP=\\#
NULLDEV=NUL
MKDIR=mkdir
JOIN=&
endif #########################################################

###############################################################
##                     Target OS definition                  ##
###############################################################

TARGETOS=$(firstword $(TARGETOS.$(PLATFORM)) $(HOSTOS))

VARPREFIX.Darwin=MAC
VARPREFIX.iOS=MAC
VARPREFIX.linux=LINUX
VARPREFIX.Linux=LINUX
VARPREFIX.unix-like.1=UNIX_LIKE
VARPREFIX.win32=WIN32

###############################################################
##                  Select build configuration               ##
###############################################################

CONFIG=$(firstword $(findstring release, $(MAKECMDGOALS)) $(findstring debug, \
 $(MAKECMDGOALS)) $(findstring ferret, $(MAKECMDGOALS)) $(findstring profile, $(MAKECMDGOALS)) release)

CONFIG_ID	= $(PLATFORM)-$(TARGETOS)-$(CONFIG)
DLIBS_CONFIG_ID	= $(PLATFORM)-$(TARGETOS)-$(firstword \
		  $(DLIBS_CONFIG) $(CONFIG))

# Intermediate directory

INTERMEDIATE_DIR= $(CONFIG_ID)
USE_SUFFIXES	= $(firstword $(INTEGRIT_SUFFIXES) $(INTEGRIT_OLD_SUFFIXES))
POSTFIX		= $(firstword \
	$(POSTFIX.$(CONFIG_ID).$(USE_SUFFIXES)) -$(CONFIG_ID))
POSTFIX_DLC	= $(firstword \
	$(POSTFIX.$(DLIBS_CONFIG_ID).$(USE_SUFFIXES)) -$(DLIBS_CONFIG_ID))

$(INTERMEDIATE_DIR)/dirinfo.txt:
	-$(MKDIR) $(INTERMEDIATE_DIR)
	-$(ECHO) Intermediate directory.>$(INTERMEDIATE_DIR)/dirinfo.txt

###############################################################
##                     Configure DevTools                    ##
###############################################################
CC			= $(firstword $(CC.$(PLATFORM)) gcc)
LD			= $(firstword $(LD.$(PLATFORM)) gcc)
AR			= $(firstword $(AR.$(PLATFORM)) ar)
MAKE			= $(firstword $(MAKE.$(PLATFORM)) make)
OBJCOPY			= $(firstword $(OBJCOPY.$(PLATFORM)) objcopy)
RUN			= $(RUN.$(PLATFORM))
PROF			= $(firstword $(PROF.$(PLATFORM)) gprof)

CFLAGS_DEPENDENT	+= $(CFLAGS.$(PLATFORM))

CFLAGS.default.debug.	= $(CFLAGS.default.debug) -g -O0 -D_DEBUG
CFLAGS.default.ferret.	= $(CFLAGS.default.ferret) -g -O0 -D_DEBUG
CFLAGS.default.profile.	= $(CFLAGS.default.profile) -g -pg -O3
CFLAGS.default.release.	= $(CFLAGS.default.release) -O3
LFLAGS.default.debug.	= $(LFLAGS.default.debug)
LFLAGS.default.ferret.	= $(LFLAGS.default.ferret)
LFLAGS.default.profile.	= $(LFLAGS.default.profile) -g -pg -O
LFLAGS.default.release.EXECUTABLE = -s
LFLAGS.default.ferret.EXECUTABLE = 
LFLAGS.default.release.	= $(LFLAGS.default.release) -O \
			$(LFLAGS.default.release.$(BUILD_TYPE))
LFLAGS.default.ferret.	= $(LFLAGS.default.ferret) -O \
			$(LFLAGS.default.ferret.$(BUILD_TYPE))			

CFLAGS.debug		= $(CFLAGS.debug.$(PLATFORM)) \
			$(CFLAGS.default.debug.$(CFLAGS.debug.$(PLATFORM)))
CFLAGS.ferret		= $(CFLAGS.ferret.$(PLATFORM)) \
			$(CFLAGS.default.ferret.$(CFLAGS.ferret.$(PLATFORM)))
CFLAGS.profile		= $(CFLAGS.profile.$(PLATFORM)) \
			$(CFLAGS.default.profile.$(CFLAGS.profile.$(PLATFORM)))
CFLAGS.release		= $(CFLAGS.release.$(PLATFORM)) \
			$(CFLAGS.default.release.$(CFLAGS.release.$(PLATFORM)))

CFLAGADDINCDIR		= $(firstword $(CFLAGADDINCDIR.$(PLATFORM)) -I)
CFLAGMAKEDEP		= $(firstword $(CFLAGMAKEDEP.$(PLATFORM)) -MM)
CFLAGMAKEOBJ		= $(firstword $(CFLAGMAKEDEP.$(PLATFORM)) -c)
CFLAGSAVETEMP.1		= $(firstword $(CFLAGSAVETEMP.$(PLATFORM)) -save-temps)

LFLAGS_DEPENDENT	+= $(LFLAGS.$(PLATFORM))
LFLAGS.debug		= $(LFLAGS.debug.$(PLATFORM)) \
			$(LFLAGS.default.debug.$(LFLAGS.debug.$(PLATFORM)))
LFLAGS.debug		= $(LFLAGS.ferret.$(PLATFORM)) \
			$(LFLAGS.default.ferret.$(LFLAGS.ferret.$(PLATFORM)))
LFLAGS.profile		= $(LFLAGS.profile.$(PLATFORM)) \
			$(LFLAGS.default.profile.$(LFLAGS.profile.$(PLATFORM)))
LFLAGS.release		= $(LFLAGS.release.$(PLATFORM)) \
			$(LFLAGS.default.release.$(LFLAGS.release.$(PLATFORM)))

LFLAGS.LIBRARY.1	+=--no-standard-libraries
LFLAGS.LIBRARY.1	+=-Wl,-r,--retain-symbols-file,$(TARGET_SYMFILE)

ARFLAGS			= $(firstword $(ARFLAGS.$(PLATFORM)) rcs)

RUNFLAGS		+= $(RUNFLAGS.$(PLATFORM))
RUNFLAGS		+= $(RUNFLAGS.$(CONFIG).$(PLATFORM))

PROFFLAGS		+= $(PROFFLAGS.$(PLATFORM))

LFLAGADDLIBDIR		= $(firstword $(LFLAGADDLIBDIR.$(PLATFORM)) -L)

INCDIRS			+= $($(VARPREFIX.$(PLATFORM))_INCDIRS)
SRCDIRS			+= $($(VARPREFIX.$(PLATFORM))_SRCDIRS)
LIBDIRS			+= $($(VARPREFIX.$(PLATFORM))_LIBDIRS)
SRCDIRS			+= $(SRCDIR)

# Header search options
CFLAGS_DEPENDENT += $(foreach idir, $(INCDIRS), $(CFLAGADDINCDIR) $(idir))

CFLAGS_DEPENDENT	+= $(CFLAGSAVETEMP.$(SAVETEMPS))

# Library search options
LFLAGS_DEPENDENT += $(foreach ldir, $(LIBDIRS), $(LFLAGADDLIBDIR) $(ldir))

# Configuration-dependent options
RFLAGS.release = -D__RENAMING__
RFLAGS.debug = -D__RENAMING__
RFLAGS.ferret =


CFLAGS_DEPENDENT += $(CFLAGS.$(CONFIG))
LFLAGS_DEPENDENT += $(LFLAGS.$(CONFIG)) $(LFLAGS.$(BUILD_TYPE).$(SOLID_LIBRARY))
CFLAGS_DEPENDENT += -Wall -Wno-return-type
ifeq (0, $(CPP))
CFLAGS_DEPENDENT += -std=c99
endif
#!!!!CFLAGS_DEPENDENT += -Wno-error
CFLAGS_DEPENDENT += -Werror
#CFLAGS_DEPENDENT += -Wno-error
CFLAGS_DEPENDENT += $(RFLAGS.$(CONFIG))

###############################################################
##                     Create target spec                    ##
###############################################################

TARGET_SUFFIX.win32	= .exe

TARGET_SUFFIX		= $(firstword $(TARGET_SUFFIX.$(BUILD_TYPE))\
			  $(TARGET_SUFFIX.$(HOSTOS)))

TARGET_SPEC.LIBRARY	= $(OUTDIR)/$(TARGET)$(POSTFIX).a
TARGET_SPEC.EXECUTABLE	= $(OUTDIR)/$(TARGET)$(POSTFIX)$(TARGET_SUFFIX)

TARGET_INTERMEDIATE.LIBRARY.1.1	= $(INTERMEDIATE_DIR)/$(TARGET).solid.1.o
TARGET_INTERMEDIATE.LIBRARY.1.2	= $(INTERMEDIATE_DIR)/$(TARGET).solid.2.o
TARGET_INTERMEDIATE.LIBRARY.1.3	= $(INTERMEDIATE_DIR)/$(TARGET).solid.o

###############################################################
##                    Complete module list                   ##
###############################################################

MODULES += $($(VARPREFIX.$(PLATFORM))_MODULES)
MODULES += $($(VARPREFIX.$(TARGETOS))_MODULES)
MODULES += $($(VARPREFIX.unix-like.$(ULIS))_MODULES)
# Keep compatibility with IntegrIT makefiles below 0.30
MODULES += $(OBJS:.o=)
MODULES += $($(VARPREFIX.$(PLATFORM))_OBJS:.o=)
MODULES += $($(VARPREFIX.$(TARGETOS))_OBJS:.o=)
MODULES += $($(VARPREFIX.unix-like.$(ULIS))_OBJS:.o=)

#     WIN32_MODULES, UNIX_LIKE_MODULES, MAC_MODULES

###############################################################
##           Generate list of objects to be built            ##
###############################################################

ALL_OBJECTS = $(foreach module, $(MODULES), $(INTERMEDIATE_DIR)/$(module).o)

###############################################################
##        Generate list of extra libraries to be linked      ##
###############################################################

ALL_DLIBS=$(foreach lib,$(DLIBS),$(lib)$(POSTFIX_DLC).a)

###############################################################
##                     Dependency generator                  ##
###############################################################

ifneq (1,$(CPP))
vpath %.c $(SRCDIRS) $(HIFI_CSTUB_SRCDIRS)
else
vpath %.c $(SRCDIRS)
vpath %.cpp $(SRCDIRS) $(HIFI_CSTUB_SRCDIRS)
endif

COMMAND_DEPGEN =  $(CC)
COMMAND_DEPGEN += $(CFLAGS_DEPENDENT)
COMMAND_DEPGEN += $(CFLAGS_DIR_$(subst -,_,$(notdir $(dir $(1)))))
COMMAND_DEPGEN += $(CFLAGS_FILE_$(subst -,_,$(notdir $(basename $(1)))))
COMMAND_DEPGEN += $(CFLAGS)
COMMAND_DEPGEN += $(CFLAGMAKEDEP)
COMMAND_DEPGEN += -MQ $(2)
COMMAND_DEPGEN += -MQ $(2:.d=.o)
COMMAND_DEPGEN += $(1)
COMMAND_DEPGEN += -o $(2)

$(INTERMEDIATE_DIR)/%.d: %.c $(INTERMEDIATE_DIR)/dirinfo.txt
	$(call COMMAND_DEPGEN,$<,$@)

$(INTERMEDIATE_DIR)/%.d: %.cpp $(INTERMEDIATE_DIR)/dirinfo.txt
	$(call COMMAND_DEPGEN,$<,$@)

# Include dependencies
-include $(addprefix $(INTERMEDIATE_DIR)/, $(addsuffix .d, $(MODULES)))

###############################################################
##                        Compiling rules                    ##
###############################################################

COMMAND_COMPILE =  $(CC)
COMMAND_COMPILE += $(CFLAGS_DEPENDENT)
COMMAND_COMPILE += $(CFLAGS_DIR_$(subst -,_,$(notdir $(dir $(1)))))
COMMAND_COMPILE += $(CFLAGS_FILE_$(subst -,_,$(notdir $(basename $(1)))))
COMMAND_COMPILE += $(CFLAGS)
COMMAND_COMPILE += $(CFLAGMAKEOBJ)
COMMAND_COMPILE += $(1)
COMMAND_COMPILE += -o $(2) 

$(INTERMEDIATE_DIR)/%.o: %.c $(INTERMEDIATE_DIR)/dirinfo.txt
	$(call COMMAND_COMPILE,$<,$@)

$(INTERMEDIATE_DIR)/%.o: %.cpp $(INTERMEDIATE_DIR)/dirinfo.txt
	$(call COMMAND_COMPILE,$<,$@)

###############################################################
##                    Linking/archiving rules                ##
###############################################################

ALL_LIBS = $(ALL_DLIBS) $(foreach lib,$(LIBS),-l$(lib))

LD_LIST.  = $(ALL_OBJECTS)
LD_LIST.0 = $(ALL_OBJECTS)
LD_LIST.1 = $(TARGET_INTERMEDIATE.$(BUILD_TYPE).$(SOLID_LIBRARY).3)

$(TARGET_INTERMEDIATE.LIBRARY.1.1): $(ALL_OBJECTS) $(ALL_DLIBS)
	$(LD) $(LFLAGS_DEPENDENT) $(LFLAGS) -o \
	$(TARGET_INTERMEDIATE.LIBRARY.1.1) $(ALL_OBJECTS) $(ALL_LIBS)

$(TARGET_INTERMEDIATE.LIBRARY.1.2):	$(TARGET_INTERMEDIATE.LIBRARY.1.1)
	$(OBJCOPY) --keep-global-symbols=$(TARGET_SYMFILE) \
	$(TARGET_INTERMEDIATE.LIBRARY.1.1) $(TARGET_INTERMEDIATE.LIBRARY.1.2)

$(TARGET_INTERMEDIATE.LIBRARY.1.3):	$(TARGET_INTERMEDIATE.LIBRARY.1.2)
	$(OBJCOPY) --strip-unneeded \
	$(TARGET_INTERMEDIATE.LIBRARY.1.2) $(TARGET_INTERMEDIATE.LIBRARY.1.3)

genlist.cmd:
	$(ECHO) @ECHO OFF>genlist.cmd
	$(ECHO) IF EXIST %%2 DEL %%2>>genlist.cmd
	$(ECHO) FOR %%%%i IN (%%1\*.o) DO (>>genlist.cmd
	$(ECHO)   ECHO %%%%i^>^>%%2>>genlist.cmd
	$(ECHO) )>>genlist.cmd

GENLIST = $(if $(WINDIR),genlist.cmd,)

$(INTERMEDIATE_DIR)/$(TARGET).ld: $(LD_LIST.$(SOLID_LIBRARY)) $(GENLIST)
ifeq (,$(WINDIR))
	$(shell $(RM) $@) $(foreach mod,$(LD_LIST.$(SOLID_LIBRARY)),$(shell $(ECHO) $(mod)>>$@))
else
	$(GENLIST) $(INTERMEDIATE_DIR) $(subst /,$(PSEP),$@)
endif

$(TARGET_SPEC.LIBRARY):	$(EXTERNAL_PROJECTS) $(LD_LIST.$(SOLID_LIBRARY)) $(INTERMEDIATE_DIR)/$(TARGET).ld
	$(RM) $(subst /,$(PSEP),$@)
	$(AR) rcs $@ @$(INTERMEDIATE_DIR)/$(TARGET).ld

$(TARGET_SPEC.EXECUTABLE): $(EXTERNAL_PROJECTS) $(ALL_OBJECTS) $(ALL_DLIBS)
	$(LD) $(LFLAGS_DEPENDENT) $(LFLAGS) -o \
	$(TARGET_SPEC.EXECUTABLE) $(ALL_OBJECTS) $(ALL_LIBS)

all:	$(TARGET_SPEC.$(BUILD_TYPE))

$(foreach proj,$(EXTERNAL_PROJECTS),$(proj)):
	make -C $@ CONFIG=$(CONFIG) all

###############################################################
##                         Clean rules                       ##
###############################################################
$(foreach proj,$(EXTERNAL_PROJECTS),$(proj)/clean):
	make -C $(dir $@) CONFIG=$(CONFIG) clean
$(foreach proj,$(EXTERNAL_PROJECTS),$(proj)/cleanest):
	make -C $(dir $@) CONFIG=$(CONFIG) cleanest

clean:	$(foreach proj,$(EXTERNAL_PROJECTS),$(proj)/clean)
	-$(RM) $(subst /,$(PSEP),$(INTERMEDIATE_DIR)/*.o)
	-$(RM) $(subst /,$(PSEP),$(TARGET_SPEC.$(BUILD_TYPE)))

cleanest:	$(foreach proj,$(EXTERNAL_PROJECTS),$(proj)/cleanest)
	-$(RM) $(subst /,$(PSEP),$(INTERMEDIATE_DIR)/*.d)
	-$(RM) $(subst /,$(PSEP),$(INTERMEDIATE_DIR)/*.o)
	-$(RM) $(subst /,$(PSEP),$(TARGET_SPEC.$(BUILD_TYPE)))

cleanlib:
	-$(RM) $(subst /,$(PSEP),$(TARGET_SPEC.LIBRARY)) 2>$(NULLDEV)
