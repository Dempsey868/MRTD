
#include "stdafx.h"
#include "PCSCSample.h"
#include "ReaderDlg.h"
#include "PCSCSampleDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPCSCSampleDlg dialog

CPCSCSampleDlg::CPCSCSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPCSCSampleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPCSCSampleDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPCSCSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPCSCSampleDlg)
	DDX_Control(pDX, IDC_STATIC_READER, m_Ctrl_ReaderName);
	//DDX_Control(pDX, IDC_TRACK3, m_ctrl_TRACK3);
	//DDX_Control(pDX, IDC_TRACK2, m_ctrl_TRACK2);
	//DDX_Control(pDX, IDC_TRACK1, m_ctrl_TRACK1);
	DDX_Control(pDX, IDC_EDIT_OUT, m_ctrl_OUT);
	DDX_Control(pDX, IDC_COMBO_IN, m_ctrl_IN);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPCSCSampleDlg, CDialog)
	ON_MESSAGE(WM_CARDINSERTED, OnCardInserted) // Your user-defined Window Message handler function
	ON_MESSAGE(WM_CARDREMOVED, OnCardRemoved) // Your user-defined Window Message handler function

	//{{AFX_MSG_MAP(CPCSCSampleDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CARD_DISCONNECT, OnCardDisconnect)
	ON_BN_CLICKED(IDC_CONNECT_SC, OnConnectSc)
	ON_BN_CLICKED(IDC_TRANSMIT_SC, OnTransmitSc)
	ON_WM_DESTROY()
	//ON_BN_CLICKED(IDC_CONNECT_MC, OnConnectMc)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPCSCSampleDlg message handlers

BOOL CPCSCSampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
//-- TODO: Add extra initialization here ------------------------------------------
	pscmc = (LPPCSC_SCMC) malloc(sizeof(PCSC_SCMC));
	ASSERT( pscmc );
	memset(pscmc, 0, sizeof(PCSC_SCMC));
	
//-- Establish a contexts which communicate with the Resource Manager------------
	if ( !PCSC_EstablishContext(pscmc) )
	{		
		MessageBox(PCSC_GetLastError(pscmc), _T("CCID Demo"), MB_OK| MB_ICONSTOP);
		EndDialog(-1);
	}

//-- Get Readers' name ------------------------------------------------------------
	if ( !PCSC_GetReaderNames(pscmc) )
	{
		MessageBox(PCSC_GetLastError(pscmc), _T("CCID Demo"), MB_OK| MB_ICONSTOP);
		EndDialog(-1);
	}

//-- Select a reader -----------------------------------------------------------
	CReaderDlg dlg;
	
	dlg.SetCount(pscmc->dwReaderCount);
	dlg.SetName( (LPTSTR)pscmc->mszReaderName );

	if ( dlg.DoModal() != IDOK ) 
	{
		EndDialog(-1);
	}

	pscmc->dwActReader = dlg.GetActReader();   
	m_Ctrl_ReaderName.SetWindowText((char*)pscmc->mszReaderName);
//---------------------------------------------------------------------------------
	m_ctrl_OUT.SetWindowText(_T("Please:\r\n1. Insert a SMART CARD and press <Connect Smart Card> button to communicate with it.\r\n2. Put a MIFARE CARD on the antenna and press <Connect Smart Card> button to communicate with it."));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPCSCSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPCSCSampleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPCSCSampleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//-- PC/SC commands----------------------------------
void CPCSCSampleDlg::OnConnectSc() 
{	
	// smart card connecting
	if ( !PCSC_SCARD_Connect(pscmc) )
	{
		_tcscpy(pscmc->szMsg, PCSC_GetLastError(pscmc));
		_tcscat(pscmc->szMsg, _T("\r\nError: Connecting Smart Card is failed. \r\nPlease insert your Smart Card once again (the chip must face upward and near the front side).") );
		m_ctrl_OUT.SetWindowText(pscmc->szMsg);

		return;
	}

	if ( !PCSC_GetCardStatus(pscmc) ) 
	{
		m_ctrl_OUT.SetWindowText( PCSC_GetLastError(pscmc) );
		return;
	}

	// pscmc->ret == SCARD_S_SUCCESS
	switch (pscmc->dwActProtocol) 
	{
		case SCARD_PROTOCOL_T0: 
			_tcscpy(pscmc->szMsg, _T("Connecting Smart Card is OK!\r\nProtocoltype = T0"));
			break;
		case SCARD_PROTOCOL_T1: 
			_tcscpy(pscmc->szMsg, _T("Connecting Smart Card is OK!\r\nProtocoltype = T1"));
			break;
		case SCARD_PROTOCOL_RAW:
		case SCARD_PROTOCOL_UNDEFINED:
			_tcscpy(pscmc->szMsg, _T("Connecting Smart Card is OK!\r\nProtocoltype = Undefined type or no communication"));
			break;
	}

	// pscmc->ret == SCARD_S_SUCCESS
	PCSC_ByteToOutFormat(pscmc->brRecvBuf, pscmc->dwRecvLen, pscmc->szOutFormat);
	_tcscat( pscmc->szMsg, _T("\r\nATR = ") ); 
	_tcscat( pscmc->szMsg, pscmc->szOutFormat ); 
	
	m_ctrl_OUT.SetWindowText(pscmc->szMsg);		

   	(CButton *)GetDlgItem(IDC_CONNECT_SC)->EnableWindow(FALSE);
  	(CButton *)GetDlgItem(IDC_TRANSMIT_SC)->EnableWindow(TRUE);          
	//(CButton *)GetDlgItem(IDC_CONNECT_MC)->EnableWindow(FALSE);
	//(CButton *)GetDlgItem(IDC_CONNECT_RFID)->EnableWindow(FALSE);
   	(CButton *)GetDlgItem(IDC_CARD_DISCONNECT)->EnableWindow(TRUE);
	
	m_ctrl_IN.SetCurSel(0);
}

static TCHAR	szTemp[1024];	
void CPCSCSampleDlg::OnTransmitSc() 
{
	// TODO: Add your control notification handler code here
	memset(pscmc->brSendBuf, 0, MAX_SEND);
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);

	m_ctrl_IN.GetWindowText(szTemp, sizeof(szTemp));
		
	if (!PCSC_AToSendBuf(szTemp, pscmc))  // szTemp to pscmc->brSendBuf
	{
		MessageBox(_T("WARNING: Please input Hex number!!"), _T("CCID Demo"), MB_ICONWARNING);
		m_ctrl_IN.SetFocus();
		return;	
	}

	if ( !PCSC_SCARD_Transmit(pscmc) )
	{
		m_ctrl_OUT.SetWindowText( PCSC_GetLastError(pscmc) );	
		return;
	}

	// pscmc->ret == SCARD_S_SUCCESS
	PCSC_ByteToOutFormat(pscmc->brRecvBuf, pscmc->dwRecvLen, pscmc->szOutFormat);
	m_ctrl_OUT.SetWindowText(pscmc->szOutFormat);	
}

/*
void CPCSCSampleDlg::OnConnectReader() 
{	
	pscmc->hTargetWnd = this->GetSafeHwnd(); // Get the target-window handle 

	if ( !PCSC_READER_Connect(pscmc) )
	{
		_tcscpy(pscmc->szMsg, PCSC_GetLastError(pscmc));
		_tcscat(pscmc->szMsg, _T("\r\nError: Connecting reader is failed.") );
		m_ctrl_OUT.SetWindowText(pscmc->szMsg);

		return;
	}

	// pscmc->ret == SCARD_S_SUCCESS
	_tcscpy(pscmc->szMsg, _T("Connecting reader is ok!"));

	_tcscat( pscmc->szMsg, _T("\r\nPlease insert a Memory Card (the chip must face upward and near the front side) \r\nor insert and remove a Magnetic Card (the black track faces downward and near the right side).") ); 
	m_ctrl_OUT.SetWindowText(pscmc->szMsg);

	if ( !PCSC_GetCardStatus(pscmc) )
	{
		m_ctrl_OUT.SetWindowText( PCSC_GetLastError(pscmc) );
		return;
	}

	// pscmc->ret == SCARD_S_SUCCESS
	if ( (pscmc->dwCardStatus==SCARD_PRESENT) || (pscmc->dwCardStatus==SCARD_SPECIFIC) )
		pscmc->iCardInsert = CARD_INSERTED; // a magnetic card in the reader
	else
		pscmc->iCardInsert = CARD_NOCARD; // no magnetic card in the reader

   	(CButton *)GetDlgItem(IDC_CONNECT_SC)->EnableWindow(FALSE);  
	(CButton *)GetDlgItem(IDC_CONNECT_MC)->EnableWindow(FALSE); 
  	(CButton *)GetDlgItem(IDC_TRANSMIT_MC)->EnableWindow(TRUE);   
   	(CButton *)GetDlgItem(IDC_CARD_DISCONNECT)->EnableWindow(TRUE);                  
}
*/

/*
void CPCSCSampleDlg::OnTransmitReader() 
{
	// TODO: Add your control notification handler code here
	memset(pscmc->brSendBuf, 0, MAX_SEND);
	memset(pscmc->brRecvBuf, 0, MAX_RECEIVE);

	m_ctrl_IN.GetWindowText(szTemp, sizeof(szTemp));
		
	if (!PCSC_AToSendBuf(szTemp, pscmc))  // to pscmc->brSendBuf
	{
		MessageBox(_T("WARNING: Please input Hex number!!"), _T("PCSCSample"), MB_ICONWARNING);
		m_ctrl_IN.SetFocus();
		return;	
	}

	if ( !PCSC_READER_Transmit(pscmc) )
	{
		m_ctrl_OUT.SetWindowText( PCSC_GetLastError(pscmc) );		
		return;
	}

	// pscmc->ret == SCARD_S_SUCCESS
	PCSC_ByteToOutFormat(pscmc->brRecvBuf, pscmc->dwRecvLen, pscmc->szOutFormat);
	m_ctrl_OUT.SetWindowText(pscmc->szOutFormat);	
}
*/

VOID CPCSCSampleDlg::OnCardInserted(WPARAM wParam, LPARAM lParam)
{
	m_ctrl_OUT.SetWindowText(_T("Info: User inserts a Card!"));
	return;
}

VOID CPCSCSampleDlg::OnCardRemoved(WPARAM wParam, LPARAM lParam)
{
	m_ctrl_OUT.SetWindowText(_T("Info: User removes a Card!"));		


/*	if (   
		!PCSC_READER_GetT1Data(pscmc)				
		|| !PCSC_READER_GetT2Data(pscmc)
		|| !PCSC_READER_GetT3Data(pscmc)  )
	{
		KillTimer(1);
		m_ctrl_OUT.SetWindowText( PCSC_GetLastError(pscmc) );
		return;
	}

	if (pscmc->dwMCardMode==0)
	{
		m_ctrl_TRACK1.SetWindowText(pscmc->TrackData[0]);
		m_ctrl_TRACK2.SetWindowText(pscmc->TrackData[1]);
		m_ctrl_TRACK3.SetWindowText(pscmc->TrackData[2]);	
	}
	else // pscmc->dwMCardMode==1
	{
		PCSC_ByteToOutFormat(pscmc->TrackBitmap[0], sizeof(pscmc->TrackBitmap[0]), pscmc->szOutFormat);
		m_ctrl_TRACK1.SetWindowText(pscmc->szOutFormat);
		PCSC_ByteToOutFormat(pscmc->TrackBitmap[1], sizeof(pscmc->TrackBitmap[1]), pscmc->szOutFormat);
		m_ctrl_TRACK2.SetWindowText(pscmc->szOutFormat);
		PCSC_ByteToOutFormat(pscmc->TrackBitmap[2], sizeof(pscmc->TrackBitmap[2]), pscmc->szOutFormat);
		m_ctrl_TRACK3.SetWindowText(pscmc->szOutFormat);
	}
*/
}


void CPCSCSampleDlg::OnCardDisconnect() 
{
	// TODO: Add your control notification handler code here
	KillTimer(1);

	if (pscmc->iConnectionStatus == CONNECTION_SCARD)
		PCSC_SCARD_Disconnect(pscmc);

	//if (pscmc->iConnectionStatus == CONNECTION_READER)
	//	PCSC_READER_Disconnect(pscmc);

	if (pscmc->iConnectionStatus == CONNECTION_RFID)
	{
		// Halt
		pscmc->brSendBuf[0] = 0x81;
		pscmc->brSendBuf[1] = 0x06;
		pscmc->dwSendLen = 2;
//		PCSC_RFID_Transmit(pscmc);
		// Disconnect
//		PCSC_RFID_Disconnect(pscmc);
	}

	if (pscmc->iConnectionStatus == CONNECTION_MAGCARD)
		PCSC_MagCard_Disconnect(pscmc);

	if (pscmc->ret != SCARD_S_SUCCESS)
		m_ctrl_OUT.SetWindowText( PCSC_GetLastError(pscmc) );
	else
		m_ctrl_OUT.SetWindowText( _T("Disconnect ok!") );
	
   	(CButton *)GetDlgItem(IDC_CONNECT_SC)->EnableWindow(TRUE); 
	(CButton *)GetDlgItem(IDC_TRANSMIT_SC)->EnableWindow(FALSE);  
	//(CButton *)GetDlgItem(IDC_CONNECT_MC)->EnableWindow(TRUE); 
   	(CButton *)GetDlgItem(IDC_CARD_DISCONNECT)->EnableWindow(FALSE); 
	



}


void CPCSCSampleDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (pscmc->iConnectionStatus == CONNECTION_SCARD)
		PCSC_SCARD_Disconnect(pscmc);

	//if (pscmc->iConnectionStatus == CONNECTION_READER)
	//	PCSC_READER_Disconnect(pscmc);

	PCSC_ReleaseContext(pscmc);
	
	free(pscmc);	
}



void CPCSCSampleDlg::OnConnectRfid() 
{
	// TODO: Add your control notification handler code here
	// smart card connecting
	if ( !PCSC_RFID_Connect(pscmc) )
	{
		_tcscpy(pscmc->szMsg, PCSC_GetLastError(pscmc));
		_tcscat(pscmc->szMsg, _T("\r\nError: Connecting Mifare Card is failed.") );
		m_ctrl_OUT.SetWindowText(pscmc->szMsg);

		return;
	}

	// pscmc->ret == SCARD_S_SUCCESS
	PCSC_ByteToOutFormat(pscmc->brRecvBuf, pscmc->dwRecvLen, pscmc->szOutFormat);
	_tcscat( pscmc->szMsg, _T("\r\nATR = ") ); 
	_tcscat( pscmc->szMsg, pscmc->szOutFormat ); 
	
	m_ctrl_OUT.SetWindowText(pscmc->szMsg);
	// F/W Version
/*	pscmc->brSendBuf[0] = 0x81;
	pscmc->brSendBuf[1] = 0x07;
	pscmc->dwSendLen = 2;
	if ( !PCSC_RFID_Transmit(pscmc) )
	{
		_tcscpy(pscmc->szMsg, PCSC_GetLastError(pscmc));
		_tcscat(pscmc->szMsg, _T("\r\nError: Getting F/W Version is failed.") );
		m_ctrl_OUT.SetWindowText(pscmc->szMsg);

		return;
	}
	else
		m_Ctrl_FWVersion.SetWindowText((char*)pscmc->brRecvBuf);*/

   	(CButton *)GetDlgItem(IDC_CONNECT_SC)->EnableWindow(FALSE);
  	(CButton *)GetDlgItem(IDC_TRANSMIT_SC)->EnableWindow(FALSE);          
	(CButton *)GetDlgItem(IDC_CONNECT_MC)->EnableWindow(FALSE);   
   	(CButton *)GetDlgItem(IDC_CARD_DISCONNECT)->EnableWindow(TRUE);	
	(CButton *)GetDlgItem(IDC_CONNECT_RFID)->EnableWindow(FALSE); 
	(CButton *)GetDlgItem(IDC_TRANSMIT_MIFARECARD)->EnableWindow(TRUE);

//	m_ctrl_IN.ResetContent();
/*	m_ctrl_IN.AddString("(01) Mifare Card Auth Key");
	m_ctrl_IN.AddString("(02) Mifare Card Read");
	m_ctrl_IN.AddString("(03) Mifare Card Write");
	m_ctrl_IN.AddString("(04) Mifare Card Increment");
	m_ctrl_IN.AddString("(05) Mifare Card Decrement");
	m_ctrl_IN.AddString("(06) Mifare Card Halt");
*/
	m_ctrl_IN.SetCurSel(21);
}

/*
void CPCSCSampleDlg::OnConnectMc() 
{
	// TODO: Add your control notification handler code here
	_tcscpy( pscmc->szMsg, _T("Please insert the Magnetic Card and swipe the Card !") );
	m_ctrl_OUT.SetWindowText(pscmc->szMsg);

	(CButton *)GetDlgItem(IDC_CONNECT_SC)->EnableWindow(FALSE);
  	(CButton *)GetDlgItem(IDC_TRANSMIT_SC)->EnableWindow(FALSE);          
	(CButton *)GetDlgItem(IDC_CONNECT_MC)->EnableWindow(FALSE);   
   	(CButton *)GetDlgItem(IDC_CARD_DISCONNECT)->EnableWindow(TRUE);	
	(CButton *)GetDlgItem(IDC_CONNECT_RFID)->EnableWindow(FALSE); 
	(CButton *)GetDlgItem(IDC_TRANSMIT_MIFARECARD)->EnableWindow(FALSE);
//	(CButton *)GetDlgItem(IDC_GETMAGDATA)->EnableWindow(FALSE);
	
	m_ctrl_TRACK1.SetWindowText("");
	m_ctrl_TRACK2.SetWindowText("");
	m_ctrl_TRACK3.SetWindowText("");

	SetTimer(1,200,0);

}
*/
void CPCSCSampleDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnTimer(nIDEvent);
/*
	char temp[256] = {0};
	int	i=0;
	int	j=0;

	KillTimer(1);
	if ( !PCSC_MagCard_Connect(pscmc) )
	{
		SetTimer(1,200,0);
	}
	else
	{
		if(pscmc->brRecvBuf[1] != 0x6A)	 //Other card
			SetTimer(1,200,0);
		else	//Magnetic Card ATR=3B 6A 00 00 AA AA AA AA AA AA AA AA AA
		{
			// pscmc->ret == SCARD_S_SUCCESS
			//PCSC_ByteToOutFormat(pscmc->brRecvBuf, pscmc->dwRecvLen, pscmc->szOutFormat);
			//_tcscat( pscmc->szMsg, _T("\r\nATR = ") ); 
			//_tcscat( pscmc->szMsg, pscmc->szOutFormat ); 
			
			//m_ctrl_OUT.SetWindowText(pscmc->szMsg);

  			(CButton *)GetDlgItem(IDC_CONNECT_SC)->EnableWindow(TRUE);
  			(CButton *)GetDlgItem(IDC_TRANSMIT_SC)->EnableWindow(FALSE);          
			(CButton *)GetDlgItem(IDC_CONNECT_MC)->EnableWindow(TRUE);   
   			(CButton *)GetDlgItem(IDC_CARD_DISCONNECT)->EnableWindow(FALSE);	
			(CButton *)GetDlgItem(IDC_CONNECT_RFID)->EnableWindow(TRUE); 
			(CButton *)GetDlgItem(IDC_TRANSMIT_MIFARECARD)->EnableWindow(FALSE);
//			(CButton *)GetDlgItem(IDC_GETMAGDATA)->EnableWindow(FALSE);

			PCSC_AToSendBuf(SIN_ReadMagneticCardData, pscmc);
			if ( !PCSC_MagCard_GetTrackData(pscmc) )
			{
				m_ctrl_OUT.SetWindowText( PCSC_GetLastError(pscmc) );		
				return;
			}
			// pscmc->ret == SCARD_S_SUCCESS
//			memset( temp, 0, 256 );
			memset( pscmc->TrackData[0], 0, MAX_TRACK_DATA );
			memset( pscmc->TrackData[1], 0, MAX_TRACK_DATA );
			memset( pscmc->TrackData[2], 0, MAX_TRACK_DATA );
			//for(i=0;i<(int)pscmc->dwRecvLen;i++)
			//{
			//	temp[i] = pscmc->brRecvBuf[i];
			//}
			//temp[i] = '\0';
			
			//i=0;
			if(pscmc->brRecvBuf[1] == 0x4E)
			{
				m_ctrl_TRACK1.SetWindowText("Empty");
				i+=3;
			}
			else if(pscmc->brRecvBuf[1] == 0x46)
			{
				m_ctrl_TRACK1.SetWindowText("Error");
				i+=3;
			}
			else
			{
				for ( i=0; pscmc->brRecvBuf[i+1]!=0x0A; i++)
				{
					pscmc->TrackData[0][i] = pscmc->brRecvBuf[i+1];
				}
				pscmc->TrackData[0][i-1] = '\0';
				m_ctrl_TRACK1.SetWindowText((char *)pscmc->TrackData[0]);
				i+=2;
			}

			if(pscmc->brRecvBuf[i] == 0x4E)
			{
				m_ctrl_TRACK2.SetWindowText("Empty");
				i+=2;
			}
			else if(pscmc->brRecvBuf[i] == 0x46)
			{
				m_ctrl_TRACK2.SetWindowText("Error");
				i+=2;
			}
			else
			{
				for ( j=0; pscmc->brRecvBuf[i]!=0x0A; j++)
				{
					pscmc->TrackData[1][j] = pscmc->brRecvBuf[i++];
				}
				pscmc->TrackData[1][j-1] = '\0';
				m_ctrl_TRACK2.SetWindowText((char *)pscmc->TrackData[1]);
				i+=1;
			}
			//char Temp[2] = {0};
			//_stprintf( Temp, _T("%.2x "), temp[i] );
			//AfxMessageBox(Temp);
			if(pscmc->brRecvBuf[i] == 0x4E)			m_ctrl_TRACK3.SetWindowText("Empty");
			else if(pscmc->brRecvBuf[i] == 0x46)	m_ctrl_TRACK3.SetWindowText("Error");
			else
			{
				for ( j=0; pscmc->brRecvBuf[i]!=0x03; j++)
				{
					pscmc->TrackData[2][j] = pscmc->brRecvBuf[i++];
				}
				pscmc->TrackData[2][j-1] = '\0';
				m_ctrl_TRACK3.SetWindowText((char *)pscmc->TrackData[2]);
			}
			
			PCSC_MagCard_Disconnect(pscmc);
			if (pscmc->ret != SCARD_S_SUCCESS)
				m_ctrl_OUT.SetWindowText( PCSC_GetLastError(pscmc) );
			else
				m_ctrl_OUT.SetWindowText( _T("Card Disconnect ok!") );
		}	
	}
*/
}
