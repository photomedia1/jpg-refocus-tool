// PicView.cpp : implementation file
//

#include "stdafx.h"
#include "TuneFocus.h"
#include "PicViewDlg.h"
#include "afxdialogex.h"

#include "TuneFocusDlg.h"

// CPicView dialog
CTuneFocusDlg* g_clsTuneDlg = NULL; 

IMPLEMENT_DYNAMIC(CPicViewDlg, CDialogEx)

CPicViewDlg::CPicViewDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPicViewDlg::IDD, pParent)
	, m_strFileInfo(_T(""))
	, m_strFileName(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);

	g_nRectWW = -1;
	g_nRectHH = -1;
	g_nPicWidth1 = -1;
	g_nPicHeight1 = -1;
	memset(&g_stImgInfo1, 0, sizeof(ST_HuaImgInfo));

	g_nCursorPosX = -1;
	g_nCursorPosY = -1;
	g_nMouseAllow = 0;
}

CPicViewDlg::~CPicViewDlg()
{
}

void CPicViewDlg::ChangePicViewPos(int* pos0, int width, int height)
{
	CRect rect, imgRect, rect1;
	int maxsize0, width1, height1;
	int maxstdsize = 1000;
	int width2, height2;
	int width3, height3;

	::GetWindowRect(GetSafeHwnd(), &rect);
	m_ctrlPicA.GetWindowRect(&imgRect);	

	maxsize0 = width;
	if (width < height){
		maxsize0 = height;
	}
	width1 = width * maxstdsize / maxsize0;
	height1 = height * maxstdsize / maxsize0;

	width3 = imgRect.Width();
	height3 = imgRect.Height();
	if (g_nRectWW < 0 || g_nRectHH < 0){
		g_nRectWW = rect.Width() - width3;
		g_nRectHH = rect.Height() - height3;
	}	

	rect1.top = rect.top;
	rect1.bottom = rect.top + height1 + g_nRectHH;
	rect1.left = rect.left;
	rect1.right = rect.left + width1 + g_nRectWW;
	MoveWindow(&rect1);	
 	m_ctrlPicA.MoveWindow(g_nRectWW/3, g_nRectHH-40, width1, height1);

	g_nPicWidth1 = width1;
	g_nPicHeight1 = height1;

	return;
}

int CPicViewDlg::ShowPreviewPic(int* pos0, const TCHAR* imgfile, int nFocalVal)
{
	int ret = 0;
	TCHAR szFile1[1024] = {0};		
	CRect rect, rect1;
	int nImageValidity = 0;
	int width1, height1;
	int nJpegType = 0; 
	BYTE* img3 = NULL;

	m_strFileInfo = _T("");
	m_strFileName = _T("");
	UpdateData(FALSE);
	Sleep(10);

	ret = GetCurHuaFileInfo(&g_stImgInfo1);
	if (ret != 0 || _tcscmp(g_stImgInfo1.m_szCurImage, imgfile) != 0)
	{
		ret = OpenPhoneImageFile(imgfile);
		if (ret != 0){
			MessageBox(_T("It failed to open Image File!"), MB_OK);
			return ret;
		}
	}
	
	ret = GetFocusedImage(imgfile, nFocalVal);
	if (ret != 0){
//		MessageBox(_T("GetFocusedImage Failed!"), MB_OK);
//		ret = REFOCUS_Err_CommonJPEG;
//		return ret;		
	}
	ret = GetCurHuaFileInfo(&g_stImgInfo1);
	if (ret != 0){
		MessageBox(_T("GetCurHuaFileInfo Failed!"), MB_OK);
		return ret;
	}

	::GetWindowRect(GetSafeHwnd(), &rect);
	rect1.left = pos0[0];
	rect1.right = rect1.left + rect.Width();
	rect1.top = pos0[1];
	rect1.bottom = rect1.top + rect.Height();
	MoveWindow(&rect1);

	SetWindowText(g_stImgInfo1.m_szCurImage);	
	m_strFileInfo.Format(_T("Width : %d Height : %d"), g_stImgInfo1.m_nImgWidth, g_stImgInfo1.m_nImgHeight); 
	UpdateData(FALSE);

	ChangePicViewPos(pos0, g_stImgInfo1.m_nImgWidth, g_stImgInfo1.m_nImgHeight);
	
	ret = GetTmpJpgFileName(g_stImgInfo1.m_szCurImage, szFile1, &nImageValidity);
	if (ret != 0){
		MessageBox(_T("GetTmpJpgFileName failed!"), MB_OK);
		return ret;
	}

	if (nImageValidity){
		m_strFileName = _T("Huawei P9 Image");
		ret = 0;
	}
	else{
		m_strFileName = _T("Common Image");
		ret = REFOCUS_Err_CommonJPEG; 
	}

	UpdateData(FALSE);

	//. show
	m_ctrlPicA.Load(CString(szFile1));

	return ret;
}

void CPicViewDlg::DrawCursorPosA(int posX, int posY, int mode)
{
	int xx, yy;
	CClientDC dc( this );
	CPen pen;
	pen.CreatePen( PS_DOT, 3, RGB(255,0,0) );
	CPen* oldPen = dc.SelectObject( &pen );
	int old_rop = SetROP2(dc.m_hDC, R2_XORPEN);
	CBrush brush;
	brush.CreateSolidBrush( RGB(255,255,255) );
	CBrush* oldBrush = dc.SelectObject( &brush );

	if (g_nCursorPosX >= 0 && g_nCursorPosY >= 0)
	{
		xx = g_nCursorPosX + 10;
		yy = g_nCursorPosY + 50;
		dc.Rectangle( xx-10, yy-10, xx+10, yy+10 );
	}

	xx = posX + 10;
	yy = posY + 50;
	dc.Rectangle( xx-10, yy-10, xx+10, yy+10 );
	dc.SelectObject( oldBrush );
	SetROP2(dc.m_hDC, old_rop);

	g_nCursorPosX = posX;
	g_nCursorPosY = posY;
	return;
}

BOOL CPicViewDlg::PreTranslateMessage(MSG* pMsg) 
{
	int mx, my;
	int mx1, my1;
	int nFValPerc;
	TCHAR szMsg[256] = {0};

	if (pMsg->message == WM_MOUSEMOVE)
	{
		if (pMsg->hwnd == GetDlgItem(IDC_STATIC_PIC)->GetSafeHwnd())
		{
			if (g_nRectWW == -1 || g_nRectHH == -1 || g_nPicWidth1 < 1 || g_nPicHeight1 < 1 ||
				g_stImgInfo1.m_szCurImage[0] == 0)
			{

			}
			else
			{	
				if (g_nMouseAllow != 0)
				{
					mx1 = LOWORD(pMsg->lParam);
					my1 = HIWORD(pMsg->lParam);		
					DrawCursorPosA(mx1, my1, 0);
				}
			}
		}
	}
	else if (pMsg->message == WM_LBUTTONDOWN)
	{
		if (pMsg->hwnd == GetDlgItem(IDC_STATIC_PIC)->GetSafeHwnd())
		{
			if (g_nRectWW == -1 || g_nRectHH == -1 || g_nPicWidth1 < 1 || g_nPicHeight1 < 1 ||
				g_stImgInfo1.m_szCurImage[0] == 0)
			{

			}
			else
			{
				mx1 = LOWORD(pMsg->lParam);
				my1 = HIWORD(pMsg->lParam);

				DrawCursorPosA(mx1, my1, 0);				
				g_nMouseAllow = 0;
				g_nCursorPosX = -1;
				g_nCursorPosY = -1;

				mx = (mx1 - g_nRectWW/2) * g_stImgInfo1.m_nImgWidth / g_nPicWidth1;
				my = (my1 - 0) * g_stImgInfo1.m_nImgHeight / g_nPicHeight1;
				if (mx < 0)
					mx = 0;
				else if (mx >= g_stImgInfo1.m_nImgWidth)
					mx = g_stImgInfo1.m_nImgWidth-1;
				if (my < 0)
					my = 0;
				else if (my >= g_stImgInfo1.m_nImgHeight)
					my = g_stImgInfo1.m_nImgHeight-1;
// 				swprintf_s(szMsg, 200, _T("POS = %d, %d"), mx, my);
// 				MessageBox(szMsg, MB_OK);

				int ret = GetFValPerc(mx, my, &nFValPerc);
				if (ret == 0)
				{
					g_clsTuneDlg->SetFValCtrl(nFValPerc);

					g_clsTuneDlg->ShowSelectedImage(nFValPerc, g_stImgInfo1.m_szCurImage);
				}
			}
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CPicViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_SIZE_INFO, m_strFileInfo);
	DDX_Text(pDX, IDC_STATIC_PATH, m_strFileName);
	DDX_Control(pDX, IDC_STATIC_PIC, m_ctrlPicA);
}

BOOL CPicViewDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	return TRUE;
}

BEGIN_MESSAGE_MAP(CPicViewDlg, CDialogEx)
	ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// CPicView message handlers


void CPicViewDlg::OnNcDestroy()
{
//	((CTuneFocusDlg*)GetActiveWindow())->m_nPreviewDlgCreated = 0;

	CDialogEx::OnNcDestroy();
	// TODO: Add your message handler code here
}
