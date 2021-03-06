#pragma once

#include "Graphics/GraphicsResource.h"

#include <glad/gl.h>

#if SWAP_DEBUG
#  include <string>
#endif // SWAP_DEBUG

enum class ShaderType : GLuint
{
   Vertex = GL_VERTEX_SHADER,
   TessellationControl = GL_TESS_CONTROL_SHADER,
   TessellationEvaluation = GL_TESS_EVALUATION_SHADER,
   Geometry = GL_GEOMETRY_SHADER,
   Fragment = GL_FRAGMENT_SHADER
};

class Shader : public GraphicsResource
{
public:
   Shader(ShaderType shaderType);
   Shader(const Shader& other) = delete;
   Shader(Shader&& other);
   ~Shader();
   Shader& operator=(const Shader& other) = delete;
   Shader& operator=(Shader&& other);

private:
   void move(Shader&& other);
   void release();

public:
   bool compile(const char* source);

   ShaderType getType() const
   {
      return type;
   }

#if SWAP_DEBUG
   const char* getTypeName() const;
   std::string getInfoLog() const;
#endif // SWAP_DEBUG

private:
   ShaderType type;
   bool compiled;
};
