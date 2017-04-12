#pragma once

#include "PictureCtrl.h"
#include "afxwin.h"

#include "ImgFocus/ImgFocus.h"
// CPicView dialog

class CPicViewDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPicViewDlg)

public:
	CPicViewDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPicViewDlg();

// Dialog Data
	enum { IDD = IDD_PIC_VIEW_DLG };

public:
	int ShowPreviewPic(int* pos0, const TCHAR* imgfile, int nFocalVal);
	void DrawCursorPosA(int xx, int yy, int mode);

	int g_nRectWW;
	int g_nRectHH;
	int g_nPicWidth1;
	int g_nPicHeight1;
	ST_HuaImgInfo g_stImgInfo1;

	int g_nMouseAllow;
	int g_nCursorPosX;
	int g_nCursorPosY;
//	int g_nCursorDraw;

protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	void ChangePicViewPos(int* pos0, int width, int height);
	
	virtual BOOL PreTranslateMessage(MSG* pMsg); 

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strFileInfo;
	CString m_strFileName;
//	CStatic m_ctrlPicA;
	CPictureCtrl m_ctrlPicA;
	afx_msg void OnNcDestroy();
};
