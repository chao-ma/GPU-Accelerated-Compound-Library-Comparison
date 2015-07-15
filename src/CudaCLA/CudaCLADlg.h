// CudaCLADlg.h : header file
//
#include "Fp.h"

#pragma once

// CCudaCLADlg dialog
class CCudaCLADlg : public CDialog
{
// Construction
public:
	CCudaCLADlg(CWnd* pParent = NULL);	// standard constructor
	~CCudaCLADlg();

// Dialog Data
	enum { IDD = IDD_CUDACLA_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	int SPAIndex(bool ColumnMajor, CString inputFileName, CString outputFileName);
	int IFPIndex(bool ColumnMajor, CString inputFileName, CString outputFileName);
	
	int nTargetRealCount;
	int nTargetCount;
	int nTargetDim;
	int nRefRealCount;
	int nRefCount;
	int nRefDim;
	int nRefRealDim;
	
	int* pTargetMagArray;
	int* pTargetMatrix;
	int* pRefMagArray;
	int* pRefMatrix;
	int TargetLibFormat;
	int RefLibFormat;
	int Hist[100];
	int HistMax;
	int HistSum;

	char DeviceName[256];
	int nDevice;
	int nProcessor;
	size_t TotalMem;
	int ClockRate;

	int DisplayMode;
	int JobCode;
	CString JobMsg;
	CBrush YellowBrush;
	CBrush BrownBrush;
	CBrush LightBrownBrush;
	CBrush BlueBrush;
	CMenu menu;
	//for multi threading
	int ReturnValue;
	int SelfCompare;
	int Progress;
	DWORD time_begin;
	DWORD time_end;
	CString inputFileName;
	CString outputFileName;
	bool ColumnMajor;

	afx_msg void OnSparsearrayLoadcandidatelibrary();
	afx_msg void OnSparsearrayComparedatabase();
	
	afx_msg void OnCompressedfingerprintIndexascandidatelibrary();
	afx_msg void OnCompressedfingerprintIndexasreferencelibrary();
	afx_msg void OnCompressedfingerprintLoadreferencelibrary();
	afx_msg void OnCompressedfingerprintLoadcandidatelibrary();
	afx_msg void OnCompressedfingerprintComparedatabase();
	afx_msg void OnToolsIndexunityfingerprintasreferencelibrary();

	afx_msg void OnSparsearrayIndexascandidatelibrary();
	afx_msg void OnSparsearrayLoadreferencelibrary();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnOptionShowstatus();
	afx_msg void OnOptionShowhistogram();
	afx_msg void OnUpdateCompressedfingerprintComparedatabase(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSparsearrayComparedatabase(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCompressedfingerprintIndexascandidatelibrary(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCompressedfingerprintIndexasreferencelibrary(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSparsearrayIndexascandidatelibrary(CCmdUI *pCmdUI);
	afx_msg void OnUpdateToolsIndexunityfingerprintasreferencelibrary(CCmdUI *pCmdUI);
	int DrawHist(bool flag);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnOptionExport();
	afx_msg void OnUpdateOptionExport(CCmdUI *pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnOptionAbout();
};
