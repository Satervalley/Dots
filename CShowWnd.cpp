// CShowWnd.cpp : implementation file
//

#include "pch.h"
#include "Dots.h"
#include "CShowWnd.h"
#include "CExplosion.h"

#ifdef USING_AVX
#include <intrin.h>
#endif // USING_AVX



// CShowWnd

IMPLEMENT_DYNAMIC(CShowWnd, CWnd)


//----------------------------------------------------------------------------------------------------------------------
CShowWnd::CShowWnd()
{
	pWorld = new CWorld;
}


//----------------------------------------------------------------------------------------------------------------------
CShowWnd::~CShowWnd()
{
	delete pWorld;
}


BEGIN_MESSAGE_MAP(CShowWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_REGISTERED_MESSAGE(AFX_WM_DRAW2D, &CShowWnd::OnDraw2d)
	ON_MESSAGE(WM_USER_DRAW_WORLD, &CShowWnd::OnUserDrawWorld)
	ON_MESSAGE(WM_USER_DRAW_WORLD2, &CShowWnd::OnUserDrawWorld2)
END_MESSAGE_MAP()



//----------------------------------------------------------------------------------------------------------------------
// CShowWnd message handlers
int CShowWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	SThreadParams::hwndTarget = m_hWnd;
	EnableD2DSupport();

	return 0;
}


//----------------------------------------------------------------------------------------------------------------------
void CShowWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!bRunning)
	{
		InitWorldRaw(1, 100, 100);
		pWorld->rdRawData.faPosX[0] = point.x;
		pWorld->rdRawData.faPosY[0] = point.y;
		pWorld->rdRawData.caColor[0] = RGB(100, 100, 100);
		CExplosion exp(0, pWorld->rdRawData, 100.f, 1.f);
		CRenderTarget* prt = GetRenderTarget();
		bool b;
		for (;;)
		{
			b = exp.Pass(pWorld->rdRawData, 0.0167f);
			if (!b)
				break;
			prt->BeginDraw();
			prt->Clear(D2D1::ColorF(D2D1::ColorF::Black));
			exp.Draw(prt);
			prt->EndDraw();
			::Sleep(15);
		}
	}
	/*
	if (nCurrBmp >= 0 && nCurrBmp < (int)dbpvBmps.size())
	{
		SDotInfo si;
		si.point = point;
		si.nWhitch = nCurrBmp;
		dilDots.push_back(si);
		Invalidate();
		UpdateWindow();
	}
	*/
	CWnd::OnLButtonDown(nFlags, point);
}


//----------------------------------------------------------------------------------------------------------------------
DotBmpPointer CShowWnd::GenDot(int ns, COLORREF clr, bool bLoose, EDotType dt)
{
	return std::make_shared<CDotBmp>(ns, clr, GetRenderTarget(), bLoose, dt);
}


//----------------------------------------------------------------------------------------------------------------------
bool CShowWnd::Generate(int ns, COLORREF clr)
{
	bool bRes = false, bFind = false;
	for (auto dbp : dbpvBmps)
	{
		if (dbp->Same(ns, clr))
		{
			bFind = true;
			break;
		}
	}
	if (!bFind)
	{
		auto p = GenDot(ns, clr, false);
		dbpvBmps.push_back(p);
		bRes = true;
	}
	return bRes;
}


//----------------------------------------------------------------------------------------------------------------------
afx_msg LRESULT CShowWnd::OnDraw2d(WPARAM wParam, LPARAM lParam)
{
	if (bRunning)
		return 0;

	CHwndRenderTarget* pRender = (CHwndRenderTarget*)lParam;
	pRender->SetDpi(CD2DSizeF(96., 96.));
	pRender->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	for (auto di : dilDots)
	{
		int nd = dbpvBmps[di.nWhitch]->Size() / 2;
		CD2DRectF rect(float(di.point.x - nd), float(di.point.y - nd), float(di.point.x + nd + 1), float(di.point.y + nd + 1));
		pRender->DrawBitmap(*dbpvBmps[di.nWhitch], rect);
	}
	return 0;
}


//----------------------------------------------------------------------------------------------------------------------
afx_msg LRESULT CShowWnd::OnUserDrawWorld(WPARAM wParam, LPARAM lParam)
{
/*
	CHwndRenderTarget* pRender = GetRenderTarget();
	if (!pRender)
		return 0;
	pRender->BeginDraw();
	pRender->SetDpi(CD2DSizeF(96., 96.));
	pRender->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	if (bRunning)
	{
		SBody* pbs = pWorld->Bodys();
		SBody* pb;
		CD2DRectF rect;
		for (int i = 0; i < nAmount; i++)
		{
			pb = &pbs[i];
			rect.left = pb->pos.x - pb->nRadius;
			rect.top = pb->pos.y - pb->nRadius;
			rect.right = pb->pos.x + pb->nRadius + 1.f;
			rect.bottom = pb->pos.y + pb->nRadius + 1.f;
			pRender->DrawBitmap(*bmpBodys[i], rect);
		}
		rect.left = rect.top = 4.f;
		rect.right = 300.f;
		rect.bottom = 100.f;
		CD2DSolidColorBrush brush(pRender, D2D1::ColorF::White);
		pRender->DrawText(pWorld->strStat, rect, &brush);
	}
	pRender->EndDraw();
*/
	return 0;
}


//----------------------------------------------------------------------------------------------------------------------
afx_msg LRESULT CShowWnd::OnUserDrawWorld2(WPARAM wParam, LPARAM lParam)
{
//	static bool bo = false;


	CHwndRenderTarget* pRender = GetRenderTarget();
	if (!pRender)
		return 0;
	pRender->BeginDraw();
	pRender->SetDpi(CD2DSizeF(96.f, 96.f));
	pRender->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	if (bRunning)
	{
		SRawData& rrd = pWorld->rdRawData;
		
		CD2DRectF rect;
		for (int i = 0; i < nAmount; i++)
		{
			rect.left = rrd.faPosX[i] - rrd.naRadius[i];
			rect.top = rrd.faPosY[i] - rrd.naRadius[i];
			rect.right = rrd.faPosX[i] + rrd.naRadius[i] + 1.f;
			rect.bottom = rrd.faPosY[i] + rrd.naRadius[i] + 1.f;
			if (rrd.pEffects[i])
			{
				CEffect* pe = reinterpret_cast<CEffect*>(rrd.pEffects[i]);
				switch (pe->DrawType())
				{
				case EDrawType::dtNone:
					break;
				case EDrawType::dtBefore:
					pe->Draw(pRender);
					pRender->DrawBitmap(*bmpBodys[i], rect);
					break;
				case EDrawType::dtAfter:
					pRender->DrawBitmap(*bmpBodys[i], rect);
					pe->Draw(pRender);
					break;
				case EDrawType::dtReplace:
					pe->Draw(pRender);
					break;
				default:
					break;
				}
			}
			else
				pRender->DrawBitmap(*bmpBodys[i], rect);
		}
		rect.left = rect.top = 4.f;
		rect.right = 300.f;
		rect.bottom = 100.f;
		CD2DSolidColorBrush brush(pRender, D2D1::ColorF::YellowGreen);
		pRender->DrawText(SThreadParams::strStat, rect, &brush);

		//if (!bo)
		//{
		//	bo = true;
		//	rrd.Output(L"mt1.txt", nAmount);
		//}

	}
	pRender->EndDraw();
	
	return 0;
}


//----------------------------------------------------------------------------------------------------------------------
COLORREF CShowWnd::RandomColor(CRandomGen& rg)
{
	BYTE r = rg.GenInt(256, 128), g = rg.GenInt(256, 128), b = rg.GenInt(256, 128);
	return RGB(r, g, b);
}


//----------------------------------------------------------------------------------------------------------------------
COLORREF CShowWnd::RandomColor(CRandomGen& rg, int cons[6])
{
	BYTE r = rg.GenInt(cons[0], cons[1]), g = rg.GenInt(cons[2], cons[3]), b = rg.GenInt(cons[4], cons[5]);
	return RGB(r, g, b);
}


//----------------------------------------------------------------------------------------------------------------------
COLORREF CShowWnd::RandomColorBlue(CRandomGen& rg)
{
	static int cons[6] = { 64, 0, 192, 0, 256, 192 };
	return RandomColor(rg, cons);
}


//----------------------------------------------------------------------------------------------------------------------
COLORREF CShowWnd::RandomColorGreen(CRandomGen& rg)
{
	static int cons[6] = { 64, 0, 256, 192, 192, 0 };
	return RandomColor(rg, cons);
}


//----------------------------------------------------------------------------------------------------------------------
COLORREF CShowWnd::RandomColorRed(CRandomGen& rg)
{
	static int cons[6] = { 256, 192, 128, 0, 64, 0 };
	return RandomColor(rg, cons);
}


//----------------------------------------------------------------------------------------------------------------------
COLORREF CShowWnd::RandomColorPurple(CRandomGen& rg)
{
	static int cons[6] = { 256, 192, 64, 0, 256, 192 };
	return RandomColor(rg, cons);
}


//----------------------------------------------------------------------------------------------------------------------
COLORREF CShowWnd::RandomColorYellow(CRandomGen& rg)
{
	static int cons[6] = { 256, 192, 256, 192, 64, 0 };
	return RandomColor(rg, cons);
}


//----------------------------------------------------------------------------------------------------------------------
COLORREF CShowWnd::RandomColorWhite(CRandomGen& rg)
{
	static int cons[6] = { 256, 224, 256, 224, 256, 224 };
	return RandomColor(rg, cons);
}


//----------------------------------------------------------------------------------------------------------------------
// normal color: blue, green, red, purple
COLORREF CShowWnd::RandomNormalColor(CRandomGen& rg)
{
	COLORREF clr;
	switch (rg.GenInt(4))
	{
	case 0:
		clr = RandomColorBlue(rg);
		break;
	case 1:
		clr = RandomColorGreen(rg);
		break;
	case 2:
		clr = RandomColorRed(rg);
		break;
	case 3:
		clr = RandomColorPurple(rg);
		break;
	}
	return clr;
}


//----------------------------------------------------------------------------------------------------------------------
// anti-G color: white, yellow
COLORREF CShowWnd::RandomAntiGColor(CRandomGen& rg)
{
	COLORREF clr;
	switch (rg.GenInt(2))
	{
	case 0:
		clr = RandomColorWhite(rg);
		break;
	case 1:
		clr = RandomColorYellow(rg);
		break;
	}
	return clr;
}


//----------------------------------------------------------------------------------------------------------------------
/*
void CShowWnd::InitWorld(int nG, int nc, int ngs, int nbh, int nc2, int ngs2, int nbh2)
{
	static int nDias[6] = {5, 7, 9, 11, 13, 15 };
	static int nDias_bh[4] = { 9, 11, 13, 15 };
	static int nms[7] = { 2, 4, 8, 16, 32, 64, 128 };
	static float fef[6] = { 0.90f, 0.88f, 0.86f, 0.84f, 0.82f, 0.80f };	// elastic factor
	static float fef2[6] = { 0.80f, 0.78f, 0.76f, 0.74f, 0.72f, 0.70f };	// elastic factor for loose star
	static float fef_bh[4] = { 0.68f, 0.66f, 0.64f, 0.62f }; // elastic factor for blackhole
	static float fcrf[6] = {1.0f, 0.98f, 0.96f, 0.94f, 0.92f, 0.90f}; // core radius factor
	static float fcrf2[6] = { 0.96f, 0.94f, 0.92f, 0.90f, 0.88f, 0.86f }; // core radius factor for loose star
	static float fcrf_bh[4] = {1.0f, 0.98f, 0.96f, 0.94f};	// core radius factor for black hole
	float fef_gs = 0.68f;	// elastic factor for giant star
	float fcrf_gs = 0.84f;	// core radius factor for giant star
	CRandomGen rg;
	CRect rect;
	int n, nw, nh;

	nAmount = nc + ngs + nbh + nc2 + ngs2 + nbh2;
	GetClientRect(&rect);
	nw = rect.Width();
	nh = rect.Height();
	pWorld->Init(float(nG), nw, nh, nAmount, m_hWnd);
	bmpBodys.clear();
	SBody * pb = pWorld->Bodys();
	int nCount = 0;
	for(int i = 0; i < ngs; i++)	// normal giant star
	{
		int nr = rg.GenInt(20, 15);
		pb[i].nRadius = nr;
		pb[i].fCoreRadius = float(nr) * fcrf_gs;
		pb[i].m = float(nr * nr * nr);
		pb[i].pos.x = (float)rg.GenInt(nw - nw / 4, nw / 4);
		pb[i].pos.y = (float)rg.GenInt(nh - nh / 4, nh / 4);
		pb[i].velocity.x = (float)rg.GenInt(10, -10);
		pb[i].velocity.y = (float)rg.GenInt(10, -10);
		pb[i].fElasticFactor = fef_gs;
		pb[i].fAnti = 1.0f;
		bmpBodys.push_back(GenDot(nr * 2 + 1, RandomNormalColor(rg), false));
	}
	nCount += ngs;
	for (int i = 0; i < ngs2; i++)	// anti-G giant star
	{
		int nr = rg.GenInt(20, 15);
		pb[i + nCount].nRadius = nr;
		pb[i + nCount].fCoreRadius = float(nr) * fcrf_gs;
		pb[i + nCount].m = float(nr * nr * nr);
		pb[i + nCount].pos.x = (float)rg.GenInt(nw - nw / 4, nw / 4);
		pb[i + nCount].pos.y = (float)rg.GenInt(nh - nh / 4, nh / 4);
		pb[i + nCount].velocity.x = (float)rg.GenInt(10, -10);
		pb[i + nCount].velocity.y = (float)rg.GenInt(10, -10);
		pb[i + nCount].fElasticFactor = fef_gs;
		pb[i + nCount].fAnti = -1.0f;
		bmpBodys.push_back(GenDot(nr * 2 + 1, RandomAntiGColor(rg), false));
	}
	nCount += ngs2;
	for (int i = 0; i < nbh; i++)	// normal black hole
	{
		n = rg.GenInt(4);
		int nd = nDias_bh[n];
		pb[i + nCount].nRadius = nd / 2;
		pb[i + nCount].fCoreRadius = float(nd / 2) * fcrf_bh[n];
		pb[i + nCount].m = float(nd * nd * nd * nd / 2);
		pb[i + nCount].pos.x = (float)rg.GenInt(nw - nw / 4, nw / 4);
		pb[i + nCount].pos.y = (float)rg.GenInt(nh - nh / 4, nh / 4);
		pb[i + nCount].velocity.x = (float)rg.GenInt(10, -10);
		pb[i + nCount].velocity.y = (float)rg.GenInt(10, -10);
		pb[i + nCount].fElasticFactor = fef_bh[n];
		pb[i + nCount].fAnti = 1.0f;
		bmpBodys.push_back(GenDot(nd, RandomNormalColor(rg), true, EDotType::dtBlackHole));
	}
	nCount += nbh;
	for (int i = 0; i < nbh2; i++)	// anti-G black hole
	{
		n = rg.GenInt(4);
		int nd = nDias_bh[n];
		pb[i + nCount].nRadius = nd / 2;
		pb[i + nCount].fCoreRadius = float(nd / 2) * fcrf_bh[n];
		pb[i + nCount].m = float(nd * nd * nd * nd / 2);
		pb[i + nCount].pos.x = (float)rg.GenInt(nw - nw / 4, nw / 4);
		pb[i + nCount].pos.y = (float)rg.GenInt(nh - nh / 4, nh / 4);
		pb[i + nCount].velocity.x = (float)rg.GenInt(10, -10);
		pb[i + nCount].velocity.y = (float)rg.GenInt(10, -10);
		pb[i + nCount].fElasticFactor = fef_bh[n];
		pb[i + nCount].fAnti = -1.0f;
		bmpBodys.push_back(GenDot(nd, RandomAntiGColor(rg), true, EDotType::dtBlackHole));
	}
	nCount += nbh2;
	for (int i = 0; i < nc; i++)	// normal common star
	{
		bool bLoose = rg.GenBool();
		n = rg.GenInt(6);
		pb[i + nCount].nRadius = nDias[n] / 2;
		if(bLoose)
			pb[i + nCount].fCoreRadius = float(pb[i + nCount].nRadius) * fcrf2[n];
		else
			pb[i + nCount].fCoreRadius = float(pb[i + nCount].nRadius) * fcrf[n];
		pb[i + nCount].m = float(rg.GenInt(nms[n + 1], nms[n]));
		if (bLoose)
			pb[i + nCount].m = pb[i + nCount].m / 2;
		pb[i + nCount].pos.x = (float)rg.GenInt(nw - nw / 4, nw / 4);
		pb[i + nCount].pos.y = (float)rg.GenInt(nh - nh / 4, nh / 4);
		pb[i + nCount].velocity.x = (float)rg.GenInt(20, -20);
		pb[i + nCount].velocity.y = (float)rg.GenInt(20, -20);
		if(bLoose)
			pb[i + nCount].fElasticFactor = fef2[n];
		else
			pb[i + nCount].fElasticFactor = fef[n];
		pb[i + nCount].fAnti = 1.0f;
		bmpBodys.push_back(GenDot(nDias[n], RandomNormalColor(rg), bLoose));
	}
	nCount += nc;
	for (int i = 0; i < nc2; i++)	// anti-G common star
	{
		bool bLoose = rg.GenBool();
		n = rg.GenInt(6);
		pb[i + nCount].nRadius = nDias[n] / 2;
		if (bLoose)
			pb[i + nCount].fCoreRadius = float(pb[i + nCount].nRadius) * fcrf2[n];
		else
			pb[i + nCount].fCoreRadius = float(pb[i + nCount].nRadius) * fcrf[n];
		pb[i + nCount].m = float(rg.GenInt(nms[n + 1], nms[n]));
		if (bLoose)
			pb[i + nCount].m = pb[i + nCount].m / 2;
		pb[i + nCount].pos.x = (float)rg.GenInt(nw - nw / 4, nw / 4);
		pb[i + nCount].pos.y = (float)rg.GenInt(nh - nh / 4, nh / 4);
		pb[i + nCount].velocity.x = (float)rg.GenInt(20, -20);
		pb[i + nCount].velocity.y = (float)rg.GenInt(20, -20);
		if (bLoose)
			pb[i + nCount].fElasticFactor = fef2[n];
		else
			pb[i + nCount].fElasticFactor = fef[n];
		pb[i + nCount].fAnti = -1.0f;
		bmpBodys.push_back(GenDot(nDias[n], RandomAntiGColor(rg), bLoose));
	}
//	pWorld->Output(L"st0.txt");
}
*/


//----------------------------------------------------------------------------------------------------------------------
void CShowWnd::InitWorldRaw(int ntc, int nG, int nc, int ngs, int nbh, int nc2, int ngs2, int nbh2)
{
	static int nDias[6] = { 5, 7, 9, 11, 13, 15 };
	static int nDias_bh[4] = { 9, 11, 13, 15 };
	static int nms[7] = { 2, 4, 8, 16, 32, 64, 128 };
	static float fef[6] = { 0.90f, 0.88f, 0.86f, 0.84f, 0.82f, 0.80f };	// elastic factor
	static float fef2[6] = { 0.80f, 0.78f, 0.76f, 0.74f, 0.72f, 0.70f };	// elastic factor for loose star
	static float fef_bh[4] = { 0.68f, 0.66f, 0.64f, 0.62f }; // elastic factor for blackhole
	static float fcrf[6] = { 1.0f, 0.98f, 0.96f, 0.94f, 0.92f, 0.90f }; // core radius factor
	static float fcrf2[6] = { 0.96f, 0.94f, 0.92f, 0.90f, 0.88f, 0.86f }; // core radius factor for loose star
	static float fcrf_bh[4] = { 1.0f, 0.98f, 0.96f, 0.94f };	// core radius factor for black hole
	float fef_gs = 0.68f;	// elastic factor for giant star
	float fcrf_gs = 0.84f;	// core radius factor for giant star
	CRandomGen rg;
	CRect rect;
	int n, nw, nh;
	float fG = float(nG);

	SRawData& rrd = pWorld->rdRawData;
	nAmount = nc + ngs + nbh + nc2 + ngs2 + nbh2;
	rrd.Prepare(nAmount, ntc);
	GetClientRect(&rect);
	nw = rect.Width();
	nh = rect.Height();
	pWorld->Init(float(nG), nw, nh, nAmount, m_hWnd);
	bmpBodys.clear();
	int nCount = 0;
	COLORREF clr;
	rrd.fG = (float)nG;
	for (int i = 0; i < ngs; i++)	// normal giant star
	{
		rrd.eaStarType[i] = EStarType::stGiant;
		int nr = rg.GenInt(20, 15);
		rrd.naRadius[i] = nr;
		rrd.faCoreRadius[i] = float(nr) * fcrf_gs;
		rrd.faGm[i] = fG * float(nr * nr * nr);
		rrd.naLooseLevel[i] = 1;	// giant star not loose, normal hard
		rrd.faPosX[i] = (float)rg.GenInt(nw - nw / 4, nw / 4);
		rrd.faPosY[i] = (float)rg.GenInt(nh - nh / 4, nh / 4);
		rrd.faVelocityX[i] = (float)rg.GenInt(10, -10);
		rrd.faVelocityY[i] = (float)rg.GenInt(10, -10);
		rrd.faElasticFactor[i] = fef_gs;
		rrd.faAntiG[i] = 1.0f;
		clr = RandomNormalColor(rg);
		rrd.caColor[i] = clr;
		bmpBodys.push_back(GenDot(nr * 2 + 1, clr, false));
	}
	nCount += ngs;
	for (int i = 0; i < ngs2; i++)	// anti-G giant star
	{
		rrd.eaStarType[i + nCount] = EStarType::stAnti_Giant;
		int nr = rg.GenInt(20, 15);
		rrd.naRadius[i + nCount] = nr;
		rrd.faCoreRadius[i + nCount] = float(nr) * fcrf_gs;
		rrd.faGm[i + nCount] = fG * float(nr * nr * nr);
		rrd.naLooseLevel[i + nCount] = 1;
		rrd.faPosX[i + nCount] = (float)rg.GenInt(nw - nw / 4, nw / 4);
		rrd.faPosY[i + nCount] = (float)rg.GenInt(nh - nh / 4, nh / 4);
		rrd.faVelocityX[i + nCount] = (float)rg.GenInt(10, -10);
		rrd.faVelocityY[i + nCount] = (float)rg.GenInt(10, -10);
		rrd.faElasticFactor[i + nCount] = fef_gs;
		rrd.faAntiG[i + nCount] = -1.0f;
		clr = RandomAntiGColor(rg);
		rrd.caColor[i + nCount] = clr;
		bmpBodys.push_back(GenDot(nr * 2 + 1, clr, false));
	}
	nCount += ngs2;
	for (int i = 0; i < nbh; i++)	// normal black hole
	{
		rrd.eaStarType[i + nCount] = EStarType::stBlackhole;
		n = rg.GenInt(4);
		int nd = nDias_bh[n];
		rrd.naRadius[i + nCount] = nd / 2;
		rrd.faCoreRadius[i + nCount] = float(nd / 2) * fcrf_bh[n];
		rrd.faGm[i + nCount] = fG * float(nd * nd * nd * nd / 2);
		rrd.naLooseLevel[i + nCount] = 2;	// black hole is most hard
		rrd.faPosX[i + nCount] = (float)rg.GenInt(nw - nw / 4, nw / 4);
		rrd.faPosY[i + nCount] = (float)rg.GenInt(nh - nh / 4, nh / 4);
		rrd.faVelocityX[i + nCount] = (float)rg.GenInt(10, -10);
		rrd.faVelocityY[i + nCount] = (float)rg.GenInt(10, -10);
		rrd.faElasticFactor[i + nCount] = fef_bh[n];
		rrd.faAntiG[i + nCount] = 1.0f;
		clr = RandomNormalColor(rg);
		rrd.caColor[i + nCount] = clr;
		bmpBodys.push_back(GenDot(nd, clr, true, EDotType::dtBlackHole));
	}
	nCount += nbh;
	for (int i = 0; i < nbh2; i++)	// anti-G black hole
	{
		rrd.eaStarType[i + nCount] = EStarType::stAnti_Blackhole;
		n = rg.GenInt(4);
		int nd = nDias_bh[n];
		rrd.naRadius[i + nCount] = nd / 2;
		rrd.faCoreRadius[i + nCount] = float(nd / 2) * fcrf_bh[n];
		rrd.faGm[i + nCount] = fG * float(nd * nd * nd * nd / 2);
		rrd.naLooseLevel[i + nCount] = 2;
		rrd.faPosX[i + nCount] = (float)rg.GenInt(nw - nw / 4, nw / 4);
		rrd.faPosY[i + nCount] = (float)rg.GenInt(nh - nh / 4, nh / 4);
		rrd.faVelocityX[i + nCount] = (float)rg.GenInt(10, -10);
		rrd.faVelocityY[i + nCount] = (float)rg.GenInt(10, -10);
		rrd.faElasticFactor[i + nCount] = fef_bh[n];
		rrd.faAntiG[i + nCount] = -1.0f;
		clr = RandomAntiGColor(rg);
		rrd.caColor[i + nCount] = clr;
		bmpBodys.push_back(GenDot(nd, clr, true, EDotType::dtBlackHole));
	}
	nCount += nbh2;
	for (int i = 0; i < nc; i++)	// normal common star
	{
		rrd.eaStarType[i + nCount] = EStarType::stNormal;
		bool bLoose = rg.GenBool();
		n = rg.GenInt(6);
		rrd.naRadius[i + nCount] = nDias[n] / 2;
		if (bLoose)
			rrd.faCoreRadius[i + nCount] = float(rrd.naRadius[i + nCount]) * fcrf2[n];
		else
			rrd.faCoreRadius[i + nCount] = float(rrd.naRadius[i + nCount]) * fcrf[n];
		rrd.faGm[i + nCount] = fG * float(rg.GenInt(nms[n + 1], nms[n]));
		rrd.naLooseLevel[i + nCount] = bLoose? 0 : 1;
		if (bLoose)
			rrd.faGm[i + nCount] = rrd.faGm[i + nCount] / 2.f;
		rrd.faPosX[i + nCount] = (float)rg.GenInt(nw - nw / 4, nw / 4);
		rrd.faPosY[i + nCount] = (float)rg.GenInt(nh - nh / 4, nh / 4);
		rrd.faVelocityX[i + nCount] = (float)rg.GenInt(20, -20);
		rrd.faVelocityY[i + nCount] = (float)rg.GenInt(20, -20);
		if (bLoose)
			rrd.faElasticFactor[i + nCount] = fef2[n];
		else
			rrd.faElasticFactor[i + nCount] = fef[n];
		rrd.faAntiG[i + nCount] = 1.0f;
		clr = RandomNormalColor(rg);
		rrd.caColor[i + nCount] = clr;
		bmpBodys.push_back(GenDot(nDias[n], clr, bLoose));
	}
	nCount += nc;
	for (int i = 0; i < nc2; i++)	// anti-G common star
	{
		rrd.eaStarType[i + nCount] = EStarType::stAnti_Normal;
		bool bLoose = rg.GenBool();
		n = rg.GenInt(6);
		rrd.naRadius[i + nCount] = nDias[n] / 2;
		if (bLoose)
			rrd.faCoreRadius[i + nCount] = float(rrd.naRadius[i + nCount]) * fcrf2[n];
		else
			rrd.faCoreRadius[i + nCount] = float(rrd.naRadius[i + nCount]) * fcrf[n];
		rrd.faGm[i + nCount] = fG * float(rg.GenInt(nms[n + 1], nms[n]));
		rrd.naLooseLevel[i + nCount] = bLoose ? 0 : 1;
		if (bLoose)
			rrd.faGm[i + nCount] = rrd.faGm[i + nCount] / 2;
		rrd.faPosX[i + nCount] = (float)rg.GenInt(nw - nw / 4, nw / 4);
		rrd.faPosY[i + nCount] = (float)rg.GenInt(nh - nh / 4, nh / 4);
		rrd.faVelocityX[i + nCount] = (float)rg.GenInt(20, -20);
		rrd.faVelocityY[i + nCount] = (float)rg.GenInt(20, -20);
		if (bLoose)
			rrd.faElasticFactor[i + nCount] = fef2[n];
		else
			rrd.faElasticFactor[i + nCount] = fef[n];
		rrd.faAntiG[i + nCount] = -1.0f;
		clr = RandomAntiGColor(rg);
		rrd.caColor[i + nCount] = clr;
		bmpBodys.push_back(GenDot(nDias[n], clr, bLoose));
	}

//	rrd.Output(L"mt0.txt", nAmount);
}


//----------------------------------------------------------------------------------------------------------------------
void CShowWnd::Run(void)
{
	if (!bRunning)
	{
		bRunning = true;
		pWorld->SetStop(false);
		CRect rect;
		GetClientRect(&rect);
		SThreadParams::Init(1, &pWorld->rdRawData, rect);
		::AfxBeginThread(WorldRunFunction, pWorld);
		::SetEvent(SThreadParams::hRenderEnd);
		::AfxBeginThread(RenderFunction, nullptr);
	}
}


//----------------------------------------------------------------------------------------------------------------------
void CShowWnd::RunGrid(void)
{
	if (!bRunning)
	{
		bRunning = true;
		pWorld->SetStop(false);
		CRect rect;
		GetClientRect(&rect);
		SThreadParams::Init(1, &pWorld->rdRawData, rect);
		pWorld->PrepareGrid(rect);
		::AfxBeginThread(WorldRunFunction, pWorld);
	}
}


//----------------------------------------------------------------------------------------------------------------------
void CShowWnd::Stop(void)
{
	pWorld->SetStop();
	bRunning = false;
	::Sleep(500);
	Invalidate();
	UpdateWindow();
}


//----------------------------------------------------------------------------------------------------------------------
void CShowWnd::RunMultiThread(int nThreadCount)
{
	if (!bRunning)
	{
		bRunning = true;
		pWorld->SetStop(false);
		CRect rect;
		GetClientRect(&rect);
		SThreadParams::Init(nThreadCount, &pWorld->rdRawData, rect);
		for (int i = 1; i < 4; i++)
		{
			pWorld->tpThreadParams[i].nIndex = i;
			::AfxBeginThread(WorldRunFunctionWorker, &(pWorld->tpThreadParams[i]));
		}
		::AfxBeginThread(WorldRunFunctionControl, pWorld);
		::SetEvent(SThreadParams::hRenderEnd);
		::AfxBeginThread(RenderFunction, nullptr);
	}
}


//----------------------------------------------------------------------------------------------------------------------
void CShowWnd::StopMultiThread(void)
{
	Stop();
}


//----------------------------------------------------------------------------------------------------------------------
UINT __cdecl WorldRunFunction(LPVOID pParam)
{
	CWorld* pw = reinterpret_cast<CWorld*>(pParam);
	pw->Run();
	return 0;
}


//----------------------------------------------------------------------------------------------------------------------
UINT __cdecl WorldRunFunctionWorker(LPVOID pParam)
{
	SThreadParams* ptp = reinterpret_cast<SThreadParams*>(pParam);
	int ni = ptp->nIndex;
	int nJobFrom = SThreadParams::nJobSegs[ni], nJobTo = SThreadParams::nJobSegs[ni + 1];
	SRawData* pRawData = SThreadParams::pRawData;
	auto pPassFun = &SRawData::PassWorker;
	if (SThreadParams::bUsingSIMD)
		pPassFun = &SRawData::PassWorker_SIMD;
	for (;;)
	{
		if (SThreadParams::bStop)
			break;
		::WaitForSingleObject(SThreadParams::hStarts[ni - 1], INFINITE);
		(pRawData->*pPassFun)(ni, nJobFrom, nJobTo, SThreadParams::fDeltaTime);
		//pRawData->PassWorker(ni, nJobFrom, nJobTo, SThreadParams::fDeltaTime);
		::SetEvent(SThreadParams::hEnds[ni - 1]);
	}
	return 0;
}


//----------------------------------------------------------------------------------------------------------------------
UINT __cdecl WorldRunFunctionControl(LPVOID pParam)
{
	CWorld * pw = reinterpret_cast<CWorld*>(pParam);

	CHiTimer tmRun, tmPass;
	tmPass.Start();
	int nms;
	int nStat = 0;
	float fCT = 0.f, fRT = 0.f, fFT = 0.f;
	float fCTT = 0.f, fRTT = 0.f, fFTT = 0.f;
	ULONGLONG ullFrame = 0ull;

	int ni = 0;
	int nJobFrom = SThreadParams::nJobSegs[0], nJobTo = SThreadParams::nJobSegs[1];
	SRawData* pRawData = SThreadParams::pRawData;

	auto pPassFun = &SRawData::PassWorker;
	if (SThreadParams::bUsingSIMD)
		pPassFun = &SRawData::PassWorker_SIMD;

	CTimeCap tc;
	for (;;)
	{
		if (SThreadParams::bStop)
		{
			SThreadParams::SetStartCaculation();
			::SetEvent(SThreadParams::hRenderStart);
			break;
		}
		SThreadParams::fDeltaTime = (float)tmPass.GetPassed(true);
		tmRun.Start();
		SThreadParams::SetStartCaculation();
		(pRawData->*pPassFun)(0, nJobFrom, nJobTo, SThreadParams::fDeltaTime);
		//pRawData->PassWorker(0, nJobFrom, nJobTo, SThreadParams::fDeltaTime);
		SThreadParams::WaitEndCaculation();
		fCT = (float)tmRun.GetPassed();
		::WaitForSingleObject(SThreadParams::hRenderEnd, INFINITE);
		tmRun.Start();
		pw->ComboOutput(true, SThreadParams::fDeltaTime, 
			SThreadParams::rectConstraint);
		fCT += (float)tmRun.GetPassed();
		::SetEvent(SThreadParams::hRenderStart);
/*
		fCT = (float)tmRun.GetPassed(true);
		if (::IsWindow(SThreadParams::hwndTarget))
		{
			::SendMessage(SThreadParams::hwndTarget, WM_USER_DRAW_WORLD2, 0, 0);
		}
		fRT = (float)tmRun.GetPassed();
*/
		fRT = SThreadParams::fRenderTime;
		fFT = SThreadParams::fDeltaTime;
		nms = int(fFT * 1000. + .5);
		nms = 16 - nms;
		if (nms > 1)
			::Sleep(nms);
		fCTT += fCT;
		fRTT += fRT;
		fFTT += fFT;
		nStat++;
		if (fFTT > 1.f)
		{
			ullFrame += nStat;
			float fsr = float(nStat);
			nStat = 0;
			int nFPS = int(fsr / fFTT + .5f);
			int nCalTime = int(fCTT / fsr * 1000.f + .5f);
			int nRenTime = int(fRTT / fsr * 1000.f + .5f);
			SThreadParams::strStat.Format(_T("FPS: %d    Frames: %llu\nCaculate Time: %dms\nRender Time: %dms"), nFPS, ullFrame, nCalTime, nRenTime);
			fCTT = fRTT = fFTT = 0.f;
		}
	}
	return 0;
}


//----------------------------------------------------------------------------------------------------------------------
UINT __cdecl RenderFunction(LPVOID pParam)
{
	CHiTimer tm;
	for (;;)
	{
		if (SThreadParams::bStop)
			break;
		::WaitForSingleObject(SThreadParams::hRenderStart, INFINITE);
		tm.Start();
		if (::IsWindow(SThreadParams::hwndTarget))
		{
			::SendMessage(SThreadParams::hwndTarget, WM_USER_DRAW_WORLD2, 0, 0);
		}
		SThreadParams::fRenderTime = (float)tm.GetPassed();
		::SetEvent(SThreadParams::hRenderEnd);
	}
	return 0;
}

