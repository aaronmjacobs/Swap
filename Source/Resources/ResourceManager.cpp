#include "Resources/ResourceManager.h"

SPtr<Model> ResourceManager::loadModel(const ModelSpecification& specification)
{
   return modelLoader.loadModel(specification, shaderLoader, textureLoader);
}

SPtr<Shader> ResourceManager::loadShader(const ShaderSpecification& specification)
{
   return shaderLoader.loadShader(specification);
}

SPtr<ShaderProgram> ResourceManager::loadShaderProgram(const std::vector<ShaderSpecification>& specifications)
{
   return shaderLoader.loadShaderProgram(specifications);
}

SPtr<Texture> ResourceManager::loadTexture(const std::string& path, Tex::Wrap wrap /*= Tex::Wrap::Repeat*/,
	Tex::MinFilter minFilter /*= Tex::MinFilter::NearestMipmapLinear*/,
	Tex::MagFilter magFilter /*= Tex::MagFilter::Linear*/)
{
   return textureLoader.loadTexture(path, wrap, minFilter, magFilter);
}

SPtr<Texture> ResourceManager::loadCubemap(const std::array<std::string, 6>& paths,
	Tex::Wrap wrap /*= Tex::Wrap::Repeat*/, Tex::MinFilter minFilter /*= Tex::MinFilter::NearestMipmapLinear*/,
	Tex::MagFilter magFilter /*= Tex::MagFilter::Linear*/)
{
   return textureLoader.loadCubemap(paths, wrap, minFilter, magFilter);
}

void ResourceManager::clearCachedData()
{
   modelLoader.clearCachedData();
   shaderLoader.clearCachedData();
   textureLoader.clearCachedData();
}
