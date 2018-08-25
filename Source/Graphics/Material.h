#pragma once

#include "Core/Assert.h"
#if SWAP_DEBUG
#include "Core/Delegate.h"
#endif // SWAP_DEBUG
#include "Core/Pointers.h"
#include "Graphics/MaterialParameter.h"

#include <string>
#include <unordered_map>

class ShaderProgram;

class Material
{
public:
   Material(const SPtr<ShaderProgram>& program);
   Material(const Material& other) = delete;
   Material(Material&& other);
   ~Material();
   Material& operator=(const Material& other) = delete;
   Material& operator=(Material&& other);

private:
   void move(Material&& other);
   void release();

public:
   void commit();

   template<typename T>
   bool setParameter(const std::string& name, const T& value, bool assertOnFailure = true)
   {
      bool success = false;

      auto location = parameters.find(name);
      if (location != parameters.end())
      {
         success = location->second->setValue(value);
      }

      ASSERT(success || !assertOnFailure, "Material parameter with given name doesn't exist: %s", name.c_str());

      return success;
   }

   bool isParameterEnabled(const std::string& name) const;
   void setParameterEnabled(const std::string& name, bool enabled);

   ShaderProgram& getShaderProgram()
   {
      ASSERT(shaderProgram);
      return *shaderProgram;
   }

   int consumeTextureUnit()
   {
      ASSERT(textureUnitCounter >= 0 && textureUnitCounter < 16);
      return textureUnitCounter++;
   }

   bool hasParameter(const std::string& name) const
   {
      return parameters.count(name) > 0;
   }

private:
   void generateParameters();

#if SWAP_DEBUG
   void bindOnLinkDelegate();
   DelegateHandle onLinkDelegateHandle;
#endif // SWAP_DEBUG

   std::unordered_map<std::string, UPtr<MaterialParameterBase>> parameters;
   SPtr<ShaderProgram> shaderProgram;
   int textureUnitCounter;
};
