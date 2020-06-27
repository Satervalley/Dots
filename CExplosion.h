#pragma once

#include "framework.h"
#include "CRandomGen.h"
#include "vecm.h"
#include "CEffect.h"

#include <vector>
#include <list>
#include <memory>

class CExplosion : public CSingleEffect
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


    CExplosion(int ni, const SRawData& rd, /*float r = 150.f, */float f = 1.0f) : CSingleEffect(ni)
    {
        dtDrawType = EDrawType::dtAfter/*EDrawType::dtReplace*/;
        ptPassType = EPassType::ptBeforeVelocity;

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
        fGmAmp = float(rgRandom.GenInt(512, 256)) * rd.faRadius[nIndex] * rd.fG * 4.f;
        Gen(ptPos.x, ptPos.y);
    }

    // return true: effect over
    virtual bool Pass(SRawData& rd, float fDeltaTime) override
    {
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
                a = fGm / r3 / rd.faGm[i];
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
    float fGmAmp{ 1000.f };
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



class CExplosion2 : public CSingleEffect
{
public:
    struct SLine
    {
        COLORREF clr;
        SVec2 ptStart, ptEnd;
        SVec2 vSpeed;
        float fWidth{ 1.0f };
        float fFadeSpeed{ 3.f };
        float fAlpha{ 1.0f };

        // return true: need next draw
        bool Pass(float dx, float dy, float dt)
        {
            float ldx = vSpeed.x * dt + dx, ldy = vSpeed.y * dt + dy;
            ptStart.x += ldx;
            ptStart.y += ldy;
            ptEnd.x += ldx;
            ptEnd.y += ldy;
            fAlpha -= fFadeSpeed * dt;
            return fAlpha > 0.f;
        }


        // return true: need continue to draw, return false: no drawing needed
        void Draw(CRenderTarget* prt)
        {
            CD2DSolidColorBrush brush(prt, clr, int(255.f * fAlpha));
            prt->DrawLine(CD2DPointF(ptStart.x, ptStart.y), CD2DPointF(ptEnd.x, ptEnd.y), &brush, fWidth);
        }
    };

    typedef std::shared_ptr<SLine> SLine_Pointer;
    typedef std::vector<SLine_Pointer> SLine_Pointer_Vector;

    struct SWave
    {
        SVec2 vSpeedBase;
        SLine_Pointer_Vector lpvLines;

        SWave(int nc, float fx, float fy, float fvx, float fvy, COLORREF clr, int nvmax, int nvmin, 
            int nlsmax, int nlsmin, int nlmax, int nlmin, int nwmax, int nwmin, float ffg, float ffl, CRandomGen& rg)
        {
            vSpeedBase.x = fvx;
            vSpeedBase.y = fvy;
            Gen(nc, fx, fy, clr, nvmax, nvmin, nlsmax, nlsmin, nlmax, nlmin, nwmax, nwmin, ffg, ffl, rg);
        }


        // return true: need next draw
        bool Pass(float fdt)
        {
            bool bRes = false, b;
            float dx = vSpeedBase.x * fdt, dy = vSpeedBase.y * fdt;
            for (auto line : lpvLines)
            {
                b = line->Pass(dx, dy, fdt);
                bRes = bRes || b;
            }
            return bRes;
        }


        void Draw(CRenderTarget* prt)
        {
            for (auto it : lpvLines)
                it->Draw(prt);
        }

    protected:

        void Gen(int nc, float x, float y, COLORREF clr, int nvmax, int nvmin, int nlsmax, int nlsmin, 
            int nlmax, int nlmin, int nwmax, int nwmin, float ffg, float ffl, CRandomGen& rg)
        {
            //float ffl2 = ::sqrtf(ffl);
            for (int i = 0; i < nc; i++)
            {
                SLine_Pointer lp = std::make_shared<SLine>();
                lp->clr = GenColor(clr, rg);
                lp->ptStart.x = x;
                lp->ptStart.y = y;
                float flen = ((float)rg.GenInt(nlmax, nlmin));
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
                lp->vSpeed.Scale(float(rg.GenInt(nvmax, nvmin)) * ffg * ffl);
                lp->fWidth = float(rg.GenInt(nwmax, nwmin));
                lp->fFadeSpeed = 1000.f / (float(rg.GenInt(nlsmax, nlsmin)) * ffl);
                lpvLines.push_back(lp);
            }
        }


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
    };


    typedef std::shared_ptr<SWave> SWave_Pointer;
    typedef std::list<SWave_Pointer> SWave_Pointer_List;


    CExplosion2(int ni, const SRawData& rd, float fInterval = 0.03f, float f = 1.0f, bool bAddWave = true) : CSingleEffect(ni)
    {
        dtDrawType = EDrawType::dtAfter/*EDrawType::dtReplace*/;
        ptPassType = EPassType::ptBeforeVelocity;

        nLMax = int(rd.faRadius[ni] * 2.f);
        nLMin = nLMax / 4;
        nLMin = nLMin < 2 ? 2 : nLMin;
        nWMax = int(rd.faRadius[ni]);
        nWMin = nWMax / 2;
        nWMin = nWMin < 1 ? 1 : nWMin;
        nWMax = nWMax <= nWMin ? nWMin + 1 : nWMax;
        fRadius = float((nVMin + nVMax) / 2) * float(nLifeSpanMin + nLifeSpanMax) / 2000.f * f;
        fFactor = f;
        clrColor = rd.caColor[ni];
        fGmAmp = float(rgRandom.GenInt(512, 256)) * rd.faRadius[nIndex] * rd.fG * 4.f;
        fWaveInterval = fInterval;
        nWaveCount = rgRandom.GenInt(nWaveCountMax, nWaveCountMin);
        nWaveNumber = nWaveCount;
        if (bAddWave)
        {
            AddWave(rd.faPosX[ni], rd.faPosY[ni], rd.faVelocityX[ni], rd.faVelocityY[ni], LocalFactor());
            nWaveCount--;
        }
    }

    // return true: effect over
    virtual bool Pass(SRawData& rd, float fDeltaTime) override
    {
        fTimePassed2 += fDeltaTime;
        Push(rd, fDeltaTime);
        if (nWaveCount > 0)
        {
            if (fTimePassed2 - fTimePassed1 > fWaveInterval)
            {
                fTimePassed1 = fTimePassed2;
                AddWave(rd.faPosX[nIndex], rd.faPosY[nIndex], rd.faVelocityX[nIndex], rd.faVelocityY[nIndex], LocalFactor());
                nWaveCount--;
            }
        }

        bool bRes = true;
        for (auto it = wplWaves.begin(); it != wplWaves.end(); )
        {
            if ((*it)->Pass(fDeltaTime))
            {
                bRes = false;
                it++;
            }
            else
                it = wplWaves.erase(it);
        }
        return bRes;
    }


    virtual void Draw(CRenderTarget* prt) override
    {
        for (auto it : wplWaves)
        {
            it->Draw(prt);
        }
    }

protected:
    COLORREF clrColor;
    int nLMin{ 2 }, nLMax{ 8 }, nWMin{ 1 }, nWMax{ 4 };
    int nVMin{ 200 }, nVMax{ 500 };
    int nCountMin{ 40 }, nCountMax{ 120 };   // per wave line count
    int nLifeSpanMin{ 200 }, nLifeSpanMax{ 400 }; // must divide 1000, wave life span
    int nWaveCountMin{ 10 }, nWaveCountMax{ 70 };
    int nWaveCount, nWaveNumber;   // how many waves, number is fixed
    float fRadius;
    float fFactor;
    float fWaveInterval;
    float fTimePassed1{ 0.f }, fTimePassed2{ 0.f };
    float fGmAmp{ 1000.f };
    SWave_Pointer_List wplWaves;
    CRandomGen rgRandom;


    void AddWave(float fx, float fy, float fvx, float fvy, float ffl)
    {
        int nc = rgRandom.GenInt(nCountMax, nCountMin);
        
        SWave_Pointer wp = std::make_shared<SWave>(nc, fx, fy, fvx, fvy, clrColor, nVMax, nVMin, 
            nLifeSpanMax, nLifeSpanMin, nLMax, nLMin, nWMax, nWMin, fFactor, ffl, rgRandom);
        wplWaves.push_back(wp);
    }


    float LocalFactor(void)
    {
        float f = (1.f - ::fabsf(float(nWaveNumber) / 2.f - float(nWaveCount)) / float(nWaveNumber));
        f = ::sqrtf(f);
        return f;
    }


    void Push(SRawData& rd, float fDeltaTime)
    {
        float dx, dy;
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
            a = fGm / r3 / rd.faGm[i];
            dvx = a * dx * fDeltaTime;
            dvy = a * dy * fDeltaTime;
            rd.faThreadOutputDVX[0][i] += dvx;
            rd.faThreadOutputDVY[0][i] += dvy;
        }
    }
};


class CExplosion3 : public CExplosion2
{
public:
    CExplosion3(int ni, const SRawData& rd, float fInterval = 0.01f, float f = 1.0f) : CExplosion2(ni, rd, fInterval, f, false)
    {
        nLMax = int(rd.faRadius[ni] * 2.f);
        nLMin = nLMax / 4;
        nLMin = nLMin < 2 ? 2 : nLMin;
        nWMax = nLMax;
        nWMin = nLMin;

        nVMin = 160;
        nVMax = 240;
        nCountMin = 60;
        nCountMax = 100;
        nLifeSpanMin = 1000;
        nLifeSpanMax = 1500;
        nWaveCountMin = 8;
        nWaveCountMax = 12;

        fRadius = float((nVMin + nVMax) / 2) * float(nLifeSpanMin + nLifeSpanMax) / 2000.f * f;
        nWaveCount = rgRandom.GenInt(nWaveCountMax, nWaveCountMin);
        nWaveNumber = nWaveCount;
        AddWave(rd.faPosX[ni], rd.faPosY[ni], rd.faVelocityX[ni], rd.faVelocityY[ni], LocalFactor());
        nWaveCount--;
    }

};


class CExplosion4 : public CExplosion2
{
public:
    CExplosion4(int ni, const SRawData& rd, float fInterval = 0.01f, float f = 1.0f) : CExplosion2(ni, rd, fInterval, f, false)
    {
        nLMax = int(rd.faRadius[ni] * 2.f);
        nLMin = nLMax / 4;
        nLMin = nLMin < 2 ? 2 : nLMin;
        nWMax = nLMax;
        nWMin = nLMin;

        nVMin = 190;
        nVMax = 210;
        nCountMin = 48;
        nCountMax = 64;
        nLifeSpanMin = 1000;
        nLifeSpanMax = 1200;
        nWaveCountMin = 8;
        nWaveCountMax = 12;

        fRadius = float((nVMin + nVMax) / 2) * float(nLifeSpanMin + nLifeSpanMax) / 2000.f * f;
        nWaveCount = GenGroups();
        nWaveNumber = nWaveCount;
    }

    // return true: effect over
    virtual bool Pass(SRawData& rd, float fDeltaTime) override
    {
        fTimePassed2 += fDeltaTime;
        Push(rd, fDeltaTime);
        if (nCurrWave < nWaveNumber)
        {
            if (fTimePassed2 > faTimes[nCurrWave])
            {
                AddWave(rd.faPosX[nIndex], rd.faPosY[nIndex], rd.faVelocityX[nIndex], rd.faVelocityY[nIndex], LocalFactor());
                nCurrWave++;
            }
        }

        bool bRes = true;
        for (auto it = wplWaves.begin(); it != wplWaves.end(); )
        {
            if ((*it)->Pass(fDeltaTime))
            {
                bRes = false;
                it++;
            }
            else
                it = wplWaves.erase(it);
        }
        return bRes;
    }

protected:
    int nGroupMin{ 3 }, nGroupMax{ 8 }; // how many groups
    int nGroupWaveMin{ 3 }, nGroupWaveMax{ 6 }; // how many waves each group
    float fGroupInterval{ 0.15f };
    float faTimes[40];
    int nCurrWave{ 0 };

    int GenGroups(void)
    {
        int nGroupNumber = rgRandom.GenInt(nGroupMax, nGroupMin);
        int n, nt = 0;
        float ftt = 0.f;
        for (int i = 0; i < nGroupNumber; i++)
        {
            n = rgRandom.GenInt(nGroupWaveMax, nGroupWaveMin);
            for (int j = 0; j < n; j++)
            {
                ftt += fWaveInterval;
                faTimes[nt] = ftt;
                nt++;
            }
            ftt += fGroupInterval;
        }
        return nt;
    }
};


