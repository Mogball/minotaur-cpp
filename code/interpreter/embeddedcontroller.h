#ifndef MINOTAUR_CPP_EMBEDDEDCONTROLLER_H
#define MINOTAUR_CPP_EMBEDDEDCONTROLLER_H

#undef slots
#define _hypot hypot // hack to fix a bug with CPython
#include <Python.h>

#include <memory>

#include "../controller/controller.h"

// Singleton controller instance
class EmbeddedController {
public:
    static EmbeddedController &getInstance() {
        static EmbeddedController controller;
        return controller;
    }
private:
    // The shared pointer that indicates which controller is active
    // so that commands issued from the engine can be forwarded
    std::shared_ptr<Controller> *m_controller_ptr;

    EmbeddedController();

public:
    // Singleton
    EmbeddedController(EmbeddedController const &) = delete;
    void operator=(EmbeddedController const &) = delete;

    // Set a controller as the active controller
    void bind_controller(std::shared_ptr<Controller> *controller_ptr);

    bool send_movement(Vector2i &move_vector, int dt);
    bool send_actuation(int actuator, int duration, int delay);
    bool send_movement_to(int x, int y, int duration, int delay);

    // Simulator specific commands
    bool reset_simulator();
    bool set_proportion(double K_p);
    bool set_integral(double K_i);
    bool set_derivative(double K_d);
    std::tuple<float, float> cal_currents(float, float);
};

namespace Embedded {
    // Embedded move function exposed to Python API
    static PyObject *emb_move(PyObject *, PyObject *args) {
        int x = 0;
        int y = 0;
        int dt = 1000;
        if (!PyArg_ParseTuple(args, "ii|i", &x, &y, &dt))
            return PyLong_FromLong(-1);
        Vector2i move_vector(x, y);
        bool res = EmbeddedController::getInstance().send_movement(move_vector, dt);
        return PyLong_FromLong(res);
    }

    // Embedded actuation function
    static PyObject *emb_actuate(PyObject *, PyObject *args) {
        int actuator = 0;
        int delay = 0;
        int duration = 1000;
        if (!PyArg_ParseTuple(args, "i|ii", &actuator, &duration, &delay))
            return PyLong_FromLong(-1);
        bool res = EmbeddedController::getInstance().send_actuation(actuator, duration, delay);
        return PyLong_FromLong(res);
    }

    static PyObject *emb_move_to(PyObject *, PyObject *args) {
        int x = 0;
        int y = 0;
        int duration = 1000;
        int delay = 0;
        if (!PyArg_ParseTuple(args, "ii|ii", &x, &y, &duration, &delay))
            return PyLong_FromLong(-1);
        bool res = EmbeddedController::getInstance().send_movement_to(x, y, duration, delay);
        return PyLong_FromLong(res);
    }

    static PyObject *sim_reset(PyObject *, PyObject *args) {
        if (!PyArg_ParseTuple(args, ""))
            return PyLong_FromLong(-1);
        bool res = EmbeddedController::getInstance().reset_simulator();
        return PyLong_FromLong(res);
    }

    static PyObject *sim_set_prop(PyObject *, PyObject *args) {
        double K_p;
        if (!PyArg_ParseTuple(args, "d", &K_p))
            return PyLong_FromLong(-1);
        bool res = EmbeddedController::getInstance().set_proportion(K_p);
        return PyLong_FromLong(res);
    }

    static PyObject *sim_set_integ(PyObject *, PyObject *args) {
        double K_i;
        if (!PyArg_ParseTuple(args, "d", &K_i))
            return PyLong_FromLong(-1);
        bool res = EmbeddedController::getInstance().set_integral(K_i);
        return PyLong_FromLong(res);
    }

    static PyObject *sim_set_deriv(PyObject *, PyObject *args) {
        double K_d;
        if (!PyArg_ParseTuple(args, "d", &K_d))
            return PyLong_FromLong(-1);
        bool res = EmbeddedController::getInstance().set_derivative(K_d);
        return PyLong_FromLong(res);
    }

    static PyObject *sim_test(PyObject *, PyObject *args) {
        float v2x, v2y;
        PyArg_ParseTuple(args, "ff", &v2x, &v2y);
        float Ih, Iv;
        std::tie(Ih, Iv) = EmbeddedController::getInstance().cal_currents(v2x, v2y);
        return Py_BuildValue("ff", Ih, Iv);
    }

    // Embedded python configuration which describes which methods
    // should be exposed in which module
    static PyMethodDef emb_methods[]{
            {"move",  emb_move, METH_VARARGS, "Send move command to active controller."},
            {"actuate", emb_actuate, METH_VARARGS, "Send actuate command to active controller."},
            {"moveto", emb_move_to, METH_VARARGS, "Send move to command to active controller."},
            {nullptr, nullptr, 0, nullptr}
    };
    // Method 'move' is exposed in module 'emb' as 'emb.move'
    static PyModuleDef emb_module {
            PyModuleDef_HEAD_INIT, "emb", nullptr, -1, emb_methods,
            nullptr, nullptr, nullptr, nullptr
    };

    static PyMethodDef sim_methods[]{
            {"reset", sim_reset, METH_VARARGS, "Reset the simulator state"},
            {"kp", sim_set_prop, METH_VARARGS, "Set K_p"},
            {"ki", sim_set_integ, METH_VARARGS, "Set K_i"},
            {"kd", sim_set_deriv, METH_VARARGS, "Set K_d"},
            {"test", sim_test, METH_VARARGS, "Test function"},
            {nullptr, nullptr, 0, nullptr}
    };
    static PyModuleDef sim_module {
            PyModuleDef_HEAD_INIT, "sim", nullptr, -1, sim_methods,
            nullptr, nullptr, nullptr, nullptr
    };

    static PyObject *PyInit_emb() {
        return PyModule_Create(&emb_module);
    }
    static PyObject *PyInit_sim() {
        return PyModule_Create(&sim_module);
    }
}

#endif //MINOTAUR_CPP_EMBEDDEDCONTROLLER_H
