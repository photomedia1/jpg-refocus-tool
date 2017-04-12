#ifndef __IMG_FOCUS_H__
#define __IMG_FOCUS_H__

#include <WTypes.h>

//////////////////////////////////////////////////////////////////////////
#define ImgF_Type_CommonJPEG	(1)
#define ImgF_Type_HwaWeiJPEG	(2)
#define ImgF_Type_Unknown		(3)

//////////////////////////////////////////////////////////////////////////
typedef struct tagST_HuaImgMeta
{
	int width;
	int height;
	int nFocalMin;
	int nFocalMax;
	BYTE* metaBin;
}ST_HuaImgMeta;

typedef struct tagST_HuaImgFile
{
	int imgType;
	int nDataLen1;
	BYTE* m_pbData1;
	int nDataLen2;
	BYTE* m_pbData2;
	int nDataLen3;
	BYTE* m_pbData3;	
}ST_HuaImgBinData;

typedef struct tagST_HuaImgSet
{
	int m_width;
	int m_height;
	BYTE* m_pbGImage;
	BYTE* m_pbFImage;
	ST_HuaImgMeta m_stMeta;
}ST_HuaImgSet;

typedef struct tagST_HuaImgInfo
{
	TCHAR m_szCurImage[1024];
	int m_nImgWidth;
	int m_nImgHeight;
	int m_nImgDepth;
	int m_nFocalLength;		
}ST_HuaImgInfo;

//////////////////////////////////////////////////////////////////////////
//	Return Code
#define		REFOCUS_Err_LoadFailed		(0x101)
#define		REFOCUS_Err_MemOverFlow		(0x102)
#define		REFOCUS_Err_SaveFailed		(0x103)
#define		REFOCUS_Err_InvalidParam	(0x104)
#define		REFOCUS_Err_InvalidInfo		(0x105)
#define		REFOCUS_Err_CommonJPEG		(0x106)

int SetAnaTempPath(const TCHAR* szTmpPath);
int OpenPhoneImageFile(const TCHAR* szFile);
int GetTmpJpgFileName(const TCHAR* p_pszFile, TCHAR* szFile, int* p_pnImageValid);
int GetCurHuaFileInfo(ST_HuaImgInfo* ptr);
int ReleaseImgAnaMem();
int GetFocusedImage(const TCHAR* p_pszImage, int p_nFocalVal);
int GetFValPerc(int posX, int posY, int* p_pnFVal);
void CleanFilesInTempPath();

#endif//..__IMG_FOCUS_H__
