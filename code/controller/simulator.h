#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "controller.h"
#include "../utility/vector2i.h"

class SimulatorScene;

/**
 *
 */
class Simulator : public Controller {
public:
    explicit Simulator(int t_invert_x = 1, int t_invert_y = 1);

protected:
    void performMove(Vector2i vec, int timer) override;

    void performActuation(int actuator, int duration, int delay) override;
};

class TimerDelegate : public QObject {
Q_OBJECT

};

#endif // SIMULATOR_H
