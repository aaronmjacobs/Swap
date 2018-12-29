#pragma once

#include "Core/Pointers.h"
#include "Graphics/TextureInfo.h"

#include <glad/glad.h>

#include <array>
#include <string>
#include <unordered_map>

class Texture;

struct LoadedTextureParameters
{
   Tex::Wrap wrap = Tex::Wrap::Repeat;
   Tex::MinFilter minFilter = Tex::MinFilter::NearestMipmapLinear;
   Tex::MagFilter magFilter = Tex::MagFilter::Linear;
   bool flipVerticallyOnLoad = true;

   bool operator==(const LoadedTextureParameters& other) const
   {
      return wrap == other.wrap && minFilter == other.minFilter && magFilter == other.magFilter;
   }
};

struct LoadedTextureSpecification
{
   std::string path;
   LoadedTextureParameters params;

   bool operator==(const LoadedTextureSpecification& other) const
   {
      return path == other.path && params == other.params;
   }
};

struct LoadedCubemapSpecification
{
   std::array<std::string, 6> paths;
   LoadedTextureParameters params;

   LoadedCubemapSpecification()
   {
      params.flipVerticallyOnLoad = false;
   }

   bool operator==(const LoadedCubemapSpecification& other) const
   {
      return paths == other.paths && params == other.params;
   }
};

// Provide a template specialization to allow using the specifications as keys in std::unordered_map
namespace std
{
   template<>
   struct hash<LoadedTextureSpecification>
   {
      size_t operator()(const LoadedTextureSpecification& specification) const;
   };

   template<>
   struct hash<LoadedCubemapSpecification>
   {
      size_t operator()(const LoadedCubemapSpecification& specification) const;
   };
}

class TextureLoader
{
public:
   SPtr<Texture> loadTexture(const LoadedTextureSpecification& specification);
   SPtr<Texture> loadCubemap(const LoadedCubemapSpecification& specification);

   void clearCachedData();

private:
   std::unordered_map<LoadedTextureSpecification, WPtr<Texture>> textureMap;
   std::unordered_map<LoadedCubemapSpecification, WPtr<Texture>> cubemapMap;
};
