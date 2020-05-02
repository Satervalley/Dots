#pragma once

#include "framework.h"
#include "CRandomGen.h"
#include "vecm.h"
#include "CEffect.h"

#include <vector>
#include <memory>

class CExplosion : public CEffect
{
public:
	struct SLine
	{
		COLORREF clr;
		SVec2 ptStart, ptEnd;
        SVec2 vSpeed;
        SVec2 vSpeedDec;
        float fCoolSpeed{ 0.02f };
        float fWidth{ 1.0f };
        float fBeginFrom{ 0.f };
        float fTotalTime{ 0.f };
        float fLifeSpan{ 1.25f };
        float fAlpha;

        // return true: need next draw
        bool Pass(float dx, float dy, float dt)
        {
            float ldx = vSpeed.x * dt + dx, ldy = vSpeed.y * dt + dy;
            ptStart.x += ldx;
            ptStart.y += ldy;
            ptEnd.x += ldx;
            ptEnd.y += ldy;
            vSpeed.x -= vSpeedDec.x * dt;
            vSpeed.y -= vSpeedDec.y * dt;
            fTotalTime += dt;
            if (fTotalTime > fBeginFrom)
            {
                fAlpha = (fLifeSpan - fTotalTime + fBeginFrom) / fLifeSpan;
                return fAlpha > 0.f;
            }
            else
                return true;
        }

        // return true: need continue to draw, return false: no drawing needed
        void Draw(CRenderTarget* prt)
        {
            if (fTotalTime >= fBeginFrom)
            {
                CD2DSolidColorBrush brush(prt, clr, int(255.f * fAlpha));
                prt->DrawLine(CD2DPointF(ptStart.x, ptStart.y), CD2DPointF(ptEnd.x, ptEnd.y), &brush, fWidth);
            }
        }
	};

    typedef std::shared_ptr<SLine> SLine_Pointer;
    typedef std::vector<SLine_Pointer> SLine_Pointer_Vector;


    CExplosion(int ni, const SRawData& rd, float r = 150.f, float f = 1.0f)
    {
        dtDrawType = EDrawType::dtAfter/*EDrawType::dtReplace*/;
        ptPassType = EPassType::ptBeforeVelocity;

        nIndex = ni;
        ptPos.x = rd.faPosX[ni];
        ptPos.y = rd.faPosY[ni];
        nLWMax = rd.naRadius[ni] + 1;
        nLWMin = nLWMax / 4;
        nLWMin = nLWMin < 1 ? 1 : nLWMin;
        fRadius = r * f;
        fFactor = f;
        clrColor = rd.caColor[ni];
        Gen(ptPos.x, ptPos.y);
    }

    // return true: effect over
    virtual bool Pass(SRawData& rd, float fDeltaTime) override
    {
        static float fGmAmp = 256.f;
        static float fPushStart = 0.2f, fPushEnd = 0.75f;
        static float fElapsed = 0.f;

        float dx, dy;
        if (fElapsed > fPushStart && fElapsed < fPushEnd)
        {
            float r3;
            float fDistance2 = fRadius * fRadius;
            float fGm = rd.faGm[nIndex] * fGmAmp;
            float a, dvx, dvy;
            for (int i = 0; i < rd.nAmount; i++)
            {
                if (i == nIndex)
                    continue;
                dx = rd.faPosX[i] - rd.faPosX[nIndex];
                dy = rd.faPosY[i] - rd.faPosY[nIndex];
                r3 = dx * dx + dy * dy;
                if (fDistance2 < r3)
                    continue;
                r3 = (r3 + 0.001f) * ::sqrtf(r3 + 0.001f);
                a = fGm / r3;
                dvx = a * dx * fDeltaTime;
                dvy = a * dy * fDeltaTime;
                rd.faThreadOutputDVX[0][i] += dvx;
                rd.faThreadOutputDVY[0][i] += dvy;
            }
        }
        bool bRes = true;
        dx = rd.faPosX[nIndex] - ptPos.x;
        dy = rd.faPosY[nIndex] - ptPos.y;
        ptPos.x = rd.faPosX[nIndex];
        ptPos.y = rd.faPosY[nIndex];
        for (auto it : lpvLines)
        {
            if (it->Pass(dx, dy, fDeltaTime))
                bRes = false;
        }
        fElapsed += fDeltaTime;
        if (bRes)
            fElapsed = 0.f;
        return bRes;
    }


    virtual void Draw(CRenderTarget* prt) override
    {
        for (auto it : lpvLines)
        {
            it->Draw(prt);
        }
    }

protected:
    int nIndex;
    SVec2 ptPos;
    COLORREF clrColor;
    int nLWMin{ 2 }, nLWMax{ 8 };
    float fRadius;
    float fFactor;
    SLine_Pointer_Vector lpvLines;

    COLORREF GenColor(COLORREF clr, CRandomGen& rg)
    {
        BYTE r, g, b;
        r = GetRValue(clr);
        g = GetGValue(clr);
        b = GetBValue(clr);
        r = rg.GenInt(256, r);
        g = rg.GenInt(256, g);
        b = rg.GenInt(256, b);
        return RGB(r, g, b);
    }

    void Gen(float x, float y)
    {
//        static int nSpeedsMin[8] = { 60, 70, 80, 90, 90, 80, 70, 60 };
        static int nSpeedsMin[8] = { 10, 10, 10, 10, 10, 10, 10, 10 };
        static int nSpeedsMax[8] = { 80, 100, 120, 140, 130, 120, 110, 100 };
        //static int nSpeedDecsMin[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };
        //static int nSpeedDecsMax[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };
        static int nSpeedDecMin = 8, nSpeedDecMax = 16;
        static int nNumbers[8] = { 16, 32, 64, 128, 256, 128, 64 };
        static float fFrameTime = 0.0167f;

        lpvLines.clear();
        CRandomGen rg;
        for (int i = 0; i < 8; i++)
        {
            //int n = int(float(nNumbers[i]) * ::sqrtf(fFactor));
            int n = nNumbers[i];
            for (int j = 0; j < n; j++)
            {
                SLine_Pointer lp = std::make_shared<SLine>();
                lp->clr = GenColor(clrColor, rg);
                lp->fBeginFrom = float(i) * fFrameTime;
                lp->ptStart.x = x;
                lp->ptStart.y = y;
                float flen = ((float)rg.GenInt(nLWMax, nLWMin))/* * fFactor*/;
                // gen direction
                SVec2 varc;
                for (;;)
                {
                    varc.x = (float)rg.GenInt(50, -50);
                    varc.y = (float)rg.GenInt(50, -50);
                    if (::fabs(varc.x) + ::fabs(varc.y) > 0.5f)
                        break;
                }
                varc.Scale(flen);
                lp->ptEnd.x = lp->ptStart.x + varc.x;
                lp->ptEnd.y = lp->ptStart.y + varc.y;
                lp->vSpeed.x = lp->ptStart.dx(lp->ptEnd);
                lp->vSpeed.y = lp->ptStart.dy(lp->ptEnd);
                lp->vSpeed.Scale((float)rg.GenInt(nSpeedsMax[i], nSpeedsMin[i]) * fFactor);
                lp->vSpeedDec = lp->vSpeed;
                lp->vSpeedDec.Scale(float(rg.GenInt(nSpeedDecMax, nSpeedDecMin)) * fFactor);
                lp->fWidth = float(rg.GenInt(nLWMax, nLWMin))/* * fFactor*/;
                lpvLines.push_back(lp);
            }
        }
    }
};

