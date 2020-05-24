
// DotsDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Dots.h"
#include "DotsDlg.h"
//#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDotsDlg dialog



CDotsDlg::CDotsDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DOTS_DIALOG, pParent), cpColorPicker(this)
	, bMT(FALSE)
	, bSIMD(FALSE)
	, bGrid(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDotsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SIZE, cmbSize);
	DDX_Control(pDX, IDC_COMBO_DOTS, cmbDots);
	DDX_Control(pDX, IDC_COMBO_G, cmbG);
	DDX_Control(pDX, IDC_COMBO_AMOUNT, cmbAmount);
	DDX_Control(pDX, IDC_BUTTON_RUN, btnRun);
	DDX_Control(pDX, IDC_COMBO_AMOUNT2, cmbAmout2);
	DDX_Control(pDX, IDC_COMBO_GIANT, cmbGiant);
	DDX_Control(pDX, IDC_COMBO_BLACKHOLE, cmbBlackHole);
	DDX_Control(pDX, IDC_COMBO_BLACKHOLE2, cmbBlackHole2);
	DDX_Control(pDX, IDC_COMBO_GIANT2, cmbGiant2);
	DDX_Check(pDX, IDC_CHECK_MULTITHREADS, bMT);
	DDX_Check(pDX, IDC_CHECK_SIMD, bSIMD);
	DDX_Check(pDX, IDC_CHECK_GRID, bGrid);
}

BEGIN_MESSAGE_MAP(CDotsDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_COLOR, &CDotsDlg::OnClickedButtonColor)
//	ON_CBN_SELCHANGE(IDC_COMBO_DOTS, &CDotsDlg::OnSelchangeComboDots)
//	ON_BN_CLICKED(IDC_BUTTON_GENERATE, &CDotsDlg::OnClickedButtonGenerate)
	ON_MESSAGE(WM_USER_SELECT_COLOR, &CDotsDlg::OnUserSelectColor)
	ON_BN_CLICKED(IDC_BUTTON_RUN, &CDotsDlg::OnClickedButtonRun)
	ON_BN_CLICKED(IDC_CHECK_GRID, &CDotsDlg::OnClickedCheckGrid)
END_MESSAGE_MAP()


// CDotsDlg message handlers

BOOL CDotsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	cmbSize.SetCurSel(2);
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(8, 8, 8, 8);
	if (!swShow.Create(::AfxRegisterWndClass(CS_OWNDC), nullptr, WS_CHILD, rect, this, 1))
		::AfxMessageBox(_T("Create d2d show window failed!"));
	if (::IsWindow(swShow) && ::IsWindow(cmbG))
	{
		CRect rect, rc1;
		GetClientRect(&rect);
		cmbG.GetWindowRect(&rc1);
		ScreenToClient(&rc1);
		rect.left += 8;
		rect.top = rc1.bottom + rc1.top;//rc1.top;
		rect.right -= 8;
		rect.bottom -= 8;
		swShow.MoveWindow(&rect);
	}
	swShow.ShowWindow(SW_SHOW);

	cmbG.SetCurSel(8);
	cmbAmount.SetCurSel(6);
	cmbGiant.SetCurSel(0);
	cmbBlackHole.SetCurSel(0);
	cmbAmout2.SetCurSel(0);
	cmbGiant2.SetCurSel(0);
	cmbBlackHole2.SetCurSel(0);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDotsDlg::OnPaint()
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
HCURSOR CDotsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CDotsDlg::OnClickedButtonColor()
{
	CPoint point;
	::GetCursorPos(&point);
	cpColorPicker.ShowAt(point, clrCurrent);

}

/*
void CDotsDlg::OnSelchangeComboDots()
{
	swShow.SetCurrBmp(cmbDots.GetCurSel());
}
*/

/*
void CDotsDlg::OnClickedButtonGenerate()
{
	int ns = cmbSize.GetCurSel();
	ns = ns < 0 ? 0 : ns;
	ns = ns * 2 + 3;	// 0: 3, 1: 5, 2: 7, 3: 9...
	if (!swShow.Generate(ns, clrCurrent))
		::AfxMessageBox(_T("Already existed!"));
	else
	{
		CString str;
		str.Format(_T("Size: %d, Color: %8x"), ns, clrCurrent);
		cmbDots.SetCurSel(cmbDots.AddString(str));
		OnSelchangeComboDots();
	}
}
*/

afx_msg LRESULT CDotsDlg::OnUserSelectColor(WPARAM wParam, LPARAM lParam)
{
	if (wParam)
	{
		clrCurrent = COLORREF(lParam);
	}
	return 0;
}


//----------------------------------------------------------------------------------------------------------------------
void CDotsDlg::OnClickedButtonRun()
{
	UpdateData();
	int nG = 1 << cmbG.GetCurSel();
	int nC = cmbAmount.GetCurSel();
	if (nC > 0)
		nC = 2 << (nC - 1);
	int ngs = cmbGiant.GetCurSel();
	int nbh = cmbBlackHole.GetCurSel();
	int nC2 = cmbAmout2.GetCurSel();
	if (nC2 > 0)
		nC2 = 1 << (nC2 - 1);
	int ngs2 = cmbGiant2.GetCurSel();
	int nbh2 = cmbBlackHole2.GetCurSel();
	
	int nTotal = nC + ngs + nbh + nC2 + ngs2 + nbh2;
	if (nTotal < 1)
	{
		::AfxMessageBox(_T("No star!"));
		return;
	}
	bRunning = !bRunning;
	if (bRunning)
		btnRun.SetWindowText(_T("Stop"));
	else
		btnRun.SetWindowText(_T("Run"));
	if (bRunning)
	{
		int ntc = GetThreadCount();
		if (bGrid)
		{
			if (nC + nC2 < 512)
			{
				bGrid = FALSE;
				UpdateData(FALSE);
				::AfxMessageBox(_T("Using grid is unnecessary£¡"));
			}
			if (bGrid)
			{
				swShow.InitWorldRaw(ntc, nG, nC, ngs, nbh, nC2, ngs2, nbh2);
				swShow.RunGrid();
			}
			else
			{
				swShow.InitWorldRaw(ntc, nG, nC, ngs, nbh, nC2, ngs2, nbh2);
				swShow.Run();
			}
		}
		else
		{
			SThreadParams::bUsingSIMD = bSIMD;
			if (bMT)
			{
				if (ntc < 2)
				{
					bMT = FALSE;
					UpdateData(FALSE);
					::AfxMessageBox(_T("Mulit threads can not run on single core CPU£¡"));
				}
				else if (nC + nC2 < 256)
				{
					bMT = FALSE;
					UpdateData(FALSE);
					::AfxMessageBox(_T("Using multi threads is unnecessary£¡"));
				}
			}
			if (bMT)
			{
				swShow.InitWorldRaw(ntc, nG, nC, ngs, nbh, nC2, ngs2, nbh2);
				swShow.RunMultiThread(ntc);
			}
			else
			{
				//swShow.InitWorld(nG, nC, ngs, nbh, nC2, ngs2, nbh2);
				swShow.InitWorldRaw(ntc, nG, nC, ngs, nbh, nC2, ngs2, nbh2);
				swShow.Run();
			}
		}
	}
	else
	{
		swShow.Stop();
	}
}


//----------------------------------------------------------------------------------------------------------------------
int CDotsDlg::GetThreadCount(void) const
{
	SYSTEM_INFO si;
	::GetSystemInfo(&si);
	int ntc = (int)si.dwNumberOfProcessors;
	if (ntc > MAX_THREADS_NUMBER)
		ntc = MAX_THREADS_NUMBER;
	return ntc;
}


//----------------------------------------------------------------------------------------------------------------------
void CDotsDlg::OnClickedCheckGrid()
{
	UpdateData();
	if (bGrid)
	{
		if (bSIMD || bMT)
		{
			::AfxMessageBox(_T("Grid mode not support SIMD and Multi threads!"));
			bSIMD = FALSE;
			bMT = FALSE;
			UpdateData(FALSE);
		}
		GetDlgItem(IDC_CHECK_SIMD)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_MULTITHREADS)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_SIMD)->EnableWindow();
		GetDlgItem(IDC_CHECK_MULTITHREADS)->EnableWindow();
	}
}


