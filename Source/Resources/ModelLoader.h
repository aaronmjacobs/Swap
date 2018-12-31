#pragma once

#include "Core/Pointers.h"
#include "Resources/ShaderLoader.h"
#include "Resources/TextureLoader.h"

#include <cstdint>
#include <string>
#include <unordered_map>

class Model;

enum class NormalGenerationMode : uint8_t
{
   None,
   Flat,
   Smooth
};

struct ModelSpecification
{
   std::string path;
   NormalGenerationMode normalGenerationMode = NormalGenerationMode::Smooth;
   std::vector<ShaderSpecification> shaderSpecifications;
   LoadedTextureParameters textureParams;
   bool cache = true;
   bool cacheTextures = true;

   bool operator==(const ModelSpecification& other) const
   {
      return path == other.path
         && normalGenerationMode == other.normalGenerationMode
         && shaderSpecifications == other.shaderSpecifications
         && textureParams == other.textureParams
         && cache == other.cache
         && cacheTextures == other.cacheTextures;
   }
};

// Provide a template specialization to allow using ShaderSpecification as a key in std::unordered_map
namespace std
{
   template<>
   struct hash<ModelSpecification>
   {
      size_t operator()(const ModelSpecification& specification) const;
   };
}

class ModelLoader
{
public:
   SPtr<Model> loadModel(const ModelSpecification& specification, ShaderLoader& shaderLoader,
      TextureLoader& textureLoader);

   void clearCachedData();

private:
   std::unordered_map<ModelSpecification, WPtr<Model>> modelMap;
};
