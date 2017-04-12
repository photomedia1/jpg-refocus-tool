
// TuneFocusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TuneFocus.h"
#include "TuneFocusDlg.h"
#include "afxdialogex.h"

#include "ImgFocus/ImgFocus.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CTuneFocusDlg* g_clsTuneDlg;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTuneFocusDlg dialog




CTuneFocusDlg::CTuneFocusDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTuneFocusDlg::IDD, pParent)
	, m_PreviewDlg(0)
	, m_strMainStatus(_T(""))
	, m_nFValue1(0)
	, m_bChkDrawCursor(FALSE)
	, m_nFocusValP(0)
	, m_pnFileProcState(NULL)
{
//	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);	

	memset(m_strCutFile, 0, sizeof(m_strCutFile));

	m_nSelectRow = -1;
}

void CTuneFocusDlg::SetFValCtrl(int val)
{
	if (val < 0)
		val = 0;
	else if (val > 100)
		val = 100;
	m_nFValue1 = val;
	UpdateData(FALSE);
	return;
}

void CTuneFocusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUS, m_strMainStatus);
	DDX_Slider(pDX, IDC_SLIDER1, m_nFValue1);
	DDV_MinMaxInt(pDX, m_nFValue1, 0, 100);	
	DDX_Text(pDX, IDC_STATIC_FVAL, m_nFocusValP);
	DDV_MinMaxInt(pDX, m_nFocusValP, 0, 100);
	DDX_Control(pDX, IDC_LIST_FILES, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CTuneFocusDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_OPEN, &CTuneFocusDlg::OnBnClickedBtnOpen)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, &CTuneFocusDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CTuneFocusDlg::OnBnClickedBtnSave)	
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CTuneFocusDlg::OnNMCustomdrawSlider1)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, &CTuneFocusDlg::OnReleasedcaptureSlider1)			
END_MESSAGE_MAP()


// CTuneFocusDlg message handlers

BOOL CTuneFocusDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	HICON m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON1));	
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CRect rect, rect1;

	::GetWindowRect(GetSafeHwnd(), &rect);
	rect1.left = 10; 
	rect1.right = rect1.left + rect.right - rect.left;
	rect1.top = 10;
	rect1.bottom = rect1.top + rect.bottom - rect.top;
	this->MoveWindow(&rect1);

	TCHAR w_szFileName[1024] = {0};
	GetModuleFileName(NULL, w_szFileName, 1000);
	PathRemoveFileSpec(w_szFileName);

	int ret = SetAnaTempPath(w_szFileName);
	if (ret != 0)
	{
		MessageBox(_T("SetAnaTempPath Failed!"), MB_OK);
	}

	g_clsTuneDlg = this;

	m_ListCtrl.ModifyStyle(0, LVS_REPORT, NULL);
	m_ListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_ONECLICKACTIVATE);

	m_ListCtrl.InsertColumn(0, _T("FileName"), LVCFMT_LEFT, 230);
	m_ListCtrl.InsertColumn(1, _T("F"), LVCFMT_LEFT, 60);
	m_ListCtrl.InsertColumn(2, _T("Status"), LVCFMT_LEFT, 60);	
	m_ListCtrl.InsertColumn(3, _T("Path"), LVCFMT_LEFT, 100);	
	
// 	CTime w_stCur = CTime::GetCurrentTime();
// 	CTime w_stEndTime = CTime(2017, 5, 25, 0, 0, 0);
// 	if (w_stCur > w_stEndTime){
// 		MessageBox(_T("\nExpired Now!"), MB_OK);
// 		return FALSE;
// 	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTuneFocusDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTuneFocusDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTuneFocusDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTuneFocusDlg::OnBnClickedBtnOpen()
{
	CString File, filem, strre;
	CString strFileList;
	int nItemCnt = 0;
	const int c_cMaxFiles = 1000;
	const int c_cbBuffSize = (c_cMaxFiles * (MAX_PATH + 1)) + 1;
	TCHAR		szFilter[] = _T("Image(*.bmp, *.jpg, *.png)|*.BMP;*.JPG;*.PNG;*.TIF|All Files(*.*)|*.*||");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT, szFilter);
	dlg.GetOFN().lpstrFile = strFileList.GetBuffer(c_cbBuffSize);
	dlg.GetOFN().nMaxFile = c_cbBuffSize;

	if (dlg.DoModal() == IDOK)
	{
		m_ListCtrl.DeleteAllItems();

		SetCmdButtonState(0);

		CleanFilesInTempPath();
		
		for(POSITION pos=dlg.GetStartPosition(); pos != NULL;)
		{
			File = dlg.GetNextPathName(pos);
			filem = File.Right( File.GetLength()-File.ReverseFind( _T('\\') )-1 );
//			filem = filem.Left( filem.GetLength() - 4 );

			int nRowPos = m_ListCtrl.InsertItem(nItemCnt, filem);
			m_ListCtrl.SetItemText(nRowPos, 1, _T("0"));			
			m_ListCtrl.SetItemText(nRowPos, 3, File.GetBuffer(0));			
			nItemCnt ++;
		}

		m_nFValue1 = 0;
		if (nItemCnt == 1)
			m_strMainStatus.Format(_T("%d Image Loaded!"), nItemCnt);
		else
			m_strMainStatus.Format(_T("%d Images Loaded!"), nItemCnt);

		UpdateData(FALSE);

		if (m_pnFileProcState){
			free(m_pnFileProcState);
			m_pnFileProcState = NULL;
		} 
		
		if (nItemCnt > 0)
		{
			TCHAR szFile2[1024] = {0};

			m_pnFileProcState = (int*)malloc(sizeof(int) * nItemCnt);
			memset(m_pnFileProcState, 0, sizeof(int) * nItemCnt);

			m_nSelectRow = 0;
			m_ListCtrl.GetItemText(0, 3, szFile2, 1000);
			m_ListCtrl.SetItemState(-1, 0, LVIS_SELECTED | LVIS_FOCUSED);
			m_ListCtrl.SetItemState(m_nSelectRow, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			memset(m_strCutFile, 0, sizeof(m_strCutFile));
			_tcscpy(m_strCutFile, szFile2);
			ShowSelectedImage(0, szFile2);
		}		
	}
	return;
}

//////////////////////////////////////////////////////////////////////////
void CTuneFocusDlg::ShowSelectedImage(int focalNum, TCHAR* szFile)
{
	CRect rect;
	int xyarr[4];
	int maxsize0;
	int ret;
	TCHAR szTemp[100] = {0};

	m_strMainStatus = _T("Starting Process...");
	UpdateData(FALSE);
	Sleep(10);

	SetCmdButtonState(0);

	if (m_nPreviewDlgCreated == 1)
	{
		m_PreviewDlg->SetFocus();
	}
	else
	{
		if (m_PreviewDlg)
		{
			delete m_PreviewDlg;
		}
		m_PreviewDlg = new CPicViewDlg;
		m_PreviewDlg->Create(IDD_PIC_VIEW_DLG, this);		
		m_nPreviewDlgCreated = 1;		
		m_PreviewDlg->ShowWindow(SW_SHOW);		
	}

	::GetWindowRect(GetSafeHwnd(), &rect);

	xyarr[0] = rect.right;
	xyarr[1] = rect.top;
	
	ret = m_PreviewDlg->ShowPreviewPic(xyarr, szFile, focalNum);

	if (ret == 0)
	{		
		swprintf_s(szTemp, _T("%d"), focalNum);
		m_ListCtrl.SetItemText(m_nSelectRow, 1, szTemp);

		m_PreviewDlg->g_nMouseAllow = 1;
		m_pnFileProcState[m_nSelectRow] = 1;

		m_strMainStatus = _T("Process completed!");
	}
	else if (ret == REFOCUS_Err_CommonJPEG)
	{		
		m_ListCtrl.SetItemText(m_nSelectRow, 1, _T("-"));

		m_PreviewDlg->g_nMouseAllow = 0;
		m_pnFileProcState[m_nSelectRow] = 2;

		m_strMainStatus = _T("This image is common file, it can't be re-focused.");
	}

	SetCmdButtonState(1);
	
	UpdateData(FALSE);	
	return;
}

void CTuneFocusDlg::SaveResultJpeg()
{
	TCHAR szFilePath[1024] = {0};	
	TCHAR szFileName[1024] = {0};
	TCHAR szFile2[1024] = {0};	
	int ret, mm, ii, nn;
	int w_nFVal0;

	if (m_strCutFile[0] == 0 || m_pnFileProcState == NULL){
		MessageBox(_T("Error!"), MB_OK);
		return;
	}

	if (m_pnFileProcState[m_nSelectRow] == 2)
	{
		return;
	}

	UpdateData(TRUE);
	w_nFVal0 = m_nFValue1;

	_tcscpy(szFilePath, m_strCutFile);
	nn = _tcslen(szFilePath);
	szFilePath[nn-4] = 0;	
	swprintf_s(szFileName, _T("%s_Focal%03d.jpg"), szFilePath, w_nFVal0);
	ret = GetTmpJpgFileName(m_strCutFile, szFile2, &mm);
	if (ret == 0)
	{
		if (PathFileExists(szFile2))
		{
			CopyFile(szFile2, szFileName, FALSE);			
		}
		m_ListCtrl.SetItemText(m_nSelectRow, 2, _T("Saved"));
	}
	else
	{
		m_ListCtrl.SetItemText(m_nSelectRow, 2, _T("-"));
	}

// 	BOOL isOpen = FALSE;
// 	CString defaultDir;
// 	CString fileName;
// 	defaultDir.Format(_T("%s"), szFile1);
// 	_tcscpy(szFile4, szFile3);
// 	nn = _tcslen(szFile4);
// 	szFile4[nn-4] = 0;	
// 	_tcscat(szFile4, _T("_2.jpg"));
// 	fileName.Format(_T("%s"), szFile4);
// 	TCHAR	szFilter[] = _T("Image(*.bmp, *.jpg, *.tif, *.png)|*.BMP;*.JPG;*.PNG;*.TIF|All Files(*.*)|*.*||");
// 	CFileDialog openFileDlg(isOpen, defaultDir, fileName, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, NULL); 
// 	swprintf_s(szFile5, _T("%s\\%s"), defaultDir, fileName);
// 	openFileDlg.GetOFN().lpstrInitialDir = szFile5;  
// 	INT_PTR result = openFileDlg.DoModal();  
// 	CString filePath = defaultDir + "\\" + fileName; 
// 	if (result == IDOK)
// 	{
// 		filePath = openFileDlg.GetPathName(); 
// 		CopyFile(szFile2, filePath.GetBuffer(0), FALSE);
// 
// 		memset(szFile3, 0, sizeof(szFile3));
// 		_tcscpy(szFile3, filePath.GetBuffer(0));
// 		PathStripPath(szFile3);
// 		m_strMainStatus.Format(_T("%s was Saved!"), szFile3);
// 	}
//	UpdateData(FALSE);	

	m_pnFileProcState[m_nSelectRow] = 2;

	Sleep(10);
	return;
}

void CTuneFocusDlg::SetCmdButtonState(int value)
{
	if (value){
		GetDlgItem(IDC_BTN_OPEN)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_SAVE)->EnableWindow(TRUE);		
		GetDlgItem(IDC_SLIDER1)->EnableWindow(TRUE);
		GetDlgItem(IDC_LIST_FILES)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BTN_OPEN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_SAVE)->EnableWindow(FALSE);		
		GetDlgItem(IDC_SLIDER1)->EnableWindow(FALSE);
		GetDlgItem(IDC_LIST_FILES)->EnableWindow(FALSE);
	}
	return;
}

void CTuneFocusDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	if (m_PreviewDlg){
		delete m_PreviewDlg;
	}

	if (m_pnFileProcState)
	{
		free(m_pnFileProcState);
	}
	
	ReleaseImgAnaMem();

	return;
}



void CTuneFocusDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}

void CTuneFocusDlg::OnBnClickedBtnSave()
{
	int nRows;
	int ii;
	int w_nFVal;
	TCHAR szFile2[1024] = {0};

	UpdateData(TRUE);
	w_nFVal = m_nFValue1;

	nRows = m_ListCtrl.GetItemCount();
	
	for( ii = 0; ii < nRows; ii ++)
	{
		m_nSelectRow = ii;
		memset(szFile2, 0, sizeof(szFile2));
		m_ListCtrl.GetItemText(m_nSelectRow, 3, szFile2, 1000);
		memset(m_strCutFile, 0, sizeof(m_strCutFile));
		_tcscpy(m_strCutFile, szFile2);

		if (m_pnFileProcState[ii] == 0)
		{			
			ShowSelectedImage(w_nFVal, szFile2);
			if (m_pnFileProcState[ii] == 2){
				continue;
			}
			m_pnFileProcState[ii] = 1;
		}
		if (m_pnFileProcState[ii] == 1)
		{
			SaveResultJpeg();
		}
	}
}

// void CTuneFocusDlg::OnBnClickedBtnShow()
// {
// 	SaveResultJpeg();
// 	return;
// }

void CTuneFocusDlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_nFocusValP = m_nFValue1;
	UpdateData(FALSE);
	*pResult = 0;
}

void CTuneFocusDlg::OnReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int w_nFVal1;
	// TODO: Add your control notification handler code here
//	MessageBox(_T("Set"), MB_OK);
	UpdateData(TRUE);
	w_nFVal1 = m_nFValue1;
	ShowSelectedImage(w_nFVal1, m_strCutFile);
	*pResult = 0;
}
