#include "Platform/OSUtils.h"

#include "Core/Pointers.h"

#include <cstdint>
#include <ctime>

#if SWAP_PLATFORM_MACOS
#include <sys/stat.h>
#endif // SWAP_PLATFORM_MACOS

#if SWAP_PLATFORM_LINUX
#include <cstring>
#include <limits.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif // SWAP_PLATFORM_LINUX

#if SWAP_PLATFORM_WINDOWS
#include <codecvt>
#include <cstring>
#include <locale>
#include <ShlObj.h>
#include <sys/stat.h>
#include <sys/types.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // SWAP_PLATFORM_WINDOWS

namespace OSUtils
{
#if SWAP_PLATFORM_LINUX
   bool getExecutablePath(std::string& executablePath)
   {
      char path[PATH_MAX + 1];

      ssize_t numBytes = readlink("/proc/self/exe", path, PATH_MAX);
      if (numBytes == -1)
      {
         return false;
      }
      path[numBytes] = '\0';

      executablePath = std::string(path);
      return true;
   }

   bool getAppDataPath(const std::string& appName, std::string& appDataPath)
   {
      // First, check the HOME environment variable
      char* homePath = secure_getenv("HOME");

      // If it isn't set, grab the directory from the password entry file
      if (!homePath)
      {
         homePath = getpwuid(getuid())->pw_dir;
      }

      if (!homePath)
      {
         return false;
      }

      appDataPath = std::string(homePath) + "/.config/" + appName;
      return true;
   }

   bool setWorkingDirectory(const std::string& dir)
   {
      return chdir(dir.c_str()) == 0;
   }

   bool createDirectory(const std::string& dir)
   {
      return mkdir(dir.c_str(), 0755) == 0;
   }
#endif // SWAP_PLATFORM_LINUX

#if SWAP_PLATFORM_WINDOWS
   bool getExecutablePath(std::string& executablePath)
   {
      TCHAR buffer[MAX_PATH + 1];
      DWORD length = GetModuleFileName(nullptr, buffer, MAX_PATH);
      buffer[length] = '\0';

      if (length == 0 || length == MAX_PATH || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
      {
         static const DWORD kUnreasonablyLargeStringLength = 32767;
         UPtr<TCHAR[]> unreasonablyLargeBuffer = std::make_unique<TCHAR[]>(kUnreasonablyLargeStringLength + 1);
         length = GetModuleFileName(nullptr, unreasonablyLargeBuffer.get(), kUnreasonablyLargeStringLength);
         unreasonablyLargeBuffer[length] = '\0';

         if (length == 0 || length == kUnreasonablyLargeStringLength || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
         {
            return false;
         }

         executablePath = std::string(unreasonablyLargeBuffer.get());
      }
      else
      {
         executablePath = std::string(buffer);
      }

      return true;
   }

   bool getAppDataPath(const std::string& appName, std::string& appDataPath)
   {
      PWSTR path;
      if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path) != S_OK)
      {
         CoTaskMemFree(path);
         return false;
      }

      std::wstring widePathStr(path);
      CoTaskMemFree(path);

      std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
      appDataPath = converter.to_bytes(widePathStr) + "/" + appName;
      return true;
   }

   bool setWorkingDirectory(const std::string& dir)
   {
      return SetCurrentDirectory(dir.c_str()) != 0;
   }

   bool createDirectory(const std::string& dir)
   {
      return CreateDirectory(dir.c_str(), nullptr) != 0;
   }
#endif // SWAP_PLATFORM_WINDOWS

   bool getDirectoryFromPath(const std::string& path, std::string& dir)
   {
      size_t pos = path.find_last_of("/\\");
      if (pos == std::string::npos)
      {
         return false;
      }

#if SWAP_PLATFORM_WINDOWS
      bool isRoot = pos == 2;
#else
      bool isRoot = pos == 0;
#endif

      if (isRoot)
      {
         dir = path;
      }
      else
      {
         dir = path.substr(0, pos);
      }

      return true;
   }

   bool fixWorkingDirectory()
   {
      std::string executablePath;
      if (!getExecutablePath(executablePath))
      {
         return false;
      }

      std::string executableDir;
      if (!getDirectoryFromPath(executablePath, executableDir))
      {
         return false;
      }

      return setWorkingDirectory(executableDir);
   }

   bool directoryExists(const std::string& dir)
   {
      struct stat info;
      if (stat(dir.c_str(), &info) != 0)
      {
         return false;
      }

      return (info.st_mode & S_IFDIR) != 0;
   }

   int64_t getTime()
   {
      static_assert(sizeof(std::time_t) <= sizeof(int64_t), "std::time_t will not fit in an int64_t");

      return static_cast<int64_t>(std::time(nullptr));
   }
}
