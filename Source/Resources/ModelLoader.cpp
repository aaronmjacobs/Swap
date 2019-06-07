#include "Resources/ModelLoader.h"

#include "Core/Hash.h"
#include "Core/Log.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/Model.h"
#include "Platform/IOUtils.h"
#include "Resources/TextureLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cstring>

namespace
{
   SPtr<Texture> loadMaterialTexture(const aiMaterial& assimpMaterial, aiTextureType textureType,
      const LoadedTextureParameters& textureParams, const std::string& directory, bool cache,
      TextureLoader& textureLoader)
   {
      if (assimpMaterial.GetTextureCount(textureType) > 0)
      {
         aiString textureName;
         if (assimpMaterial.GetTexture(textureType, 0, &textureName) == aiReturn_SUCCESS)
         {
            LoadedTextureSpecification specification;

            specification.path = directory + "/" + textureName.C_Str();
            specification.params = textureParams;
            specification.cache = cache;

            return textureLoader.loadTexture(specification);
         }
      }

      return nullptr;
   }

   Material processAssimpMaterial(const aiMaterial& assimpMaterial, const ModelSpecification& specification,
      const std::string& directory, TextureLoader& textureLoader)
   {
      const LoadedTextureParameters& textureParams = specification.textureParams;
      bool cache = specification.cacheTextures;

      SPtr<Texture> diffuseTexture = loadMaterialTexture(assimpMaterial, aiTextureType_DIFFUSE, textureParams,
         directory, cache, textureLoader);
      SPtr<Texture> specularTexture = loadMaterialTexture(assimpMaterial, aiTextureType_SPECULAR, textureParams,
         directory, cache, textureLoader);
      SPtr<Texture> normalTexture = loadMaterialTexture(assimpMaterial, aiTextureType_NORMALS, textureParams,
         directory, cache, textureLoader);
      if (!normalTexture)
      {
         normalTexture = loadMaterialTexture(assimpMaterial, aiTextureType_HEIGHT, textureParams, directory,
            cache, textureLoader);
      }

      Material material;

      if (diffuseTexture)
      {
         material.setParameter(CommonMaterialParameterNames::get(CommonMaterialParameter::DiffuseTexture), diffuseTexture);
      }
      if (specularTexture)
      {
         material.setParameter(CommonMaterialParameterNames::get(CommonMaterialParameter::SpecularTexture), specularTexture);
      }
      if (normalTexture)
      {
         material.setParameter(CommonMaterialParameterNames::get(CommonMaterialParameter::NormalTexture), normalTexture);
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
      if (assimpMesh.mTextureCoords && assimpMesh.mTextureCoords[0] && assimpMesh.mNumUVComponents[0] == 2)
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
      const ModelSpecification& specification, const std::string& directory, TextureLoader& textureLoader)
   {
      for (unsigned int i = 0; i < assimpNode.mNumMeshes; ++i)
      {
         const aiMesh& assimpMesh = *assimpScene.mMeshes[assimpNode.mMeshes[i]];

         Mesh mesh = processAssimpMesh(assimpMesh);
         Material material = processAssimpMaterial(*assimpScene.mMaterials[assimpMesh.mMaterialIndex], specification,
            directory, textureLoader);

         model.addSection(ModelSection(std::move(mesh), std::move(material)));
      }

      for (unsigned int i = 0; i < assimpNode.mNumChildren; ++i)
      {
         processAssimpNode(model, assimpScene, *assimpNode.mChildren[i], specification, directory, textureLoader);
      }
   }

   SPtr<Model> loadModelFromFile(const ModelSpecification& specification, TextureLoader& textureLoader)
   {
      std::string directory;
      if (!IOUtils::getSanitizedDirectory(specification.path, directory))
      {
         LOG_ERROR("Unable to get directory from model file path: " << specification.path);
         return nullptr;
      }

      unsigned int normalFlag = 0;
      switch (specification.normalGenerationMode)
      {
      case NormalGenerationMode::Flat:
         normalFlag = aiProcess_GenNormals;
         break;
      case NormalGenerationMode::Smooth:
         normalFlag = aiProcess_GenSmoothNormals;
         break;
      default:
         break;
      }
      unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | normalFlag;

      Assimp::Importer importer;
      const aiScene* assimpScene = importer.ReadFile(specification.path, flags);
      if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
      {
         LOG_ERROR("Unable to load model from file (" << specification.path << "): " << importer.GetErrorString());
         return nullptr;
      }

      SPtr<Model> model(new Model);
      processAssimpNode(*model, *assimpScene, *assimpScene->mRootNode, specification, directory, textureLoader);

      return model;
   }
}

namespace std
{
   size_t hash<ModelSpecification>::operator()(const ModelSpecification& specification) const
   {
      size_t seed = 0;

      Hash::combine(seed, specification.path);

      Hash::combine(seed, specification.textureParams.wrap);
      Hash::combine(seed, specification.textureParams.minFilter);
      Hash::combine(seed, specification.textureParams.magFilter);

      return seed;
   }
}

SPtr<Model> ModelLoader::loadModel(const ModelSpecification& specification, TextureLoader& textureLoader)
{
   if (specification.cache)
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
   }

   SPtr<Model> model = loadModelFromFile(specification, textureLoader);
   if (model && specification.cache)
   {
      modelMap.emplace(specification, WPtr<Model>(model));
   }

   return model;
}

void ModelLoader::clearCachedData()
{
   modelMap.clear();
}
