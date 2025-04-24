#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <filesystem>

namespace filelog {

    class Logger {
    public:
        Logger()
        {
            std::string path = "log.txt";
            log_file_.open(path, std::ios::out | std::ios::trunc);
        }

        ~Logger()
        {
            if (log_file_.is_open())
            {
                log_file_.close();
            }
        }

        void log(const std::string& message)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (log_file_.is_open())
            {
                log_file_ << timestamp() << " - " << message << std::endl;
            }
        }

        static Logger& instance()
        {
            static Logger logger;
            return logger;
        }

    private:
        std::ofstream log_file_;
        std::mutex mutex_;

        std::string timestamp()
        {
            auto now = std::time(nullptr);
            std::tm buf{};
#if defined(_WIN32)
            localtime_s(&buf, &now);
#else
            localtime_r(&now, &buf);
#endif
            std::ostringstream ss;
            ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
            return ss.str();
        }
    };

    inline void LogMessage(const std::string& msg)
    {
        Logger::instance().log(msg);
    }

} // namespace filelog

#define LOG(msg) ::filelog::LogMessage(msg)