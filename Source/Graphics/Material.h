#pragma once

#include "Core/Assert.h"
#include "Core/Pointers.h"

#include <string>
#include <unordered_map>

class MaterialParameterBase;
class ShaderProgram;

class Material
{
public:
   Material(const SPtr<ShaderProgram>& program);

   void commit();

   template<typename T>
   void setParameter(const std::string& name, const T& value);

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

private:
   std::unordered_map<std::string, UPtr<MaterialParameterBase>> parameters;
   SPtr<ShaderProgram> shaderProgram;
   int textureUnitCounter;
};
