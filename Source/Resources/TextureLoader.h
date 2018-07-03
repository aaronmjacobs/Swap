#pragma once

#include "Core/Pointers.h"
#include "Graphics/TextureInfo.h"

#include <glad/glad.h>

#include <array>
#include <string>
#include <unordered_map>

class Texture;

class TextureLoader
{
public:
   SPtr<Texture> loadTexture(const std::string& path, Tex::Wrap wrap = Tex::Wrap::Repeat,
      Tex::MinFilter minFilter = Tex::MinFilter::NearestMipmapLinear, Tex::MagFilter magFilter = Tex::MagFilter::Linear);

   SPtr<Texture> loadCubemap(const std::array<std::string, 6>& paths, Tex::Wrap wrap = Tex::Wrap::Repeat,
      Tex::MinFilter minFilter = Tex::MinFilter::NearestMipmapLinear, Tex::MagFilter magFilter = Tex::MagFilter::Linear);

private:
   std::unordered_map<std::string, WPtr<Texture>> textureMap;
   std::unordered_map<std::string, WPtr<Texture>> cubemapMap;
};
