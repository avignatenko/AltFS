
// AltFS.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CAltFSApp:
// See AltFS.cpp for the implementation of this class
//

class CAltFSApp : public CWinApp
{
public:
	CAltFSApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

    afx_msg void OnCustomMessage(WPARAM wParam, LPARAM lParam);

};

extern CAltFSApp theApp;
