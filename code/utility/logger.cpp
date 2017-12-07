#include "logger.h"

std::string Logger::m_buffer;
QTextEdit *Logger::m_outfield = nullptr;

void Logger::setStream(QTextEdit *log_stream) {
    Logger::m_outfield = log_stream;
    Logger::log("Logger's stream has been set.");
}

//TODO: Overload ostream operator
bool Logger::log(const std::string &message, LogType type) {
    std::string color = Logger::getTextColor(type);
    Logger::m_buffer = "<font color=\"" + color + "\">" + ClockTime::getCurrentTime() + message + "</font>";

    if (Logger::m_outfield != nullptr) {
        //log it
        Logger::m_outfield->append(QString::fromStdString(m_buffer));
    } else {
        std::cout << m_buffer << std::endl;
    }
    return true;
}

bool Logger::info(const std::string &message) {
    return Logger::log(message, Logger::INFO);
}

#ifndef NDEBUG
bool Logger::debug(const std::string &message) {
    return Logger::log(message, Logger::DEBUG);
}
#endif

bool Logger::fatal(const std::string &message) {
    return Logger::log(message, Logger::FATAL);
}
