
// AltFS.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AltFS.h"
#include "AltFSDlg.h"
#include "FSUIPCEngine.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAltFSApp

BEGIN_MESSAGE_MAP(CAltFSApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAltFSApp construction

CAltFSApp::CAltFSApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CAltFSApp object

CAltFSApp theApp;




BOOL CAltFSApp::InitInstance()
{
    // get filename of the executable
    CString path;
    GetModuleFileName(NULL, path.GetBuffer(MAX_PATH), MAX_PATH);
    path.ReleaseBuffer();

    std::filesystem::path exeName = static_cast<LPCSTR>(path);
    std::filesystem::path exePath = exeName.parent_path();

    // Set the default logger to file logger
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    
    
    auto logFilename = exePath / "altfs.log";
    auto file_logger = spdlog::basic_logger_mt("basic_logger", logFilename.string());
    spdlog::set_default_logger(file_logger);

    spdlog::info("AltFs started");

    // InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

    // register FS98MAIN as classname for the dialog
    WNDCLASS wc;

  	// Get the info for this class.
           // #32770 is the default class name for dialogs boxes.
  	::GetClassInfo(AfxGetInstanceHandle(), "#32770", &wc);

  	// Change the name of the class.
  	wc.lpszClassName = "FS98MAIN";

  	// Register this class so that MFC can use it.
  	AfxRegisterClass(&wc);

  

	// Standard initialization
	SetRegistryKey(_T("Alexey Ignatenko"));

    FSUIPCEngine engine((exePath / "lua" / "script.lua").string());
	CAltFSDlg dlg(engine);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

