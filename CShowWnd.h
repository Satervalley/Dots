#pragma once

#include <list>
#include "CDotBmp.h"
#include "CWorld.h"
#include "CRandomGen.h"


struct SDotInfo
{
	CPoint point;
	int nWhitch;
};

typedef std::list<SDotInfo> DotInfoList;


// CShowWnd

class CShowWnd : public CWnd
{
	DECLARE_DYNAMIC(CShowWnd)

public:
	CWorld* pWorld;

	CShowWnd();
	virtual ~CShowWnd();

	bool Generate(int ns, COLORREF clr);
	void SetCurrBmp(int ni) { nCurrBmp = ni; }
	void InitWorld(int nG, int nc, int ngs = 0, int nbh = 0, int nc2 = 0, int ngs2 = 0, int nbh2 = 0);
	void InitWorldRaw(int ntc, int nG, int nc, int ngs = 0, int nbh = 0, int nc2 = 0, int ngs2 = 0, int nbh2 = 0);
	void Run(void);
	void Stop(void);
	void RunMultiThread(int nThreadCount);
	void StopMultiThread(void);
	void RunGrid(void);
protected:
	DECLARE_MESSAGE_MAP()

	DotInfoList dilDots;
	DotBmpPointerVector dbpvBmps; 
	DotBmpPointerVector bmpBodys;

	int nCurrBmp{ -1 };
	int nAmount;
	bool bRunning{ false };

	DotBmpPointer GenDot(int ns, COLORREF clr, bool bLoose, EDotType dt = EDotType::dtNormal);
	static COLORREF RandomColor(CRandomGen& rg);
	static COLORREF RandomColor(CRandomGen& rg, int cons[6]);
	static COLORREF RandomColorBlue(CRandomGen& rg);
	static COLORREF RandomColorGreen(CRandomGen& rg);
	static COLORREF RandomColorRed(CRandomGen& rg);
	static COLORREF RandomColorPurple(CRandomGen& rg);
	static COLORREF RandomColorYellow(CRandomGen& rg);
	static COLORREF RandomColorWhite(CRandomGen& rg);
	static COLORREF RandomNormalColor(CRandomGen& rg);
	static COLORREF RandomAntiGColor(CRandomGen& rg);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnDraw2d(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserDrawWorld(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserDrawWorld2(WPARAM wParam, LPARAM lParam);
};


//----------------------------------------------------------------------------------------------------------------------
UINT __cdecl WorldRunFunction(LPVOID pParam);

UINT __cdecl WorldRunFunctionWorker(LPVOID pParam);
UINT __cdecl WorldRunFunctionControl(LPVOID pParam);
UINT __cdecl RenderFunction(LPVOID pParam);
