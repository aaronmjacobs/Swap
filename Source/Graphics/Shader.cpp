#include "Graphics/Shader.h"

#include "Core/Log.h"
#include "Core/Pointers.h"

#include <utility>

namespace
{
#if SWAP_DEBUG
   const char* getShaderTypeName(ShaderType type)
   {
      switch (type)
      {
      case ShaderType::Vertex:
         return "vertex";
      case ShaderType::TessellationControl:
         return "tessellation control";
      case ShaderType::TessellationEvaluation:
         return "tessellation evaluation";
      case ShaderType::Geometry:
         return "geometry";
      case ShaderType::Fragment:
         return "fragment";
      default:
         return "invalid";
      }
   }

   void logShaderCompileError(GLuint id, ShaderType type)
   {
      GLint infoLogLength;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);

      if (infoLogLength < 1)
      {
         return;
      }

      UPtr<GLchar[]> infoLog = std::make_unique<GLchar[]>(infoLogLength);
      glGetShaderInfoLog(id, infoLogLength, nullptr, infoLog.get());

      // If the log ends in a newline, nuke it
      if (infoLogLength >= 2 && infoLog[infoLogLength - 2] == '\n')
      {
         infoLog[infoLogLength - 2] = '\0';
      }

      LOG_WARNING("Failed to compile " << getShaderTypeName(type) << " shader " << id << ":\n" << infoLog.get());
   }
#endif // SWAP_DEBUG
}

Shader::Shader(ShaderType shaderType)
   : id(glCreateShader(static_cast<GLuint>(shaderType)))
   , type(shaderType)
{
}

Shader::Shader(Shader&& other)
{
   move(std::move(other));
}

Shader::~Shader()
{
   release();
}

Shader& Shader::operator=(Shader&& other)
{
   release();
   move(std::move(other));
   return *this;
}

void Shader::move(Shader&& other)
{
   id = other.id;
   other.id = 0;

   type = other.type;
}

void Shader::release()
{
   if (id != 0)
   {
      glDeleteShader(id);
      id = 0;
   }
}

bool Shader::compile(const char* source)
{
   glShaderSource(id, 1, &source, nullptr);
   glCompileShader(id);

   GLint status = 0;
   glGetShaderiv(id, GL_COMPILE_STATUS, &status);
   bool success = status == GL_TRUE;

#if SWAP_DEBUG
   if (!success)
   {
      logShaderCompileError(id, type);
   }
#endif // SWAP_DEBUG

   return success;
}
