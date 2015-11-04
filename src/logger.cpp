#include "logger.h"
#include <chrono>
#include <ctime>
#include <time.h>
#include <iomanip>
#include <sstream>
#include <boost/thread.hpp>
#include <stdarg.h>

namespace ucorf
{
    static const char* eLogLevelName[] =
    {
        "VERB",
        "DEBUG",
        "INFO",
        "NOTICE",
        "WARN",
        "ERROR",
        "FATAL",
    };

    Logger& Logger::default_instance()
    {
        static Logger lg;
        return lg;
    }

    Logger::~Logger()
    {
        if (file_ && file_ != stdout && file_ != stderr)
            std::fclose(file_);
    }

    bool Logger::Reopen(std::string destination)
    {
        std::FILE* file = std::fopen(destination.c_str(), "a+");
        if (!file) return false;

        if (file_ != stdout && file_ != stderr)
            std::fclose(file_);

        file_ = file;
        destination_ = destination;
        return true;
    }

    void Logger::SetLevel(eLogLevel level)
    {
        level_ = level;
    }

    void Logger::SetMask(size_t mask)
    {
        mask_ = mask;
    }

    void Logger::Write(eLogLevel level, const char* file,
            int line, const char* func,
            const char* fmt, ...)
    {
        if (!file_) return ;

        level = std::max(std::min(eLogLevel::fatal, level), eLogLevel::verbose);

        std::ostringstream info;
        if (mask_ & (size_t)eLogFlag::time) {
            auto now = std::chrono::high_resolution_clock::now();
            std::time_t t = std::chrono::high_resolution_clock::to_time_t(now);
            tm m;
            localtime_r(&t, &m);
            char buf[128];
            snprintf(buf, sizeof(buf), "%d-%02d-%02d %02d:%02d:%02d.%03d",
                    1900 + m.tm_year, m.tm_mon, m.tm_mday, m.tm_hour, m.tm_min, m.tm_sec,
                    (int)(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000));
            info << buf << "|";
        }

        if (mask_ & (size_t)eLogFlag::tid) {
            info << "T:" << boost::this_thread::get_id() << "|";
        }

        info << eLogLevelName[(int)level] << "|";

        if (mask_ & (size_t)eLogFlag::file) {
            const char* p = strrchr(file, '/');
            if (!p) p = strrchr(file, '\\');

            if (p) ++p;
            else p = file;
            info << "F:" << p << "|";
        }

        if (mask_ & (size_t)eLogFlag::line) {
            info << "L:" << line << "|";
        }

        if (mask_ & (size_t)eLogFlag::func) {
            info << "(" << func << "): ";
        }

        va_list args;
        char buf[1024];
        va_start(args, fmt);
        int len = vsnprintf(buf, sizeof(buf), fmt, args);
        if (len > (int)sizeof(buf)) {
            char *b = new char[len + 1];
            vsnprintf(b, len + 1, fmt, args);
            info << b;
            delete b;
        } else {
            info << buf;
        }

        info << "\n";
        std::string msg = info.str();
        std::fwrite(msg.c_str(), msg.length(), 1, file_);
        std::fflush(file_);
    }

} //namespace ucorf
