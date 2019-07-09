
// AltFSDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AltFS.h"
#include "AltFSDlg.h"
#include "afxdialogex.h"
#include "FSUIPCEngine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CAltFSDlg dialog



CAltFSDlg::CAltFSDlg(FSUIPCEngine& engine, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_ALTFS_DIALOG, pParent)
    , m_engine(engine)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAltFSDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

// register our message
static UINT    XC_CALL = RegisterWindowMessage(XC_CALL_CONNECTION);

BEGIN_MESSAGE_MAP(CAltFSDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_REGISTERED_MESSAGE(XC_CALL, OnFDSCall)
END_MESSAGE_MAP()


// CAltFSDlg message handlers

BOOL CAltFSDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon




    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAltFSDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAltFSDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CAltFSDlg::OnFDSCall(WPARAM wParam, LPARAM lParam)
{
    return m_engine.processMessage(wParam, lParam);
}