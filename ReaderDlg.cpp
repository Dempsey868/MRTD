// ReaderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCSCSample.h"
#include "ReaderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define READERNAME_LENGTH	128
/////////////////////////////////////////////////////////////////////////////
// CReaderDlg dialog

CReaderDlg::CReaderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReaderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReaderDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CReaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReaderDlg)
	DDX_Control(pDX, IDC_READERS, m_ctrl_Reader);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReaderDlg, CDialog)
	//{{AFX_MSG_MAP(CReaderDlg)
	ON_CBN_SELCHANGE(IDC_READERS, OnSelchangeReaders)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReaderDlg message handlers

BOOL CReaderDlg::OnInitDialog() 
{

	CDialog::OnInitDialog();
	
	m_ctrl_Reader.ResetContent();
	for (DWORD i=0; i<dwReaderCount; i++)
		m_ctrl_Reader.InsertString(i, pmszReaderName+(i* sizeof(TCHAR)*READERNAME_LENGTH));	

	dwActReader = 0;
	m_ctrl_Reader.SetCurSel(0);	                            

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CReaderDlg::OnSelchangeReaders() 
{
	dwActReader = m_ctrl_Reader.GetCurSel();
	
}
