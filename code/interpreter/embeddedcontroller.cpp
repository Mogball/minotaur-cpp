#include "embeddedcontroller.h"

#include "../controller/simulator.h"
#include "../simulator/renderscene.h"
#include "../simulator/sam.h"

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

bool EmbeddedController::reset_simulator() {
    if (!m_controller_ptr || !*m_controller_ptr) { return false; }
    auto p_simulator = std::dynamic_pointer_cast<Simulator>(*m_controller_ptr);
    if (p_simulator != nullptr) {
        p_simulator->getRenderScene()->sam()->reset();
        return true;
    }
    return false;
}

bool EmbeddedController::set_proportion(double K_p) {
    if (!m_controller_ptr || !*m_controller_ptr) { return false; }
    auto p_simulator = std::dynamic_pointer_cast<Simulator>(*m_controller_ptr);
    if (p_simulator != nullptr) {
        p_simulator->setProportionConstant(K_p);
        return true;
    }
    return false;
}

bool EmbeddedController::set_integral(double K_i) {
    if (!m_controller_ptr || !*m_controller_ptr) { return false; }
    auto p_simulator = std::dynamic_pointer_cast<Simulator>(*m_controller_ptr);
    if (p_simulator != nullptr) {
        p_simulator->setIntegralConstant(K_i);
        return true;
    }
    return false;
}

bool EmbeddedController::set_derivative(double K_d) {
    if (!m_controller_ptr || !*m_controller_ptr) { return false; }
    auto p_simulator = std::dynamic_pointer_cast<Simulator>(*m_controller_ptr);
    if (p_simulator != nullptr) {
        p_simulator->setDerivativeConstant(K_d);
        return true;
    }
    return false;
}

std::tuple<float, float> EmbeddedController::cal_currents(float v2x, float v2y) {
    if (!m_controller_ptr || !*m_controller_ptr) { std::make_tuple(0, 0); }
    auto p_simulator = std::dynamic_pointer_cast<Simulator>(*m_controller_ptr);
    if (p_simulator != nullptr) {
        return dynamic_cast<RenderScene *>(p_simulator->getRenderScene())->calculateCurrents(RenderScene::vector2f(v2x, v2y), 10);
    }
    return std::make_tuple(0, 0);
}
