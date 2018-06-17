#pragma once

#include <string>

namespace OSUtils
{
   bool getExecutablePath(std::string& executablePath);
   bool getAppDataPath(const std::string& appName, std::string& appDataPath);
   bool getDirectoryFromPath(const std::string& path, std::string& dir);

   bool setWorkingDirectory(const std::string& dir);
   bool fixWorkingDirectory();

   bool directoryExists(const std::string& dir);
   bool createDirectory(const std::string& dir);

   int64_t getTime();
}
