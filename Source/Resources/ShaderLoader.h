#pragma once

#include "Core/Hash.h"
#include "Core/Pointers.h"
#include "Graphics/Shader.h"

#include <glad/glad.h>
#include <gsl/span>

#include <string>
#include <unordered_map>

class ShaderProgram;

using ShaderDefinitions = std::unordered_map<std::string, std::string>;

struct ShaderSpecification
{
   ShaderDefinitions definitions;
   std::string path;
   ShaderType type = ShaderType::Vertex;

   bool operator==(const ShaderSpecification& other) const
   {
      return definitions == other.definitions && path == other.path && type == other.type;
   }
};

// Provide a template specialization to allow using ShaderSpecification as a key in std::unordered_map
namespace std
{
   template<>
   struct hash<ShaderSpecification>
   {
      size_t operator()(const ShaderSpecification& specification) const;
   };
}

class ShaderLoader
{
public:
   SPtr<Shader> loadShader(const ShaderSpecification& specification);
   SPtr<ShaderProgram> loadShaderProgram(gsl::span<ShaderSpecification> specifications);

   void reloadShaders();

private:
   std::unordered_map<ShaderSpecification, WPtr<Shader>> shaderMap;
   std::unordered_map<gsl::span<ShaderSpecification>, WPtr<ShaderProgram>> shaderProgramMap;

#if SWAP_DEBUG
   std::unordered_map<Shader*, ShaderSpecification> inverseShaderMap;
#endif // SWAP_DEBUG

   SPtr<Shader> defaultVertexShader;
   SPtr<Shader> defaultGeometryShader;
   SPtr<Shader> defaultFragmentShader;
   SPtr<ShaderProgram> defaultShaderProgram;
};
