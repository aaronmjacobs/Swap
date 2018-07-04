#pragma once

#include "Core/Hash.h"
#include "Core/Pointers.h"
#include "Graphics/Shader.h"

#include <glad/glad.h>

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
      return type == other.type && path == other.path && definitions == other.definitions;
   }
};

using ShaderSourceMap = std::unordered_map<std::string, std::string>;
using ShaderMap = std::unordered_map<ShaderSpecification, WPtr<Shader>>;
using ShaderPrograMap = std::unordered_map<std::vector<ShaderSpecification>, WPtr<ShaderProgram>>;
#if SWAP_DEBUG
using InverseShaderMap = std::unordered_map<Shader*, ShaderSpecification>;
#endif // SWAP_DEBUG

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
   SPtr<ShaderProgram> loadShaderProgram(std::vector<ShaderSpecification> specifications);

   void reloadShaders();

private:
   ShaderSourceMap shaderSourceMap;
   ShaderMap shaderMap;
   ShaderPrograMap shaderProgramMap;

#if SWAP_DEBUG
   InverseShaderMap inverseShaderMap;
#endif // SWAP_DEBUG

   SPtr<Shader> defaultVertexShader;
   SPtr<Shader> defaultGeometryShader;
   SPtr<Shader> defaultFragmentShader;
   SPtr<ShaderProgram> defaultShaderProgram;
};
