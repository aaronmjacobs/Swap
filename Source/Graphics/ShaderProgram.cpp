#include "Graphics/ShaderProgram.h"

#include "Core/Assert.h"
#include "Core/Log.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Shader.h"
#include "Graphics/UniformBufferObject.h"
#include "Graphics/UniformTypes.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <sstream>
#include <utility>

namespace
{
   UPtr<Uniform> createUniform(const std::string& uniformName, const GLint uniformLocation, const GLenum uniformType, const GLuint program)
   {
      switch (uniformType)
      {
      case GL_FLOAT:
         return std::make_unique<FloatUniform>(uniformName, uniformLocation, uniformType, program);
      case GL_INT:
         return std::make_unique<IntUniform>(uniformName, uniformLocation, uniformType, program);
      case GL_UNSIGNED_INT:
         return std::make_unique<UintUniform>(uniformName, uniformLocation, uniformType, program);
      case GL_BOOL:
         return std::make_unique<BoolUniform>(uniformName, uniformLocation, uniformType, program);

      case GL_FLOAT_VEC2:
         return std::make_unique<Float2Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_FLOAT_VEC3:
         return std::make_unique<Float3Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_FLOAT_VEC4:
         return std::make_unique<Float4Uniform>(uniformName, uniformLocation, uniformType, program);

      case GL_INT_VEC2:
         return std::make_unique<Int2Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_INT_VEC3:
         return std::make_unique<Int3Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_INT_VEC4:
         return std::make_unique<Int4Uniform>(uniformName, uniformLocation, uniformType, program);

      case GL_UNSIGNED_INT_VEC2:
         return std::make_unique<Uint2Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_UNSIGNED_INT_VEC3:
         return std::make_unique<Uint3Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_UNSIGNED_INT_VEC4:
         return std::make_unique<Uint4Uniform>(uniformName, uniformLocation, uniformType, program);

      case GL_BOOL_VEC2:
         return std::make_unique<Bool2Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_BOOL_VEC3:
         return std::make_unique<Bool3Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_BOOL_VEC4:
         return std::make_unique<Bool4Uniform>(uniformName, uniformLocation, uniformType, program);

      case GL_FLOAT_MAT2:
         return std::make_unique<Float2x2Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_FLOAT_MAT2x3:
         return std::make_unique<Float2x3Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_FLOAT_MAT2x4:
         return std::make_unique<Float2x4Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_FLOAT_MAT3x2:
         return std::make_unique<Float3x2Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_FLOAT_MAT3:
         return std::make_unique<Float3x3Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_FLOAT_MAT3x4:
         return std::make_unique<Float3x4Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_FLOAT_MAT4x2:
         return std::make_unique<Float4x2Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_FLOAT_MAT4x3:
         return std::make_unique<Float4x3Uniform>(uniformName, uniformLocation, uniformType, program);
      case GL_FLOAT_MAT4:
         return std::make_unique<Float4x4Uniform>(uniformName, uniformLocation, uniformType, program);

      case GL_SAMPLER_1D:
      case GL_SAMPLER_2D:
      case GL_SAMPLER_3D:
      case GL_SAMPLER_CUBE:
      case GL_SAMPLER_1D_SHADOW:
      case GL_SAMPLER_2D_SHADOW:
      case GL_SAMPLER_1D_ARRAY:
      case GL_SAMPLER_2D_ARRAY:
      case GL_SAMPLER_1D_ARRAY_SHADOW:
      case GL_SAMPLER_2D_ARRAY_SHADOW:
      case GL_SAMPLER_2D_MULTISAMPLE:
      case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
      case GL_SAMPLER_CUBE_SHADOW:
      case GL_SAMPLER_BUFFER:
      case GL_SAMPLER_2D_RECT:
      case GL_SAMPLER_2D_RECT_SHADOW:
      case GL_INT_SAMPLER_1D:
      case GL_INT_SAMPLER_2D:
      case GL_INT_SAMPLER_3D:
      case GL_INT_SAMPLER_CUBE:
      case GL_INT_SAMPLER_1D_ARRAY:
      case GL_INT_SAMPLER_2D_ARRAY:
      case GL_INT_SAMPLER_2D_MULTISAMPLE:
      case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
      case GL_INT_SAMPLER_BUFFER:
      case GL_INT_SAMPLER_2D_RECT:
      case GL_UNSIGNED_INT_SAMPLER_1D:
      case GL_UNSIGNED_INT_SAMPLER_2D:
      case GL_UNSIGNED_INT_SAMPLER_3D:
      case GL_UNSIGNED_INT_SAMPLER_CUBE:
      case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
      case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
      case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
      case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
      case GL_UNSIGNED_INT_SAMPLER_BUFFER:
      case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
         return std::make_unique<TextureUniform>(uniformName, uniformLocation, uniformType, program);

      default:
         ASSERT(false, "Invalid uniform type: %u", uniformType);
         return nullptr;
      }
   }

   std::vector<std::string> getUniformNames(const char* nameBuf, GLsizei length, GLint size)
   {
      ASSERT(size > 0);

      if (size == 1)
      {
         // Not an array, return as is
         return { nameBuf };
      }

      ASSERT(length > 3 && nameBuf[length - 3] == '[' && nameBuf[length - 2] == '0' && nameBuf[length - 1] == ']');
      std::string baseName(nameBuf, static_cast<std::size_t>(length) - 3);

      std::vector<std::string> names(size);
      for (GLint i = 0; i < size; ++i)
      {
         std::stringstream ss;
         ss << baseName << "[" << i << "]";
         names[i] = ss.str();
      }

      return names;
   }

   void createProgramUniformsAtIndex(ShaderProgram::UniformMap& uniformMap, GLuint program, GLuint index)
   {
      std::array<GLchar, 256> nameBuf;
      GLsizei length = 0;
      GLint size = 0;
      GLenum type = GL_FLOAT;
      glGetActiveUniform(program, index, static_cast<GLsizei>(nameBuf.size()), &length, &size, &type, nameBuf.data());

      if (length < 1)
      {
         LOG_WARNING("Unable to get active uniform " << index << " for program " << program);
         return;
      }

      std::vector<std::string> uniformNames = getUniformNames(nameBuf.data(), length, size);
      for (const std::string& uniformName : uniformNames)
      {
         GLint location = glGetUniformLocation(program, uniformName.c_str());
         if (location >= 0) // Uniforms in blocks will not have a location, which is fine as they will be bound to uniform buffer objects
         {
            uniformMap.emplace(uniformName, createUniform(uniformName, location, type, program));
         }
      }
   }

   void createProgramUniforms(ShaderProgram::UniformMap& uniformMap, GLuint program)
   {
      GLint numUniforms = 0;
      glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);

      for (GLint i = 0; i < numUniforms; ++i)
      {
         createProgramUniformsAtIndex(uniformMap, program, i);
      }
   }
}

ShaderProgram::ShaderProgram()
   : GraphicsResource(GraphicsResourceType::Program)
   , linked(false)
{
   id = glCreateProgram();
}

ShaderProgram::ShaderProgram(ShaderProgram&& other)
   : GraphicsResource(GraphicsResourceType::Program)
{
   move(std::move(other));
}

ShaderProgram::~ShaderProgram()
{
   release();
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other)
{
   release();
   move(std::move(other));
   return *this;
}

void ShaderProgram::move(ShaderProgram&& other)
{
   uniforms = std::move(other.uniforms);
   shaders = std::move(other.shaders);

   linked = other.linked;
   other.linked = false;

#if SWAP_DEBUG
   onLink = std::move(other.onLink);
#endif // SWAP_DEBUG

   GraphicsResource::move(std::move(other));
}

void ShaderProgram::release()
{
   if (id != 0)
   {
      GraphicsContext::current().onProgramDestroyed(id);

      glDeleteProgram(id);
      id = 0;
   }
}

void ShaderProgram::attach(const SPtr<Shader>& shader)
{
   ASSERT(shader);

   glAttachShader(id, shader->getId());
   shaders.push_back(shader);
}

void ShaderProgram::detach(const SPtr<Shader>& shader)
{
   ASSERT(shader);

   for (auto it = shaders.begin(); it != shaders.end(); )
   {
      if (*it == shader)
      {
         glDetachShader(id, shader->getId());
         it = shaders.erase(it);
      }
      else
      {
         ++it;
      }
   }
}

bool ShaderProgram::link()
{
   ASSERT(shaders.size() >= 2, "Need at least two shaders to link (currently have %lu)", shaders.size());

#if SWAP_DEBUG
   uniforms.clear();
#else // SWAP_DEBUG
   // Don't allow re-linking shader programs in release builds
   if (linked)
   {
      return true;
   }
#endif // SWAP_DEBUG

   glLinkProgram(id);

   GLint linkStatus;
   glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);
   if (linkStatus != GL_TRUE)
   {
#if SWAP_DEBUG
      LOG_WARNING("Failed to link shader program " << id << ":\n" << getInfoLog());

      onLink.broadcast(*this, false);
#endif // SWAP_DEBUG

      return false;
   }

   createProgramUniforms(uniforms, id);
   linked = true;

#if SWAP_DEBUG
   onLink.broadcast(*this, true);

   for (const SPtr<UniformBufferObject>& uniformBuffer : uniformBuffers)
   {
      bindUniformBuffer(uniformBuffer);
   }
#endif // SWAP_DEBUG

   return true;
}

void ShaderProgram::commit()
{
   GraphicsContext::current().useProgram(id);

   for (const auto& pair : uniforms)
   {
      pair.second->commit();
   }
}

void ShaderProgram::bindUniformBuffer(const SPtr<UniformBufferObject>& buffer)
{
   ASSERT(buffer);
   ASSERT(buffer->getBoundIndex() != UniformBufferObjectIndex::Invalid);

   GLuint blockIndex = glGetUniformBlockIndex(id, buffer->getBlockName().c_str());
   if (blockIndex == GL_INVALID_INDEX)
   {
      LOG_WARNING("Uniform block not found: " << buffer->getBlockName());
      return;
   }

   glUniformBlockBinding(id, blockIndex, static_cast<GLuint>(buffer->getBoundIndex()));

#if SWAP_DEBUG
   if (std::find(uniformBuffers.begin(), uniformBuffers.end(), buffer) == uniformBuffers.end())
   {
      uniformBuffers.push_back(buffer);
   }
#endif // SWAP_DEBUG
}

#if SWAP_DEBUG
std::string ShaderProgram::getInfoLog() const
{
   std::string infoLog;

   GLint infoLogLength;
   glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);

   if (infoLogLength > 0)
   {
      // Allocating a buffer here seems silly, but std::string::data() doesn't return a non-const pointer until C++17
      UPtr<GLchar[]> rawInfoLog = std::make_unique<GLchar[]>(infoLogLength);
      glGetProgramInfoLog(id, infoLogLength, nullptr, rawInfoLog.get());

      // If the log ends in a newline, nuke it
      if (infoLogLength >= 2 && rawInfoLog[static_cast<std::size_t>(infoLogLength) - 2] == '\n')
      {
         rawInfoLog[static_cast<std::size_t>(infoLogLength) - 2] = '\0';
      }

      infoLog = rawInfoLog.get();
   }

   return infoLog;
}
#endif // SWAP_DEBUG
