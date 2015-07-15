#pragma once


// CAbtDlg dialog

class CAbtDlg : public CDialog
{
	DECLARE_DYNAMIC(CAbtDlg)

public:
	CAbtDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAbtDlg();

// Dialog Data
	enum { IDD = IDD_ABTDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
