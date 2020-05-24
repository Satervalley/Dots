#pragma once

#include "pch.h"
#include "framework.h"
#include <math.h>
#include <memory>
#include <vector>


enum class EDotType
{
	dtNormal, dtBlackHole
};


class CDotBmp
{
public:
	CDotBmp(int ns, COLORREF clr, CRenderTarget* prt, bool bLoose = false, EDotType dt = EDotType::dtNormal)
	{
		ASSERT(ns > 2 && ns < 500);
		ASSERT_VALID(prt);

		dwData = new DWORD[ns * ns];
		nSize = ns;
		BYTE r = GetRValue(clr), g = GetGValue(clr), b = GetBValue(clr);
		clrColor = RGB(b, g, r);	// we must translate color
		if(dt == EDotType::dtNormal)
			Make(prt, bLoose);
		if (dt == EDotType::dtBlackHole)
			MakeBlackHole(prt, bLoose);
	}

	~CDotBmp()
	{
		if (dwData)
			delete[] dwData;
	}


	bool operator ==(const CDotBmp& db)
	{
		return nSize == db.nSize && clrColor == db.clrColor;
	}

	
	operator CD2DBitmap* ()
	{
		return pbmpDot;
	}

	bool Same(int ns, COLORREF clr) const
	{
		return nSize == ns && clrColor == clr;
	}

	int Size(void) const { return nSize; }
protected:
	EDotType dtType{ EDotType::dtNormal };
	int nSize{ 0 };
	COLORREF clrColor;
	DWORD* dwData{ nullptr };
	CBitmap bmpDot;
	CD2DBitmap* pbmpDot{ nullptr };

	
	void MakeRaw(bool bLoose)
	{
		float nc = float(nSize - 1) / 2.f;
		float fstep = float(1.f / float(nc + 1.f));
		float r, xd, yd;
		DWORD dw;
		for (int i = 0; i < nSize; i++)
		{
			for (int j = 0; j < nSize; j++)
			{
				xd = float(i) - nc;
				yd = float(j) - nc;
				r = ::sqrtf(xd * xd + yd * yd);
				r = float(1.f - fstep * r);
				if (r <= 0.)
					dw = 0;
				else
				{
					if (!bLoose)
						r = ::sqrtf(r);
					dw = BYTE(255. * r);
					dw <<= 24;
				}
				dw |= (clrColor & 0x00ffffff);
				dwData[j * nSize + i] = dw;
			}
		}
	}


	void MakeBmp(CRenderTarget* prt)
	{
		bmpDot.CreateBitmap(nSize, nSize, 1, 32, dwData);
		pbmpDot = new CD2DBitmap(prt, bmpDot);
	}


	void Make(CRenderTarget* prt, bool bLoose)
	{
		MakeRaw(bLoose);
		MakeBmp(prt);
	}


	void MakeHole(void)
	{
		if (nSize == 3)
		{
			dwData[4] = 0xff000000;
		}
		else
		{
			int nRim = nSize / 8;
			nRim = nRim < 1 ? 1 : nRim;
			int nHS = nSize - nRim * 2;
			if (nSize % 2)
			{
				if (nHS % 2 == 0)
					nHS--;
			}
			else
			{
				if (nHS % 2)
					nHS--;
			}
			//int nHS = int(float(nSize) * .5f + .6f);
			//if (nHS % 2 == 0)
			//	nHS++;
			int ndiff = (nSize - nHS) / 2;
			float nc = float(nHS - 1) / 2.f;
			float fstep = float(1.f / float(nc + 1.f));
			float r, xd, yd;
			DWORD dw;
			for (int i = 0; i < nHS; i++)
			{
				for (int j = 0; j < nHS; j++)
				{
					xd = float(i - nc);
					yd = float(j - nc);
					r = ::sqrtf(xd * xd + yd * yd);
					r = float(1.f - fstep * r);
					if (r <= 0.)
						dw = 0;
					else
					{
						//r = ::sqrtf(r);
						//r = ::sqrtf(r);
						//r = ::sqrtf(r);
						r = ::powf(r, 0.2f);
						dw = BYTE(255. * r);
						dw <<= 24;
					}
					dw = Mix(dw, dwData[(ndiff + j) * nSize + ndiff + i]);
					dwData[(ndiff + j) * nSize + ndiff + i] = dw;
				}
			}
		}
	}


	void MakeBlackHole(CRenderTarget* prt, bool bLoose)
	{
		MakeRaw(bLoose);
		MakeHole();
		MakeBmp(prt);
	}

	DWORD Mix(DWORD dw1, DWORD dw2)
	{
		DWORD dw = 0xff000000;
		float a1 = float(dw1 >> 24) / 255.f;
		float a2 = float(dw2 >> 24) / 255.f;
		if (a2 > 1.f - a1)
			a2 = 1.f - a1;
		DWORD dwb1, dwb2;
		dwb1 = (dw1 >> 16) & 0x000000ff;
		dwb2 = (dw2 >> 16) & 0x000000ff;
		dwb1 = DWORD(float(dwb1) * a1 + float(dwb2) * a2);
		dw += (dwb1 << 16);
		dwb1 = (dw1 >> 8) & 0x000000ff;
		dwb2 = (dw2 >> 8) & 0x000000ff;
		dwb1 = DWORD(float(dwb1) * a1 + float(dwb2) * a2);
		dw += (dwb1 << 8);
		dwb1 = dw1 & 0x000000ff;
		dwb2 = dw2 & 0x000000ff;
		dwb1 = DWORD(float(dwb1) * a1 + float(dwb2) * a2);
		dw += dwb1;
		return dw;
	}
};


typedef std::shared_ptr<CDotBmp> DotBmpPointer;
typedef std::vector<DotBmpPointer> DotBmpPointerVector;
