#include "simulator.h"

Simulator::Simulator(int t_invert_x, int t_invert_y) :
        Controller(t_invert_x, t_invert_y) {
}

void Simulator::performMove(Vector2i vec, int timer) {

}

void Simulator::performActuation(int actuator, int duration, int delay) {
    int actuator_keys[] = {Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down};
    if (actuator < 0) { actuator = 0; }
    else if (actuator > 3) { actuator = 3; }

}
