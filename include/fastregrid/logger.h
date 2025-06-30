/*
 * logger.h
 * Implements logging functionality for FastRegrid, supporting multiple log levels.
 *
 * Author: Kevin Takyi Yeboah
 * Created: June, 2025
 *
 * Copyright (c) 2025 Kevin Takyi Yeboah
 * License: [MIT License, see LICENSE file]
 */

#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include "filesystem.h"

namespace fastregrid
{

    class FastRegridLogger
    {
    public:
        enum class LogLevel
        {
            DEBUG,
            INFO,
            WARN,
            ERROR
        };

        static FastRegridLogger &getInstance();

        void initialize(const std::string &baseDir = "./", LogLevel minLevel = LogLevel::INFO);

        void log(LogLevel level, const std::string &message, const std::string &details = "");

        void debug(const std::string &message, const std::string &details = "")
        {
            log(LogLevel::DEBUG, message, details);
        }
        void info(const std::string &message, const std::string &details = "")
        {
            log(LogLevel::INFO, message, details);
        }
        void warn(const std::string &message, const std::string &details = "")
        {
            log(LogLevel::WARN, message, details);
        }
        void error(const std::string &message, const std::string &details = "")
        {
            log(LogLevel::ERROR, message, details);
        }

        void setMinLevel(LogLevel level);

    private:
        FastRegridLogger() = default;
        ~FastRegridLogger();

        FastRegridLogger(const FastRegridLogger &) = delete;
        FastRegridLogger &operator=(const FastRegridLogger &) = delete;

        std::string getTimestamp() const;
        std::string levelToString(LogLevel level) const;

        std::ofstream logFile_;
        LogLevel minLevel_ = LogLevel::INFO;
        bool initialized_ = false;
    };

    FastRegridLogger &FastRegridLogger::getInstance()
    {
        static FastRegridLogger instance;
        return instance;
    }

    void FastRegridLogger::initialize(const std::string &baseDir, LogLevel minLevel)
    {
        if (initialized_)
        {
            logFile_.close();
        }

        filesystem::path logDir = filesystem::path(baseDir) / "logs";
        if (!filesystem::create_directory(logDir.string()))
        {
            std::cerr << "[ERROR] Failed to create log directory: " << logDir.string() << "\n";
            return;
        }

        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
        std::string timestamp = ss.str();
        filesystem::path logFilePath = logDir / ("fastregrid_" + timestamp + ".log");

        logFile_.open(logFilePath.string(), std::ios::out | std::ios::app);
        if (!logFile_.is_open())
        {
            std::cerr << "[ERROR] Failed to open log file: " << logFilePath.string() << "\n";
            return;
        }

        minLevel_ = minLevel;
        initialized_ = true;

        log(LogLevel::INFO, "FastRegrid Logger initialized", "Log file: " + logFilePath.string());
    }

    void FastRegridLogger::log(LogLevel level, const std::string &message, const std::string &details)
    {
        if (!initialized_ || level < minLevel_)
        {
            return;
        }

        std::string timestamp = getTimestamp();
        std::string levelStr = levelToString(level);
        std::string prefix = (level == LogLevel::WARN) ? "[WARNING]" : (level == LogLevel::ERROR) ? "[ERROR]"
                                                                                                  : "[" + levelStr + "]";
        std::string logMessage = "[FastRegrid][" + timestamp + "] " + prefix + " " + message;
        if (!details.empty())
        {
            logMessage += " [" + details + "]";
        }

        // Output destinations based on log level
        if (level == LogLevel::DEBUG)
        {
            // DEBUG: File-only unless minLevel_ is DEBUG
            if (minLevel_ == LogLevel::DEBUG && level == LogLevel::DEBUG)
            {
                std::cout << logMessage << std::endl;
            }
        }
        else if (level == LogLevel::ERROR)
        {
            // ERROR: Console (cerr), file
            std::cerr << logMessage << std::endl;
            std::cout << logMessage << std::endl;
        }
        else
        {
            // INFO, WARN: Console (cout), file
            std::cout << logMessage << std::endl;
        }

        if (logFile_.is_open())
        {
            logFile_ << logMessage << std::endl;
            if (level == LogLevel::ERROR)
            {
                logFile_.flush(); // Immediate flush for errors
            }
        }
    }

    void FastRegridLogger::setMinLevel(LogLevel level)
    {
        minLevel_ = level;
    }

    FastRegridLogger::~FastRegridLogger()
    {
        if (logFile_.is_open())
        {
            logFile_.close();
        }
    }

    std::string FastRegridLogger::getTimestamp() const
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::string FastRegridLogger::levelToString(LogLevel level) const
    {
        switch (level)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

} // namespace fastregrid