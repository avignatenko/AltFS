
// AltFSDlg.h : header file
//

#pragma once

#define XC_CALL_CONNECTION			"FSASMLIB:IPC"

class FSUIPCEngine;

// CAltFSDlg dialog
class CAltFSDlg : public CDialog
{
    // Construction
public:
    CAltFSDlg(FSUIPCEngine& engine, CWnd* pParent = nullptr);	// standard constructor
    ~CAltFSDlg()
    {
    }
// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ALTFS_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

private:

    LRESULT OnFDSCall(WPARAM wParam, LPARAM lParam);
    FSUIPCEngine& m_engine;
};
