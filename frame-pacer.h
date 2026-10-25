#ifndef FRAME_PACER_H
#define FRAME_PACER_H
#include <windows.h>

// No catch-up bursts: each rendered frame starts at least 1/maxFps seconds later.
class FramePacer {
    LONGLONG frequency, interval, last = 0;
public:
    FramePacer(LONGLONG counterFrequency, int maxFps)
        : frequency(counterFrequency), interval((counterFrequency + maxFps - 1)/maxFps) {}
    void Reset(){last=0;}
    LARGE_INTEGER Wait(){
        LARGE_INTEGER now; QueryPerformanceCounter(&now);
        if(last){
            const LONGLONG target=last+interval;
            while(now.QuadPart<target){
                const double remainingMs=(double)(target-now.QuadPart)*1000/frequency;
                if(remainingMs>2.0) Sleep((DWORD)(remainingMs-1.0));
                else YieldProcessor();
                QueryPerformanceCounter(&now);
            }
        }
        last=now.QuadPart;
        return now;
    }
};
#endif
