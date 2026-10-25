#ifndef WEAPON_ATTACHMENT_H
#define WEAPON_ATTACHMENT_H
#include <cmath>
// Match the weapon's local translation, pitch, yaw, and world translation.
// Simulation positions must never depend on a graphics context or viewport.
inline void WeaponAttachmentPosition(float px,float py,float pz,float mx,float my,float mz,
                                     float yaw,float pitch,float* x,float* y,float* z) {
    const float localY=my*std::cos(pitch)-mz*std::sin(pitch);
    const float localZ=my*std::sin(pitch)+mz*std::cos(pitch);
    *x=px+mx*std::cos(yaw)+localZ*std::sin(yaw);
    *y=py+localY;
    *z=pz-mx*std::sin(yaw)+localZ*std::cos(yaw);
}
#endif
