
// TuneFocusDlg.h : header file
//

#pragma once

#include "PicViewDlg.h"
#include "afxcmn.h"

// CTuneFocusDlg dialog
class CTuneFocusDlg : public CDialogEx
{
// Construction
public:
	CTuneFocusDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TUNEFOCUS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
public:
	CPicViewDlg*	m_PreviewDlg;
	int				m_nPreviewDlgCreated;	

	void SetCmdButtonState(int value);	
	void SetFValCtrl(int val);	
	void SaveResultJpeg();

protected:
	HICON m_hIcon;

	TCHAR m_strCutFile[1024];
	int m_nSelectRow;
	int* m_pnFileProcState;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnOpen();
	void ShowSelectedImage(int focalNum, TCHAR* szFile);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnSave();
	CString m_strMainStatus;
//	afx_msg void OnBnClickedBtnShow();
	int m_nFValue1;
	BOOL m_bChkDrawCursor;
	int m_nFocusValP;
	CListCtrl m_ListCtrl;
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult);			
};
