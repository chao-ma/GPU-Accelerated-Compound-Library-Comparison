// CudaCLADlg.cpp : implementation file
//

#include "stdafx.h"
#include "CudaCLA.h"
#include "CudaCLADlg.h"
#include "AbtDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define UNITY_FP_LEN 992
#define SPA_FORMAT_ID 0x19880515
#define IFP_FORMAT_ID 0x19840925

#define MODE_TEXT 0
#define MODE_HIST 1

#define JOB_IDLE 0
#define JOB_INDEX 1
#define JOB_DBCMPR 2

#define TIMER_DBCMPR 100
#define TIMER_INDEX 101
// CCudaCLADlg dialog
extern "C" int GetDeviceInfo(char* pDeviceName, size_t* pTotalMem, int* pClockRate, int* pNumProcessor);
extern "C" int SPADBTC(int nTargetCount, int nTargetRealCount, int nTargetDim, int* pTargetMagArray, int* pTargetMatrix,
						int nRefCount, int nRefRealCount, int nRefDim, int* pRefMagArray, int* pRefMatrix, int SelfCompare, int* res,int* pProgress);

extern "C" int INTDBTC(int nTargetCount, int nTargetRealCount, int nTargetDim, int* pTargetMagArray, int* pTargetMatrix,
						int nRefCount, int nRefRealCount, int nRefDim, int* pRefMagArray, int* pRefMatrix, int SelfCompare, int* res,int* pProgress);

UINT ThreadINTDBTC( LPVOID pParam )
{
	int ReturnValue;
	CCudaCLADlg* pDlg=(CCudaCLADlg*)pParam;
	pDlg->Progress=0;
	pDlg->SetTimer(TIMER_DBCMPR,50,NULL);
	pDlg->time_begin=GetTickCount();
	ReturnValue=INTDBTC(pDlg->nTargetCount,pDlg->nTargetRealCount,pDlg->nTargetDim,pDlg->pTargetMagArray,pDlg->pTargetMatrix,
				pDlg->nRefCount,pDlg->nRefRealCount,pDlg->nRefDim,pDlg->pRefMagArray,pDlg->pRefMatrix,pDlg->SelfCompare,pDlg->Hist,&(pDlg->Progress));
	pDlg->time_end=GetTickCount();
	pDlg->ReturnValue=ReturnValue;
	pDlg->DrawHist(true);
	pDlg->KillTimer(TIMER_DBCMPR);
	return ReturnValue;
}
UINT ThreadSPADBTC( LPVOID pParam )
{
	int ReturnValue;
	CCudaCLADlg* pDlg=(CCudaCLADlg*)pParam;
	pDlg->Progress=0;
	pDlg->SetTimer(TIMER_DBCMPR,50,NULL);
	pDlg->time_begin=GetTickCount();
	ReturnValue=SPADBTC(pDlg->nTargetCount,pDlg->nTargetRealCount,pDlg->nTargetDim,pDlg->pTargetMagArray,pDlg->pTargetMatrix,
				pDlg->nRefCount,pDlg->nRefRealCount,pDlg->nRefDim,pDlg->pRefMagArray,pDlg->pRefMatrix,pDlg->SelfCompare,pDlg->Hist,&(pDlg->Progress));
	pDlg->time_end=GetTickCount();
	pDlg->ReturnValue=ReturnValue;
	pDlg->DrawHist(true);
	pDlg->KillTimer(TIMER_DBCMPR);
	return ReturnValue;
}
UINT IndexIFP( LPVOID pParam )
{
	CCudaCLADlg* pDlg=(CCudaCLADlg*)pParam;
	pDlg->SetTimer(TIMER_INDEX,50,NULL);
	pDlg->IFPIndex(pDlg->ColumnMajor,pDlg->inputFileName,pDlg->outputFileName);
	pDlg->DrawHist(false);
	pDlg->KillTimer(TIMER_INDEX);
	return 0;
}
UINT IndexSPA( LPVOID pParam )
{
	CCudaCLADlg* pDlg=(CCudaCLADlg*)pParam;
	pDlg->SetTimer(TIMER_INDEX,50,NULL);
	pDlg->SPAIndex(pDlg->ColumnMajor,pDlg->inputFileName,pDlg->outputFileName);
	pDlg->DrawHist(false);
	pDlg->KillTimer(TIMER_INDEX);
	return 0;
}
CCudaCLADlg::CCudaCLADlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCudaCLADlg::IDD, pParent)
{
	pTargetMagArray=NULL;
	pTargetMatrix=NULL;
	pRefMagArray=NULL;
	pRefMatrix=NULL;
	nTargetRealCount=0;
	nTargetCount=0;
	nTargetDim=0;
	nRefRealCount=0;
	nRefCount=0;
	nRefDim=0;
	nRefRealDim=0;
	HistMax=0;
	HistSum=0;
	nDevice=0;
	ZeroMemory(Hist,sizeof(int)*100);

	DisplayMode=MODE_TEXT;
	JobCode=JOB_IDLE;
	YellowBrush.CreateSolidBrush(RGB(216,216,30));
	BrownBrush.CreateSolidBrush(RGB(192,80,77));
	LightBrownBrush.CreateSolidBrush(RGB(230,185,184));
	BlueBrush.CreateSolidBrush(RGB(198,230,254));
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
CCudaCLADlg::~CCudaCLADlg()
{
	YellowBrush.DeleteObject();
	BrownBrush.DeleteObject();
	BlueBrush.DeleteObject();
	LightBrownBrush.DeleteObject();
	if(pTargetMagArray)
		delete pTargetMagArray;
	if(pTargetMatrix)
		delete pTargetMatrix;
	if(pRefMagArray)
		delete pRefMagArray;
	if(pRefMatrix)
		delete pRefMatrix;

}

void CCudaCLADlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCudaCLADlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_TOOLS_INDEXUNITYFINGERPRINTASREFERENCELIBRARY, &CCudaCLADlg::OnToolsIndexunityfingerprintasreferencelibrary)
	ON_COMMAND(ID_SPARSEARRAY_INDEXASCANDIDATELIBRARY, &CCudaCLADlg::OnSparsearrayIndexascandidatelibrary)
	ON_COMMAND(ID_SPARSEARRAY_LOADREFERENCELIBRARY, &CCudaCLADlg::OnSparsearrayLoadreferencelibrary)
	ON_COMMAND(ID_SPARSEARRAY_LOADCANDIDATELIBRARY, &CCudaCLADlg::OnSparsearrayLoadcandidatelibrary)
	ON_COMMAND(ID_SPARSEARRAY_COMPAREDATABASE, &CCudaCLADlg::OnSparsearrayComparedatabase)
	ON_COMMAND(ID_COMPRESSEDFINGERPRINT_INDEXASCANDIDATELIBRARY, &CCudaCLADlg::OnCompressedfingerprintIndexascandidatelibrary)
	ON_COMMAND(ID_COMPRESSEDFINGERPRINT_INDEXASREFERENCELIBRARY, &CCudaCLADlg::OnCompressedfingerprintIndexasreferencelibrary)
	ON_COMMAND(ID_COMPRESSEDFINGERPRINT_LOADREFERENCELIBRARY, &CCudaCLADlg::OnCompressedfingerprintLoadreferencelibrary)
	ON_COMMAND(ID_COMPRESSEDFINGERPRINT_LOADCANDIDATELIBRARY, &CCudaCLADlg::OnCompressedfingerprintLoadcandidatelibrary)
	ON_COMMAND(ID_COMPRESSEDFINGERPRINT_COMPAREDATABASE, &CCudaCLADlg::OnCompressedfingerprintComparedatabase)
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_OPTION_SHOWSTATUS, &CCudaCLADlg::OnOptionShowstatus)
	ON_COMMAND(ID_OPTION_SHOWHISTOGRAM, &CCudaCLADlg::OnOptionShowhistogram)
	ON_UPDATE_COMMAND_UI(ID_COMPRESSEDFINGERPRINT_COMPAREDATABASE, &CCudaCLADlg::OnUpdateCompressedfingerprintComparedatabase)
	ON_UPDATE_COMMAND_UI(ID_SPARSEARRAY_COMPAREDATABASE, &CCudaCLADlg::OnUpdateSparsearrayComparedatabase)
	ON_UPDATE_COMMAND_UI(ID_COMPRESSEDFINGERPRINT_INDEXASCANDIDATELIBRARY, &CCudaCLADlg::OnUpdateCompressedfingerprintIndexascandidatelibrary)
	ON_UPDATE_COMMAND_UI(ID_COMPRESSEDFINGERPRINT_INDEXASREFERENCELIBRARY, &CCudaCLADlg::OnUpdateCompressedfingerprintIndexasreferencelibrary)
	ON_UPDATE_COMMAND_UI(ID_SPARSEARRAY_INDEXASCANDIDATELIBRARY, &CCudaCLADlg::OnUpdateSparsearrayIndexascandidatelibrary)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_INDEXUNITYFINGERPRINTASREFERENCELIBRARY, &CCudaCLADlg::OnUpdateToolsIndexunityfingerprintasreferencelibrary)
	ON_WM_TIMER()
	ON_COMMAND(ID_OPTION_EXPORT, &CCudaCLADlg::OnOptionExport)
	ON_UPDATE_COMMAND_UI(ID_OPTION_EXPORT, &CCudaCLADlg::OnUpdateOptionExport)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_OPTION_ABOUT, &CCudaCLADlg::OnOptionAbout)
END_MESSAGE_MAP()


// CCudaCLADlg message handlers
void CCudaCLADlg::OnUpdateCompressedfingerprintComparedatabase(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(JobCode!=JOB_DBCMPR && nDevice>0);
}

void CCudaCLADlg::OnUpdateSparsearrayComparedatabase(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(JobCode!=JOB_DBCMPR && nDevice>0);
}

void CCudaCLADlg::OnUpdateCompressedfingerprintIndexascandidatelibrary(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(JobCode!=JOB_INDEX);
}

void CCudaCLADlg::OnUpdateCompressedfingerprintIndexasreferencelibrary(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(JobCode!=JOB_INDEX);
}

void CCudaCLADlg::OnUpdateSparsearrayIndexascandidatelibrary(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(JobCode!=JOB_INDEX);
}

void CCudaCLADlg::OnUpdateToolsIndexunityfingerprintasreferencelibrary(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(JobCode!=JOB_INDEX);
}

void CCudaCLADlg::OnUpdateOptionExport(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(HistMax>0 && HistSum>0);
}

void CCudaCLADlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialog::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: Add your message handler code here
	 ASSERT(pPopupMenu != NULL);
    // Check the enabled state of various menu items.

    CCmdUI state;
    state.m_pMenu = pPopupMenu;
    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pParentMenu == NULL);

    // Determine if menu is popup in top-level menu and set m_pOther to
    // it if so (m_pParentMenu == NULL indicates that it is secondary popup).
    HMENU hParentMenu;
    if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
        state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
    else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
    {
        CWnd* pParent = this;
           // Child windows don't have menus--need to go to the top!
        if (pParent != NULL &&
           (hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
        {
           int nIndexMax = ::GetMenuItemCount(hParentMenu);
           for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
           {
            if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
            {
                // When popup is found, m_pParentMenu is containing menu.
                state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
                break;
            }
           }
        }
    }

    state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;state.m_nIndex++)
    {
        state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
        if (state.m_nID == 0)
           continue; // Menu separator or invalid cmd - ignore it.

        ASSERT(state.m_pOther == NULL);
        ASSERT(state.m_pMenu != NULL);
        if (state.m_nID == (UINT)-1)
        {
           // Possibly a popup menu, route to first item of that popup.
           state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
           if (state.m_pSubMenu == NULL ||
            (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
            state.m_nID == (UINT)-1)
           {
            continue;       // First item of popup can't be routed to.
           }
           state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
        }
        else
        {
           // Normal menu item.
           // Auto enable/disable if frame window has m_bAutoMenuEnable
           // set and command is _not_ a system command.
           state.m_pSubMenu = NULL;
           state.DoUpdate(this, FALSE);
        }

        // Adjust for menu deletions and additions.
        UINT nCount = pPopupMenu->GetMenuItemCount();
        if (nCount < state.m_nIndexMax)
        {
           state.m_nIndex -= (state.m_nIndexMax - nCount);
           while (state.m_nIndex < nCount &&
            pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
           {
            state.m_nIndex++;
           }
        }
        state.m_nIndexMax = nCount;
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCudaCLADlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CCudaCLADlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	menu.LoadMenuA(IDR_MENU1);
	SetMenu(&menu);
	nDevice=GetDeviceInfo(DeviceName,&TotalMem,&ClockRate,&nProcessor);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCudaCLADlg::OnTimer(UINT_PTR nIDEvent)
{
	CRect rect;
	GetClientRect(&rect);
	InvalidateRect(CRect(15,195,rect.right,215),1);
	InvalidateRect(CRect(0,225,rect.right,240),0);
	UpdateWindow();

	CDialog::OnTimer(nIDEvent);
}

void CCudaCLADlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

	}
	else
	{
		CDialog::OnPaint();
		
	}
	CRect rect;
	GetClientRect(&rect);

	int i;
	CString tmpstr;
	dc.SetBkMode(TRANSPARENT);
	if(DisplayMode==MODE_HIST && HistMax>0)
	{
		for(i=0;i<100;i++)
		{
			dc.Rectangle(20+i*4,rect.bottom-30-rect.bottom*0.8*Hist[i]/HistMax,20+(i+1)*4,rect.bottom-30);
			if((i+1)%10==0)
			{
				if(i<10)
					tmpstr.Format("0.0%d",i);
				else
					tmpstr.Format("0.%d",i);
				dc.TextOutA(17+i*4,rect.bottom-25,tmpstr);
				dc.MoveTo(20+i*4,rect.bottom-29);
				dc.LineTo(20+i*4,rect.bottom-26);
			}
		}
		dc.MoveTo(20,rect.bottom-30);
		dc.LineTo(420,rect.bottom-30);
		CPen gridPen(PS_DOT,1,RGB(0,255,255));
		CPen* pOldPen=dc.SelectObject(&gridPen);
		for(i=1;i<=5;i++)
		{
			dc.MoveTo(20,rect.bottom-30-rect.bottom*0.8*i/5);
			dc.LineTo(420,rect.bottom-30-rect.bottom*0.8*i/5);
			dc.SetBkMode(TRANSPARENT);
			tmpstr.Format("%.1f%%",HistMax*100.0*i/5/HistSum);
			dc.TextOutA(5,rect.bottom-30-rect.bottom*0.8*i/5,tmpstr);
		}
		dc.SelectObject(pOldPen);
	}
	if(DisplayMode==MODE_HIST && HistMax<=0)
	{
		CSize SizeText;
		SizeText=dc.GetTextExtent("No Histogram Available for Display");
		dc.TextOutA((rect.Width()-SizeText.cx)/2,(rect.Height()-SizeText.cy)/2,"No Histogram Available for Display");
	}

	if(DisplayMode==MODE_TEXT)
	{
		dc.SetBkMode(TRANSPARENT);
		dc.FillRect(CRect(0,10,rect.right,70),&BrownBrush);
		dc.SetTextColor(RGB(255,255,0));
		if(nDevice>0)
		{
			tmpstr=DeviceName;
			dc.TextOutA(20,14,"Device Name:");
			dc.TextOutA(160,14,tmpstr);
			tmpstr.Format("Total Global Memory:        %d M Bytes ",TotalMem/1024/1024);
			dc.TextOutA(20,30,tmpstr);
			tmpstr.Format("Number of Multi-Processor: %d        Clock Rate: %d kHz",nProcessor,ClockRate/1000);
			dc.TextOutA(20,46,tmpstr);

		}
		else
		{
			if(nDevice==-1)
				dc.TextOutA(20,15,"CUDA Driver and Runtime version may be mismatched!");
			if(nDevice==0)
				dc.TextOutA(20,15,"No Compatiable CUDA Device is Found!");
		}

		dc.FillRect(CRect(0,80,rect.right,160),&LightBrownBrush);
		dc.SetTextColor(RGB(100,17,192));
		dc.TextOutA(20,84,"Reference Library");
		dc.TextOutA(rect.right/2+20,85,"Candidate Library");
		dc.SetTextColor(RGB(10,17,92));
		if(nTargetRealCount>0 && nTargetDim>0)
		{
			tmpstr.Format("%d Compounds",nTargetRealCount);
			dc.TextOutA(20,100,tmpstr);
			tmpstr.Format("%d-byte fingerprint",nTargetDim*4);
			dc.TextOutA(20,116,tmpstr);
			if(TargetLibFormat==SPA_FORMAT_ID)
				dc.TextOutA(20,132,"Sparse Array Format");
			if(TargetLibFormat==IFP_FORMAT_ID)
				dc.TextOutA(20,132,"Compressed Integer Format");

		}
		else
			dc.TextOutA(25,110,"No Data Available");
		
		if(nRefRealCount>0 && nRefRealDim>0)
		{
			tmpstr.Format("%d Compounds",nRefRealCount);
			dc.TextOutA(rect.right/2+20,100,tmpstr);
			tmpstr.Format("%d-byte fingerprint",nRefRealDim*4);
			dc.TextOutA(rect.right/2+20,116,tmpstr);
			if(RefLibFormat==SPA_FORMAT_ID)
				dc.TextOutA(rect.right/2+20,132,"Sparse Array Format");
			if(RefLibFormat==IFP_FORMAT_ID)
				dc.TextOutA(rect.right/2+20,132,"Compressed Integer Format");
		}
		else
			dc.TextOutA(rect.right/2+25,110,"No Data Available");


		dc.FillRect(CRect(0,170,rect.right,rect.bottom-10),&BlueBrush);
		dc.SetTextColor(RGB(96,18,33));
		dc.TextOutA(20,174,"Job Status:");
		dc.SetTextColor(RGB(0,69,10));
		if(JobCode==JOB_IDLE)
		{
			dc.TextOutA(120,174,"Idle");
			dc.TextOutA(15,195,JobMsg);
		}
		if(JobCode==JOB_INDEX)
		{
			dc.TextOutA(120,174,"Converting Unity Fingerprint...");
			
			if(Progress>=0 && Progress<=100)
			{
				dc.TextOutA(50,195,"Reading Unity Fingerprint...");
				dc.FillRect(CRect(0,225,rect.right*Progress/100,240),&YellowBrush);
			}
			if(Progress==101)
				dc.TextOutA(50,195,"Indexing...");
			
		}
		if(JobCode==JOB_DBCMPR)
		{
			dc.TextOutA(120,174,"Comparing Two Libraries...");
			DWORD time_elapsed=(GetTickCount()-time_begin);
			tmpstr.Format("Time: %.2f sec      Throughput: %d kilo Tc/sec",time_elapsed/1000.0,Progress*128*nTargetCount/time_elapsed);
			dc.TextOutA(15,195,tmpstr);
			dc.FillRect(CRect(0,225,rect.right*Progress*128/(nRefCount+127),240),&YellowBrush);
		}
		
	}
}


void CCudaCLADlg::OnToolsIndexunityfingerprintasreferencelibrary()
{
	char szInputFilters[]="Unity Fingerprint Files (*.txt)|*.txt|All Files (*.*)|*.*||";
	CFileDialog inputFileDlg(TRUE, "txt", "*.txt",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szInputFilters, this);
	if(inputFileDlg.DoModal()==IDOK )
	{
		inputFileName = inputFileDlg.GetPathName();
		TRACE("%s\n",inputFileName);
		char szOutputFilters[]="Indexed Sparse Array Files (*.spa)|*.spa|All Files (*.*)|*.*||";
		CFileDialog outputFileDlg (FALSE, "spa", "*.spa",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szOutputFilters, this);
		if(outputFileDlg.DoModal()==IDOK )
		{
			outputFileName=outputFileDlg.GetPathName();
			TRACE("%s\n",outputFileName);
			JobCode=JOB_INDEX;
			Invalidate();
			UpdateWindow();
			ColumnMajor=TRUE;
			AfxBeginThread(IndexSPA,(LPVOID)this);
			/*SPAIndex(TRUE,inputFileName,outputFileName);
			JobCode=JOB_IDLE;
			Invalidate();
			UpdateWindow();*/
		}    
	}
}
void CCudaCLADlg::OnSparsearrayIndexascandidatelibrary()
{
	char szInputFilters[]="Unity Fingerprint Files (*.txt)|*.txt|All Files (*.*)|*.*||";
	CFileDialog inputFileDlg(TRUE, "txt", "*.txt",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szInputFilters, this);
	if(inputFileDlg.DoModal()==IDOK )
	{
		inputFileName = inputFileDlg.GetPathName();
		TRACE("%s\n",inputFileName);
		char szOutputFilters[]="Indexed Sparse Array Files (*.spa)|*.spa|All Files (*.*)|*.*||";
		CFileDialog outputFileDlg (FALSE, "spa", "*.spa",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szOutputFilters, this);
		if(outputFileDlg.DoModal()==IDOK )
		{
			outputFileName=outputFileDlg.GetPathName();
			TRACE("%s\n",outputFileName);
			JobCode=JOB_INDEX;
			Invalidate();
			UpdateWindow();
			ColumnMajor=FALSE;
			AfxBeginThread(IndexSPA,(LPVOID)this);
			/*SPAIndex(FALSE,inputFileName,outputFileName);
			JobCode=JOB_IDLE;
			Invalidate();
			UpdateWindow();*/
		}    
	}
}

int CCudaCLADlg::SPAIndex(bool ColumnMajor, CString inputFileName, CString outputFileName)
{
	CFile inputFile;
	CFile outputFile;
	char buffer[4096];
	ULONGLONG fileOffset=0;
	ULONGLONG fileLength;
	int readlength;
	CString str;
	CString substr;
	int upper,lower,i;
	bool bReadFingerprint=false;//used to track fingerprint section of the original unity fingerprint file
	int nFpCount=0;
	CObList FpList;//the link list to save fingerprints
	CFp* pFp;
	POSITION pos;

	bool bColumnMajor=ColumnMajor;
	inputFile.Open(inputFileName,CFile::modeRead);
	if(inputFile.m_hFile==CFile::hFileNull)
	{
		AfxMessageBox("cannot open raw unity fingerprint file.");
		return 0;
	}
	outputFile.Open(outputFileName,CFile::modeCreate|CFile::modeWrite);
	if(outputFile.m_hFile==CFile::hFileNull)
	{
		AfxMessageBox("cannot open output file.");
		return 0;
	}
	fileLength=inputFile.GetLength();
	while(fileOffset<fileLength)
	{
		if(fileLength-fileOffset<4096)
			readlength=fileLength-fileOffset;
		else
			readlength=4096;
		
		inputFile.Read(buffer,sizeof(char)*readlength);
		str=buffer;
		str=str.Left(readlength);
		lower=upper=0;
		upper=str.Find('\n',lower);
		
		if(upper<0)
		{
			TRACE("increase the buffer size");
			break;
		}
		while(upper>=0)
		{
			substr=str.Mid(lower,upper-lower+1);
			bool flag=false;//to see if one line only contins 0 or 1
			bool endofline=false;
			for(i=0;i<upper-lower;i++)
				if(substr[i]!='0' && substr[i]!='1')
					flag=true;
			if(upper-lower<=1)
				flag=true;
			if(bReadFingerprint)
			{
				if(flag==true)
				{
					bReadFingerprint=false;
					endofline=true;
				}
			}
			else
			{
				if(flag==false)
				{
					bReadFingerprint=true;
					pFp=new CFp;
				}
			}
			if(bReadFingerprint)
			{
				pFp->strFp+=substr.Left(substr.GetLength()-1);
			}
			if(endofline)
			{
				for(i=0;i<UNITY_FP_LEN;i++)
					if(pFp->strFp[i]=='1')
						pFp->nTotal++;
				FpList.AddTail(pFp);
				nFpCount++;
			}
			lower=upper+1;
			upper=str.Find('\n',lower);
		}
		fileOffset+=lower;
		Progress=100*fileOffset/fileLength;
		inputFile.Seek(fileOffset,CFile::begin);

	}
	
	Progress=101;
	int Max=0;
	pos=FpList.GetHeadPosition();
	while(pos)
	{
		pFp=(CFp*)FpList.GetNext(pos);
		if(pFp->nTotal>Max)
			Max=pFp->nTotal;
	}
	int* pMatrix=NULL;
	int* pArray=NULL;
	CObList** ppFpList=(CObList**)malloc(sizeof(CObList*)*(Max+1));
	for(i=0;i<=Max;i++)
	{
		ppFpList[i]=new CObList;
	}
	pos=FpList.GetHeadPosition();
	while(pos)
	{
		pFp=(CFp*)FpList.GetNext(pos);
		ppFpList[pFp->nTotal]->AddTail(pFp);
	}
	pMatrix=(int*)malloc(sizeof(int)*Max*FpList.GetCount());
	ZeroMemory(pMatrix,sizeof(int)*Max*FpList.GetCount());
	pArray=(int*)malloc(sizeof(int)*FpList.GetCount());
	ZeroMemory(pArray,sizeof(int)*FpList.GetCount());
	if(pMatrix && pArray)
	{
		int Xindex=0;
		int Yindex=0;
		for(i=0;i<=Max;i++)
		{
			pos=ppFpList[i]->GetHeadPosition();
			while(pos)
			{
				pFp=(CFp*)ppFpList[i]->GetNext(pos);
				for(int j=0;j<pFp->strFp.GetLength();j++)
				{
					if(bColumnMajor)
					{
						if(pFp->strFp[j]=='1')
						{
							pMatrix[Yindex*nFpCount+Xindex]=j;
							Yindex++;
						}
					}
					else
					{
						if(pFp->strFp[j]=='1')
						{
							pMatrix[Yindex+Xindex*Max]=j;
							Yindex++;
						}
					}
				}
				pArray[Xindex]=pFp->nTotal;			
				Xindex++;
				Yindex=0;
				
			}
		}
		unsigned int formatid=SPA_FORMAT_ID;
		outputFile.Write(&formatid,sizeof(int));
		outputFile.Write(&bColumnMajor,sizeof(bool));

		outputFile.Write(&nFpCount,sizeof(nFpCount));
		outputFile.Write(&Max,sizeof(Max));
		outputFile.Write(pArray,sizeof(int)*nFpCount);
		outputFile.Write(pMatrix,sizeof(int)*nFpCount*Max);

		delete pMatrix;
		delete pArray;
	}
	else
	{
		AfxMessageBox("Memory Allocation Failed for Fingerprint Array.");
	}

	for(i=0;i<=Max;i++)
	{
		ppFpList[i]->RemoveAll();
		delete ppFpList[i];
	}
	delete ppFpList;

	while(!FpList.IsEmpty())
		delete FpList.RemoveTail();
	

	if(inputFile.m_hFile!=CFile::hFileNull)
		inputFile.Close();
	if(outputFile.m_hFile!=CFile::hFileNull)
		outputFile.Close();
	return 1;

}


void CCudaCLADlg::OnSparsearrayLoadreferencelibrary()
{
	CFile targetLibFile;
	CString TargetFileName;

	char szInputFilters[]="Indexed Sparse Array Files (*.spa)|*.spa|All Files (*.*)|*.*||";
	CFileDialog inputFileDlg(TRUE, "spa", "*.spa",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szInputFilters, this);
	if(inputFileDlg.DoModal()!=IDOK )
		return;

	TargetFileName=inputFileDlg.GetPathName();
	targetLibFile.Open(TargetFileName,CFile::modeRead);
	
	
	if(targetLibFile.m_hFile==CFile::hFileNull)
	{
		AfxMessageBox("cannot open indexed fingerprint");
	}
	else
	{
		int i;
		unsigned int FormatID;
		bool bColumnMajor;
		bool flag=true;

		TargetLibFormat=0;
		targetLibFile.Read(&FormatID,sizeof(int));
		targetLibFile.Read(&bColumnMajor,sizeof(bool));
		targetLibFile.Read(&nTargetRealCount,sizeof(int));
		targetLibFile.Read(&nTargetDim,sizeof(int));
		nTargetCount=(nTargetRealCount+15)/16*16;
		if(FormatID!=SPA_FORMAT_ID)
		{
			AfxMessageBox("File format does not comply with the standard of the program!");
			flag=false;
		}
		if(bColumnMajor!=TRUE && flag)
		{
			AfxMessageBox("This file is not a reference library");
			flag=false;
		}
		if((sizeof(int)*(nTargetRealCount*nTargetDim+nTargetRealCount+3)+sizeof(bool))!=targetLibFile.GetLength())
		{
			AfxMessageBox("File size is not correct. Please check the file format!");
			flag=false;
		}

		TRACE("Target Library: total count %d Dimension %d virtual count %d\n",nTargetRealCount,nTargetDim,nTargetCount);

		if(flag)
		{
			if(pTargetMagArray)
			{
				delete pTargetMagArray;
				pTargetMagArray=NULL;
			}
			if(pTargetMatrix)
			{
				delete pTargetMatrix;
				pTargetMatrix=NULL;
			}
			pTargetMagArray=(int*)malloc(sizeof(int)*nTargetCount);
			ZeroMemory(pTargetMagArray,sizeof(int)*nTargetCount);
			pTargetMatrix=(int*)malloc(sizeof(int)*nTargetCount*nTargetDim);
			ZeroMemory(pTargetMatrix,sizeof(int)*nTargetCount*nTargetDim);
			if(pTargetMagArray && pTargetMatrix)
			{
				targetLibFile.Read(pTargetMagArray,sizeof(int)*nTargetRealCount);
				for(i=0;i<nTargetDim;i++)
					targetLibFile.Read(pTargetMatrix+i*nTargetCount,sizeof(int)*nTargetRealCount);
				TargetLibFormat=FormatID;
			}
			else
				AfxMessageBox("Memory Allocation Failed!\n");
		}
		else
		{
			TargetLibFormat=0;
			nTargetRealCount=0;
			nTargetDim=0;
		}

		targetLibFile.Close();
		Invalidate();
		UpdateWindow();
	}
}

void CCudaCLADlg::OnSparsearrayLoadcandidatelibrary()
{
	CFile refLibFile;
	CString RefFileName;
	char szInputFilters[]="Indexed Sparse Array (*.spa)|*.spa|All Files (*.*)|*.*||";
	CFileDialog inputFileDlg(TRUE, "spa", "*.spa",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szInputFilters, this);
	if(inputFileDlg.DoModal()!=IDOK )
		return;
	RefFileName=inputFileDlg.GetPathName();
	refLibFile.Open(RefFileName,CFile::modeRead);
	if(refLibFile.m_hFile==CFile::hFileNull)
	{
		AfxMessageBox("cannot open indexed fingerprint");
	}
	else
	{
		int i;
		unsigned int FormatID;
		bool bColumnMajor;
		bool flag=true;

		RefLibFormat=0;
		refLibFile.Read(&FormatID,sizeof(int));
		refLibFile.Read(&bColumnMajor,sizeof(bool));
		refLibFile.Read(&nRefRealCount,sizeof(int));
		refLibFile.Read(&nRefRealDim,sizeof(int));
		nRefCount=nRefRealCount;
		nRefDim=(nRefRealDim+15)/16*16;

		TRACE("Reference Library: total count %d Dimension %d virtual count %d\n",nRefRealCount,nRefDim,nRefCount);
		
		if(FormatID!=SPA_FORMAT_ID)
		{
			AfxMessageBox("File format does not comply with the standard of the program!");
			flag=false;
		}
		if(bColumnMajor!=FALSE && flag)
		{
			AfxMessageBox("This file is not a candidate library");
			flag=false;
		}
		if((sizeof(int)*(nRefRealCount*nRefRealDim+nRefRealCount+3)+sizeof(bool))!=refLibFile.GetLength())
		{
			AfxMessageBox("File size is not correct. Please check the file format!");
			flag=false;
		}
		TRACE("%d  %d\n",sizeof(int)*(nRefRealCount*nRefRealDim+nRefRealCount+3)+sizeof(bool),refLibFile.GetLength());

		if(flag)
		{
			if(pRefMagArray)
			{
				delete pRefMagArray;
				pRefMagArray=NULL;
			}
			if(pRefMatrix)
			{
				delete pRefMatrix;
				pRefMatrix=NULL;
			}
			pRefMagArray=(int*)malloc(sizeof(int)*nRefCount);
			ZeroMemory(pRefMagArray,sizeof(int)*nRefCount);
			pRefMatrix=(int*)malloc(sizeof(int)*nRefCount*nRefDim);
			ZeroMemory(pRefMatrix,sizeof(int)*nRefCount*nRefDim);
			if(pRefMagArray && pRefMatrix)
			{
				

				refLibFile.Read(pRefMagArray,sizeof(int)*nRefRealCount);
				for(i=0;i<nRefRealCount;i++)
					refLibFile.Read(pRefMatrix+i*nRefDim,sizeof(int)*nRefRealDim);
				RefLibFormat=FormatID;

			}
			else
				AfxMessageBox("Memory Allocation Failed!\n");
		}
		else
		{
			RefLibFormat=0;
			nRefRealCount=0;
			nRefRealDim=0;

		}

		
		refLibFile.Close();
		Invalidate();
		UpdateWindow();
	}
}

void CCudaCLADlg::OnSparsearrayComparedatabase()
{
	if(nRefDim==0 || nTargetDim==0 || nRefCount==0 || nTargetCount==0)
	{
		AfxMessageBox("Please load reference library and candidate library");
		return;
	}
	if(TargetLibFormat==SPA_FORMAT_ID && RefLibFormat==SPA_FORMAT_ID)
	{
		if(nRefRealDim==nTargetDim && nRefCount==nTargetRealCount)
		{
			int res=AfxMessageBox("The reference library and candidate library are potentially identical. Do you want to carryout self-comparison?",MB_YESNO);
			JobCode=JOB_DBCMPR;
			Invalidate();
			UpdateWindow();
			if(res==IDYES)
			{
				SelfCompare=1;
				AfxBeginThread(ThreadSPADBTC,(LPVOID)this);
			}
			else
			{
				SelfCompare=0;
				AfxBeginThread(ThreadSPADBTC,(LPVOID)this);
			}
		}	
		else
		{
			JobCode=JOB_DBCMPR;
			Invalidate();
			UpdateWindow();
			SelfCompare=1;
			AfxBeginThread(ThreadSPADBTC,(LPVOID)this);
		}
	}
	else
		AfxMessageBox("The indexed library is not in sparse array format");
}

int CCudaCLADlg::IFPIndex(bool ColumnMajor, CString inputFileName, CString outputFileName)
{
	CFile inputFile;
	CFile outputFile;
	char buffer[4096];
	ULONGLONG fileOffset=0;
	ULONGLONG fileLength;
	int readlength;
	CString str;
	CString substr;
	int upper,lower,i;
	bool bReadFingerprint=false;//used to track fingerprint section of the original unity fingerprint file
	int nFpCount=0;
	CObList FpList;//the link list to save fingerprints
	CFp* pFp;
	POSITION pos;

	bool bColumnMajor=ColumnMajor;
	inputFile.Open(inputFileName,CFile::modeRead);
	if(inputFile.m_hFile==CFile::hFileNull)
	{
		AfxMessageBox("cannot open raw unity fingerprint file.");
		return 0;
	}
	outputFile.Open(outputFileName,CFile::modeCreate|CFile::modeWrite);
	if(outputFile.m_hFile==CFile::hFileNull)
	{
		AfxMessageBox("cannot open output file.");
		return 0;
	}
	fileLength=inputFile.GetLength();
	while(fileOffset<fileLength)
	{
		if(fileLength-fileOffset<4096)
			readlength=fileLength-fileOffset;
		else
			readlength=4096;
		
		inputFile.Read(buffer,sizeof(char)*readlength);
		str=buffer;
		str=str.Left(readlength);
		lower=upper=0;
		upper=str.Find('\n',lower);
		
		if(upper<0)
		{
			TRACE("increase the buffer size");
			break;
		}
		while(upper>=0)
		{
			substr=str.Mid(lower,upper-lower+1);
			bool flag=false;//to see if one line only contins 0 or 1
			bool endofline=false;
			for(i=0;i<upper-lower;i++)
				if(substr[i]!='0' && substr[i]!='1')
					flag=true;
			if(upper-lower<=1)
				flag=true;
			if(bReadFingerprint)
			{
				if(flag==true)
				{
					bReadFingerprint=false;
					endofline=true;
				}
			}
			else
			{
				if(flag==false)
				{
					bReadFingerprint=true;
					pFp=new CFp;
				}
			}
			if(bReadFingerprint)
			{
				pFp->strFp+=substr.Left(substr.GetLength()-1);
			}
			if(endofline)
			{
				for(i=0;i<UNITY_FP_LEN;i++)
					if(pFp->strFp[i]=='1')
						pFp->nTotal++;
				FpList.AddTail(pFp);
				nFpCount++;
			}
			lower=upper+1;
			upper=str.Find('\n',lower);
		}
		fileOffset+=lower;
		inputFile.Seek(fileOffset,CFile::begin);
		Progress=100*fileOffset/fileLength;

	}
	
	Progress=101;
	int Max=0;
	pos=FpList.GetHeadPosition();
	while(pos)
	{
		pFp=(CFp*)FpList.GetNext(pos);
		if(pFp->nTotal>Max)
			Max=pFp->nTotal;
	}
	int* pMatrix=NULL;
	int* pArray=NULL;
	CObList** ppFpList=(CObList**)malloc(sizeof(CObList*)*(Max+1));
	for(i=0;i<=Max;i++)
	{
		ppFpList[i]=new CObList;
	}
	pos=FpList.GetHeadPosition();
	while(pos)
	{
		pFp=(CFp*)FpList.GetNext(pos);
		ppFpList[pFp->nTotal]->AddTail(pFp);
	}

	int FpLen=(UNITY_FP_LEN+31)/32;
	pMatrix=(int*)malloc(sizeof(int)*FpLen*FpList.GetCount());
	ZeroMemory(pMatrix,sizeof(int)*FpLen*FpList.GetCount());
	pArray=(int*)malloc(sizeof(int)*FpList.GetCount());
	ZeroMemory(pArray,sizeof(int)*FpList.GetCount());
	int* pIntFp=(int*)malloc(FpLen*sizeof(int));
	if(pMatrix && pArray && pIntFp)
	{
		int Xindex=0;
		int Yindex=0;
		for(i=0;i<=Max;i++)
		{
			pos=ppFpList[i]->GetHeadPosition();
			while(pos)
			{
				pFp=(CFp*)ppFpList[i]->GetNext(pos);
				ZeroMemory(pIntFp,FpLen*sizeof(int));
				int IntFpPointer=-1;
				unsigned int IntMask=1;
				for(int j=0;j<pFp->strFp.GetLength();j++)
				{
					if(j%32==0)
					{
						IntFpPointer++;
						IntMask=1;
					}
					if(pFp->strFp[j]=='1')
						pIntFp[IntFpPointer]=pIntFp[IntFpPointer]|IntMask;

					IntMask=IntMask*2;
				}
				if(bColumnMajor)
				{
					for(int j=0;j<FpLen;j++)
					{
						pMatrix[j*nFpCount+Xindex]=pIntFp[j];
					}
				}
				else
				{
					for(int j=0;j<FpLen;j++)
					{
						pMatrix[j+FpLen*Xindex]=pIntFp[j];
					}
				}
				pArray[Xindex]=pFp->nTotal;

				if(Xindex==0)
				{
					if(bColumnMajor)
					{
						TRACE("\n%s\n",pFp->strFp);
						for(int j=0;j<FpLen;j++)
						{
							unsigned int mask=1;
							for(int k=0;k<32;k++)
							{
								if(pMatrix[j*nFpCount+Xindex] & mask)
									TRACE("1");
								else
									TRACE("0");
								mask=mask*2;
							}
						}
						TRACE("\n%d\n",pArray[Xindex]);
					}
					else
					{
						TRACE("%s\n",pFp->strFp);
						for(int j=0;j<FpLen;j++)
						{
							unsigned int mask=1;
							for(int k=0;k<32;k++)
							{
								if(pMatrix[j+Xindex*FpLen] & mask)
									TRACE("1");
								else
									TRACE("0");
								mask=mask*2;
							}
						}
						TRACE("\n%d\n",pArray[Xindex]);
					}
				}
				
				Xindex++;
				Yindex=0;
				
			}
		}
		unsigned int formatid=IFP_FORMAT_ID;
		outputFile.Write(&formatid,sizeof(int));
		outputFile.Write(&bColumnMajor,sizeof(bool));

		outputFile.Write(&nFpCount,sizeof(nFpCount));
		outputFile.Write(&FpLen,sizeof(FpLen));
		outputFile.Write(pArray,sizeof(int)*nFpCount);
		outputFile.Write(pMatrix,sizeof(int)*nFpCount*FpLen);

		delete pMatrix;
		delete pArray;
		delete pIntFp;
		TRACE("%d %d\n",Xindex,Yindex);
	}
	else
	{
		AfxMessageBox("Memory Allocation Failed for Fingerprint Array.");
	}

	for(i=0;i<=Max;i++)
	{
		ppFpList[i]->RemoveAll();
		delete ppFpList[i];
	}
	delete ppFpList;

	while(!FpList.IsEmpty())
		delete FpList.RemoveTail();
	

	if(inputFile.m_hFile!=CFile::hFileNull)
		inputFile.Close();
	if(outputFile.m_hFile!=CFile::hFileNull)
		outputFile.Close();
	return 1;
}

void CCudaCLADlg::OnCompressedfingerprintIndexascandidatelibrary()
{
	char szInputFilters[]="Unity Fingerprint Files (*.txt)|*.txt|All Files (*.*)|*.*||";
	CFileDialog inputFileDlg(TRUE, "txt", "*.txt",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szInputFilters, this);
	if(inputFileDlg.DoModal()==IDOK )
	{
		inputFileName = inputFileDlg.GetPathName();
		TRACE("%s\n",inputFileName);
		char szOutputFilters[]="Indexed Compressed Fingerprint Files (*.ifp)|*.ifp|All Files (*.*)|*.*||";
		CFileDialog outputFileDlg (FALSE, "ifp", "*.ifp",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szOutputFilters, this);
		if(outputFileDlg.DoModal()==IDOK )
		{
			outputFileName=outputFileDlg.GetPathName();
			TRACE("%s\n",outputFileName);
			JobCode=JOB_INDEX;
			Invalidate();
			UpdateWindow();
			ColumnMajor=FALSE;
			AfxBeginThread(IndexIFP,(LPVOID)this);
			/*IFPIndex(FALSE,inputFileName,outputFileName);
			JobCode=JOB_IDLE;
			Invalidate();
			UpdateWindow();*/
		}    
	}
}

void CCudaCLADlg::OnCompressedfingerprintIndexasreferencelibrary()
{
	char szInputFilters[]="Unity Fingerprint Files (*.txt)|*.txt|All Files (*.*)|*.*||";
	CFileDialog inputFileDlg(TRUE, "txt", "*.txt",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szInputFilters, this);
	if(inputFileDlg.DoModal()==IDOK )
	{
		inputFileName = inputFileDlg.GetPathName();
		TRACE("%s\n",inputFileName);
		char szOutputFilters[]="Indexed Compressed Fingerprint Files (*.ifp)|*.ifp|All Files (*.*)|*.*||";
		CFileDialog outputFileDlg (FALSE, "ifp", "*.ifp",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szOutputFilters, this);
		if(outputFileDlg.DoModal()==IDOK )
		{
			outputFileName=outputFileDlg.GetPathName();
			TRACE("%s\n",outputFileName);
			JobCode=JOB_INDEX;
			Invalidate();
			UpdateWindow();
			ColumnMajor=TRUE;
			AfxBeginThread(IndexIFP,(LPVOID)this);
			/*IFPIndex(TRUE,inputFileName,outputFileName);
			JobCode=JOB_IDLE;
			Invalidate();
			UpdateWindow();*/
		}    
	}
}

void CCudaCLADlg::OnCompressedfingerprintLoadreferencelibrary()
{
	CFile targetLibFile;
	CString TargetFileName;

	char szInputFilters[]="Indexed Compressed Fingerprint Files (*.ifp)|*.ifp|All Files (*.*)|*.*||";
	CFileDialog inputFileDlg(TRUE, "ifp", "*.ifp",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szInputFilters, this);
	if(inputFileDlg.DoModal()!=IDOK )
		return;

	TargetFileName=inputFileDlg.GetPathName();
	targetLibFile.Open(TargetFileName,CFile::modeRead);
	
	
	if(targetLibFile.m_hFile==CFile::hFileNull)
	{
		AfxMessageBox("cannot open indexed fingerprint");
	}
	else
	{
		int i;
		unsigned int FormatID;
		bool bColumnMajor;
		bool flag=true;

		TargetLibFormat=0;
		targetLibFile.Read(&FormatID,sizeof(int));
		targetLibFile.Read(&bColumnMajor,sizeof(bool));
		targetLibFile.Read(&nTargetRealCount,sizeof(int));
		targetLibFile.Read(&nTargetDim,sizeof(int));
		nTargetCount=(nTargetRealCount+15)/16*16;
		if(FormatID!=IFP_FORMAT_ID)
		{
			AfxMessageBox("File format does not comply with the standard of the program!");
			flag=false;
		}
		if(bColumnMajor!=TRUE && flag)
		{
			AfxMessageBox("This file is not a reference library");
			flag=false;
		}
		if((sizeof(int)*(nTargetRealCount*nTargetDim+nTargetRealCount+3)+sizeof(bool))!=targetLibFile.GetLength())
		{
			AfxMessageBox("File size is not correct. Please check the file format!");
			flag=false;
		}

		TRACE("Target Library: total count %d Dimension %d virtual count %d\n",nTargetRealCount,nTargetDim,nTargetCount);

		if(flag)
		{
			if(pTargetMagArray)
			{
				delete pTargetMagArray;
				pTargetMagArray=NULL;
			}
			if(pTargetMatrix)
			{
				delete pTargetMatrix;
				pTargetMatrix=NULL;
			}
			pTargetMagArray=(int*)malloc(sizeof(int)*nTargetCount);
			ZeroMemory(pTargetMagArray,sizeof(int)*nTargetCount);
			pTargetMatrix=(int*)malloc(sizeof(int)*nTargetCount*nTargetDim);
			ZeroMemory(pTargetMatrix,sizeof(int)*nTargetCount*nTargetDim);
			if(pTargetMagArray && pTargetMatrix)
			{
				targetLibFile.Read(pTargetMagArray,sizeof(int)*nTargetRealCount);
				for(i=0;i<nTargetDim;i++)
					targetLibFile.Read(pTargetMatrix+i*nTargetCount,sizeof(int)*nTargetRealCount);
				TargetLibFormat=FormatID;
			}
			else
				AfxMessageBox("Memory Allocation Failed!\n");
		}
		else
		{
			TargetLibFormat=0;
			nTargetRealCount=0;
			nTargetDim=0;
		}

		targetLibFile.Close();
		Invalidate();
		UpdateWindow();
	}
}

void CCudaCLADlg::OnCompressedfingerprintLoadcandidatelibrary()
{
	CFile refLibFile;
	CString RefFileName;
	char szInputFilters[]="Indexed Sparse Array (*.ifp)|*.ifp|All Files (*.*)|*.*||";
	CFileDialog inputFileDlg(TRUE, "ifp", "*.ifp",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szInputFilters, this);
	if(inputFileDlg.DoModal()!=IDOK )
		return;
	RefFileName=inputFileDlg.GetPathName();
	refLibFile.Open(RefFileName,CFile::modeRead);
	if(refLibFile.m_hFile==CFile::hFileNull)
	{
		AfxMessageBox("cannot open indexed fingerprint");
	}
	else
	{
		int i;
		unsigned int FormatID;
		bool bColumnMajor;
		bool flag=true;

		RefLibFormat=0;
		refLibFile.Read(&FormatID,sizeof(int));
		refLibFile.Read(&bColumnMajor,sizeof(bool));
		refLibFile.Read(&nRefRealCount,sizeof(int));
		refLibFile.Read(&nRefRealDim,sizeof(int));
		nRefCount=nRefRealCount;
		nRefDim=(nRefRealDim+15)/16*16;

		TRACE("Reference Library: total count %d Dimension %d virtual count %d\n",nRefRealCount,nRefDim,nRefCount);
		
		if(FormatID!=IFP_FORMAT_ID)
		{
			AfxMessageBox("File format does not comply with the standard of the program!");
			flag=false;
		}
		if(bColumnMajor!=FALSE && flag)
		{
			AfxMessageBox("This file is not a candidate library");
			flag=false;
		}
		if((sizeof(int)*(nRefRealCount*nRefRealDim+nRefRealCount+3)+sizeof(bool))!=refLibFile.GetLength())
		{
			AfxMessageBox("File size is not correct. Please check the file format!");
			flag=false;
		}
		TRACE("%d  %d\n",sizeof(int)*(nRefRealCount*nRefRealDim+nRefRealCount+3)+sizeof(bool),refLibFile.GetLength());

		if(flag)
		{
			if(pRefMagArray)
			{
				delete pRefMagArray;
				pRefMagArray=NULL;
			}
			if(pRefMatrix)
			{
				delete pRefMatrix;
				pRefMatrix=NULL;
			}
			pRefMagArray=(int*)malloc(sizeof(int)*nRefCount);
			ZeroMemory(pRefMagArray,sizeof(int)*nRefCount);
			pRefMatrix=(int*)malloc(sizeof(int)*nRefCount*nRefDim);
			ZeroMemory(pRefMatrix,sizeof(int)*nRefCount*nRefDim);
			if(pRefMagArray && pRefMatrix)
			{
				

				refLibFile.Read(pRefMagArray,sizeof(int)*nRefRealCount);
				for(i=0;i<nRefRealCount;i++)
					refLibFile.Read(pRefMatrix+i*nRefDim,sizeof(int)*nRefRealDim);
				RefLibFormat=FormatID;

			}
			else
				AfxMessageBox("Memory Allocation Failed!\n");
		}
		else
		{
			RefLibFormat=0;
			nRefRealCount=0;
			nRefRealDim=0;
		}

		refLibFile.Close();
		Invalidate();
		UpdateWindow();
	}
}

void CCudaCLADlg::OnCompressedfingerprintComparedatabase()
{
	// TODO: Add your command handler code here
	if(nRefDim==0 || nTargetDim==0 || nRefCount==0 || nTargetCount==0)
	{
		AfxMessageBox("Please load reference library and candidate library");
		return;
	}
	if(TargetLibFormat==IFP_FORMAT_ID && RefLibFormat==IFP_FORMAT_ID)
	{
		if(nRefRealDim==nTargetDim && nRefCount==nTargetRealCount)
		{
			
			int res=AfxMessageBox("The reference library and candidate library are potentially identical. Do you want to carryout self-comparison?",MB_YESNO);
			JobCode=JOB_DBCMPR;
			Invalidate();
			UpdateWindow();
			if(res==IDYES)
			{
				SelfCompare=1;
				AfxBeginThread(ThreadINTDBTC,(LPVOID)this);
			}
			else
			{
				SelfCompare=0;
				AfxBeginThread(ThreadINTDBTC,(LPVOID)this);
			}
		}	
		else
		{
			JobCode=JOB_DBCMPR;
			Invalidate();
			UpdateWindow();
			SelfCompare=0;
			AfxBeginThread(ThreadINTDBTC,(LPVOID)this);
		}
	
	}
	else
		AfxMessageBox("The indexed library is not in sparse array format");
	
}
int CCudaCLADlg::DrawHist(bool flag)
{
	if(flag)
	{
		if(ReturnValue==1)
		{
			HistSum=0;
			HistMax=0;
			for(int i=0;i<100;i++)
			{
				HistSum+=Hist[i];
				if(Hist[i]>HistMax)
					HistMax=Hist[i];
			}
			OnOptionShowhistogram();
			JobMsg.Format("Calculation Time: %.2f sec  Throughput: %d kilo Tc/sec",(time_end-time_begin)/1000.0,nTargetRealCount*nRefRealCount/(time_end-time_begin));
		}
		else
		{
			CString tmpstr;
			tmpstr.Format("Error Occurs. Error code: %d", ReturnValue);
			AfxMessageBox(tmpstr);
		}
	}
	else
		JobMsg="Unity Fingerprint Conversion is finished.";
	JobCode=JOB_IDLE;
	Invalidate();
	UpdateWindow();
	return 0;
}

void CCudaCLADlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnMouseMove(nFlags, point);
	if(DisplayMode==MODE_HIST && HistMax>0)
	{
		CRect rect;
		GetClientRect(&rect);
		if(point.x>=20 && point.x<420 && point.y<rect.bottom-30 && point.y > rect.bottom*0.2-30)
		{
			CString tmpstr;
			CClientDC dc(this);
			int index=(point.x-20)/4;
			tmpstr.Format("[%.2f, %.2f]: %.1f%%",index/100.0,(index+1)/100.0,Hist[index]*100.0/HistSum);
			dc.FillRect(CRect(0,0,rect.right,17),&YellowBrush);
			dc.SetBkMode(TRANSPARENT);
			dc.TextOutA(10,0,tmpstr);
		}
	}
}

void CCudaCLADlg::OnOptionShowstatus()
{
	// TODO: Add your command handler code here
	CMenu* mmenu = GetMenu();
    CMenu* submenu = mmenu->GetSubMenu(2);
	submenu->CheckMenuItem(ID_OPTION_SHOWSTATUS, MF_CHECKED | MF_BYCOMMAND);
	submenu->CheckMenuItem(ID_OPTION_SHOWHISTOGRAM, MF_UNCHECKED | MF_BYCOMMAND);
	DisplayMode=MODE_TEXT;
	Invalidate();
	UpdateWindow();

}

void CCudaCLADlg::OnOptionShowhistogram()
{
	// TODO: Add your command handler code here
	CMenu* mmenu = GetMenu();
    CMenu* submenu = mmenu->GetSubMenu(2);
	submenu->CheckMenuItem(ID_OPTION_SHOWSTATUS, MF_UNCHECKED | MF_BYCOMMAND);
	submenu->CheckMenuItem(ID_OPTION_SHOWHISTOGRAM, MF_CHECKED | MF_BYCOMMAND);
	DisplayMode=MODE_HIST;
	Invalidate();
	UpdateWindow();	
}

void CCudaCLADlg::OnOptionExport()
{
	// TODO: Add your command handler code here
	CString outputFileName;
	CString tmpstr;
	char szOutputFilters[]="Text Files (*.txt)|*.txt|All Files (*.*)|*.*||";
	CFileDialog outputFileDlg (FALSE, "txt", "*.txt",OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szOutputFilters, this);
	if(outputFileDlg.DoModal()==IDOK )
	{
		outputFileName=outputFileDlg.GetPathName();
		CFile outputFile;
		outputFile.Open(outputFileName,CFile::modeCreate|CFile::modeWrite);
		tmpstr.Format("Reference Library: %d compounds\r\nCandidate Library: %d compounds\r\n",nTargetRealCount,nRefRealCount);
		outputFile.Write(tmpstr,tmpstr.GetLength());
		tmpstr="Bin \t Raw Counts \t Percentage\r\n";
		outputFile.Write(tmpstr,tmpstr.GetLength());
		for(int i=0;i<100;i++)
		{
			tmpstr.Format("%d        \t%d \t%.2f%%\r\n",i,Hist[i],100.0f*Hist[i]/HistSum);
			outputFile.Write(tmpstr,tmpstr.GetLength());
		}
		outputFile.Close();
	}
}

void CCudaCLADlg::OnOptionAbout()
{
	// TODO: Add your command handler code here
	CAbtDlg dlg;
	dlg.DoModal();
}
