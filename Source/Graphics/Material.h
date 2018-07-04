#pragma once

#include "Core/Assert.h"
#include "Core/Pointers.h"
#include "Graphics/MaterialParameter.h"

#include <string>
#include <unordered_map>

class ShaderProgram;

class Material
{
public:
   Material(const SPtr<ShaderProgram>& program);

   void commit();

   template<typename T>
   void setParameter(const std::string& name, const T& value)
   {
      auto location = parameters.find(name);
      if (location != parameters.end())
      {
         location->second->setValue(value);
      }
      else
      {
         ASSERT(false, "Material parameter with given name doesn't exist: %s", name.c_str());
      }
   }

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

   bool hasParameter(const std::string& name)
   {
      return parameters.count(name) > 0;
   }

private:
   std::unordered_map<std::string, UPtr<MaterialParameterBase>> parameters;
   const SPtr<ShaderProgram> shaderProgram;
   int textureUnitCounter;
};
