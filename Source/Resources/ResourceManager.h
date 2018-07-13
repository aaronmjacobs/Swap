#pragma once

#include "Resources/ModelLoader.h"
#include "Resources/ShaderLoader.h"
#include "Resources/TextureLoader.h"

class ResourceManager
{
public:
   SPtr<Model> loadModel(const ModelSpecification& specification);

   SPtr<Shader> loadShader(const ShaderSpecification& specification);
   SPtr<ShaderProgram> loadShaderProgram(const std::vector<ShaderSpecification>& specifications);

   SPtr<Texture> loadTexture(const std::string& path, Tex::Wrap wrap = Tex::Wrap::Repeat,
      Tex::MinFilter minFilter = Tex::MinFilter::NearestMipmapLinear,
      Tex::MagFilter magFilter = Tex::MagFilter::Linear);

   SPtr<Texture> loadCubemap(const std::array<std::string, 6>& paths, Tex::Wrap wrap = Tex::Wrap::Repeat,
      Tex::MinFilter minFilter = Tex::MinFilter::NearestMipmapLinear,
      Tex::MagFilter magFilter = Tex::MagFilter::Linear);

   void clearCachedData();

#if SWAP_DEBUG
   void reloadShaders()
   {
      shaderLoader.reloadShaders();
   }
#endif // SWAP_DEBUG

   ModelLoader& getModelLoader()
   {
      return modelLoader;
   }

   ShaderLoader& getShaderLoader()
   {
      return shaderLoader;
   }

   TextureLoader& getTextureLoader()
   {
      return textureLoader;
   }

private:
   ModelLoader modelLoader;
   ShaderLoader shaderLoader;
   TextureLoader textureLoader;
};
