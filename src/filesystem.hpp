#pragma once
#include <string>
#include "config.hpp"

class NX_HIDDEN Filesystem
{
public:
    Filesystem() = default;
    ~Filesystem();

    bool initialize(const char* arg0);

    static bool mountDir(const std::string& dir, const std::string& point, bool append = true);
    static bool mountRes(const std::string& dir, const std::string& point, bool append = true);
    static bool mountArchive(const std::string& file, const std::string& point, bool append = true);
    static bool setWriteDir(const std::string& dir);

    static std::string getErrorMessage();
    static std::string getPrefsDir();
    static std::string getBaseDir();
};