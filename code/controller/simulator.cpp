#include "simulator.h"
#include "../simulator/sam.h"

#include <QTimer>

Simulator::Simulator(int t_invert_x, int t_invert_y) :
        Controller(t_invert_x, t_invert_y) {
}

void Simulator::setRenderScene(RenderSceneBase *render_scene) {
    m_render_scene = render_scene;
}

RenderSceneBase *Simulator::getRenderScene() {
    return m_render_scene;
}

void Simulator::setProportionConstant(double K_p) {
    m_K_p = K_p;
}

void Simulator::setIntegralConstant(double K_i) {
    m_K_i = K_i;
}

void Simulator::setDerivativeConstant(double K_d) {
    m_K_d = K_d;
}

void Simulator::performMove(Vector2i vec, int timer) {
}

void Simulator::performActuation(int actuator, int duration, int delay) {
#ifndef NDEBUG
    Logger::log(
            "Actuating [" + std::to_string(actuator) +
            " ms] for [" + std::to_string(duration) +
            "] after [" + std::to_string(delay) + " ms]",
            Logger::DEBUG
    );
#endif

    int actuator_keys[] = {Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down};
    if (actuator < 0) { actuator = 0; }
    else if (actuator > 3) { actuator = 3; }
    auto *delegate = new ActuateDelegate(this, actuator_keys[actuator], delay, duration);
    delegate->execute();
}

void Simulator::performMoveTo(double x, double y, int duration, int delay) {
    auto *delegate = new MoveToDelegate(this, x, y, delay, duration, m_K_p, m_K_i, m_K_d);
    delegate->execute();
}

ActuateDelegate::ActuateDelegate(Simulator *simulator, int key, int delay, int duration)
        : m_simulator(simulator),
          m_key(key),
          m_delay(delay),
          m_duration(duration) {}

void ActuateDelegate::execute() {
    if (m_delay > 0) {
        QTimer::singleShot(m_delay, this, SLOT(doKeyPress()));
    } else {
        doKeyPress();
    }
}

void ActuateDelegate::destroy() {
    delete this;
}

void ActuateDelegate::doKeyPress() {
    m_simulator->keyPressed(m_key);
    if (m_duration > 0) {
        QTimer::singleShot(m_duration, this, SLOT(doKeyRelease()));
    } else {
        doKeyRelease();
    }
}

void ActuateDelegate::doKeyRelease() {
    m_simulator->keyReleased(m_key);
    destroy();
}

MoveToDelegate::MoveToDelegate(
        Simulator *simulator, double x, double y, int delay, int duration,
        double K_p, double K_i, double K_d)
        : m_thread(this),
          m_simulator(simulator),
          m_target_x(x),
          m_target_y(y),
          m_delay(delay),
          m_duration(duration),
          m_curr_x(simulator->getRenderScene()->sam()->pos().x()),
          m_curr_y(simulator->getRenderScene()->sam()->pos().y()),
          m_err_x(m_target_x - m_curr_x),
          m_err_y(m_target_y - m_curr_y),
          m_integral_x(0),
          m_integral_y(0),
          m_deriv_x(0),
          m_deriv_y(0),
          m_time_elapsed(0),
          m_K_p(K_p),
          m_K_i(K_i),
          m_K_d(K_d) {}

void MoveToDelegate::execute() {
    if (m_delay > 0) {
        QTimer::singleShot(m_delay, this, SLOT(start()));
    } else {
        start();
    }
}

void MoveToDelegate::destroy() {
    delete this;
}

void MoveToDelegate::start() {
    moveToThread(&m_thread);
    connect(&m_thread, SIGNAL(started()), this, SLOT(startControl()));
    connect(&m_thread, SIGNAL(finished()), this, SLOT(destroy()));
    m_thread.start();
}

void MoveToDelegate::startControl() {
#ifndef NDEBUG
    Logger::debug("Starting move_to control (" + std::to_string(m_target_x) + ", " + std::to_string(m_target_y) + ")");
#endif
    long long int time_now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
    ).count();
    m_simulator->getRenderScene()->suspendControls(true);
    m_simulator->getRenderScene()->powerHorizontal(0);
    m_simulator->getRenderScene()->powerVertical(0);
    double K_p = m_K_p;
    double K_i = m_K_i;
    double K_d = m_K_d;
    double amp = 1000;
    long long int time_start = time_now;
    unsigned long time_step = 10;
    while (m_time_elapsed < m_duration) {
        double u_x = K_p * m_err_x + K_i * m_integral_x + K_d + m_deriv_x;
        double u_y = K_p * m_err_y + K_i * m_integral_y + K_d + m_deriv_y;
        m_simulator->getRenderScene()->powerHorizontal(u_x * amp);
        m_simulator->getRenderScene()->powerVertical(u_y * amp);
        QThread::msleep(time_step);
        m_curr_x = m_simulator->getRenderScene()->sam()->pos().x();
        m_curr_y = m_simulator->getRenderScene()->sam()->pos().y();
        double err_x = m_curr_x - m_target_x;
        double err_y = m_curr_y - m_target_y;
        time_now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();
        long long int dt = time_now - time_start;
        time_start = time_now;
        double delta_time = dt / 1000.0;
        m_integral_x += err_x * delta_time;
        m_integral_y += err_y * delta_time;
        m_deriv_x = (err_x - m_err_x) / delta_time;
        m_deriv_y = (err_y - m_err_y) / delta_time;
        m_err_x = err_x;
        m_err_y = err_y;
        m_time_elapsed += dt;
    }
    m_simulator->getRenderScene()->powerHorizontal(0);
    m_simulator->getRenderScene()->powerVertical(0);
    m_simulator->getRenderScene()->suspendControls(false);
#ifndef NDEBUG
    Logger::debug("Completed move_to command");
#endif
}
