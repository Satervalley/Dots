#pragma once

#include "framework.h"
#include <timeapi.h>
#include "timer.h"
#include "CRandomGen.h"
#include "vecm.h"
#include "SRawData.h"
#include "CEffectManager.h"
#include "CTrailEffect.h"
#include "CExplosion.h"

#include <mmintrin.h>

#define MAX_AMOUNT 10240
#define MAX_THREADS_NUMBER 8


//----------------------------------------------------------------------------------------------------------------------
class CGrid
{
public:
    ~CGrid()
    {
        Release();
    }


    void Init(const CRect& rc, int ncs, int nthf = 10)
    {
        Release();
        int w, h;
        w = rc.Width() / ncs;
        if (rc.Width() % ncs)
            w++;
        h = rc.Height() / ncs;
        if (rc.Height() % ncs)
            h++;
        szSize.cx = w;
        szSize.cy = h;
        fCellSize = float(ncs);
        fThresHold = fCellSize * float(nthf);
        fThresHold *= fThresHold;
        pCells = new SCell_Pointer[w * h];
        for (int i = 0; i < w; i++)
        {
            for (int j = 0; j < h; j++)
            {
                SCell_Pointer cp = std::make_shared<SCell>();
                cp->v2Center.x = (float(i) + .5f) * fCellSize;
                cp->v2Center.y = (float(j) + .5f) * fCellSize;
                pCells[j * w + i] = cp;
            }
        }
    }


    void Reset(void)
    {
        if (pCells)
        {
            for (int i = 0; i < szSize.cx * szSize.cy; i++)
            {
                pCells[i]->Reset();
            }
        }
    }


    void Pass(SRawData& rd, float fDeltaTime)
    {
        Reset();
        // put bodies to grid cell
        for (int i = 0; i < rd.nAmount; i++)
        {
            int nx, ny;
            nx = int(rd.faPosX[i] / fCellSize);
            ny = int(rd.faPosY[i] / fCellSize);
            nx = nx < 0 ? 0 : nx;
            nx = nx >= szSize.cx ? szSize.cx - 1 : nx;
            ny = ny < 0 ? 0 : ny;
            ny = ny >= szSize.cy ? szSize.cy - 1 : ny;
            pCells[ny * szSize.cx + nx]->Put(i, rd.faPosX[i], rd.faPosY[i], rd.faGm[i], rd.faAntiG[i] < 0.f);
        }
        //nCalculateCount = 0;
        // caculate dv based on cells
        int nctc = szSize.cx * szSize.cy;
        bool bFar;
        ::ZeroMemory(rd.faThreadOutputDVX[0], sizeof(float) * rd.nAmount);
        ::ZeroMemory(rd.faThreadOutputDVY[0], sizeof(float) * rd.nAmount);
        for (int i = 0; i < nctc; i++)
        {
            rd.PassCellInternal(*(pCells[i]), fDeltaTime);

            //int n = pCells[i]->viIndexes.size();
            //nCalculateCount += (n * (n - 1) / 2);

            for (int j = i + 1; j < nctc; j++)
            {
                bFar = pCells[i]->v2Center.DistanceP2(pCells[j]->v2Center) > fThresHold;
                rd.PassInterCells(*(pCells[i]), *(pCells[j]), fDeltaTime, bFar);

                //if (bFar)
                //{
                //    nCalculateCount += (pCells[i]->viIndexes.size() + pCells[j]->viIndexes.size());
                //}
                //else
                //    nCalculateCount += (pCells[i]->viIndexes.size() * pCells[j]->viIndexes.size());

            }
        }
        nCalculateCount;
    }
protected:
    CSize szSize;
    float fCellSize;
    float fThresHold;   // the minimal r^2 of two cells can using simplified caculating
    SCell_Pointer* pCells{ nullptr };
    int nCalculateCount;

    void Release(void)
    {
        if (pCells)
        {
            delete[] pCells;
            pCells = nullptr;
        }
    }
};


//----------------------------------------------------------------------------------------------------------------------
struct SThreadParams
{
    static int nWorkerCount;  // total should +1, there is a control thread
    static SRawData* pRawData;
    static int nJobSegs[MAX_THREADS_NUMBER + 1];
    static bool bStop;
    static HANDLE hStarts[MAX_THREADS_NUMBER - 1];
    static HANDLE hEnds[MAX_THREADS_NUMBER - 1];
    static HANDLE hRenderStart, hRenderEnd;
    static float fDeltaTime;
    static float fFixedDeltaTime;
    static float fDivergenceFactor;
    static float fRenderTime;
    static bool bUsingSIMD;

    static HWND hwndTarget;
    static CString strStat;

    int nIndex{ 0 };

    static void Init(int nThreadCount, SRawData* prd)
    {
        nWorkerCount = nThreadCount - 1;
        pRawData = prd;
        Split(prd->nAmount, nThreadCount, nJobSegs);
        bStop = false;
        fDeltaTime = 0.f;
        for (int i = 0; i < nWorkerCount; i++)
        {
            if(!hStarts[i])
                hStarts[i] = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
            else
                ::ResetEvent(hStarts[i]);
        }
        for (int i = 0; i < nWorkerCount; i++)
        {
            if(!hEnds[i])
                hEnds[i] = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
            else
                ::ResetEvent(hEnds[i]);
        }
        if (!hRenderStart)
            hRenderStart = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
        else
            ::ResetEvent(hRenderStart);
        if (!hRenderEnd)
            hRenderEnd = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
        else
            ::ResetEvent(hRenderEnd);
    }

    static void Release(void)
    {
        for (int i = 0; i < nWorkerCount; i++)
        {
            if(hStarts[i])
                ::CloseHandle(hStarts[i]);
            if(hEnds[i])
                ::CloseHandle(hEnds[i]);
        }
        if (hRenderStart)
            ::CloseHandle(hRenderStart);
        if (hRenderEnd)
            ::CloseHandle(hRenderEnd);
    }

    static void SetStartCaculation(void)
    {
        for (int i = 0; i < nWorkerCount; i++)
            ::SetEvent(hStarts[i]);
    }

    static void WaitEndCaculation(void)
    {
        DWORD dw = ::WaitForMultipleObjects(nWorkerCount, hEnds, TRUE, INFINITE);
        //for (int i = 0; i < nWorkerCount; i++)
        //    ::ResetEvent(phEnds[i]);
    }


//----------------------------------------------------------------------------------------------------------------------
    static void ResetAllStartEvents(void)
    {
        for (int i = 0; i < nWorkerCount; i++)
            ::ResetEvent(hStarts[i]);
    }


//----------------------------------------------------------------------------------------------------------------------
    static void Split(int nc, int nseg, int* pRes)
    {
        int nTotal = nc * (nc - 1) / 2;
        int nPart = nTotal / nseg;
        int nTop = nc - 1;
        int nBottom = 0;
        pRes[0] = 0;
        for (int i = 0; i < nseg - 1; i++)
        {
            nBottom = Solve(nTop, nPart);
            pRes[i + 1] = nc - nBottom;
            nTop = nBottom - 1;
        }
        pRes[nseg] = nc - 1;
    }

    //----------------------------------------------------------------------------------------------------------------------
    // if star amount is 1024, total caculation is 1024 * (1024 - 1) / 2 = 523776, if 4 threads, nFactor = 523776 / 4 = 130944
    // nTop = 1024 - 1, (nTop + x)(nTop + 1 - x) = nFactor * 2
    // nTop * nTop + nTop * x + x - nTop * x - x*x - nFactor * 2 = 0
    // nTop*nTop + x - x*x - nFactor * 2 = 0
    // x*x - x + (nFactor * 2 - nTop*nTop) = 0
    // after solved, nJobFrom = amount - 1 - nTop, nJobTo = amount - res, use: for(int i = nJobFrom; i++; i < nJobTo)
    static int Solve(int nTop, int nFactor)
    {
        float a = 1.f, b = -1.f, c = float(nFactor * 2 - nTop * nTop);
        float b24ac = float(b * b - 4.f * a * c);
        int res = int((::sqrtf(b24ac) - b) / (2.f * a));
        return res;
    }


    static void SetStepLength(DWORD dw)
    {
        dw = dw % 5u;
        fFixedDeltaTime = float(dw * 5u + 5u) / 1000.f;
    }
};


//----------------------------------------------------------------------------------------------------------------------
/*
struct SBody
{
    int nRadius;
    float fCoreRadius;
    float m;
    SVec2 pos;
    SVec2 velocity;
    float fElasticFactor;
    float fAnti;    // is anti g
};
*/

extern const UINT WM_USER_DRAW_WORLD;
extern const UINT WM_USER_DRAW_WORLD2;


//----------------------------------------------------------------------------------------------------------------------
class CTimeCap
{
public:
    CTimeCap(UINT ms = 1u)
    {
        uiMS = ms;
        TIMECAPS tc;
        ::timeGetDevCaps(&tc, sizeof(tc));
        if (uiMS < tc.wPeriodMin)
            uiMS = tc.wPeriodMin;
        ::timeBeginPeriod(uiMS);
    }

    ~CTimeCap()
    {
        ::timeEndPeriod(uiMS);
    }

private:
    UINT uiMS;
};



//----------------------------------------------------------------------------------------------------------------------
// object of this class must be created on heap
class CWorld
{
public:
    bool bStop{ true };
    // below form multi thread caculating
    SRawData rdRawData;
    SThreadParams tpThreadParams[4];
    CGrid grid;

    CString strStat{ _T("FPS: Frames: \nCaculate Time: \nRender Time: ") };

    void Init(int nw, int nh, int nb, HWND hw = nullptr)
    {
        ASSERT(nb <= MAX_AMOUNT);

        rectArea.left = rectArea.top = 0;
        rectArea.right = nw;
        rectArea.bottom = nh;
        nAmount = nb;
        bStop = false;
        hwndTarget = hw;

        emEffectMan.Release();
        fExpLeftSecond = 0.f;
        if (bGenExp)
            fExpLeftSecond = float(rgRandom.GenInt(nExpGenTop, nExpGenBottom));
    }


    void SetConstraintRect(const CRect& rect, int nDef = 24)
    {
        rectConstraint = rect;
        if (nDef < 8)
            nDef = 8;
        if (nDef > 48)
            nDef = 48;
        rectConstraint.DeflateRect(nDef, nDef);
    }


//----------------------------------------------------------------------------------------------------------------------
/*
    void Pass(float deltaTime, float& tmCal, float& tmRender)
    {
        static CHiTimer timer;
        timer.Start();
        for (int i = 0; i < nAmount; i++)
        {
            // update position
            bodys[i].pos.x += bodys[i].velocity.x * deltaTime;
            bodys[i].pos.y += bodys[i].velocity.y * deltaTime;
            Reflect(bodys[i]);
        }
        // update velocity
        for (int i = 0; i < nAmount; i++)
        {
            for (int j = i + 1; j < nAmount; j++)
            {
                float r2 = bodys[i].pos.DistanceP2(bodys[j].pos);
                float rl2 = bodys[i].fCoreRadius + bodys[j].fCoreRadius;
                rl2 *= rl2;
                if (r2 < rl2)
                    r2 = rl2;
                float r3 = ::sqrtf(r2) * r2;
                float dx = bodys[j].pos.x - bodys[i].pos.x;
                float dy = bodys[j].pos.y - bodys[i].pos.y;
                float a1 = G * bodys[j].m / r3;
                float a2 = G * bodys[i].m / r3;
                float fax1 = a1 * dx * bodys[i].fAnti * bodys[j].fAnti;
                float fay1 = a1 * dy * bodys[i].fAnti * bodys[j].fAnti;
                float fax2 = -a2 * dx * bodys[i].fAnti * bodys[j].fAnti;
                float fay2 = -a2 * dy * bodys[i].fAnti * bodys[j].fAnti;
                bodys[i].velocity.x += fax1 * deltaTime;
                bodys[i].velocity.y += fay1 * deltaTime;
                bodys[j].velocity.x += fax2 * deltaTime;
                bodys[j].velocity.y += fay2 * deltaTime;
            }
        }
        tmCal = (float)timer.GetPassed();
        tmRender = 0.f;
        if (::IsWindow(hwndTarget))
        {
            timer.Start();
            ::SendMessage(hwndTarget, WM_USER_DRAW_WORLD, 0, 0);
            tmRender = (float)timer.GetPassed();
        }
    }
*/


//----------------------------------------------------------------------------------------------------------------------
    void Pass2(float deltaTime, float& tmCal)
    {
        static CHiTimer timer;
        timer.Start();
        // update velocity
        rdRawData.PassWorker(0, 0, nAmount - 1, deltaTime);
        tmCal = (float)timer.GetPassed();
        ::WaitForSingleObject(SThreadParams::hRenderEnd, INFINITE);
        // update position and reflection
        timer.Start();
        ComboOutput(false, deltaTime);
        tmCal += (float)timer.GetPassed();
        ::SetEvent(SThreadParams::hRenderStart);

        //tmRender = 0.f;
        //if (::IsWindow(hwndTarget))
        //{
        //    timer.Start();
        //    ::SendMessage(hwndTarget, WM_USER_DRAW_WORLD2, 0, 0);
        //    tmRender = (float)timer.GetPassed();
        //}
    }


//----------------------------------------------------------------------------------------------------------------------
    void Pass2_SIMD(float deltaTime, float& tmCal)
    {
        static CHiTimer timer;
        timer.Start();
        // update velocity
        rdRawData.PassWorker_SIMD(0, 0, nAmount - 1, deltaTime);
        tmCal = (float)timer.GetPassed();
        ::WaitForSingleObject(SThreadParams::hRenderEnd, INFINITE);
        // update position and reflection
        timer.Start();
        ComboOutput(false, deltaTime);
        tmCal += (float)timer.GetPassed();
        ::SetEvent(SThreadParams::hRenderStart);
        //tmRender = 0.f;
        //if (::IsWindow(hwndTarget))
        //{
        //    timer.Start();
        //    ::SendMessage(hwndTarget, WM_USER_DRAW_WORLD2, 0, 0);
        //    tmRender = (float)timer.GetPassed();
        //}
    }


//----------------------------------------------------------------------------------------------------------------------
    void Pass2_Grid(float deltaTime, float& tmCal)
    {
        static CHiTimer timer;
        timer.Start();
        // update velocity
        grid.Pass(rdRawData, deltaTime);
        // update position and reflection
        ComboOutput(false, deltaTime);
        tmCal = (float)timer.GetPassed();
        //tmRender = 0.f;
        //if (::IsWindow(hwndTarget))
        //{
        //    timer.Start();
        //    ::SendMessage(hwndTarget, WM_USER_DRAW_WORLD2, 0, 0);
        //    tmRender = (float)timer.GetPassed();
        //}
    }


//----------------------------------------------------------------------------------------------------------------------
/*
    SBody* Bodys(void)
    {
        return bodys;
    }
*/


//----------------------------------------------------------------------------------------------------------------------
    bool ShouldStop(void) const
    {
        return bStop;
    }


//----------------------------------------------------------------------------------------------------------------------
    void SetStop(bool b = true)
    {
        bStop = b;
        SThreadParams::bStop = b;
        ::SetEvent(SThreadParams::hRenderEnd);
    }


//----------------------------------------------------------------------------------------------------------------------
    int Amount(void) const
    {
        return nAmount;
    }


//----------------------------------------------------------------------------------------------------------------------
    CSize Size(void) const
    {
        return CSize(rectArea.Width(), rectArea.Width());
    }


//----------------------------------------------------------------------------------------------------------------------
    void SetTarget(HWND hw)
    {
        hwndTarget = hw;
    }


//----------------------------------------------------------------------------------------------------------------------
    void Run(void)
    {
        CHiTimer tmRun, tmPass;
        tmPass.Start();
//        int nms;
        int nStat = 0;
        float fCT = 0.f, fFT = 0.f;
        float fCTT = 0.f, fRTT = 0.f, fFTT = 0.f;
        ULONGLONG ullFrame = 0ull;
        CTimeCap tc;
        auto pPassFunc = &CWorld::Pass2;
        if (bGrid)
            pPassFunc = &CWorld::Pass2_Grid;
        else if (SThreadParams::bUsingSIMD)
            pPassFunc = &CWorld::Pass2_SIMD;
        float deltaTime;
        for (;;)
        {
            tmRun.Start();

            if (ShouldStop())
                break;
            deltaTime = float(tmPass.GetPassed(true));
            if (deltaTime > 0.5f || deltaTime < 0.0f)
                deltaTime = 0.5f;
            (this->*pPassFunc)(/*deltaTime*/SThreadParams::fFixedDeltaTime, fCT);
/*
            nms = int(tmRun.GetPassed() * 1000. + .5);
            nms = nFrameTimeMax - nms;
            if (nms > 1)
                ::Sleep(nms);
*/
            fFT = (float)tmRun.GetPassed();
            fCTT += fCT;
            fRTT += SThreadParams::fRenderTime;
            fFTT += fFT;
            nStat++;
            if (fFTT > 0.2f)
            {
                static SYSTEMTIME st;
                ullFrame += nStat;
                float fsr = float(nStat);
                nStat = 0;
                int nFPS = int(fsr / fFTT + .5f);
                int nCalTime = int(fCTT / fsr * 1000.f + .5f);
                int nRenTime = int(fRTT / fsr * 1000.f + .5f);
//                strStat.Format(_T("FPS: %d    Frames: %llu\nCaculate Time: %dms\nRender Time: %dms"), nFPS, ullFrame, nCalTime, nRenTime);
//                SThreadParams::strStat.Format(_T("FPS: %d    Frames: %llu\nCaculate Time: %dms\nRender Time: %dms\nNext explosion: %d"), nFPS, ullFrame, nCalTime, nRenTime, nExplosionCount);
                ::GetLocalTime(&st);
                SThreadParams::strStat.Format(
                    _T("Celestial bodies: %d\nTime: %02d:%02d:%02d    FPS: %d\nCaculate: %dms    Render: %dms\nNext explosion: %.1f"), 
                    nAmount, st.wHour, st.wMinute, st.wSecond, nFPS, nCalTime, nRenTime, fExpLeftSecond);
                fCTT = fRTT = fFTT = 0.f;
            }
        }
    }


//----------------------------------------------------------------------------------------------------------------------
    void ComboOutput(bool bMT, float fdt)
    {
        float fMeanVX, fMeanVY;
        float fdvx = 0.f, fdvy = 0.f;
        float fff = rdRawData.fG * fdt / 4.f;
        //CRect rect1 = rect;
        //rect1.DeflateRect(50, 50);
        PassAllEffects(fdt, EPassType::ptBeforeVelocity);
        for (int i = 0; i < nAmount; i++)
        {
            ForceField(i, rectConstraint, fff);
            fdvx = fdvy = 0.f;
            if (bMT)
            {
                for (int j = 0; j < 4; j++)
                {
                    fdvx += rdRawData.faThreadOutputDVX[j][i];
                    fdvy += rdRawData.faThreadOutputDVY[j][i];
                }
            }
            else
            {
                fdvx = rdRawData.faThreadOutputDVX[0][i];
                fdvy = rdRawData.faThreadOutputDVY[0][i];
            }
            fMeanVX = rdRawData.faVelocityX[i];
            fMeanVY = rdRawData.faVelocityY[i];
            fMeanVX += fdvx * SThreadParams::fDivergenceFactor;
            fMeanVY += fdvy * SThreadParams::fDivergenceFactor;
            rdRawData.faVelocityX[i] += fdvx;
            rdRawData.faVelocityY[i] += fdvy;

            rdRawData.faPosX[i] += fMeanVX * fdt;
            rdRawData.faPosY[i] += fMeanVY * fdt;

            //rdRawData.faPosX[i] += rdRawData.faVelocityX[i] * fdt;
            //rdRawData.faPosY[i] += rdRawData.faVelocityY[i] * fdt;

            // reflecting
//            Reflection(i, rect);

            PassEffect(i, fdt, EPassType::ptPosition);
        }
        GenEffect();
    }


//----------------------------------------------------------------------------------------------------------------------
    void PrepareGrid(const CRect& rect)
    {
        bGrid = true;
        grid.Init(rect, 100, 5);
    }

    float ExplosionLeftSecond(void) const
    {
        return fExpLeftSecond;
    }


    void SetEventFrequency(int nTF, int nEF)
    {
        int ntfs[4] = { 2048, 2048, 4096, 8192 };
        int nefs[4] = { 8, 8, 64, 128 };
        nTF = nTF % 4;
        nEF = nEF % 4;
        if (nTF == 0)
            bGenTrail = false;
        else
        {
            bGenTrail = true;
            nTrailGenTop = ntfs[nTF];
        }
        if (nEF == 0)
        {
            fExpLeftSecond = 0.f;
            bGenExp = false;
        }
        else
        {
            bGenExp = true;
            nExpGenTop = nefs[nEF];
            fExpLeftSecond = (float)rgRandom.GenInt(nExpGenTop, nExpGenBottom);
        }
    }
protected:
    CRect rectArea;
    CRect rectConstraint;
    int nAmount;
    //SBody bodys[MAX_AMOUNT];
    HWND hwndTarget{ nullptr };
    //float fFixedDeltaTime{ SThreadParams::fFixedDeltaTime }; // fixed delta time
    const int nFrameTimeMax{ 16 };
    bool bGrid{ false };
    CRandomGen rgRandom;
    CEffectManager emEffectMan;
    //int nExplosionCount;
    float fExpLeftSecond;
    bool bGenTrail{ true }, bGenExp{ true };
    int nTrailGenTop{ 2048 };  // frames
    int nExpGenTop{ 20 }, nExpGenBottom{ 1 };    // seconds


    int GenEffect(void)
    {
        static CHiTimer timer(true);
        int n = 0;
        // generate trail effects
        if (bGenTrail)
        {
            if (rgRandom.Probility(1, nTrailGenTop/* << emEffectMan.Size()*/))
            {
                int ni = rgRandom.GenInt(nAmount);
                if (!rdRawData.pEffects[ni])
                {
                    EStarType est = rdRawData.eaStarType[ni];
                    if (est == EStarType::stNormal || est == EStarType::stAnti_Normal/*est != EStarType::stBlackhole && est != EStarType::stAnti_Blackhole*/)
                    {
                        float fa = 0.4f;
                        if (rdRawData.naLooseLevel[ni] == 0)
                            fa = 0.1f;
                        CTrailEffect* pte = new CTrailEffect(ni, rdRawData.caColor[ni], rgRandom.Probility(1, 4),
                            rgRandom.GenInt(2048, 512), rdRawData.faRadius[ni], rgRandom.GenInt(192, 64), fa);
                        rdRawData.pEffects[ni] = pte;
                        emEffectMan.Add(pte);
                        n++;
                    }
                }
            }
        }
        // generate explosion effect
        if (bGenExp)
        {
            fExpLeftSecond -= float(timer.GetPassed(true));
            if (fExpLeftSecond <= 0.f)
            {
                // gen explosion effect
                int nec = 1;
                if (rgRandom.Probility(1u, 4u))
                {
                    nec++;
                    if (rgRandom.Probility(1u, 4u))
                        nec++;
                }
                for (int i = 0; i < nec; i++)
                {
                    if (GenExplosionEffect())
                        n++;
                }
                fExpLeftSecond = (float)rgRandom.GenInt(nExpGenTop/* << emEffectMan.Size()*/, nExpGenBottom);
            }
        }
        return n;
    }


//----------------------------------------------------------------------------------------------------------------------
    bool GenExplosionEffect(void)
    {
        bool bRes = false;
        for (int i = 0; i < 8; i++)
        {
            int ni = rgRandom.GenInt(nAmount);
            if (!rdRawData.pEffects[ni])
            {
                EStarType est = rdRawData.eaStarType[ni];
                if (est == EStarType::stNormal || est == EStarType::stAnti_Normal ||
                    est == EStarType::stGiant || est == EStarType::stAnti_Giant)
                {
                    float factor = 1.f;
                    if (est == EStarType::stNormal || est == EStarType::stAnti_Normal)
                    {
                        factor -= (7.5f - rdRawData.faRadius[ni]) * .15f;
                    }
                    else // giant star
                    {
                        factor = float(rdRawData.faRadius[ni]) / 7.5f;
                    }
                    //CExplosion* pee = new CExplosion(ni, rdRawData, /*200.f, */factor);
                    CEffect* pee = nullptr;
                    switch (rgRandom.GenInt(4))
                    {
                    case 0:
                        pee = new CExplosion(ni, rdRawData, factor);
                        break;
                    case 1:
                        pee = new CExplosion2(ni, rdRawData, 0.03f, factor);
                        break;
                    case 2:
                        pee = new CExplosion3(ni, rdRawData, 0.01f, factor);
                        break;
                    case 3:
                    default:
                        pee = new CExplosion4(ni, rdRawData, 0.01f, factor);
                        break;
                    }
                    rdRawData.pEffects[ni] = pee;
                    emEffectMan.Add(pee);
                    bRes = true;
                    break;
                }
            }
        }
        return bRes;
    }


    // return true: the effect passed
    bool PassEffect(int ni, float fdt, EPassType pt)
    {
        bool bRes = false;
        if (rdRawData.pEffects[ni])
        {
            CEffect* pe = reinterpret_cast<CEffect*>(rdRawData.pEffects[ni]);
            if (pe->PassType() == pt)
            {
                if (pe->Pass(rdRawData, fdt))
                {
                    emEffectMan.Remove(pe);
                    rdRawData.pEffects[ni] = nullptr;
                }
                bRes = true;
            }
        }
        return bRes;
    }


    void PassAllEffects(float fdt, EPassType pt)
    {
        typedef std::vector<CEffect*> EV;
        EV ev;
        for (auto it : emEffectMan.esEffects)
        {
            if (it->PassType() == pt)
            {
                if (it->Pass(rdRawData, fdt))
                    ev.push_back(it);
            }
        }
        for (auto it : ev)
        {
            rdRawData.pEffects[reinterpret_cast<CSingleEffect*>(it)->Index()] = nullptr;
            emEffectMan.Remove(it);
        }
    }


    float ElasticFactor(float v, float base)
    {
        v = ::fabsf(v);
        float f = v / (v + base);
        return (1.f - f * f);
    }


    void Reflection(int i, const CRect& rect)
    {
        float fli, fri, fti, fbi;
        if (rdRawData.bReflection)
        {
            fli = float(rect.left) + rdRawData.faRadius[i];
            fri = float(rect.right) - rdRawData.faRadius[i];
            fti = float(rect.top) + rdRawData.faRadius[i];
            fbi = float(rect.bottom) - rdRawData.faRadius[i];
            if (rdRawData.faPosX[i] < fli)
            {
                rdRawData.faPosX[i] = fli + fli - rdRawData.faPosX[i];
                rdRawData.faVelocityX[i] = -rdRawData.faVelocityX[i] * ElasticFactor(rdRawData.faVelocityX[i], rdRawData.fElasticFactorBase)/*rdRawData.faElasticFactor[i]*/;
            }
            else if (rdRawData.faPosX[i] > fri)
            {
                rdRawData.faPosX[i] = fri - rdRawData.faPosX[i] + fri;
                rdRawData.faVelocityX[i] = -rdRawData.faVelocityX[i] * ElasticFactor(rdRawData.faVelocityX[i], rdRawData.fElasticFactorBase)/*rdRawData.faElasticFactor[i]*/;
            }
            if (rdRawData.faPosY[i] < fti)
            {
                rdRawData.faPosY[i] = fti + fti - rdRawData.faPosY[i];
                rdRawData.faVelocityY[i] = -rdRawData.faVelocityY[i] * ElasticFactor(rdRawData.faVelocityY[i], rdRawData.fElasticFactorBase)/*rdRawData.faElasticFactor[i]*/;
            }
            else if (rdRawData.faPosY[i] > fbi)
            {
                rdRawData.faPosY[i] = fbi - rdRawData.faPosY[i] + fbi;
                rdRawData.faVelocityY[i] = -rdRawData.faVelocityY[i] * ElasticFactor(rdRawData.faVelocityY[i], rdRawData.fElasticFactorBase)/*rdRawData.faElasticFactor[i]*/;
            }
        }
    }


    void ForceField(int i, const CRect& rect, float fff)
    {
        float fli, fri, fti, fbi;
        fli = rdRawData.faPosX[i] - float(rect.left);
        fri = float(rect.right) - rdRawData.faPosX[i];
        fti = rdRawData.faPosY[i] - float(rect.top);
        fbi = float(rect.bottom) - rdRawData.faPosY[i];
        if (fli < 0.f || fri < 0.f || fti < 0.f || fbi < 0.f)
        {
            rdRawData.faVelocityX[i] *= ElasticFactor(rdRawData.faVelocityX[i], rdRawData.fElasticFactorBase);
            rdRawData.faVelocityY[i] *= ElasticFactor(rdRawData.faVelocityY[i], rdRawData.fElasticFactorBase);
            if (fli < 0.f)
            {
                rdRawData.faVelocityX[i] -= fff * fli;
            }
            else if (fri < 0.f)
            {
                rdRawData.faVelocityX[i] += fff * fri;
            }
            if (fti < 0.f)
            {
                rdRawData.faVelocityY[i] -= fff * fti;
            }
            else if (fbi < 0.f)
            {
                rdRawData.faVelocityY[i] += fff * fbi;
            }
        }
    }

//----------------------------------------------------------------------------------------------------------------------
/*
    int Reflect(SBody& b)
    {
        int n = 0;
        CRect rect = rectArea;
        rect.DeflateRect(b.nRadius, b.nRadius, b.nRadius, b.nRadius);
        bool br = false;
        for (;;)
        {
            br = false;
            if (b.pos.x < rect.left)
            {
                b.pos.x = rect.left + rect.left - b.pos.x;
                b.velocity.x = -b.velocity.x * b.fElasticFactor;
                br = true;
            }
            else if (b.pos.x > rect.right)
            {
                b.pos.x = rect.right - b.pos.x + rect.right;
                b.velocity.x = -b.velocity.x * b.fElasticFactor;
                br = true;
            }
            if (b.pos.y < rect.top)
            {
                b.pos.y = rect.top + rect.top - b.pos.y;
                b.velocity.y = -b.velocity.y * b.fElasticFactor;
                br = true;
            }
            else if (b.pos.y > rect.bottom)
            {
                b.pos.y = rect.bottom - b.pos.y + rect.bottom;
                b.velocity.y = -b.velocity.y * b.fElasticFactor;
                br = true;
            }
            if (!br)
                break;
            else
                n++;
        }
        return n;
    }
*/

/*
public:
    void Output(LPCTSTR strfn)
    {
        std::wofstream os;
        os.open(strfn, std::ios_base::out | std::ios_base::trunc);
        for (int i = 0; i < nAmount; i++)
        {
            os << _T("PosX: ") << bodys[i].pos.x << _T("\tPosY: ") << bodys[i].pos.y << L"\tVX: " << bodys[i].velocity.x << L"\tVY: " << bodys[i].velocity.y << std::endl;
        }
    }
*/
};


