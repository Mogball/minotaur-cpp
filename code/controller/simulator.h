#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QThread>

#include "controller.h"
#include "../utility/vector2i.h"
#include "../simulator/renderscenebase.h"

class SimulatorScene;

class Simulator : public Controller {
public:
    explicit Simulator(int t_invert_x = 1, int t_invert_y = 1);

    void setRenderScene(RenderSceneBase *render_scene);
    RenderSceneBase *getRenderScene();

    void setProportionConstant(double K_p);
    void setIntegralConstant(double K_i);
    void setDerivativeConstant(double K_d);

protected:
    void performMove(Vector2i vec, int timer) override;
    void performMoveTo(double x, double y, int duration, int delay) override;
    void performActuation(int actuator, int duration, int delay) override;

private:
    RenderSceneBase *m_render_scene;

    double m_K_p = 10.0;
    double m_K_i = 0.0;
    double m_K_d = 0.0;
};

// Delegates are responsible for destroying themselves
// after their tasks are complete
class ActuateDelegate : public QObject {
Q_OBJECT
public:
    explicit ActuateDelegate(Simulator *simulator, int key, int delay, int duration);
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

class MoveToDelegate : public QObject {
Q_OBJECT
public:
    explicit MoveToDelegate(
            Simulator *simulator, double x, double y, int delay, int duration,
            double K_p, double K_i, double K_d);
    void execute();

protected Q_SLOTS:
    void startControl();
    void start();
    void destroy();

private:
    QThread m_thread;

    Simulator *m_simulator;
    double m_target_x;
    double m_target_y;
    double m_curr_x;
    double m_curr_y;
    double m_err_x;
    double m_err_y;
    double m_integral_x;
    double m_integral_y;
    double m_deriv_x;
    double m_deriv_y;
    int m_time_elapsed;
    int m_delay;
    int m_duration;

    double m_K_p;
    double m_K_i;
    double m_K_d;
};

#endif // SIMULATOR_H
