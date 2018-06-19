#pragma once

#include <glad/glad.h>

enum class ShaderType : GLuint
{
   Vertex = GL_VERTEX_SHADER,
   TessellationControl = GL_TESS_CONTROL_SHADER,
   TessellationEvaluation = GL_TESS_EVALUATION_SHADER,
   Geometry = GL_GEOMETRY_SHADER,
   Fragment = GL_FRAGMENT_SHADER
};

class Shader
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

   GLuint getId() const
   {
      return id;
   }

private:
   GLuint id;
   ShaderType type;
};
