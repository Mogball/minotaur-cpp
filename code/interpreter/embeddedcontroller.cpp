#include "embeddedcontroller.h"

EmbeddedController::EmbeddedController() :
        m_controller_ptr(nullptr) {}

void EmbeddedController::bind_controller(std::shared_ptr<Controller> *controller_ptr) {
    this->m_controller_ptr = controller_ptr;
}

bool EmbeddedController::send_movement(Vector2i &move_vector, int dt) {
    // Forward movement to the currently selected controller
    if (!m_controller_ptr || !*m_controller_ptr) { return false; }
    (*m_controller_ptr)->move(move_vector, dt);
    return true;
}

bool EmbeddedController::send_actuation(int actuator, int duration, int delay) {
    if (!m_controller_ptr || !*m_controller_ptr) { return false; }
    (*m_controller_ptr)->actuate(actuator, duration, delay);
    return true;
}

bool EmbeddedController::send_movement_to(int x, int y, int duration, int delay) {
    if (!m_controller_ptr || !*m_controller_ptr) { return false; }
    (*m_controller_ptr)->moveTo(x, y, duration, delay);
    return true;
}
