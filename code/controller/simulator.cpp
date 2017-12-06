#include "simulator.h"

#include <QTimer>

Simulator::Simulator(int t_invert_x, int t_invert_y) :
        Controller(t_invert_x, t_invert_y) {
}

void Simulator::performMove(Vector2i vec, int timer) {
}

void Simulator::performActuation(int actuator, int duration, int delay) {
    int actuator_keys[] = {Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down};
    if (actuator < 0) { actuator = 0; }
    else if (actuator > 3) { actuator = 3; }
    auto *delegate = new TimerActionDelegate(this, actuator_keys[actuator], delay, duration);
    delegate->execute();
}

TimerActionDelegate::TimerActionDelegate(Simulator *simulator, int key, int delay, int duration)
        : m_simulator(simulator),
          m_key(key),
          m_delay(delay),
          m_duration(duration) {}

void TimerActionDelegate::execute() {
    if (m_delay > 0) {
        QTimer::singleShot(m_delay, this, SLOT(doKeyPress()));
    } else {
        doKeyPress();
    }
}

void TimerActionDelegate::destroy() {
    delete this;
}

void TimerActionDelegate::doKeyPress() {
    m_simulator->keyPressed(m_key);
    if (m_duration > 0) {
        QTimer::singleShot(m_duration, this, SLOT(doKeyRelease()));
    } else {
        doKeyRelease();
    }
}

void TimerActionDelegate::doKeyRelease() {
    m_simulator->keyReleased(m_key);
    destroy();
}
