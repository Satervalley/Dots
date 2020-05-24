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
        SVec2 vSpeedBase;
        bool bEjected{ false };
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


        bool Pass2(float fx, float fy, float fvx, float fvy, float dt)
        {
            fTotalTime += dt;
            if (fTotalTime > fBeginFrom)
            {
                if (!bEjected)
                {
                    bEjected = true;
                    float dx = ptEnd.x - ptStart.x, dy = ptEnd.y - ptStart.y;

                    ptStart.x = fx;
                    ptStart.y = fy;
                    ptEnd.x = ptStart.x + dx;
                    ptEnd.y = ptStart.y + dy;
                    vSpeedBase.x = fvx;
                    vSpeedBase.y = fvy;
                }

                fAlpha = (fLifeSpan - fTotalTime + fBeginFrom) / fLifeSpan;
                if (fAlpha > 0.f)
                {
                    float ldx = vSpeed.x * dt + vSpeedBase.x * dt, ldy = vSpeed.y * dt;
                    ptStart.x += ldx;
                    ptStart.y += ldy;
                    ptEnd.x += ldx;
                    ptEnd.y += ldy;
                    vSpeed.x -= vSpeedDec.x * dt;
                    vSpeed.y -= vSpeedDec.y * dt;
                    return true;
                }
                else
                    return false;
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


    CExplosion(int ni, const SRawData& rd, /*float r = 150.f, */float f = 1.0f)
    {
        dtDrawType = EDrawType::dtAfter/*EDrawType::dtReplace*/;
        ptPassType = EPassType::ptBeforeVelocity;

        nIndex = ni;
        ptPos.x = rd.faPosX[ni];
        ptPos.y = rd.faPosY[ni];
        nLMax = int(rd.faRadius[ni] * 2.f);
        nLMin = nLMax / 4;
        nLMin = nLMin < 2 ? 2 : nLMin;
        nWMax = int(rd.faRadius[ni]);
        nWMin = nWMax / 2;
        nWMin = nWMin < 1 ? 1 : nWMin;
        nWMax = nWMax <= nWMin ? nWMin + 1 : nWMax;
        //fRadius = r * f;
        fFactor = f;
        clrColor = rd.caColor[ni];
        Gen(ptPos.x, ptPos.y);
    }

    // return true: effect over
    virtual bool Pass(SRawData& rd, float fDeltaTime) override
    {
        static float fGmAmp = float(rgRandom.GenInt(512, 256)) * rd.faRadius[nIndex];
        static float fPushStart = 0.1f, fPushEnd = 1.3f;
        static float fElapsed = 0.f;
        static float fRadius{ 0.f };

        fElapsed += fDeltaTime;
        fRadius += (75.f * fFactor * fDeltaTime);
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
            //if (it->Pass(dx, dy, fDeltaTime))
            //    bRes = false;
            if (it->Pass2(rd.faPosX[nIndex], rd.faPosY[nIndex], rd.faVelocityX[nIndex], rd.faVelocityY[nIndex], fDeltaTime))
                bRes = false;
        }
        if (bRes)
        {
            fElapsed = 0.f;
            fRadius = 0.f;
        }
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
    int nLMin{ 2 }, nLMax{ 8 }, nWMin{ 1 }, nWMax{ 4 };
//    float fRadius;
    float fFactor;
    SLine_Pointer_Vector lpvLines;
    CRandomGen rgRandom;

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
        for (int i = 0; i < 8; i++)
        {
            //int n = int(float(nNumbers[i]) * ::sqrtf(fFactor));
            int n = nNumbers[i];
            for (int j = 0; j < n; j++)
            {
                SLine_Pointer lp = std::make_shared<SLine>();
                lp->clr = GenColor(clrColor, rgRandom);
                lp->fBeginFrom = float(i * 3) * fFrameTime;
                lp->ptStart.x = x;
                lp->ptStart.y = y;
                float flen = ((float)rgRandom.GenInt(nLMax, nLMin))/* * fFactor*/;
                // gen direction
                SVec2 varc;
                for (;;)
                {
                    varc.x = (float)rgRandom.GenInt(50, -50);
                    varc.y = (float)rgRandom.GenInt(50, -50);
                    if (::fabs(varc.x) + ::fabs(varc.y) > 0.5f)
                        break;
                }
                varc.Scale(flen);
                lp->ptEnd.x = lp->ptStart.x + varc.x;
                lp->ptEnd.y = lp->ptStart.y + varc.y;
                lp->vSpeed.x = lp->ptStart.dx(lp->ptEnd);
                lp->vSpeed.y = lp->ptStart.dy(lp->ptEnd);
                lp->vSpeed.Scale((float)rgRandom.GenInt(nSpeedsMax[i], nSpeedsMin[i]) * fFactor);
                lp->vSpeedDec = lp->vSpeed;
                lp->vSpeedDec.Scale(float(rgRandom.GenInt(nSpeedDecMax, nSpeedDecMin)) * fFactor);
                lp->fWidth = float(rgRandom.GenInt(nWMax, nWMin))/* * fFactor*/;
                lpvLines.push_back(lp);
            }
        }
    }
};

