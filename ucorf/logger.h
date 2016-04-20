#pragma once

#include <cstdio>
#include <string>
#include <libgo/coroutine.h>

namespace ucorf
{
    class Logger
    {
    public:
        enum class eLogLevel
        {
            verbose,
            debug,
            info,
            notice,
            warn,
            error,
            fatal,
        };

        enum class eLogFlag
        {
            time = 0x1,
            file = 0x1 << 1,
            func = 0x1 << 2,
            line = 0x1 << 3,
            tid  = 0x1 << 4,
            all  = 0x7fffffff,
        };

        ~Logger();

        static Logger& default_instance();

        bool Reopen(std::string destination);

        void SetLevel(eLogLevel level);

        void SetMask(size_t mask);

        void Write(eLogLevel level, const char* file,
                int line, const char* func,
                const char* fmt, ...)
            __attribute__((format(printf, 6, 7)));

        inline eLogLevel GetLevel() {
            return level_; 
        }

    private:
        std::FILE* file_ = stdout;
        co_rwmutex rw_mtx_;
        std::string destination_;
        eLogLevel level_ = eLogLevel::verbose;
        size_t mask_ = (size_t)eLogFlag::all & ~(size_t)eLogFlag::tid;
    };

#define _ucorf_log(lg, level, fmt, ...) \
    do { \
        if (level >= lg.GetLevel()) \
            lg.Write(level, __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__); \
    } while(0)

#define _ucorf_log_verb(lg, fmt, ...) _ucorf_log(lg, (Logger::eLogLevel::verbose), fmt, ## __VA_ARGS__);
#define ucorf_log_verb(fmt, ...) _ucorf_log_verb((Logger::default_instance()), fmt, ## __VA_ARGS__);

#define _ucorf_log_debug(lg, fmt, ...) _ucorf_log(lg, (Logger::eLogLevel::debug), fmt, ## __VA_ARGS__);
#define ucorf_log_debug(fmt, ...) _ucorf_log_debug((Logger::default_instance()), fmt, ## __VA_ARGS__);

#define _ucorf_log_info(lg, fmt, ...) _ucorf_log(lg, (Logger::eLogLevel::info), fmt, ## __VA_ARGS__);
#define ucorf_log_info(fmt, ...) _ucorf_log_info((Logger::default_instance()), fmt, ## __VA_ARGS__);

#define _ucorf_log_notice(lg, fmt, ...) _ucorf_log(lg, (Logger::eLogLevel::notice), fmt, ## __VA_ARGS__);
#define ucorf_log_notice(fmt, ...) _ucorf_log_notice((Logger::default_instance()), fmt, ## __VA_ARGS__);

#define _ucorf_log_warn(lg, fmt, ...) _ucorf_log(lg, (Logger::eLogLevel::warn), fmt, ## __VA_ARGS__);
#define ucorf_log_warn(fmt, ...) _ucorf_log_warn((Logger::default_instance()), fmt, ## __VA_ARGS__);

#define _ucorf_log_error(lg, fmt, ...) _ucorf_log(lg, (Logger::eLogLevel::error), fmt, ## __VA_ARGS__);
#define ucorf_log_error(fmt, ...) _ucorf_log_error((Logger::default_instance()), fmt, ## __VA_ARGS__);

#define _ucorf_log_fatal(lg, fmt, ...) _ucorf_log(lg, (Logger::eLogLevel::fatal), fmt, ## __VA_ARGS__);
#define ucorf_log_fatal(fmt, ...) _ucorf_log_fatal((Logger::default_instance()), fmt, ## __VA_ARGS__);

} //namespace ucorf
