#include "Resources/ResourceManager.h"

SPtr<Model> ResourceManager::loadModel(const ModelSpecification& specification)
{
   return modelLoader.loadModel(specification, textureLoader);
}

SPtr<Shader> ResourceManager::loadShader(const ShaderSpecification& specification)
{
   return shaderLoader.loadShader(specification);
}

SPtr<ShaderProgram> ResourceManager::loadShaderProgram(const std::vector<ShaderSpecification>& specifications)
{
   return shaderLoader.loadShaderProgram(specifications);
}

SPtr<Texture> ResourceManager::loadTexture(const LoadedTextureSpecification& specification)
{
   return textureLoader.loadTexture(specification);
}

SPtr<Texture> ResourceManager::loadCubemap(const LoadedCubemapSpecification& specification)
{
   return textureLoader.loadCubemap(specification);
}

void ResourceManager::clearCachedData()
{
   modelLoader.clearCachedData();
   shaderLoader.clearCachedData();
   textureLoader.clearCachedData();
}
