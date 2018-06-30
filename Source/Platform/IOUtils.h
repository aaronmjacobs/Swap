#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace IOUtils
{
   bool readTextFile(const std::string& path, std::string& data);
   bool readBinaryFile(const std::string& path, std::vector<uint8_t>& data);

   bool writeTextFile(const std::string& path, const std::string& data);
   bool writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data);

   bool ensurePathToFileExists(const std::string& path);
   bool makeAppDataRelativePath(const std::string& appName, const std::string& fileName, std::string& path);

   std::string sanitizePath(std::string path);
   bool getDirectory(const std::string& path, std::string& directory);
}
