#pragma once

#include "Core/Pointers.h"
#include "Resources/ShaderLoader.h"

#include <string>
#include <unordered_map>

class Model;
class TextureLoader;

struct ModelSpecification
{
   std::string path;
   std::vector<ShaderSpecification> shaderSpecifications;

   bool operator==(const ModelSpecification& other) const
   {
      return path == other.path && shaderSpecifications == other.shaderSpecifications;
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
   SPtr<Model> loadModel(ModelSpecification specification, ShaderLoader& shaderLoader, TextureLoader& textureLoader);

   void clearCachedData();

private:
   std::unordered_map<ModelSpecification, WPtr<Model>> modelMap;
};
