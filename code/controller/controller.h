#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <unordered_map>
#include "../utility/logger.h"
#include "../utility/vector2i.h"

#define STEP_TIME    10

class Controller {
public:
    enum Mode {
        EMMA,
        SAM,

        NUM_MODES
    };

    enum Type {
        ACTUATOR,
        SIMULATOR,

        NUM_DELGATES
    };

    enum Dir {
        UP,        // y > 0
        DOWN,    // y < 0
        RIGHT,    // x > 0
        LEFT,    // x < 0

        NUM_DIRS
    };

    enum Axis {
        X,
        Y,

        NUM_AXES
    };

    // Common robot functions
    static Vector2i toVec2(Dir);
    void invertAxis(Axis);

    void move(Dir dir, int timer = STEP_TIME);
    void move(Vector2i dir, int timer = STEP_TIME);
    void moveTo(int x, int y, int duration = 1000, int delay = 0);
    void actuate(int actuator, int duration = 1000, int delay = 0);

    void suspendKeyboard(bool suspend);
    void keyPressed(int key);
    void keyReleased(int key);
    bool isKeyDown(int key);

protected:
    typedef typename std::unordered_map<int, bool> key_map;
    typedef typename std::pair<int, bool> key_press;

    key_map m_key_map{50};
    bool m_suspend_keyboard = false;

    int m_invert_x, m_invert_y; // +1 for no inversion in the axis, -1 otherwise

    Controller(int t_invert_x, int t_invert_y);

    virtual void performMove(Vector2i vec, int timer) = 0;
    virtual void performActuation(int actuator, int duration, int delay) = 0;
    virtual void performMoveTo(double x, double y, int duration, int delay);
};

#endif // CONTROLLER_H
