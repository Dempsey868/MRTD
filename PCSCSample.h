// PCSCSample.h : main header file for the PCSCSAMPLE application
//

#if !defined(AFX_PCSCSAMPLE_H__BD5CA6A6_9EA3_11D3_8C32_0080AD2E8837__INCLUDED_)
#define AFX_PCSCSAMPLE_H__BD5CA6A6_9EA3_11D3_8C32_0080AD2E8837__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPCSCSampleApp:
// See PCSCSample.cpp for the implementation of this class
//

class CPCSCSampleApp : public CWinApp
{
public:
	CPCSCSampleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPCSCSampleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPCSCSampleApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PCSCSAMPLE_H__BD5CA6A6_9EA3_11D3_8C32_0080AD2E8837__INCLUDED_)
