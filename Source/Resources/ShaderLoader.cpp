#include "Resources/ShaderLoader.h"

#include "Core/Log.h"
#include "Graphics/ShaderProgram.h"
#include "Platform/IOUtils.h"
#include "Platform/OSUtils.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

#if SWAP_DEBUG
#include <boxer/boxer.h>

#include <sstream>
#endif // SWAP_DEBUG

namespace
{
   std::string generateProgramLabel(const std::vector<SPtr<Shader>>& shaders)
   {
      std::string programLabel;

      for (std::size_t i = 0; i < shaders.size(); ++i)
      {
         programLabel += shaders[i]->getLabel();

         if (i != shaders.size() - 1)
         {
            programLabel += " | ";
         }
      }

      return programLabel;
   }

   void replaceAll(std::string& string, const std::string& search, const std::string& replace)
   {
      for (std::size_t pos = 0; ; pos += replace.length())
      {
         pos = string.find(search, pos);
         if (pos == std::string::npos)
         {
            break;
         }

         string.erase(pos, search.length());
         string.insert(pos, replace);
      }
   }

#if SWAP_DEBUG
   std::string addLineNumbers(const std::string& source)
   {
      std::stringstream ss;
      int lineNumber = 0;
      std::size_t pos = 0;
      std::size_t nextPos = 0;

      while (nextPos < source.size())
      {
         nextPos = source.find('\n', pos);
         if (nextPos == std::string::npos)
         {
            nextPos = source.size();
         }

         std::size_t count = (nextPos - pos) + 1;
         ss << std::to_string(++lineNumber) << ": " << source.substr(pos, count);

         pos = nextPos + 1;
      }

      return ss.str();
   }
#endif // SWAP_DEBUG

   bool findInclude(const std::string& source, std::size_t& includeStartPos, std::size_t& includeEndPos,
      std::size_t& pathStartPos, std::size_t& pathEndPos)
   {
      includeStartPos = includeEndPos = pathStartPos = pathEndPos = std::string::npos;

      includeStartPos = source.find("#include");
      if (includeStartPos == std::string::npos)
      {
         return false;
      }

      std::size_t openingQuotePos = source.find("\"", includeStartPos + 1);
      if (openingQuotePos == std::string::npos)
      {
         return false;
      }

      std::size_t closingQuotePos = source.find("\"", openingQuotePos + 1);
      if (closingQuotePos == std::string::npos)
      {
         return false;
      }

      includeEndPos = closingQuotePos + 1;
      pathStartPos = openingQuotePos + 1;
      pathEndPos = closingQuotePos - 1;

      return true;
   }

   bool findDefault(const std::string& source, std::size_t& defaultStartPos, std::size_t& defaultEndPos,
                    std::size_t& nameStartPos, std::size_t& nameEndPos, std::size_t& lineEndPos)
   {
      static const char* kDefault = "#default";
      static const std::size_t kDefaultLen = std::strlen(kDefault);

      defaultStartPos = defaultEndPos = nameStartPos = nameEndPos = lineEndPos = std::string::npos;

      defaultStartPos = source.find(kDefault);
      if (defaultStartPos == std::string::npos)
      {
         return false;
      }

      defaultEndPos = defaultStartPos + kDefaultLen;

      lineEndPos = source.find('\n', defaultStartPos + 1);
      if (lineEndPos == std::string::npos)
      {
         return false;
      }

      auto whitespaceLoc = std::find_if(source.begin() + defaultStartPos, source.end(), [](char c) { return std::isblank(c); });
      if (whitespaceLoc == source.end())
      {
         return false;
      }

      auto nameStartLoc = std::find_if(whitespaceLoc, source.end(), [](char c) { return !std::isblank(c); });
      if (nameStartLoc == source.end())
      {
         return false;
      }

      auto nameEndLoc = std::find_if(nameStartLoc, source.end(), [](char c) { return std::isspace(c); });
      if (nameEndLoc == source.end() || nameEndLoc == nameStartLoc)
      {
         return false;
      }

      nameStartPos = nameStartLoc - source.begin();
      nameEndPos = nameEndLoc - source.begin();

      return true;
   }

   void handleDefaults(const ShaderDefinitions& definitions, std::string& source)
   {
      static const char* kDefine = "#define";
      static const std::size_t kDefineLen = std::strlen(kDefine);

      std::size_t defaultStartPos = 0;
      std::size_t defaultEndPos = 0;
      std::size_t nameStartPos = 0;
      std::size_t nameEndPos = 0;
      std::size_t lineEndPos = 0;
      while (findDefault(source, defaultStartPos, defaultEndPos, nameStartPos, nameEndPos, lineEndPos))
      {
         std::string name = source.substr(nameStartPos, nameEndPos - nameStartPos);

         if (definitions.count(name) > 0)
         {
            // We have a provided definition, clear the default
            source.erase(defaultStartPos, (lineEndPos - defaultStartPos) + 1);
         }
         else
         {
            // We don't have a provided definition, use the default
            source.replace(defaultStartPos, defaultEndPos - defaultStartPos, kDefine, kDefineLen);
         }
      }
   }

   bool loadSourceRecursive(const std::string& path, std::string& source, ShaderSourceMap& shaderSourceMap,
      bool forceLoadFromDisk, std::unordered_set<std::string>& loadedFilePaths)
   {
      // Prevent circular inclusion
      if (loadedFilePaths.count(path))
      {
         return false;
      }
      loadedFilePaths.insert(path);

      if (forceLoadFromDisk)
      {
         shaderSourceMap.erase(path);
      }
      else
      {
         auto loadedLocation = shaderSourceMap.find(path);
         if (loadedLocation != shaderSourceMap.end())
         {
            source = loadedLocation->second;
            return true;
         }
      }

      if (!IOUtils::readTextFile(path, source))
      {
         return false;
      }

      std::string directory;
      if (!IOUtils::getSanitizedDirectory(path, directory))
      {
         return false;
      }

      // Recursively handle all #include directives
      std::size_t includeStartPos = 0;
      std::size_t includeEndPos = 0;
      std::size_t pathStartPos = 0;
      std::size_t pathEndPos = 0;
      while (findInclude(source, includeStartPos, includeEndPos, pathStartPos, pathEndPos))
      {
         std::string includePath = directory + '/' + source.substr(pathStartPos, (pathEndPos - pathStartPos) + 1);
         source.erase(includeStartPos, (includeEndPos - includeStartPos) + 1);

         std::string includeSource;
         if (loadSourceRecursive(includePath, includeSource, shaderSourceMap, forceLoadFromDisk, loadedFilePaths))
         {
            source.insert(includeStartPos, includeSource);
         }
      }

      // Clean up extra #version directives
      static const char* kVersion = "#version";
      std::size_t versionPos = source.find(kVersion);
      while ((versionPos = source.find(kVersion, versionPos + 1)) != std::string::npos)
      {
         std::size_t endOfLinePos = source.find('\n', versionPos);
         if (endOfLinePos == std::string::npos)
         {
            LOG_WARNING("Unable to remove #version directive (no newline)");
            break;
         }
         source.erase(versionPos, (endOfLinePos - versionPos) + 1);
      }

      shaderSourceMap.emplace(path, source);
      return true;
   }

   bool loadSource(const std::string& path, const ShaderDefinitions& definitions, std::string& source,
      ShaderSourceMap& shaderSourceMap, bool forceLoadFromDisk)
   {
      std::unordered_set<std::string> loadedFilePaths;
      if (loadSourceRecursive(path, source, shaderSourceMap, forceLoadFromDisk, loadedFilePaths))
      {
         handleDefaults(definitions, source);

         // Replace all defined values
         for (const auto& pair : definitions)
         {
            replaceAll(source, pair.first, pair.second);
         }

         return true;
      }

      return false;
   }

#if SWAP_DEBUG
   bool loadAndCompileSource(Shader& shader, const ShaderSpecification& specification, ShaderSourceMap& shaderSourceMap,
      bool forceLoadFromDisk)
   {
      bool giveUp = false;
      while (!giveUp)
      {
         std::string source;
         if (loadSource(specification.path, specification.definitions, source, shaderSourceMap, forceLoadFromDisk))
         {
            if (shader.compile(source.c_str()))
            {
               return true;
            }
            else
            {
               std::string sourceWithLineNumbers = addLineNumbers(source);
               LOG_INFO(specification.path << ":\n\n" << sourceWithLineNumbers);

               std::stringstream sstream;
               sstream << "Failed to compile " << shader.getTypeName() << " shader, try again?\n\n"
                  << specification.path << "\n\n" << shader.getInfoLog();

               boxer::Selection selection = boxer::show(sstream.str().c_str(), "Shader Compilation Failure",
                  boxer::Style::Question, boxer::Buttons::YesNo);
               giveUp = selection != boxer::Selection::Yes;

               if (giveUp)
               {
                  LOG_ERROR("Unable to compile shader: " << specification.path);
               }
               else
               {
                  forceLoadFromDisk = true;
               }
            }
         }
         else
         {
            std::stringstream sstream;
            sstream << "Failed to load shader from file, try again?\n\n" << specification.path;

            boxer::Selection selection = boxer::show(sstream.str().c_str(), "Shader Load Failure",
               boxer::Style::Question, boxer::Buttons::YesNo);
            giveUp = selection != boxer::Selection::Yes;

            if (giveUp)
            {
               LOG_ERROR("Unable to read shader source from file: " << specification.path);
            }
         }
      }

      return false;
   }
#else // SWAP_DEBUG
   bool loadAndCompileSource(Shader& shader, const ShaderSpecification& specification, ShaderSourceMap& shaderSourceMap,
      bool forceLoadFromDisk)
   {
      std::string source;
      if (loadSource(specification.path, specification.definitions, source, shaderSourceMap, forceLoadFromDisk))
      {
         return shader.compile(source.c_str());
      }

      return false;
   }
#endif // SWAP_DEBUG

#if SWAP_DEBUG
   bool linkProgram(ShaderProgram& shaderProgram, ShaderSourceMap& shaderSourceMap,
      const InverseShaderMap& inverseShaderMap)
   {
      if (shaderProgram.getAttachedShaders().empty())
      {
         return false;
      }

      bool giveUp = false;
      while (!giveUp)
      {
         if (shaderProgram.link())
         {
            return true;
         }
         else
         {
            std::stringstream sstream;
            sstream << "Failed to link shader program, try again?";

            const std::vector<SPtr<Shader>>& attachedShaders = shaderProgram.getAttachedShaders();
            for (const SPtr<Shader>& shader : attachedShaders)
            {
               auto location = inverseShaderMap.find(shader.get());
               if (location != inverseShaderMap.end())
               {
                  const ShaderSpecification& specification = location->second;
                  sstream << "\n\n" << shader->getTypeName() << ":\n" << specification.path;
               }
            }
            sstream << "\n\n" << shaderProgram.getInfoLog();

            boxer::Selection selection = boxer::show(sstream.str().c_str(), "Shader Program Link Failure",
               boxer::Style::Question, boxer::Buttons::YesNo);
            if (selection == boxer::Selection::Yes)
            {
               // First, recompile all attached shaders
               for (const SPtr<Shader>& shader : attachedShaders)
               {
                  auto location = inverseShaderMap.find(shader.get());
                  if (location != inverseShaderMap.end())
                  {
                     const ShaderSpecification& specification = location->second;
                     loadAndCompileSource(*shader, specification, shaderSourceMap, true);
                  }
               }
            }
            else
            {
               giveUp = true;
               LOG_ERROR("Unable to link shader program: " << shaderProgram.getId());
            }
         }
      }

      return false;
   }
#else // SWAP_DEBUG
   bool linkProgram(ShaderProgram& shaderProgram)
   {
      return shaderProgram.link();
   }
#endif // SWAP_DEBUG
}

namespace std
{
   size_t hash<ShaderSpecification>::operator()(const ShaderSpecification& specification) const
   {
      size_t seed = 0;

      for (const auto& pair : specification.definitions)
      {
         Hash::combine(seed, pair.first);
         Hash::combine(seed, pair.second);
      }

      Hash::combine(seed, specification.path);
      Hash::combine(seed, specification.type);

      return seed;
   }
}

SPtr<Shader> ShaderLoader::loadShader(const ShaderSpecification& specification)
{
   if (specification.cache)
   {
      auto location = shaderMap.find(specification);
      if (location != shaderMap.end())
      {
         SPtr<Shader> shader = location->second.lock();
         if (shader)
         {
            return shader;
         }
      }
   }

   SPtr<Shader> shader(new Shader(specification.type));
   loadAndCompileSource(*shader, specification, shaderSourceMap, false);

   std::string fileName;
   if (OSUtils::getFileNameFromPath(specification.path, fileName, true))
   {
      shader->setLabel(fileName);
   }

   if (specification.cache)
   {
      shaderMap.emplace(specification, WPtr<Shader>(shader));

#if SWAP_DEBUG
      inverseShaderMap.emplace(shader.get(), specification);
#endif // SWAP_DEBUG
   }

   return shader;
}

SPtr<ShaderProgram> ShaderLoader::loadShaderProgram(std::vector<ShaderSpecification> specifications)
{
   // Sort the specifications by their shader type so that hash map lookups are consistent
   std::sort(specifications.begin(), specifications.end(),
      [](const ShaderSpecification& first, const ShaderSpecification& second) -> bool
   {
      return first.type < second.type;
   });

   auto location = shaderProgramMap.find(specifications);
   if (location != shaderProgramMap.end())
   {
      SPtr<ShaderProgram> shaderProgram = location->second.lock();
      if (shaderProgram)
      {
         return shaderProgram;
      }
   }

   SPtr<ShaderProgram> shaderProgram(new ShaderProgram);
   bool cache = !specifications.empty();
   std::vector<SPtr<Shader>> shaders;
   for (const ShaderSpecification& specification : specifications)
   {
      SPtr<Shader> shader = loadShader(specification);
      if (shader)
      {
         shaderProgram->attach(shader);
         shaders.push_back(shader);
      }

      cache &= specification.cache;
   }

#if SWAP_DEBUG
   linkProgram(*shaderProgram, shaderSourceMap, inverseShaderMap);
#else // SWAP_DEBUG
   linkProgram(*shaderProgram);
#endif // SWAP_DEBUG

   shaderProgram->setLabel(generateProgramLabel(shaders));

   if (cache)
   {
      shaderProgramMap.emplace(std::move(specifications), WPtr<ShaderProgram>(shaderProgram));
   }

   return shaderProgram;
}

void ShaderLoader::clearCachedData()
{
   shaderSourceMap.clear();
   shaderMap.clear();
   shaderProgramMap.clear();

#if SWAP_DEBUG
   inverseShaderMap.clear();
#endif // SWAP_DEBUG
}

#if SWAP_DEBUG
void ShaderLoader::reloadShaders()
{
   // Clear the cached source to force loading from disk
   shaderSourceMap.clear();

   // Recompile each shader
   for (const auto& pair : shaderMap)
   {
      SPtr<Shader> shader = pair.second.lock();
      if (shader)
      {
         const ShaderSpecification& specification = pair.first;
         loadAndCompileSource(*shader, specification, shaderSourceMap, false);
      }
   }

   // Relink each program
   for (const auto& pair : shaderProgramMap)
   {
      SPtr<ShaderProgram> shaderProgram = pair.second.lock();
      if (shaderProgram)
      {
         linkProgram(*shaderProgram, shaderSourceMap, inverseShaderMap);
      }
   }
}
#endif // SWAP_DEBUG
