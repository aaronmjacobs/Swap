#include "Resources/ModelLoader.h"

#include "Core/Log.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/Model.h"
#include "Graphics/ShaderProgram.h"
#include "Platform/IOUtils.h"
#include "Resources/TextureLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cstring>

namespace
{
   SPtr<Texture> loadMaterialTexture(const aiMaterial& assimpMaterial, aiTextureType textureType,
      const std::string& directory, TextureLoader& textureLoader)
   {
      if (assimpMaterial.GetTextureCount(textureType) > 0)
      {
         aiString textureName;
         if (assimpMaterial.GetTexture(textureType, 0, &textureName) == aiReturn_SUCCESS)
         {
            return textureLoader.loadTexture(directory + "/" + textureName.C_Str());
         }
      }

      return nullptr;
   }

   Material processAssimpMaterial(const aiMaterial& assimpMaterial,
      const std::vector<ShaderSpecification>& shaderSpecifications, const std::string& directory,
      ShaderLoader& shaderLoader, TextureLoader& textureLoader)
   {
      SPtr<Texture> diffuseTexture = loadMaterialTexture(assimpMaterial, aiTextureType_DIFFUSE, directory,
         textureLoader);
      SPtr<Texture> normalTexture = loadMaterialTexture(assimpMaterial, aiTextureType_NORMALS, directory,
         textureLoader);
      if (!normalTexture)
      {
         normalTexture = loadMaterialTexture(assimpMaterial, aiTextureType_HEIGHT, directory, textureLoader);
      }
      SPtr<Texture> specularTexture = loadMaterialTexture(assimpMaterial, aiTextureType_SPECULAR, directory,
         textureLoader);

      std::vector<ShaderSpecification> localSpecifications = shaderSpecifications;
      for (ShaderSpecification& shaderSpecification : localSpecifications)
      {
         shaderSpecification.definitions["WITH_DIFFUSE_TEXTURE"] = diffuseTexture ? "1" : "0";
         shaderSpecification.definitions["WITH_NORMAL_TEXTURE"] = normalTexture ? "1" : "0";
         shaderSpecification.definitions["WITH_SPECULAR_TEXTURE"] = specularTexture ? "1" : "0";
      }

      SPtr<ShaderProgram> shaderProgram = shaderLoader.loadShaderProgram(localSpecifications);
      Material material(shaderProgram);

      if (diffuseTexture && material.hasParameter("uDiffuseTexture"))
      {
         material.setParameter("uDiffuseTexture", diffuseTexture);
      }
      if (normalTexture && material.hasParameter("uNormalTexture"))
      {
         material.setParameter("uNormalTexture", normalTexture);
      }
      if (specularTexture && material.hasParameter("uSpecularTexture"))
      {
         material.setParameter("uSpecularTexture", specularTexture);
      }

      return material;
   }

   Mesh processAssimpMesh(const aiMesh& assimpMesh)
   {
      MeshData meshData;

      std::vector<GLuint> indices(assimpMesh.mNumFaces * 3);
      for (unsigned int i = 0; i < assimpMesh.mNumFaces; ++i)
      {
         const aiFace& face = assimpMesh.mFaces[i];
         ASSERT(face.mNumIndices == 3);

         std::memcpy(&indices[i * 3], face.mIndices, 3 * sizeof(GLuint));
      }
      meshData.indices = indices;

      if (assimpMesh.mNumVertices > 0)
      {
         ASSERT(assimpMesh.mVertices);

         meshData.positions.values = gsl::span<GLfloat>(&assimpMesh.mVertices[0].x, assimpMesh.mNumVertices * 3);
         meshData.positions.valueSize = 3;
      }

      if (assimpMesh.mNormals)
      {
         meshData.normals.values = gsl::span<GLfloat>(&assimpMesh.mNormals[0].x, assimpMesh.mNumVertices * 3);
         meshData.normals.valueSize = 3;
      }

      std::vector<GLfloat> texCoords;
      if (assimpMesh.mTextureCoords && assimpMesh.mTextureCoords[0])
      {
         texCoords.resize(assimpMesh.mNumVertices * 2);

         for (unsigned int i = 0; i < assimpMesh.mNumVertices; ++i)
         {
            texCoords[2 * i + 0] = assimpMesh.mTextureCoords[0][i].x;
            texCoords[2 * i + 1] = assimpMesh.mTextureCoords[0][i].y;
         }
      }
      meshData.texCoords.values = texCoords;
      meshData.texCoords.valueSize = 2;

      if (assimpMesh.mTangents)
      {
         meshData.tangents.values = gsl::span<GLfloat>(&assimpMesh.mTangents[0].x, assimpMesh.mNumVertices * 3);
         meshData.tangents.valueSize = 3;
      }

      if (assimpMesh.mBitangents)
      {
         meshData.bitangents.values = gsl::span<GLfloat>(&assimpMesh.mBitangents[0].x, assimpMesh.mNumVertices * 3);
         meshData.bitangents.valueSize = 3;
      }

      if (assimpMesh.mColors && assimpMesh.mColors[0])
      {
         meshData.colors.values = gsl::span<GLfloat>(&assimpMesh.mColors[0][0].r, assimpMesh.mNumVertices * 4);
         meshData.colors.valueSize = 4;
      }

      Mesh mesh;
      mesh.setData(meshData);

      return mesh;
   }

   void processAssimpNode(Model& model, const aiScene& assimpScene, const aiNode& assimpNode,
      const std::vector<ShaderSpecification>& shaderSpecifications, const std::string& directory,
      ShaderLoader& shaderLoader, TextureLoader& textureLoader)
   {
      for (unsigned int i = 0; i < assimpNode.mNumMeshes; ++i)
      {
         const aiMesh& assimpMesh = *assimpScene.mMeshes[assimpNode.mMeshes[i]];

         Mesh mesh = processAssimpMesh(assimpMesh);
         Material material = processAssimpMaterial(*assimpScene.mMaterials[assimpMesh.mMaterialIndex],
            shaderSpecifications, directory, shaderLoader, textureLoader);

         model.addSection(ModelSection(std::move(mesh), std::move(material)));
      }

      for (unsigned int i = 0; i < assimpNode.mNumChildren; ++i)
      {
         processAssimpNode(model, assimpScene, *assimpNode.mChildren[i], shaderSpecifications, directory, shaderLoader,
            textureLoader);
      }
   }

   SPtr<Model> loadModelFromFile(ModelSpecification& specification, ShaderLoader& shaderLoader,
      TextureLoader& textureLoader)
   {
      std::string directory;
      if (!IOUtils::getDirectory(specification.path, directory))
      {
         LOG_ERROR("Unable to get directory from model file path: " << specification.path);
         return nullptr;
      }

      Assimp::Importer importer;
      const aiScene* assimpScene = importer.ReadFile(specification.path,
         aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
      if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
      {
         LOG_ERROR("Unable to load model from file (" << specification.path << "): " << importer.GetErrorString());
         return nullptr;
      }

      SPtr<Model> model(new Model);
      processAssimpNode(*model, *assimpScene, *assimpScene->mRootNode, specification.shaderSpecifications, directory,
         shaderLoader, textureLoader);

      return model;
   }
}

namespace std
{
   size_t hash<ModelSpecification>::operator()(const ModelSpecification& specification) const
   {
      size_t seed = 0;

      Hash::combine(seed, specification.path);

      for (const auto& shaderSpecification : specification.shaderSpecifications)
      {
         Hash::combine(seed, shaderSpecification);
      }

      return seed;
   }
}

SPtr<Model> ModelLoader::loadModel(ModelSpecification specification, ShaderLoader& shaderLoader,
   TextureLoader& textureLoader)
{
   auto location = modelMap.find(specification);
   if (location != modelMap.end())
   {
      SPtr<Model> model = location->second.lock();
      if (model)
      {
         return model;
      }
   }

   SPtr<Model> model = loadModelFromFile(specification, shaderLoader, textureLoader);
   if (model)
   {
      modelMap.emplace(std::move(specification), WPtr<Model>(model));
   }

   return model;
}

void ModelLoader::clearCachedData()
{
   modelMap.clear();
}