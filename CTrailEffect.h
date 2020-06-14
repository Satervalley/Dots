#pragma once

#include "CEffect.h"
#include "vecm.h"
#include "boost\circular_buffer.hpp"
#include <math.h>


typedef boost::circular_buffer<SVec2> SVec2_CB;
#define TRAIL_LENGTH 128


class CTrailEffect : public CSingleEffect
{
public:
	CTrailEffect(int ni, COLORREF clr, bool bd = false, int nls = 500, float fw = 1.0f, 
		size_t cap = TRAIL_LENGTH, float fam = 0.4f) : CSingleEffect(ni)
	{
		const float fBaseLength = 64.f;
		clrColor = clr;
		bDiffuse = bd;
		nLifeSpan = nls;
		fThickRoot = fw;
		if (bDiffuse)
			fThickTail = fThickRoot + float(cap) / fBaseLength * fThickRoot;
		else
			fThickTail = 1.0f;
		cbPoints.set_capacity(cap);
		fAlphaMax = fam;
		//nSteps = 0;
		dtDrawType = EDrawType::dtBefore;
		ptPassType = EPassType::ptPosition;
	}


	virtual bool Pass(SRawData& rd, float fDeltaTime) override
	{
		bool bRes = false;
		SVec2 v2;
		v2.x = rd.faPosX[nIndex];
		v2.y = rd.faPosY[nIndex];
		cbPoints.push_back(v2);
		if (nLifeSpan > 0)
		{
			nLifeSpan--;
		}
		else
		{
			if (cbPoints.size() < 2)
				bRes = true;
			else
			{
				cbPoints.pop_front();
				cbPoints.pop_front();
			}
		}

		return bRes;
	}


	virtual void Draw(CRenderTarget* prt) override
	{
		if (cbPoints.size() < 2)
			return;
		float fas = fAlphaMax / (float)(cbPoints.size() - 1);
		//float fts = (fThick - 0.5f) / (float)(cbPoints.size() - 1); 
		float fts = (fThickTail - fThickRoot) / (float)(cbPoints.capacity() - 1);
		float fa, ft;
		for (size_t i = 0; i < cbPoints.size() - 1; i++)
		{
			fa = ::sqrtf(fas * float(i + 1));
			//ft = 0.5f + fts * float(ns);
			ft = fThickRoot + fts * float(cbPoints.size() - i - 1);
			CD2DSolidColorBrush brush(prt, clrColor, int(fa * 255.f));
			CD2DPointF ptf(cbPoints[i].x, cbPoints[i].y), ptt(cbPoints[i + 1].x, cbPoints[i + 1].y);
			prt->DrawLine(ptf, ptt, &brush, ft);
		}
	}


protected:
	int nLifeSpan;
	COLORREF clrColor;
	bool bDiffuse{ false };
	SVec2_CB cbPoints;
	float fThickRoot, fThickTail;
	float fAlphaMax{ 0.4f };
};

