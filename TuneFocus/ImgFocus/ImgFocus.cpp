#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include "ImgFocus.h"
#include "CxImgA.h"

#pragma comment(lib, "CImageB.lib")

//////////////////////////////////////////////////////////////////////////
#define  IMGF_TMPFile1		_T("_AAAA1.jpg")
//#define  IMGF_TMPFile1		_T("\\A1.png")
#define  IMGF_TMPFile2		_T("\\B1.jpg")
#define  IMGF_TMPFile3		_T("\\B2.jpg")
//#define  IMGF_TMPFile4		_T("\\B4.bmp")
#define  IMGF_FRAME_CNT		(2)
#define  IMGF_FRAME_MAXCNT	(4)

TCHAR g_szTmpPath[1024] = {0}; 

ST_HuaImgInfo g_stCurImgInfo = {0};
ST_HuaImgBinData g_stCurHwaBinInfo = {0};
ST_HuaImgSet g_stImgSet0 = {0};

//////////////////////////////////////////////////////////////////////////
int SetAnaTempPath(const TCHAR* szTmpPath)
{
	int ret = 0; 
	_tcscpy(g_szTmpPath, szTmpPath);
	_tcscat(g_szTmpPath, _T("\\Temp"));
	CreateDirectory(g_szTmpPath, NULL);
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void CleanFilesInTempPath()
{
	TCHAR w_szOldPath[1024] = {0};
	TCHAR szFile1[1024];
	HANDLE find_handle;
	WIN32_FIND_DATA w_stFiles;
	int w_ret = 0;	
	int i, nn;

	if (g_szTmpPath[0] == 0){
		return;
	}
	
	::GetCurrentDirectory(1000, w_szOldPath);
	::SetCurrentDirectory(g_szTmpPath);

	find_handle = FindFirstFile( _T("*.*"), &w_stFiles );
	FindNextFile( find_handle, &w_stFiles);
	FindNextFile( find_handle, &w_stFiles);
	nn = 0;
	if (find_handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			memset(szFile1, 0, sizeof(szFile1));
			swprintf_s(szFile1, 1000, _T("%s\\%s"), g_szTmpPath, w_stFiles.cFileName);
			DeleteFile(szFile1);			
		} while( FindNextFile( find_handle , &w_stFiles ) );
	}
	FindClose( find_handle );
	::SetCurrentDirectory(w_szOldPath);
	return;
}

//////////////////////////////////////////////////////////////////////////
int GetResultTmpFilePath(const TCHAR* p_pszFile, TCHAR* p_pszTmpFile)
{
	int nn;
	TCHAR szFile2[1024] = {0};

	_tcscpy(szFile2, p_pszFile);
	PathStripPath(szFile2);
	p_pszTmpFile[0] = 0;	
	swprintf(p_pszTmpFile, _T("%s\\%s"), g_szTmpPath, szFile2);
	nn = _tcslen(p_pszTmpFile);
	p_pszTmpFile[nn-4] = 0;
	_tcscat(p_pszTmpFile, IMGF_TMPFile1);
	return 0;	
}

//////////////////////////////////////////////////////////////////////////
int ReleaseImgAnaMem()
{
	if (g_stCurHwaBinInfo.m_pbData1){
		free(g_stCurHwaBinInfo.m_pbData1);
		g_stCurHwaBinInfo.m_pbData1 = NULL;
	}
	if (g_stCurHwaBinInfo.m_pbData2){
		free(g_stCurHwaBinInfo.m_pbData2);
		g_stCurHwaBinInfo.m_pbData2 = NULL;
	}
	if (g_stCurHwaBinInfo.m_pbData3){
		free(g_stCurHwaBinInfo.m_pbData3);
		g_stCurHwaBinInfo.m_pbData3 = NULL;
	}
	memset(&g_stCurHwaBinInfo, 0, sizeof(ST_HuaImgBinData));

	if (g_stImgSet0.m_pbGImage)
	{
		free(g_stImgSet0.m_pbGImage);
	}
	if (g_stImgSet0.m_pbFImage)
	{
		free(g_stImgSet0.m_pbFImage);
	}	
	memset(&g_stImgSet0, 0, sizeof(ST_HuaImgSet));
	return 0;
}

//////////////////////////////////////////////////////////////////////////
int GetFValPerc(int posX, int posY, int* p_pnFVal)
{
	int mx, my, nFVal1, temp;
	if (p_pnFVal == NULL){
		return REFOCUS_Err_InvalidParam;
	}
	if (g_stImgSet0.m_stMeta.width == 0 || g_stImgSet0.m_stMeta.height == 0){
		return REFOCUS_Err_InvalidInfo;
	}
	mx = posX / 4;
	my = posY / 4;
	if (mx < 0)
		mx = 0;
	else if (mx >= g_stImgSet0.m_stMeta.width)
		mx = g_stImgSet0.m_stMeta.width - 1;
	if (my < 0)
		my = 0;
	else if (my >= g_stImgSet0.m_stMeta.height)
		my = g_stImgSet0.m_stMeta.height - 1;

	nFVal1 = g_stImgSet0.m_stMeta.metaBin[my*g_stImgSet0.m_stMeta.width+mx];
	temp = g_stImgSet0.m_stMeta.nFocalMax - g_stImgSet0.m_stMeta.nFocalMin;
	if (temp < 1)
		temp = 1;
	nFVal1 = ((nFVal1 - g_stImgSet0.m_stMeta.nFocalMin) * 100 + temp / 2) / temp;
	if (nFVal1 > 100)
		nFVal1 = 100;
	p_pnFVal[0] = nFVal1;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// File Format analysis
#define ImgF_Meta_WidthPos	(0x18)
#define ImgF_Meta_HeightPos	(0x1A)
#define ImgF_Meta_MatrixPos	(0x4C)

int GetMetaDataOfHuaWeiJpeg(int p_nDataSize, BYTE* p_pbData, ST_HuaImgMeta* p_pstMeta)
{
	int ret = 0;
	int width1, height1;

	if (p_nDataSize < ImgF_Meta_MatrixPos || p_pbData == NULL || p_pstMeta == NULL)
	{
		ret = REFOCUS_Err_InvalidParam;
		goto L_EXIT;
	}
	width1 = 0x100 * p_pbData[ImgF_Meta_WidthPos+1] + p_pbData[ImgF_Meta_WidthPos];
	height1 = 0x100 * p_pbData[ImgF_Meta_HeightPos+1] + p_pbData[ImgF_Meta_HeightPos];
	if (p_nDataSize < ImgF_Meta_MatrixPos + width1 * height1){
		ret = REFOCUS_Err_InvalidInfo;
		goto L_EXIT;
	}	
	p_pstMeta->width = width1;
	p_pstMeta->height = height1;
	p_pstMeta->metaBin = p_pbData + ImgF_Meta_MatrixPos;
L_EXIT:
	return ret;
}

int FindCorpusJPEG(int size0, BYTE* p_pbData, ST_HuaImgBinData* p_pstCorpus, ST_HuaImgSet* p_pstImgSet)
{
	int ret = 0;
	int ii, pos;
	int nFrameCnt = 0;
	int nFramePos[IMGF_FRAME_MAXCNT] = {0};
	int nMetaDataStart = -1;
	int yy, xx, nFocalMin, nFocalMax, nWidth1;

	for( ii = 0; ii < size0 - 4; ii ++)
	{
		if (p_pbData[ii] == 0xFF && p_pbData[ii+1] == 0xD8 && 
			p_pbData[ii+2] == 0xFF && p_pbData[ii+3] == 0xE1)
		{
			if (nFrameCnt < IMGF_FRAME_MAXCNT)
			{
				nFramePos[nFrameCnt] = ii;
				nFrameCnt ++;
			}
		}
	}
	for( ii = size0 - 2; ii >= 0; ii --)
	{
		if (p_pbData[ii] == 0xFF && p_pbData[ii+1] == 0xD9)
		{
			nMetaDataStart = ii+2;
			break;
		}
	}
	if (nMetaDataStart < 0){
		ret = REFOCUS_Err_InvalidInfo;
		goto L_EXIT;
	}
	if (nFrameCnt == 0)
	{// common JPEG
		p_pstCorpus->nDataLen1 = nMetaDataStart; 
		p_pstCorpus->m_pbData1 = (BYTE*)malloc(p_pstCorpus->nDataLen1);
		if (p_pstCorpus->m_pbData1 == NULL){
			ret = REFOCUS_Err_MemOverFlow;
			goto L_EXIT;
		}
		memcpy(p_pstCorpus->m_pbData1, p_pbData, p_pstCorpus->nDataLen1);
		p_pstCorpus->imgType = ImgF_Type_CommonJPEG;
	}
	else if (nFrameCnt != IMGF_FRAME_CNT)
	{
		p_pstCorpus->imgType = ImgF_Type_Unknown;
	}
	else
	{
		pos = 0;
		p_pstCorpus->nDataLen1 = nFramePos[1];
		p_pstCorpus->nDataLen2 = nMetaDataStart-nFramePos[1];
		p_pstCorpus->nDataLen3 = size0 - nMetaDataStart;
		p_pstCorpus->m_pbData1 = (BYTE*)malloc(p_pstCorpus->nDataLen1);
		if (p_pstCorpus->m_pbData1 == NULL){
			ret = REFOCUS_Err_MemOverFlow;
			goto L_EXIT;
		}
		memcpy(p_pstCorpus->m_pbData1, p_pbData, p_pstCorpus->nDataLen1);
		pos += p_pstCorpus->nDataLen1;

		p_pstCorpus->m_pbData2 = (BYTE*)malloc(p_pstCorpus->nDataLen2);
		if (p_pstCorpus->m_pbData2 == NULL){
			ret = REFOCUS_Err_MemOverFlow;
			goto L_EXIT;
		}
		memcpy(p_pstCorpus->m_pbData2, p_pbData+pos, p_pstCorpus->nDataLen2);
		pos += p_pstCorpus->nDataLen2;

		p_pstCorpus->m_pbData3 = (BYTE*)malloc(p_pstCorpus->nDataLen3);
		if (p_pstCorpus->m_pbData3 == NULL){
			ret = REFOCUS_Err_MemOverFlow;
			goto L_EXIT;
		}
		memcpy(p_pstCorpus->m_pbData3, p_pbData+pos, p_pstCorpus->nDataLen3);

		ret = GetMetaDataOfHuaWeiJpeg(p_pstCorpus->nDataLen3, p_pstCorpus->m_pbData3, &(p_pstImgSet->m_stMeta));

		if (ret == 0)
		{
			p_pstCorpus->imgType = ImgF_Type_HwaWeiJPEG;

			nFocalMin = 0xFF;
			nFocalMax = 0x0;
			nWidth1 = p_pstImgSet->m_stMeta.width;
			for( yy = 0; yy < p_pstImgSet->m_stMeta.height; yy ++)
			{
				for( xx = 0; xx < p_pstImgSet->m_stMeta.width; xx ++)
				{
					if (nFocalMin > p_pstImgSet->m_stMeta.metaBin[yy*nWidth1+xx])
					{
						nFocalMin = p_pstImgSet->m_stMeta.metaBin[yy*nWidth1+xx];
					}
					if (nFocalMax < p_pstImgSet->m_stMeta.metaBin[yy*nWidth1+xx])
					{
						nFocalMax = p_pstImgSet->m_stMeta.metaBin[yy*nWidth1+xx];
					}
				}
			}
			p_pstImgSet->m_stMeta.nFocalMin = nFocalMin;
			p_pstImgSet->m_stMeta.nFocalMax = nFocalMax;
		}
	}
L_EXIT:
	return ret;
}

int g_nSqrtN[256] = {
	0, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 
	4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 
	6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
	8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 
	10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 
	11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 
	12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 
	14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
};

void CreateIntImage( int p_nImW, int p_nImH, BYTE* p_pbImage, unsigned __int64** w_pnIntImage)
{
	int yy, xx, ii;
	int width1 = p_nImW + 1;
	unsigned __int64 sum0[3] = {0};
	BYTE* p1;

	for( xx = 0; xx < width1; xx ++)
	{
		w_pnIntImage[0][xx] = 0;
		w_pnIntImage[1][xx] = 0;
		w_pnIntImage[2][xx] = 0;
	}
	p1 = p_pbImage;
	for( yy = 0; yy < p_nImH; yy ++)
	{
		sum0[0] = 0;
		sum0[1] = 0;
		sum0[2] = 0;
		w_pnIntImage[0][(yy+1)*width1] = 0;
		w_pnIntImage[1][(yy+1)*width1] = 0;
		w_pnIntImage[2][(yy+1)*width1] = 0;
		for( xx = 0; xx < p_nImW; xx ++, p1 += 3)
		{
			sum0[0] += p1[0];
			sum0[1] += p1[1];
			sum0[2] += p1[2];
			w_pnIntImage[0][(yy+1)*width1+xx+1] = w_pnIntImage[0][yy*width1+xx+1] + sum0[0]; 
			w_pnIntImage[1][(yy+1)*width1+xx+1] = w_pnIntImage[1][yy*width1+xx+1] + sum0[1]; 
			w_pnIntImage[2][(yy+1)*width1+xx+1] = w_pnIntImage[2][yy*width1+xx+1] + sum0[2]; 
		}
	}
	return;
}

//////////////////////////////////////////////////////////////////////////
int GetFocusedImageImpl(ST_HuaImgSet* p_pstImgSet, int p_nFocalVal, BYTE* p_pbImg3)
{
	int ret = 0;
	int yy, xx, nFocalVal, yy1, xx1, nAvgThr1;
	int nWidth1, nWidth2; 
	BYTE* w_pbFocalMat;
	int sum0, sum1, val2, ywgt, xwgt;
	int nRadii1;
	int ii, yy2, xx2, yy3, xx3;
	int yy4[2], xx4[2];
	BYTE* p1;
	unsigned __int64 *w_pnIntImage[3] = {0}; 
	unsigned __int64 sum22;

	if (p_pstImgSet == NULL || p_pbImg3 == NULL ||
		p_pstImgSet->m_width < 1 || p_pstImgSet->m_height < 1 || p_pstImgSet->m_pbFImage == NULL || p_pstImgSet->m_pbGImage == NULL)
	{
		ret = REFOCUS_Err_InvalidParam;
		goto L_EXIT;
	}
	if (p_nFocalVal < 0){
		p_nFocalVal = 0;
	}
	else if (p_nFocalVal > 100){
		p_nFocalVal = 100;
	}
	
	nFocalVal = (p_pstImgSet->m_stMeta.nFocalMin * (100 - p_nFocalVal) + p_pstImgSet->m_stMeta.nFocalMax * p_nFocalVal + 50) / 100;

	w_pbFocalMat = p_pstImgSet->m_stMeta.metaBin;
	nWidth1 = p_pstImgSet->m_stMeta.width;
	nWidth2 = p_pstImgSet->m_width;

	if (nFocalVal == p_pstImgSet->m_stMeta.nFocalMin && p_pstImgSet->m_pbGImage != NULL)
	{
		memcpy(p_pbImg3, p_pstImgSet->m_pbGImage, p_pstImgSet->m_width * p_pstImgSet->m_height * 3);
	}	
	else
	{
		for( ii = 0; ii < 3; ii ++)
		{
			w_pnIntImage[ii] = (unsigned __int64*)malloc(sizeof(unsigned __int64) * (nWidth2 + 1) * (p_pstImgSet->m_height + 1));
			if (w_pnIntImage[ii] == NULL){
				ret = REFOCUS_Err_MemOverFlow;
				goto L_EXIT;
			}			
		}		
		CreateIntImage(nWidth2, p_pstImgSet->m_height, p_pstImgSet->m_pbFImage, w_pnIntImage);

		for( yy = 0; yy < p_pstImgSet->m_height; yy ++)
		{
			yy1 = yy / 4;
			if (yy1 >= p_pstImgSet->m_stMeta.height){
				yy1 = p_pstImgSet->m_stMeta.height - 1;
			}
			for( xx = 0; xx < p_pstImgSet->m_width; xx ++)
			{
				xx1 = xx / 4;
				if (xx1 >= p_pstImgSet->m_stMeta.width){
					xx1 = p_pstImgSet->m_stMeta.width - 1;
				}
				nAvgThr1 = abs(nFocalVal - w_pbFocalMat[yy1*nWidth1+xx1]);
				nAvgThr1 = g_nSqrtN[nAvgThr1];
				if (nAvgThr1 < 2)
				{
					p_pbImg3[(yy*nWidth2+xx)*3] = p_pstImgSet->m_pbFImage[(yy*nWidth2+xx)*3];
					p_pbImg3[(yy*nWidth2+xx)*3+1] = p_pstImgSet->m_pbFImage[(yy*nWidth2+xx)*3+1];
					p_pbImg3[(yy*nWidth2+xx)*3+2] = p_pstImgSet->m_pbFImage[(yy*nWidth2+xx)*3+2];
				}
				else
				{
					nRadii1 = nAvgThr1 * 2 - 1;
					if (xx < nRadii1 || xx >= nWidth2 - nRadii1 || yy < nRadii1 || yy >= p_pstImgSet->m_height - nRadii1)
					{
						for( ii = 0; ii < 3; ii ++)
						{
							sum0 = 0;
							sum1 = 0;
							for( yy2 = 2; yy2 <= nRadii1; yy2 += 2)
							{
								yy4[0] = yy-yy2;
								if (yy4[0] < 0)
									yy4[0] = 0;
								yy4[1] = yy+yy2+1;
								if (yy4[1] > p_pstImgSet->m_height)
									yy4[1] = p_pstImgSet->m_height;
								xx4[0] = xx-yy2;
								if (xx4[0] < 0)
									xx4[0] = 0;
								xx4[1] = xx+yy2+1;
								if (xx4[1] > nWidth2)
									xx4[1] = nWidth2;
								sum22 = w_pnIntImage[ii][yy4[1]*(nWidth2+1)+xx4[1]] + w_pnIntImage[ii][yy4[0]*(nWidth2+1)+xx4[0]] - 
									w_pnIntImage[ii][yy4[1]*(nWidth2+1)+xx4[0]] - w_pnIntImage[ii][yy4[0]*(nWidth2+1)+xx4[1]];
								sum1 += (int)sum22;
								sum0 += (yy4[1] - yy4[0]) * (xx4[1] - xx4[0]);
							}
							sum1 += p_pstImgSet->m_pbFImage[(yy*nWidth2+xx)*3+ii];
							sum0 ++;
							xx3 = sum1 / sum0;
							if (xx3 > 255){
								xx3 = 255;
							}
							p_pbImg3[(yy*nWidth2+xx)*3+ii] = (BYTE)xx3;
						}
					}
					else
					{
						for( ii = 0; ii < 3; ii ++)
						{
							sum0 = 0;
							sum1 = 0;
							for( yy2 = 2; yy2 <= nRadii1; yy2 += 2)
							{
								sum22 = w_pnIntImage[ii][(yy+yy2+1)*(nWidth2+1)+(xx+yy2+1)] + w_pnIntImage[ii][(yy-yy2)*(nWidth2+1)+(xx-yy2)] - 
									w_pnIntImage[ii][(yy+yy2+1)*(nWidth2+1)+(xx-yy2)] - w_pnIntImage[ii][(yy-yy2)*(nWidth2+1)+(xx+yy2+1)];
								sum1 += (int)sum22;
								sum0 += (yy2 * 2 + 1) * (yy2 * 2 + 1);
							}
							sum1 += p_pstImgSet->m_pbFImage[(yy*nWidth2+xx)*3+ii];
							sum0 ++;
							xx3 = sum1 / sum0;
							if (xx3 > 255){
								xx3 = 255;
							}
							p_pbImg3[(yy*nWidth2+xx)*3+ii] = (BYTE)xx3;
						}
					}					
				}
			}
		}
	}
L_EXIT:
	if (w_pnIntImage[0]){
		free(w_pnIntImage[0]);
	}
	if (w_pnIntImage[1]){
		free(w_pnIntImage[1]);
	}
	if (w_pnIntImage[2]){
		free(w_pnIntImage[2]);
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
int GetFocusedImage(const TCHAR* p_pszImage, int p_nFocalVal)
{
	int width1, height1;
	BYTE* img3 = NULL;
	int ret;
	ST_MCXImg stImage = {0};
	TCHAR szFile1[1024] = {0};

	height1 = g_stImgSet0.m_height;
	width1 = g_stImgSet0.m_width;
	img3 = (BYTE*)malloc(width1 * height1 * 3);
	if (img3 == NULL)
	{
		ret = REFOCUS_Err_MemOverFlow;
		goto L_EXIT;
	}

	ret = GetFocusedImageImpl(&g_stImgSet0, p_nFocalVal, img3);
	if (ret){
		goto L_EXIT;
	}

	stImage.width = width1;
	stImage.height = height1;
	stImage.ptr = img3;
	stImage.alpha = NULL;
	GetResultTmpFilePath(p_pszImage, szFile1);	
	ret = CxSaveImage(szFile1, &stImage, 1);
	if (ret)
	{
		ret = REFOCUS_Err_SaveFailed;
	}

L_EXIT:
	if (img3) 
		free(img3);

	return ret;
}

//////////////////////////////////////////////////////////////////////////
int AnaBinData(const TCHAR* szFile)
{
	int ret = 0;
	FILE* fp = NULL;
	BYTE* w_pbData0 = NULL;
	int ii;
	int size0;	
	ST_MCXImg stImg0 = {0};
	TCHAR szTmpFile[1024];

	//. init
	if (g_stCurHwaBinInfo.m_pbData1){
		free(g_stCurHwaBinInfo.m_pbData1);
		g_stCurHwaBinInfo.m_pbData1 = NULL;
	}
	if (g_stCurHwaBinInfo.m_pbData2){
		free(g_stCurHwaBinInfo.m_pbData2);
		g_stCurHwaBinInfo.m_pbData2 = NULL;
	}
	if (g_stCurHwaBinInfo.m_pbData3){
		free(g_stCurHwaBinInfo.m_pbData3);
		g_stCurHwaBinInfo.m_pbData3 = NULL;
	}
	memset(&g_stCurHwaBinInfo, 0, sizeof(ST_HuaImgBinData));
	
	// read file
	ret = _tfopen_s(&fp, szFile, _T("rb"));
	if (ret != 0 || fp == NULL)
	{
		ret = REFOCUS_Err_LoadFailed;
		goto L_EXIT;
	}
	fseek(fp, 0, SEEK_END);
	size0 = ftell(fp);
	w_pbData0 = (BYTE*)malloc(size0);
	if (w_pbData0 == NULL){
		ret = REFOCUS_Err_MemOverFlow;
		goto L_EXIT;
	}
	fseek(fp, 0, SEEK_SET);
	fread(w_pbData0, 1, size0, fp);
	fclose(fp);
	fp = NULL;

	ret = FindCorpusJPEG(size0, w_pbData0, &g_stCurHwaBinInfo, &g_stImgSet0);

	if (ret == 0)
	{
		if (g_stCurHwaBinInfo.imgType == ImgF_Type_CommonJPEG ||
			g_stCurHwaBinInfo.imgType == ImgF_Type_HwaWeiJPEG)
		{
			memset(szTmpFile, 0, sizeof(szTmpFile));
			_tcscpy(szTmpFile, g_szTmpPath);
			_tcscat(szTmpFile, IMGF_TMPFile2);
			ret = _tfopen_s(&fp, szTmpFile, _T("wb"));
			if (fp == NULL || ret != 0){
				ret = REFOCUS_Err_SaveFailed;
				goto L_EXIT;
			}
			fwrite(g_stCurHwaBinInfo.m_pbData1, 1, g_stCurHwaBinInfo.nDataLen1, fp);
			fclose(fp);
			fp = NULL;
		}
		if (g_stCurHwaBinInfo.imgType == ImgF_Type_HwaWeiJPEG)
		{
			memset(szTmpFile, 0, sizeof(szTmpFile));
			_tcscpy(szTmpFile, g_szTmpPath);
			_tcscat(szTmpFile, IMGF_TMPFile3);
			ret = _tfopen_s(&fp, szTmpFile, _T("wb"));
			if (fp == NULL || ret != 0){
				ret = REFOCUS_Err_SaveFailed;
				goto L_EXIT;
			}
			fwrite(g_stCurHwaBinInfo.m_pbData2, 1, g_stCurHwaBinInfo.nDataLen2, fp);
			fclose(fp);
			fp = NULL;	

// 			memset(szTmpFile, 0, sizeof(szTmpFile));
// 			_tcscpy(szTmpFile, g_szTmpPath);
// 			_tcscat(szTmpFile, IMGF_TMPFile4);
// 			stImg0.width = g_stImgSet0.m_stMeta.width;
// 			stImg0.height = g_stImgSet0.m_stMeta.height;
// 			stImg0.ptr = g_stImgSet0.m_stMeta.metaBin;
// 			stImg0.alpha = NULL;
// 			ret = CxSaveImage(szTmpFile, &stImg0, 0);
		}
	}

L_EXIT:
	if (w_pbData0){
		free(w_pbData0);
	}
	if (fp){
		fclose(fp);
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
int OpenHuaWeiPJpgFile(const TCHAR* p_pszFile)
{
	int ret = 0;
	ST_MCXImg stImage = {0};
	int imgtype;
	TCHAR szFile1[1024] = {0};	

	//. init
	memset(&g_stCurImgInfo, 0, sizeof(ST_HuaImgInfo));

	if (g_stImgSet0.m_pbGImage)
	{
		free(g_stImgSet0.m_pbGImage);
	}
	if (g_stImgSet0.m_pbFImage)
	{
		free(g_stImgSet0.m_pbFImage);
	}	
	memset(&g_stImgSet0, 0, sizeof(ST_HuaImgSet));

	ret = AnaBinData(p_pszFile);
	if (ret != 0)
	{
		goto L_EXIT;
	}

	if (g_stCurHwaBinInfo.m_pbData2 != NULL)
	{
		_tcscpy(szFile1, g_szTmpPath);
		_tcscat(szFile1, IMGF_TMPFile3);
		ret = CxLoadImage(szFile1, &imgtype, &stImage, 1);

		if (ret){
			ret = REFOCUS_Err_LoadFailed;
			goto L_EXIT;
		}

		g_stImgSet0.m_width = stImage.width;
		g_stImgSet0.m_height = stImage.height;
		g_stImgSet0.m_pbFImage = (BYTE*)malloc(stImage.width * stImage.height * 3);
		if (g_stImgSet0.m_pbFImage == NULL)
		{
			ret = REFOCUS_Err_MemOverFlow;
			goto L_EXIT;
		}
		memcpy(g_stImgSet0.m_pbFImage, stImage.ptr, stImage.width * stImage.height * 3);
		if (stImage.ptr){
			FreeCxImageBuffer(stImage.ptr);
			stImage.ptr = NULL;
		}
		if (stImage.alpha){
			FreeCxImageBuffer(stImage.alpha);	
			stImage.alpha = NULL;
		}

		_tcscpy(szFile1, g_szTmpPath);
		_tcscat(szFile1, IMGF_TMPFile2);
		ret = CxLoadImage(szFile1, &imgtype, &stImage, 1);
		if (ret){
			ret = REFOCUS_Err_LoadFailed;
			goto L_EXIT;
		}

		g_stImgSet0.m_pbGImage = (BYTE*)malloc(stImage.width * stImage.height * 3);
		if (g_stImgSet0.m_pbGImage == NULL)
		{
			ret = REFOCUS_Err_MemOverFlow;
			goto L_EXIT;
		}
		memcpy(g_stImgSet0.m_pbGImage, stImage.ptr, stImage.width * stImage.height * 3);

		if (stImage.ptr){
			FreeCxImageBuffer(stImage.ptr);
		}
		if (stImage.alpha){
			FreeCxImageBuffer(stImage.alpha);	
		}
		
// 		ret = GetFocusedImage(p_pszFile, 0);
// 		if (ret){
// 			goto L_EXIT;
// 		}
	}
	else
	{
		ret = CxLoadImage(p_pszFile, &imgtype, &stImage, 1);

		if (ret)
		{
			ret = REFOCUS_Err_LoadFailed;
			goto L_EXIT;
		}

		GetResultTmpFilePath(p_pszFile, szFile1);
		ret = CxSaveImage(szFile1, &stImage, 1);
		if (ret)
		{
			ret = REFOCUS_Err_SaveFailed;
		}

		if (stImage.ptr){
			FreeCxImageBuffer(stImage.ptr);
		}
		if (stImage.alpha){
			FreeCxImageBuffer(stImage.alpha);	
		}
	}
	
	memset(g_stCurImgInfo.m_szCurImage, 0, sizeof(g_stCurImgInfo.m_szCurImage));
	_tcscpy(g_stCurImgInfo.m_szCurImage, p_pszFile);
	g_stCurImgInfo.m_nImgWidth = stImage.width;
	g_stCurImgInfo.m_nImgHeight = stImage.height;

L_EXIT:	

	return ret;
}

//////////////////////////////////////////////////////////////////////////
int OpenPhoneImageFile(const TCHAR* szFile)
{
	int ret = 0;
	ret = OpenHuaWeiPJpgFile(szFile);
	return ret;
}

//////////////////////////////////////////////////////////////////////////
int GetTmpJpgFileName(const TCHAR* p_pszFile, TCHAR* szFile, int* p_pnImageValid)
{
	szFile[0] = 0;	
	p_pnImageValid[0] = 0;
	if (g_stCurHwaBinInfo.m_pbData2 != NULL){
		p_pnImageValid[0] = 1;
	}
	GetResultTmpFilePath(p_pszFile, szFile);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
int GetCurHuaFileInfo(ST_HuaImgInfo* ptr)
{
	if (ptr == NULL){
		return REFOCUS_Err_InvalidParam;
	}
	if (g_stCurImgInfo.m_nImgWidth == 0 ||
		g_stCurImgInfo.m_nImgHeight == 0 ||
		g_stCurImgInfo.m_szCurImage[0] == 0)
	{
		return REFOCUS_Err_InvalidInfo;
	}
	memcpy(ptr, &g_stCurImgInfo, sizeof(ST_HuaImgInfo));
	return 0;
}

