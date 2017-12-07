#ifndef MINOTAUR_CPP_RENDERSCENEBASE_H
#define MINOTAUR_CPP_RENDERSCENEBASE_H

// Magnetic permeability of free space
#define MU_0F ((float) M_PI * 4e-7f)
// Coefficient of static friction
#define MU_SF 0.3f
// Gravitational field strength
#define G_FIELD 9.809f

#include "drawable.h"

class Solenoid;
class SAMRobot;

/**
 * Base render scene referenced by Drawables to
 * break include cycles on the subclass RenderScene.
 */
class RenderSceneBase {
public:
    typedef typename Drawable::vector2f vector2f;

    virtual vector2f center() const = 0;
    virtual const std::vector<Solenoid> *solenoids() const = 0;
    virtual SAMRobot *sam() = 0;
    virtual void powerVertical(double current) = 0;
    virtual void powerHorizontal(double current) = 0;
    virtual void suspendControls(bool suspend) = 0;
    virtual void requestVelocity(vector2f v2, int time_step) = 0;
};

#endif //MINOTAUR_CPP_RENDERSCENEBASE_H
