#pragma once

 #define WIN32_LEAN_AND_MEAN
 #include <windows.h>


class CHiTimer
{
public:
    CHiTimer(bool bStart = false)
    {
        LARGE_INTEGER liFreq;
        ::QueryPerformanceFrequency(&liFreq);
        dfFreq = double(liFreq.QuadPart);
        if (bStart)
            Start();
    }


    void Start(void)
    {
        ::QueryPerformanceCounter(&liCount);
    }

    // return passed second
    double GetPassed(bool bReset = false)
    {
        static LARGE_INTEGER li;
        ::QueryPerformanceCounter(&li);
        double sec = double(li.QuadPart - liCount.QuadPart) / dfFreq;
        if(bReset)
            liCount = li;
        return sec;
    }
protected:
    LARGE_INTEGER liCount;
    double dfFreq;
};


