#include "Platform/IOUtils.h"

#include "Core/Assert.h"
#include "Platform/OSUtils.h"

#include <algorithm>
#include <fstream>

namespace IOUtils
{
   bool readTextFile(const std::string& path, std::string& data)
   {
      ASSERT(!path.empty(), "Trying to read text file with empty path");

      std::ifstream in(path);
      if (!in)
      {
         return false;
      }

      data = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
      return true;
   }

   bool readBinaryFile(const std::string& path, std::vector<uint8_t>& data)
   {
      ASSERT(!path.empty(), "Trying to read binary file with empty path");

      std::ifstream in(path, std::ifstream::binary);
      if (!in)
      {
         return false;
      }

      std::streampos start = in.tellg();
      in.seekg(0, std::ios_base::end);
      std::streamoff size = in.tellg() - start;
      ASSERT(size >= 0, "Invalid file size");
      in.seekg(0, std::ios_base::beg);

      data.resize(static_cast<size_t>(size));
      in.read(reinterpret_cast<char*>(data.data()), size);

      return true;
   }

   bool writeTextFile(const std::string& path, const std::string& data)
   {
      ASSERT(!path.empty(), "Trying to write text file with empty path");

      if (!ensurePathToFileExists(path))
      {
         return false;
      }

      std::ofstream out(path);
      if (!out)
      {
         return false;
      }

      out << data;
      return true;
   }

   bool writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data)
   {
      ASSERT(!path.empty(), "Trying to write binary file with empty path");

      if (!ensurePathToFileExists(path))
      {
         return false;
      }

      std::ofstream out(path, std::ofstream::binary);
      if (!out)
      {
         return false;
      }

      out.write(reinterpret_cast<const char*>(data.data()), data.size());
      return true;
   }

   bool ensurePathToFileExists(const std::string& path)
   {
      std::string directory;
      if (!OSUtils::getDirectoryFromPath(path, directory))
      {
         return false;
      }

      if (OSUtils::directoryExists(directory))
      {
         return true;
      }

      return OSUtils::createDirectory(directory);
   }

   bool makeAppDataRelativePath(const std::string& appName, const std::string& fileName, std::string& path)
   {
      std::string appDataPath;
      if (!OSUtils::getAppDataPath(appName, appDataPath))
      {
         return false;
      }

      path = appDataPath + "/" + fileName;
      return true;
   }

   std::string sanitizePath(std::string path)
   {
      // Convert all back slashes to forward slashes
      std::replace(path.begin(), path.end(), '\\', '/');

      // Eliminate trailing slashes
      while (path.size() > 1 && path[path.size() - 1] == '/')
      {
         path.pop_back();
      }

      // Resolve all ..
      std::size_t dotsPos;
      while ((dotsPos = path.find("..")) != std::string::npos)
      {
         std::size_t previousSlashPos = path.rfind('/', dotsPos - 2);
         if (previousSlashPos == std::string::npos)
         {
            break;
         }

         path.erase(previousSlashPos, (dotsPos - previousSlashPos) + 2);
      }

      return path;
   }

   bool getDirectory(const std::string& path, std::string& directory)
   {
      std::string sanitizedPath = sanitizePath(path);

      size_t pos = sanitizedPath.rfind('/');
      if (pos == std::string::npos)
      {
         return false;
      }

#ifdef _WIN32
      bool isRoot = pos == 2;
#else
      bool isRoot = pos == 0;
#endif

      if (isRoot)
      {
         directory = sanitizedPath;
      }
      else
      {
         directory = sanitizedPath.substr(0, pos);
      }

      return true;
   }
}
