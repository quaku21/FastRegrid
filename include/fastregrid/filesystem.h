/*
 * filesystem.h
 * Provides cross-platform filesystem utilities for FastRegrid.
 *
 * Author: Kevin Takyi Yeboah
 * Created: June, 2025
 *
 * Copyright (c) 2025 Kevin Takyi Yeboah
 * License: [MIT License, see LICENSE file]
 */

#pragma once

#include <string>
#include <sys/stat.h>
#include "logger.h"

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h> // For _mkdir
#define stat _stat
#else
#include <unistd.h> // For unlink
#endif

namespace fastregrid
{
    namespace filesystem
    {

        class path
        {
        public:
            path(const std::string &p) : path_(p) {}
            path operator/(const std::string &other) const
            {
                std::string result = path_;
                if (!result.empty() && result.back() != '/' && result.back() != '\\')
                {
                    result += '/';
                }
                result += other;
                return path(result);
            }
            std::string string() const { return path_; }

        private:
            std::string path_;
        };

        bool exists(const std::string &path)
        {
            struct stat buffer;
            return stat(path.c_str(), &buffer) == 0;
        }

        bool create_directory(const std::string &path)
        {
            FastRegridLogger::getInstance().debug("Creating directory: " + path);

            if (path.empty())
            {
                FastRegridLogger::getInstance().error("Cannot create directory: empty path");
                return false;
            }

#if defined(_WIN32) || defined(_WIN64)
            if (_mkdir(path.c_str()) == 0)
            {
                FastRegridLogger::getInstance().debug("Directory created: " + path);
                return true;
            }
#else
            if (mkdir(path.c_str(), 0755) == 0)
            {
                FastRegridLogger::getInstance().debug("Directory created: " + path);
                return true;
            }
#endif

            if (errno == EEXIST)
            {
                FastRegridLogger::getInstance().debug("Directory already exists: " + path);
                return true;
            }

            if (errno == ENOENT)
            {
                size_t pos = path.find_last_of("/\\");
                if (pos == std::string::npos || pos == 0)
                {
                    FastRegridLogger::getInstance().error("Cannot create directory: no parent path for " + path);
                    return false;
                }

                std::string parent = path.substr(0, pos);
                if (!create_directory(parent))
                {
                    FastRegridLogger::getInstance().error("Failed to create parent directory: " + parent);
                    return false;
                }

#if defined(_WIN32) || defined(_WIN64)
                if (_mkdir(path.c_str()) == 0)
                {
                    FastRegridLogger::getInstance().debug("Directory created after parent: " + path);
                    return true;
                }
#else
                if (mkdir(path.c_str(), 0755) == 0)
                {
                    FastRegridLogger::getInstance().debug("Directory created after parent: " + path);
                    return true;
                }
#endif

                FastRegridLogger::getInstance().error("Failed to create directory: " + path + " (errno: " + std::to_string(errno) + ")");
                return false;
            }

            FastRegridLogger::getInstance().error("Failed to create directory: " + path + " (errno: " + std::to_string(errno) + ")");
            return false;
        }

        bool remove(const std::string &path)
        {
            FastRegridLogger::getInstance().debug("Removing path: " + path);

            struct stat buffer;
            if (stat(path.c_str(), &buffer) != 0)
            {
                FastRegridLogger::getInstance().debug("Path does not exist: " + path);
                return false;
            }

#if defined(_WIN32) || defined(_WIN64)
            if (buffer.st_mode & _S_IFDIR)
            {
                bool success = _rmdir(path.c_str()) == 0;
                FastRegridLogger::getInstance().debug(success ? "Directory removed: " + path : "Failed to remove directory: " + path);
                return success;
            }
            else
            {
                bool success = _unlink(path.c_str()) == 0;
                FastRegridLogger::getInstance().debug(success ? "File removed: " + path : "Failed to remove file: " + path);
                return success;
            }
#else
            if (S_ISDIR(buffer.st_mode))
            {
                bool success = rmdir(path.c_str()) == 0;
                FastRegridLogger::getInstance().debug(success ? "Directory removed: " + path : "Failed to remove directory: " + path);
                return success;
            }
            else
            {
                bool success = unlink(path.c_str()) == 0;
                FastRegridLogger::getInstance().debug(success ? "File removed: " + path : "Failed to remove file: " + path);
                return success;
            }
#endif
        }

    } // namespace filesystem
} // namespace fastregrid