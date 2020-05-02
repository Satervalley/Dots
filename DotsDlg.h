
// DotsDlg.h : header file
//

#pragma once

#include "CColorPickingWnd.h"
#include "CShowWnd.h"


// CDotsDlg dialog
class CDotsDlg : public CDialog
{
// Construction
public:
	CDotsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DOTS_DIALOG };
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
	afx_msg void OnClickedButtonColor();
	afx_msg void OnSelchangeComboDots();
	afx_msg void OnClickedButtonGenerate();
	afx_msg LRESULT OnUserSelectColor(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClickedButtonRun();
	DECLARE_MESSAGE_MAP()

protected:
	CComboBox cmbSize;
	CComboBox cmbDots;
	CColorPickingWnd cpColorPicker;
	COLORREF clrCurrent{ RGB(255, 255, 255) };
	CComboBox cmbG;
	CComboBox cmbAmount;
	CButton btnRun;
	CComboBox cmbAmout2;
	CComboBox cmbGiant;
	CComboBox cmbBlackHole;
	CComboBox cmbBlackHole2;
	CComboBox cmbGiant2;
	CShowWnd swShow;

	bool bRunning{ false };
	BOOL bMT;

	int GetThreadCount(void) const;
	BOOL bSIMD;
	BOOL bGrid;
public:
	afx_msg void OnClickedCheckGrid();
};


