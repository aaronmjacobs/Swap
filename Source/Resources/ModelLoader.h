#pragma once

#include "Core/Pointers.h"
#include "Graphics/Material.h"
#include "Graphics/Model.h"
#include "Resources/TextureLoader.h"

#include <cstdint>
#include <string>
#include <unordered_map>

class Mesh;

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
   LoadedTextureParameters textureParams;
   bool cache = true;
   bool cacheTextures = true;

   bool operator==(const ModelSpecification& other) const
   {
      return path == other.path
         && normalGenerationMode == other.normalGenerationMode
         && textureParams == other.textureParams
         && cache == other.cache
         && cacheTextures == other.cacheTextures;
   }
};

// Provide a template specialization to allow using ModelSpecification as a key in std::unordered_map
namespace std
{
   template<>
   struct hash<ModelSpecification>
   {
      size_t operator()(const ModelSpecification& specification) const;
   };
}

struct ModelRef
{
   ModelRef(const Model& model);

   WPtr<Mesh> mesh;
   std::vector<Material> materials;
};

class ModelLoader
{
public:
   Model loadModel(const ModelSpecification& specification, TextureLoader& textureLoader);

   void clearCachedData();

private:
   std::unordered_map<ModelSpecification, ModelRef> modelMap;
};
