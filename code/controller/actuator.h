#ifndef ACTUATOR_H
#define ACTUATOR_H

#include "controller.h"
#include "qextserialport.h"

#define BYTE_RANGE	256
#define DATA_SIZE	4
#define CMD_SIZE	2
#define STEP_FACTOR	5

// For Zaber, settings must be: 9600 baud, no hand shaking, 8 data bits, no parity, one stop bit.
const PortSettings DEFAULT_SETTINGS = { BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_XONXOFF, 10 };


class QextSerialPort;

// Command mapping for Zaber T-LSR actuators http://www.zaber.com/wiki/Manuals/T-LSR
enum ZaberCmd {
    HOME = 1,
    RENUMBER = 2,
    ABS_MOVE = 20,
    REL_MOVE = 21,
    CONST_SPEED_MOVE = 22,
    STOP_MOVE = 23,
    SET_CURRENT_POS = 45,
    FIRM_VER = 51
};

class Actuator : public Controller
{
public:
    explicit Actuator(const QString& serial_port = "",
        const PortSettings& settings = DEFAULT_SETTINGS,
        QextSerialPort::QueryMode mode = QextSerialPort::EventDriven);

    Actuator(const Actuator&);
    ~Actuator();

    int setSerPort(const QString& serial_port);
    int changeSettings(const PortSettings& settings);
    void switchDevices();

protected:
    void performMove(Vector2i dir, int timer) override;
    void performActuation(int actuator, int duration, int delay) override;
    void performMoveTo(double x, double y, int duration, int delay) override;

private:
    QextSerialPort* m_serial_port;
    unsigned char m_x_device, m_y_device;

    //helper functions
    static char* const convertDataToBytes(long int data);
    void resetDeviceNumber();
    static int const intPow(int x, int p);
    void moveActuator(unsigned char device, int value, int time);
};

#endif // ACTUATOR_H
