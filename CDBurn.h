// CDBurn.h : main header file for the CDBURN application
//

#if !defined(AFX_CDBURN_H__FA68E6D2_6EEE_4F63_9050_CE4D96051895__INCLUDED_)
#define AFX_CDBURN_H__FA68E6D2_6EEE_4F63_9050_CE4D96051895__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "DiscRecorder.h"
#include "DiscMaster.h"
#include "DiscFormatData.h"
#include "DiscFormatDataEvent.h"
#include "fileobject.h"

#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////
// CCDBurnApp:
// See CDBurn.cpp for the implementation of this class
//

class CCDBurnApp : public CWinApp
{
public : 
// 	CList<CDiscFormatData* , CDiscFormatData*> m_listDiscFormatDate; 
	CList<CDiscRecorder * , CDiscRecorder* > m_listDiscRecorder;
	CDiscRecorder *m_pCurDiscRecorder; 
	CFile  m_cLogFile;
	BOOL m_bPopupDrivers;
	BOOL m_bLogFile;	//是否需要写入日记
	CString m_cstrCurDriver;
	CString	m_cstrFilePath;
	ULONGLONG m_ullFree;


public:
	BOOL RecorderFile(CString FileName, CString CurRecorderName);
	BOOL InitRecorder();
	BOOL ReadDisc();
	CCDBurnApp();
	~CCDBurnApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDBurnApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDBURN_H__FA68E6D2_6EEE_4F63_9050_CE4D96051895__INCLUDED_)
