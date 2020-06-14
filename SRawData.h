#pragma once

#include "framework.h"
#include <math.h>
#include <vector>
#include <memory>
#include "vecm.h"


//----------------------------------------------------------------------------------------------------------------------
struct SCell
{
    std::vector<int> viIndexes;
    bool bNormal, bAnti;
    SVec2 v2Center, v2CG, v2CG_Anti;   // centre of gravity, normal and anti
    float fGm{ 0.f }, fGm_Anti;  // total Gm, normal and anti

    void Reset(void)
    {
        viIndexes.clear();
        bNormal = bAnti = false;
        v2CG.x = v2CG.y = fGm = 0.f;
        v2CG_Anti.x = v2CG_Anti.y = fGm_Anti = 0.f;
    }

    void Put(int i, float x, float y, float gm, bool ba)
    {
        viIndexes.push_back(i);
        float f;
        if (ba)
        {
            f = fGm_Anti + gm;
            v2CG_Anti.x = (v2CG_Anti.x * fGm + x * gm) / f;
            v2CG_Anti.y = (v2CG_Anti.y * fGm + y * gm) / f;
            fGm_Anti = f;
            bAnti = true;
        }
        else
        {
            f = fGm + gm;
            v2CG.x = (v2CG.x * fGm + x * gm) / f;
            v2CG.y = (v2CG.y * fGm + y * gm) / f;
            fGm = f;
            bNormal = true;
        }
    }
};

typedef std::shared_ptr<SCell> SCell_Pointer;

enum class EStarType
{
    stNormal,
    stGiant,
    stBlackhole,
    stAnti_Normal,
    stAnti_Giant,
    stAnti_Blackhole
};


//----------------------------------------------------------------------------------------------------------------------
struct SRawData
{
    float fG{ 0.f };
    int nComm{ 0 }, nGiant{ 0 }, nBlackhole{ 0 }, nAntiComm{ 0 }, nAntiGiant{ 0 }, nAntiBlackhole{ 0 };
    int nAmount{ 0 };
    int nThreadCount{ 0 };
    EStarType* eaStarType{ nullptr };
    float* faRadius{ nullptr };
    float* faCoreRadius{ nullptr }; // alignas(32)
    float* faRadiusFactor{ nullptr };   // alignas(32)
    float* faGm{ nullptr }; // alignas(32) 
    int* naLooseLevel{ nullptr };   // loose level: 0: most loose, 1: middle hard, 2: most hard
    float* faPosX{ nullptr }; // alignas(32) 
    float* faPosY{ nullptr }; // alignas(32) 
    float* faVelocityX{ nullptr }; // alignas(32) 
    float* faVelocityY{ nullptr }; // alignas(32) 
//    float* faElasticFactor{ nullptr }; // alignas(32) 
    float* faAntiG{ nullptr }; // alignas(32) 
    float** faThreadOutputDVX{ nullptr }; // alignas(32), thread output here, delta velocity X
    float** faThreadOutputDVY{ nullptr }; // alignas(32), thread output here, delta velocity Y
    COLORREF* caColor{ nullptr };
    void** pEffects{ nullptr };
    bool bReflection{ true };
    

    void Prepare(int ncm, int ngs, int nbh, int nacm, int nags, int nabh, int ntc)
    {
        Release();
        nComm = ncm, nGiant = ngs, nBlackhole = nbh, nAntiComm = nacm, nAntiGiant = nags, nAntiBlackhole = nabh;
        nAmount = ncm + ngs + nbh + nacm + nags + nabh;
        nThreadCount = ntc;
        int nc = nAmount + 8;
        eaStarType = new EStarType[nc];
        faRadius = new float[nc];
        faCoreRadius = (float*)_aligned_malloc(sizeof(float) * nc, 32);
        faRadiusFactor = (float*)_aligned_malloc(sizeof(float) * nc, 32);
        faGm = (float*)_aligned_malloc(sizeof(float) * nc, 32);
        naLooseLevel = (int*)_aligned_malloc(sizeof(int) * nc, 32);
        faPosX = (float*)_aligned_malloc(sizeof(float) * nc, 32);
        faPosY = (float*)_aligned_malloc(sizeof(float) * nc, 32);
        faVelocityX = (float*)_aligned_malloc(sizeof(float) * nc, 32);
        faVelocityY = (float*)_aligned_malloc(sizeof(float) * nc, 32);
//        faElasticFactor = (float*)_aligned_malloc(sizeof(float) * nc, 32);
        faAntiG = (float*)_aligned_malloc(sizeof(float) * nc, 32);
        faThreadOutputDVX = new float* [nThreadCount];
        faThreadOutputDVY = new float* [nThreadCount];
        for (int i = 0; i < nThreadCount; i++)
        {
            faThreadOutputDVX[i] = (float*)_aligned_malloc(sizeof(float) * nc, 32);
            faThreadOutputDVY[i] = (float*)_aligned_malloc(sizeof(float) * nc, 32);
        }
        caColor = new COLORREF[nc];
        pEffects = new void* [nc];
        ::ZeroMemory(pEffects, sizeof(void*) * nc);
    }


    void Release(void)
    {
        if (eaStarType)
        {
            delete[] eaStarType;
            eaStarType = nullptr;
        }
        if (faRadius)
        {
            delete[] faRadius;
            faRadius = nullptr;
        }
        if (faCoreRadius)
        {
            _aligned_free(faCoreRadius);
            faCoreRadius = nullptr;
        }
        if (faRadiusFactor)
        {
            _aligned_free(faRadiusFactor);
            faRadiusFactor = nullptr;
        }
        if (faGm)
        {
            _aligned_free(faGm);
            faGm = nullptr;
        }
        if (naLooseLevel)
        {
            _aligned_free(naLooseLevel);
            naLooseLevel = nullptr;
        }
        if (faPosX)
        {
            _aligned_free(faPosX);
            faPosX = nullptr;
        }
        if (faPosY)
        {
            _aligned_free(faPosY);
            faPosY = nullptr;
        }
        if (faVelocityX)
        {
            _aligned_free(faVelocityX);
            faVelocityX = nullptr;
        }
        if (faVelocityY)
        {
            _aligned_free(faVelocityY);
            faVelocityY = nullptr;
        }
/*
        if (faElasticFactor)
        {
            _aligned_free(faElasticFactor);
            faElasticFactor = nullptr;
        }
*/
        if (faAntiG)
        {
            _aligned_free(faAntiG);
            faAntiG = nullptr;
        }
        if (faThreadOutputDVX)
        {
            for (int i = 0; i < nThreadCount; i++)
            {
                if (faThreadOutputDVX[i])
                    _aligned_free(faThreadOutputDVX[i]);
            }
            delete[] faThreadOutputDVX;
            faThreadOutputDVX = nullptr;
        }
        if (faThreadOutputDVY)
        {
            for (int i = 0; i < nThreadCount; i++)
            {
                if (faThreadOutputDVY[i])
                    _aligned_free(faThreadOutputDVY[i]);
            }
            delete[] faThreadOutputDVY;
            faThreadOutputDVY = nullptr;
        }
        if (caColor)
        {
            delete[] caColor;
            caColor = nullptr;
        }
        if (pEffects)
        {
            delete[] pEffects;
            pEffects = nullptr;
        }
    }

    ~SRawData()
    {
        Release();
    }


    void Padding(int idx)
    {
        ASSERT(idx >= 0 && idx < nAmount);

        float fef_comm = 0.98f, fef_gs = 0.97f, fef_bh = 0.98f, fef_loose = 0.97f;	// elastic factor
        float fcrf_comm = 0.95f, fcrf_gs = 0.85f, fcrf_bh = 0.95f, fcrf_loose = 0.9f;	// core radius factor for giant star

        switch (eaStarType[idx])
        {
        case EStarType::stGiant:
        case EStarType::stAnti_Giant:
            faCoreRadius[idx] = faRadius[idx] * fcrf_gs;
            faRadiusFactor[idx] = 1.f;
//            faElasticFactor[idx] = fef_gs;
            break;
        case EStarType::stBlackhole:
        case EStarType::stAnti_Blackhole:
            faCoreRadius[idx] = faRadius[idx] * fcrf_bh;
            faRadiusFactor[idx] = 0.01f;
//            faElasticFactor[idx] = fef_bh;
            break;
        case EStarType::stNormal:
        case EStarType::stAnti_Normal:
        default:
            faCoreRadius[idx] = faRadius[idx] * (naLooseLevel[idx] == 0 ? fcrf_loose : fcrf_comm);
            faRadiusFactor[idx] = 1.f;
//            faElasticFactor[idx] = fef_comm;
            break;
        }
        faAntiG[idx] = eaStarType[idx] < EStarType::stAnti_Normal ? 1.f : -1.f;
    }


    void PassWorker(int ni, int nJobFrom, int nJobTo, float fDeltaTime)
    {
        float* pDVX = faThreadOutputDVX[ni];
        float* pDVY = faThreadOutputDVY[ni];
        // below need to optimize
        ::ZeroMemory(pDVX, sizeof(float) * nAmount);
        ::ZeroMemory(pDVY, sizeof(float) * nAmount);

        for (int i = nJobFrom; i < nJobTo; i++)
        {
            for (int j = i + 1; j < nAmount; j++)
            {
                float dx = faPosX[j] - faPosX[i];
                float dy = faPosY[j] - faPosY[i];
                float r2 = dx * dx + dy * dy;
                float rl2 = faCoreRadius[i] + faCoreRadius[j];
                rl2 = rl2 * rl2 * faRadiusFactor[i] * faRadiusFactor[j];
                if (r2 < rl2)
                    r2 = rl2;
                float r3 = ::sqrtf(r2) * r2;
                float a1 = faGm[j] / r3;
                float a2 = faGm[i] / r3;
                float fAnti = faAntiG[i] * faAntiG[j];
                float fax1 = a1 * dx * fAnti;
                float fay1 = a1 * dy * fAnti;
                float fax2 = -a2 * dx * fAnti;
                float fay2 = -a2 * dy * fAnti;
                pDVX[i] += fax1 * fDeltaTime;
                pDVY[i] += fay1 * fDeltaTime;
                pDVX[j] += fax2 * fDeltaTime;
                pDVY[j] += fay2 * fDeltaTime;
            }
        }
    }


    void PassWorker_SIMD(int ni, int nJobFrom, int nJobTo, float fDeltaTime)
    {
        float* pDVX = faThreadOutputDVX[ni];
        float* pDVY = faThreadOutputDVY[ni];
        // below need to optimize
        ::ZeroMemory(pDVX, sizeof(float) * nAmount);
        ::ZeroMemory(pDVY, sizeof(float) * nAmount);

        __m256 mDT = _mm256_set1_ps(fDeltaTime), mAG;
        __m256 mdx, mdy, mr3, mdvxi, mdvyi, mdvxj, mdvyj;
        __m256 mv1, mv2, mv3, mv4;
        for (int i = nJobFrom; i < nJobTo; i++)
        {
            for (int j = i + 1; j < nAmount; j += 8)
            {
                mAG = _mm256_mul_ps(_mm256_set1_ps(faAntiG[i]), _mm256_loadu_ps(&faAntiG[j]));
                mdx = _mm256_sub_ps(_mm256_loadu_ps(&faPosX[j]), _mm256_set1_ps(faPosX[i]));
                mdy = _mm256_sub_ps(_mm256_loadu_ps(&faPosY[j]), _mm256_set1_ps(faPosY[i]));
                mv1 = _mm256_add_ps(_mm256_mul_ps(mdx, mdx), _mm256_mul_ps(mdy, mdy));  // mv1 = r^2
                mv2 = _mm256_add_ps(_mm256_set1_ps(faCoreRadius[i]), _mm256_load_ps(&faCoreRadius[j])); // mv2 = rl
                mv3 = _mm256_mul_ps(mv2, mv2);  // mv3 = rl^2
                mv2 = _mm256_mul_ps(_mm256_set1_ps(faRadiusFactor[i]), _mm256_load_ps(&faRadiusFactor[j]));
                mv4 = _mm256_mul_ps(mv3, mv2);
                mv2 = _mm256_max_ps(mv1, mv4);  // mv2 = r^2
                mr3 = _mm256_mul_ps(mv2, _mm256_sqrt_ps(mv2));
                mv1 = _mm256_div_ps(_mm256_loadu_ps(&faGm[j]), mr3);    // mv1 = a1
                mv2 = _mm256_div_ps(_mm256_set1_ps(-faGm[i]), mr3);  // mv2 = -a2
                mdvxi = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(mv1, mdx), mAG), mDT);
                mdvyi = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(mv1, mdy), mAG), mDT);
                mdvxj = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(mv2, mdx), mAG), mDT);
                mdvyj = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(mv2, mdy), mAG), mDT);
                _mm256_store_ps(&pDVX[j], _mm256_add_ps(mdvxj, _mm256_loadu_ps(&pDVX[j])));
                _mm256_store_ps(&pDVY[j], _mm256_add_ps(mdvyj, _mm256_loadu_ps(&pDVY[j])));
                for (int k = 0; k < 8 && j + k < nAmount; k++)
                {
                    pDVX[i] += mdvxi.m256_f32[k];
                    pDVY[i] += mdvyi.m256_f32[k];
                }
            }
        }
    }


    void PassCellInternal(const SCell& cell, float fDeltaTime)
    {
        int ii, ij;
        float* pDVX = faThreadOutputDVX[0], * pDVY = faThreadOutputDVY[0];
        for (int m = 0; m < cell.viIndexes.size(); m++)
        {
            ii = cell.viIndexes[m];
            for (int n = m + 1; n < cell.viIndexes.size(); n++)
            {
                ij = cell.viIndexes[n];
                Pass2Bodies(ii, ij, 0, fDeltaTime);
            }
        }
    }


    void PassInterCells(const SCell& cell1, const SCell& cell2, float fDeltaTime, bool bFar)
    {
        if (bFar)   // simplified 
        {
            for (int m = 0; m < cell1.viIndexes.size(); m++)
            {
                PassCellBody(cell2, cell1.viIndexes[m], 0, fDeltaTime);
            }
            for (int m = 0; m < cell2.viIndexes.size(); m++)
                PassCellBody(cell1, cell2.viIndexes[m], 0, fDeltaTime);
        }
        else
        {
            for (int m = 0; m < cell1.viIndexes.size(); m++)
            {
                for (int n = 0; n < cell2.viIndexes.size(); n++)
                {
                    Pass2Bodies(cell1.viIndexes[m], cell2.viIndexes[n], 0, fDeltaTime);
                }
            }
        }
    }


    //----------------------------------------------------------------------------------------------------------------------
    void Pass2Bodies(int i1, int i2, int nST, float fDeltaTime)
    {
        float dx = faPosX[i2] - faPosX[i1];
        float dy = faPosY[i2] - faPosY[i1];
        float r2 = dx * dx + dy * dy;
        float rl2 = faCoreRadius[i1] + faCoreRadius[i2];
        rl2 *= rl2;
        if (r2 < rl2)
            r2 = rl2;
        float r3 = ::sqrtf(r2) * r2;
        float a1 = faGm[i2] / r3;
        float a2 = faGm[i1] / r3;
        float fAnti = faAntiG[i1] * faAntiG[i2];
        float fax1 = a1 * dx * fAnti;
        float fay1 = a1 * dy * fAnti;
        float fax2 = -a2 * dx * fAnti;
        float fay2 = -a2 * dy * fAnti;
        faThreadOutputDVX[nST][i1] += fax1 * fDeltaTime;
        faThreadOutputDVY[nST][i1] += fay1 * fDeltaTime;
        faThreadOutputDVX[nST][i2] += fax2 * fDeltaTime;
        faThreadOutputDVY[nST][i2] += fay2 * fDeltaTime;
    }


    //----------------------------------------------------------------------------------------------------------------------
    void PassCellBody(const SCell& cell, int ib, int nST, float fDeltaTime)
    {
        float dx, dy, r3, fAnti, a2, fax2, fay2;
        if (cell.bNormal)
        {
            dx = faPosX[ib] - cell.v2CG.x;
            dy = faPosY[ib] - cell.v2CG.y;
            r3 = dx * dx + dy * dy;
            r3 = ::sqrtf(r3) * r3;
            fAnti = faAntiG[ib];
            a2 = cell.fGm / r3;
            fax2 = -a2 * dx * fAnti;
            fay2 = -a2 * dy * fAnti;
            faThreadOutputDVX[nST][ib] += fax2 * fDeltaTime;
            faThreadOutputDVY[nST][ib] += fay2 * fDeltaTime;
        }
        if (cell.bAnti)
        {
            dx = faPosX[ib] - cell.v2CG_Anti.x;
            dy = faPosY[ib] - cell.v2CG_Anti.y;
            r3 = dx * dx + dy * dy;
            r3 = ::sqrtf(r3) * r3;
            fAnti = -faAntiG[ib];
            a2 = cell.fGm_Anti / r3;
            fax2 = -a2 * dx * fAnti;
            fay2 = -a2 * dy * fAnti;
            faThreadOutputDVX[nST][ib] += fax2 * fDeltaTime;
            faThreadOutputDVY[nST][ib] += fay2 * fDeltaTime;
        }
    }

//----------------------------------------------------------------------------------------------------------------------
/*
    void Output(LPCTSTR strfn, int nc)
    {
        std::wofstream os;
        os.open(strfn, std::ios_base::out | std::ios_base::trunc);
        for (int i = 0; i < nc; i++)
        {
            os << _T("PosX: ") << faPosX[i] << _T("\tPosY: ") << faPosY[i] << L"\tVX: " << faVelocityX[i] << L"\tVY: " << faVelocityY[i] << std::endl;
        }
    }
*/
};
