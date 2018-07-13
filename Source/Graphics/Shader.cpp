#include "Graphics/Shader.h"

#include "Core/Log.h"
#include "Core/Pointers.h"

#include <utility>

Shader::Shader(ShaderType shaderType)
   : id(glCreateShader(static_cast<GLuint>(shaderType)))
   , type(shaderType)
   , compiled(false)
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
#if !SWAP_DEBUG
   // Don't allow re-compiling shaders in release builds
   if (compiled)
   {
      return true;
   }
#endif // !SWAP_DEBUG

   glShaderSource(id, 1, &source, nullptr);
   glCompileShader(id);

   GLint status = 0;
   glGetShaderiv(id, GL_COMPILE_STATUS, &status);
   bool success = status == GL_TRUE;

   if (success)
   {
      compiled = true;
   }

#if SWAP_DEBUG
   if (!success)
   {
      LOG_WARNING("Failed to compile " << getTypeName() << " shader " << id << ":\n" << getInfoLog());
   }
#endif // SWAP_DEBUG

   return success;
}

const char* Shader::getTypeName() const
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

#if SWAP_DEBUG
std::string Shader::getInfoLog() const
{
   std::string infoLog;

   GLint infoLogLength;
   glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);

   if (infoLogLength > 0)
   {
      // Allocating a buffer here seems silly, but std::string::data() doesn't return a non-const pointer until C++17
      UPtr<GLchar[]> rawInfoLog = std::make_unique<GLchar[]>(infoLogLength);
      glGetShaderInfoLog(id, infoLogLength, nullptr, rawInfoLog.get());

      // If the log ends in a newline, nuke it
      if (infoLogLength >= 2 && rawInfoLog[infoLogLength - 2] == '\n')
      {
         rawInfoLog[infoLogLength - 2] = '\0';
      }

      infoLog = rawInfoLog.get();
   }

   return infoLog;
}
#endif // SWAP_DEBUG
