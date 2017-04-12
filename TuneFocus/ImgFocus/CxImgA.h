#ifndef __CXIMG_A_H__
#define __CXIMG_A_H__

typedef struct tagST_MCXImg
{	
	int width;
	int height;
	unsigned char* ptr;
	unsigned char* alpha;
}ST_MCXImg;

int CxLoadImage(const wchar_t* p_pszFile, int* imgtype, ST_MCXImg* p_pstImg, int p_nFlag);
int CxSaveImage(const wchar_t* p_pszFile, ST_MCXImg* p_pstImg, int p_nFlag);
void FreeCxImageBuffer(void* ptr);

#endif//..__CXIMG_A_H__
