#include "pch.h"
#include "CWorld.h"


const UINT WM_USER_DRAW_WORLD = WM_USER + 1;
const UINT WM_USER_DRAW_WORLD2 = WM_USER + 2;

int SThreadParams::nWorkerCount{ 3 };
SRawData* SThreadParams::pRawData{ nullptr };
int SThreadParams::nJobSegs[MAX_THREADS_NUMBER + 1];
bool SThreadParams::bStop{ false };
HANDLE SThreadParams::hStarts[MAX_THREADS_NUMBER - 1]{ nullptr };
HANDLE SThreadParams::hEnds[MAX_THREADS_NUMBER - 1]{ nullptr };
HANDLE SThreadParams::hRenderStart{ nullptr }, SThreadParams::hRenderEnd{ nullptr };
float SThreadParams::fDeltaTime{ 0.f };
float SThreadParams::fFixedDeltaTime{ 0.015f };
float SThreadParams::fDivergenceFactor{ 0.995f };
float SThreadParams::fRenderTime{ 0.f };
bool SThreadParams::bUsingSIMD{ false };

CRect SThreadParams::rectConstraint;
HWND SThreadParams::hwndTarget;
CString SThreadParams::strStat;
