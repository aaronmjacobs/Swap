#pragma once

#include "Resources/ModelLoader.h"
#include "Resources/ShaderLoader.h"
#include "Resources/TextureLoader.h"

class ResourceManager
{
public:
   Model loadModel(const ModelSpecification& specification);

   SPtr<Shader> loadShader(const ShaderSpecification& specification);
   SPtr<ShaderProgram> loadShaderProgram(const std::vector<ShaderSpecification>& specifications);

   SPtr<Texture> loadTexture(const LoadedTextureSpecification& specification);
   SPtr<Texture> loadCubemap(const LoadedCubemapSpecification& specification);

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
