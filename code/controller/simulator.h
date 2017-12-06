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

class TimerActionDelegate : public QObject {
Q_OBJECT
public:
    explicit TimerActionDelegate(Simulator *simulator, int key, int delay, int duration);
    void execute();
    void destroy();

protected Q_SLOTS:
    void doKeyPress();
    void doKeyRelease();

private:
    Simulator *m_simulator;
    int m_key;
    int m_delay;
    int m_duration;
};

#endif // SIMULATOR_H
