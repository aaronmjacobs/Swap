#pragma once

#include "Core/Pointers.h"
#include "Graphics/Shader.h"
#include "Graphics/Uniform.h"

#include <glad/glad.h>
#include <gsl/span>

#include <unordered_map>

class ShaderProgram
{
public:
   using UniformMap = std::unordered_map<std::string, UPtr<Uniform>>;

   ShaderProgram();
   ShaderProgram(const ShaderProgram& other) = delete;
   ShaderProgram(ShaderProgram&& other);
   ~ShaderProgram();
   ShaderProgram& operator=(const ShaderProgram& other) = delete;
   ShaderProgram& operator=(ShaderProgram&& other);

private:
   void move(ShaderProgram&& other);
   void release();

public:
   bool link(gsl::span<Shader> shaders);
   void commit();

   bool hasUniform(const std::string& name) const
   {
      return uniforms.count(name) > 0;
   }

   template<typename T>
   void setUniformValue(const std::string& name, const T& value)
   {
      auto itr = uniforms.find(name);

      if (itr != uniforms.end())
      {
         itr->second->setValue(value);
      }
      else
      {
         ASSERT(false, "Uniform with given name doesn't exist: %s", name.c_str());
      }
   }

private:
   GLuint id;
   UniformMap uniforms;
};