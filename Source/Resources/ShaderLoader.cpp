#include "Resources/ShaderLoader.h"

#include "Core/Assert.h"
#include "Core/Log.h"
#include "Graphics/ShaderProgram.h"
#include "Platform/IOUtils.h"

#include <algorithm>
#include <unordered_set>

#if SWAP_DEBUG
#include <boxer/boxer.h>

#include <sstream>
#endif // SWAP_DEBUG

namespace
{
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

   bool loadShaderSourceRecursive(const std::string& path, std::string& source, ShaderSourceMap& shaderSourceMap,
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
      if (!IOUtils::getDirectory(path, directory))
      {
         return false;
      }

      // Recursively handle all #include directives
      std::size_t includeStartPos, includeEndPos, pathStartPos, pathEndPos;
      while (findInclude(source, includeStartPos, includeEndPos, pathStartPos, pathEndPos))
      {
         std::string includePath = directory + '/' + source.substr(pathStartPos, (pathEndPos - pathStartPos) + 1);
         source.erase(includeStartPos, (includeEndPos - includeStartPos) + 1);

         std::string includeSource;
         if (loadShaderSourceRecursive(includePath, includeSource, shaderSourceMap, forceLoadFromDisk, loadedFilePaths))
         {
            source.insert(includeStartPos, includeSource);
         }
      }

      // Clean up extra #version directives
      std::size_t versionPos = source.find("#version");
      while ((versionPos = source.find("#version", versionPos + 1)) != std::string::npos)
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

   bool loadShaderSource(const std::string& path, const ShaderDefinitions& definitions, std::string& source,
      ShaderSourceMap& shaderSourceMap, bool forceLoadFromDisk)
   {
      std::unordered_set<std::string> loadedFilePaths;
      if (loadShaderSourceRecursive(path, source, shaderSourceMap, forceLoadFromDisk, loadedFilePaths))
      {
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
         if (loadShaderSource(specification.path, specification.definitions, source, shaderSourceMap,
            forceLoadFromDisk))
         {
            if (shader.compile(source.c_str()))
            {
               return true;
            }
            else
            {
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
      if (loadShaderSource(specification.path, specification.definitions, source, shaderSourceMap, forceLoadFromDisk))
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
   auto location = shaderMap.find(specification);
   if (location != shaderMap.end())
   {
      SPtr<Shader> shader = location->second.lock();
      if (shader)
      {
         return shader;
      }
   }

   SPtr<Shader> shader(new Shader(specification.type));
   loadAndCompileSource(*shader, specification, shaderSourceMap, false);

   shaderMap.emplace(specification, WPtr<Shader>(shader));

#if SWAP_DEBUG
   inverseShaderMap.emplace(shader.get(), specification);
#endif // SWAP_DEBUG

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
   for (const ShaderSpecification& specification : specifications)
   {
      SPtr<Shader> shader = loadShader(specification);
      if (shader)
      {
         shaderProgram->attach(shader);
      }
   }

#if SWAP_DEBUG
   linkProgram(*shaderProgram, shaderSourceMap, inverseShaderMap);
#else // SWAP_DEBUG
   linkProgram(*shaderProgram);
#endif // SWAP_DEBUG

   shaderProgramMap.emplace(std::move(specifications), WPtr<ShaderProgram>(shaderProgram));
   return shaderProgram;
}

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
#if SWAP_DEBUG
         linkProgram(*shaderProgram, shaderSourceMap, inverseShaderMap);
#else // SWAP_DEBUG
         linkProgram(*shaderProgram);
#endif // SWAP_DEBUG
      }
   }
}
