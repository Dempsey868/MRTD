// PCSCSampleDlg.h : header file
//

#if !defined(AFX_PCSCSAMPLEDLG_H__BD5CA6A8_9EA3_11D3_8C32_0080AD2E8837__INCLUDED_)
#define AFX_PCSCSAMPLEDLG_H__BD5CA6A8_9EA3_11D3_8C32_0080AD2E8837__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPCSCSampleDlg dialog

#include "SCMC2.h"

class CPCSCSampleDlg : public CDialog
{
// Construction
public:
	CPCSCSampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CPCSCSampleDlg)
	enum { IDD = IDD_PCSCSAMPLE_DIALOG };
	CStatic	m_Ctrl_ReaderName;
	//CEdit	m_ctrl_TRACK3;
	//CEdit	m_ctrl_TRACK2;
	//CEdit	m_ctrl_TRACK1;
	CEdit	m_ctrl_OUT;
	CComboBox	m_ctrl_IN;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPCSCSampleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON			m_hIcon;

//-- Add your PCSC variables ---------------------------------------
	LPPCSC_SCMC		pscmc;	


	// Your user-defined Window Message handler function
	afx_msg VOID OnCardInserted(WPARAM wParam, LPARAM lParam); 
	afx_msg VOID OnCardRemoved(WPARAM wParam, LPARAM lParam); 

	// Generated message map functions
	//{{AFX_MSG(CPCSCSampleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnCardDisconnect();
	afx_msg void OnConnectSc();
	afx_msg void OnTransmitSc();
	afx_msg void OnDestroy();
	afx_msg void OnConnectRfid();
	afx_msg void OnTransmitMifarecard();
	//afx_msg void OnConnectMc();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PCSCSAMPLEDLG_H__BD5CA6A8_9EA3_11D3_8C32_0080AD2E8837__INCLUDED_)
