//////////////////////////////////////////////////////////////////////////
//
//	2014/12/18		Flipping was added.
//	2014/12/19		IncreaseBpp was added
//
//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "CxImage/ximage.h"
#include "CxImgA.h"
#include "memmgr.h"

////////////////////////////////////////////////////////////////////////////////
TCHAR* FindExtension(const TCHAR * name)
{
	int len = _tcslen(name);
	int i;
	for (i = len-1; i >= 0; i--){
		if (name[i] == '.'){
			return (TCHAR*)(name+i+1);
		}
	}
	return (TCHAR*)(name+len);
}

//////////////////////////////////////////////////////////////////////////
int CxLoadImage(const TCHAR* p_pszFile, int* imgtype, ST_MCXImg* p_pstImg, int p_nFlag)
{
	int w_ret = 0;
	TCHAR w_szFile1[0x200] = {0};
	CxImage w_stCxLoader;
	BYTE* w_pbImg0;
	int i, ww0;

	if (p_pszFile == NULL || p_pszFile[0] == 0 || p_pstImg == NULL){
		return -1;
	}

	//. init
	memset(p_pstImg, 0, sizeof(ST_MCXImg));
	if (imgtype){
		imgtype[0] = -1;
	}

	_tcscpy(w_szFile1, p_pszFile);
	TCHAR* extin = FindExtension(w_szFile1);
	int typein = CxImage::GetTypeIdFromName(extin);

	if (!w_stCxLoader.Load(p_pszFile, typein))
	{
		printf("\nCxImage Load : FAILED");
		w_ret = -2;
		goto L_EXIT;
	}

	p_pstImg->width = w_stCxLoader.GetWidth();
	p_pstImg->height = w_stCxLoader.GetHeight();

	if (p_nFlag == 0)
	{
		w_stCxLoader.GrayScale();
		w_pbImg0 = w_stCxLoader.GetBits();
		ww0 = w_stCxLoader.GetEffWidth();
		p_pstImg->ptr = (BYTE*)alloc_mem(p_pstImg->width * p_pstImg->height);
		// flipping
		for( i = 0; i < p_pstImg->height; i ++)
		{
			memcpy(p_pstImg->ptr+(p_pstImg->height-1-i)*p_pstImg->width, 
				w_pbImg0+i*ww0, p_pstImg->width);
		}
	}
	else
	{
		ww0 = w_stCxLoader.GetBpp();
		if (ww0 < 24)
		{		
			if (!w_stCxLoader.IncreaseBpp(24))
			{
				printf("\nIncreaseBpp : FAILED!");
				goto L_EXIT;
			}
		}
		if (w_stCxLoader.AlphaGetPointer())
		{
			p_pstImg->alpha = (BYTE*)alloc_mem(p_pstImg->width * p_pstImg->height);
			for( int y = 0; y < p_pstImg->height; y ++){
				for( int x = 0; x < p_pstImg->width; x ++){
					p_pstImg->alpha[y*p_pstImg->width+x] = w_stCxLoader.AlphaGet(x, p_pstImg->height-1-y);
				}
			}
		}
		p_pstImg->ptr = (BYTE*)alloc_mem(p_pstImg->width * p_pstImg->height * 3);
		w_pbImg0 = w_stCxLoader.GetBits();
		ww0 = w_stCxLoader.GetEffWidth();
		// flipping
		for( i = 0; i < p_pstImg->height; i ++)
		{
			memcpy(p_pstImg->ptr+(p_pstImg->height-1-i)*p_pstImg->width*3, 
				w_pbImg0+i*ww0, p_pstImg->width*3);
		}		
	}

	if (imgtype){
		imgtype[0] = typein;
	}

L_EXIT:
	return w_ret;
}

//////////////////////////////////////////////////////////////////////////
int CxSaveImage(const TCHAR* p_pszFile, ST_MCXImg* p_pstImg, int p_nFlag)
{
	int w_ret = 0;
	TCHAR w_szFile1[0x200] = {0};
	CxImage w_stCxLoader;
	int i, j, ww0, w_nBitPerPixel;
	BYTE* w_pbTemp = NULL;

	if (p_pszFile == NULL || p_pszFile[0] == 0 || p_pstImg == NULL){
		return -1;
	}
	
	_tcscpy(w_szFile1, p_pszFile);
	//strcpy(w_szFile1, p_pszFile);
	TCHAR* extin = FindExtension(w_szFile1);
	int typein = CxImage::GetTypeIdFromName(extin);

	if (p_nFlag == 0)
	{
		w_nBitPerPixel = 8;
		ww0 = p_pstImg->width;		
		w_pbTemp = (BYTE*)alloc_mem(ww0 * p_pstImg->height);
		//. flipping
		for( i = 0; i < p_pstImg->height; i ++)
		{
			memcpy(w_pbTemp+(p_pstImg->height-1-i)*ww0, p_pstImg->ptr+i*ww0, ww0);
		}
	}
	else if (p_pstImg->alpha)
	{
		w_nBitPerPixel = 32;
		ww0 = p_pstImg->width * 4;
		w_pbTemp = (BYTE*)alloc_mem(ww0 * p_pstImg->height);
		//. flipping
		for( i = 0; i < p_pstImg->height; i ++)
		{
			BYTE* p1 = w_pbTemp+(p_pstImg->height-1-i)*ww0;
			BYTE* p2 = p_pstImg->ptr+i*(ww0-p_pstImg->width);
			BYTE* pA = p_pstImg->alpha+i*p_pstImg->width;
			for( j = 0; j < p_pstImg->width; j ++, p1 += 4, p2 += 3, pA ++){
				p1[0] = p2[0];
				p1[1] = p2[1];
				p1[2] = p2[2];
				p1[3] = pA[0];
			}
		}
	}
	else
	{
		w_nBitPerPixel = 24;
		ww0 = p_pstImg->width * 3;		
		w_pbTemp = (BYTE*)alloc_mem(ww0 * p_pstImg->height);
		//. flipping
		for( i = 0; i < p_pstImg->height; i ++)
		{
			memcpy(w_pbTemp+(p_pstImg->height-1-i)*ww0, p_pstImg->ptr+i*ww0, ww0);
		}
	}			

	if (!w_stCxLoader.CreateFromArray(w_pbTemp, p_pstImg->width, p_pstImg->height, w_nBitPerPixel, ww0, 0))
	{
		printf("\nCxImage CreateFromArray : FAILED");
		w_ret = -2;
		goto L_EXIT;
	}
	//w_pbTemp = NULL;

	if (!w_stCxLoader.Save(p_pszFile, typein))
	{
		printf("\nCxImage Save : FAILED");
		w_ret = -2;
		goto L_EXIT;
	}

L_EXIT:
	if (w_pbTemp){
		free_mem(w_pbTemp);
	}
	return w_ret;
}

//////////////////////////////////////////////////////////////////////////
// int CxSaveImageAA(int* size0, const char* p_pszFile, ST_MCXImg* p_pstImg, int p_nFlag)
// {
// 	int w_ret = 0;
// 	char w_szFile1[0x200] = {0};
// 	CxImage w_stCxLoader;
// 	int i, j, ww0, w_nBitPerPixel;
// 	BYTE* w_pbTemp = NULL;
// 
// 	if (p_pszFile == NULL || p_pszFile[0] == 0 || p_pstImg == NULL){
// 		return -1;
// 	}
// 
// 	_tcscpy(w_szFile1, p_pszFile);
// 	char* extin = FindExtension(w_szFile1);
// 	int typein = CxImage::GetTypeIdFromName(extin);
// 
// 	if (p_nFlag == 0)
// 	{
// 		w_nBitPerPixel = 8;
// 		ww0 = p_pstImg->width;
// 		w_pbTemp = (BYTE*)alloc_mem(ww0 * p_pstImg->height);
// 		//. flipping
// 		for( i = 0; i < p_pstImg->height; i ++)
// 		{
// 			memcpy(w_pbTemp+(p_pstImg->height-1-i)*ww0, p_pstImg->ptr+i*ww0, ww0);
// 		}
// 	}
// 	else if (p_pstImg->alpha)
// 	{
// 		w_nBitPerPixel = 32;
// 		ww0 = p_pstImg->width * 4;
// 		w_pbTemp = (BYTE*)alloc_mem(ww0 * p_pstImg->height);
// 		//. flipping
// 		for( i = 0; i < p_pstImg->height; i ++)
// 		{
// 			BYTE* p1 = w_pbTemp+(p_pstImg->height-1-i)*ww0;
// 			BYTE* p2 = p_pstImg->ptr+i*(ww0-p_pstImg->width);
// 			BYTE* pA = p_pstImg->alpha+i*p_pstImg->width;
// 			for( j = 0; j < p_pstImg->width; j ++, p1 += 4, p2 += 3, pA ++){
// 				p1[0] = p2[0];
// 				p1[1] = p2[1];
// 				p1[2] = p2[2];
// 				p1[3] = pA[0];
// 			}
// 		}
// 	}
// 	else
// 	{
// 		w_nBitPerPixel = 24;
// 		ww0 = p_pstImg->width * 3;
// 		w_pbTemp = (BYTE*)alloc_mem(ww0 * p_pstImg->height);
// 		//. flipping
// 		for( i = 0; i < p_pstImg->height; i ++)
// 		{
// 			memcpy(w_pbTemp+(p_pstImg->height-1-i)*ww0, p_pstImg->ptr+i*ww0, ww0);
// 		}
// 	}			
// 
// 	if (!w_stCxLoader.CreateFromArray(w_pbTemp, p_pstImg->width, p_pstImg->height, w_nBitPerPixel, ww0, 0))
// 	{
// 		printf("\nCxImage CreateFromArray : FAILED");
// 		w_ret = -2;
// 		goto L_EXIT;
// 	}
// 	w_stCxLoader.Resample(size0[0], size0[1], 2);
// 	if (!w_stCxLoader.Save(p_pszFile, typein))
// 	{
// 		printf("\nCxImage Save : FAILED");
// 		w_ret = -2;
// 		goto L_EXIT;
// 	}
// 
// L_EXIT:
// 	if (w_pbTemp){
// 		free_mem(w_pbTemp);
// 	}
// 	return w_ret;
// }

void FreeCxImageBuffer(void* ptr)
{
	free_mem(ptr);
}