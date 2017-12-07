#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <QTextEdit>
#include <QString>
#include <iostream>

#include "clock_time.h"

class Logger {
public:
    enum LogType {
        INFO,
        FATAL,
        DEBUG,

        NUM_LOG_TYPES
    };
    static bool log(const std::string &message, LogType type = INFO);

#ifndef NDEBUG
    static bool debug(const std::string &message);
#endif
    static bool info(const std::string &message);
    static bool fatal(const std::string &message);

    static void setStream(QTextEdit* log_stream);

private:
    static std::string m_buffer;
    static QTextEdit* m_outfield;
    inline static std::string getTextColor(LogType type) {
        switch (type) {
            case INFO:
                return "black";
            case FATAL:
                return "red";
            case DEBUG:
                return "blue";
            default:
                return "black";
        }
    }
};

#endif // LOGGER_H
