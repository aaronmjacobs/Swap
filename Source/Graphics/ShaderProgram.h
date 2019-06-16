#pragma once

#include "Core/Assert.h"
#if SWAP_DEBUG
#include "Core/Delegate.h"
#endif // SWAP_DEBUG
#include "Core/Pointers.h"
#include "Graphics/Uniform.h"

#include <glad/glad.h>

#include <string>
#include <unordered_map>
#include <vector>

class Shader;
class UniformBufferObject;

class ShaderProgram
{
public:
   using UniformMap = std::unordered_map<std::string, UPtr<Uniform>>;
#if SWAP_DEBUG
   using OnLinkDelegate = MulticastDelegate<void, ShaderProgram& /* shaderProgram */, bool /* success */>;
#endif // SWAP_DEBUG

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
   void attach(const SPtr<Shader>& shader);
   void detach(const SPtr<Shader>& shader);
   bool link();
   void commit();

   bool hasUniform(const std::string& name) const
   {
      return uniforms.count(name) > 0;
   }

   template<typename T>
   bool setUniformValue(const std::string& name, const T& value, bool assertOnFailure = true)
   {
      auto location = uniforms.find(name);
      if (location != uniforms.end())
      {
         location->second->setValue(value);
         return true;
      }

      ASSERT(!assertOnFailure, "Uniform with given name doesn't exist: %s", name.c_str());
      return false;
   }

   void bindUniformBuffer(const UniformBufferObject& buffer);

   GLuint getId() const
   {
      return id;
   }

   const UniformMap& getUniforms() const
   {
      return uniforms;
   }

#if SWAP_DEBUG
   DelegateHandle addOnLinkDelegate(OnLinkDelegate::FuncType&& function)
   {
      return onLink.add(std::move(function));
   }

   void removeOnLinkDelegate(const DelegateHandle& handle)
   {
      onLink.remove(handle);
   }

   const std::vector<SPtr<Shader>>& getAttachedShaders() const
   {
      return shaders;
   }

   std::string getInfoLog() const;
#endif // SWAP_DEBUG

private:
   GLuint id;
   UniformMap uniforms;
   std::vector<SPtr<Shader>> shaders;
   bool linked;

#if SWAP_DEBUG
   OnLinkDelegate onLink;
#endif // SWAP_DEBUG
};
