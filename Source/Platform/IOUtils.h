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

   std::string sanitizePath(std::string path);
   bool getSanitizedDirectory(const std::string& path, std::string& directory);

   bool getResourceDirectory(std::string& resourceDirectory);
   bool getAbsoluteResourcePath(const std::string& relativePath, std::string& absolutePath);

   bool getAbsoluteAppDataPath(const std::string& appName, const std::string& relativePath, std::string& absolutePath);
}
