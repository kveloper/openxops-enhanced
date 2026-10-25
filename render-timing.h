#ifndef RENDER_TIMING_H
#define RENDER_TIMING_H
#include <cmath>
#include <cstddef>
inline float RenderInterpolationAlpha = 1.0f;
class FixedTickClock {
    double pending=0;
public:
    void Reset(){pending=0;}
    void Advance(double seconds){if(seconds>0)pending+=seconds;}
    bool Consume(){if(pending+1e-10<0.030)return false;pending-=0.030;if(pending<0)pending=0;return true;}
    float Alpha() const {return (float)(pending>=0.030?1.0:pending/0.030);}
};
template<std::size_t N> struct RenderPoseHistory {
    float previous[N]={}; bool valid=false;
    void Capture(const float* values){for(std::size_t i=0;i<N;i++)previous[i]=values[i];valid=true;}
    void Blend(float* values,float alpha) const {
        if(!valid)return;
        float distance=0;for(int i=0;i<3;i++)distance+=(values[i]-previous[i])*(values[i]-previous[i]);
        if(distance>10000)return; // Teleports and scene changes must snap.
        for(std::size_t i=0;i<N;i++){
            float delta=values[i]-previous[i];
            if(i>=3)delta=std::remainder(delta,6.28318530718f);
            values[i]=previous[i]+delta*alpha;
        }
    }
};
#endif
