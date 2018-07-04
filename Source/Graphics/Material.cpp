#include "Graphics/Material.h"

#include "Core/Assert.h"
#include "Graphics/MaterialParameter.h"
#include "Graphics/ShaderProgram.h"

#include <glm/glm.hpp>

namespace
{
   UPtr<MaterialParameterBase> createParameter(const std::string& name, UniformType type)
   {
      switch (type)
      {
#define UNIFORM_TYPE_CASE(uniform_type, data_type, param_type) case UniformType::uniform_type: return std::make_unique<uniform_type##MaterialParameter>(name);

#define FOR_EACH_UNIFORM_TYPE UNIFORM_TYPE_CASE
#include "ForEachUniformType.inl"
#undef FOR_EACH_UNIFORM_TYPE

#undef UNIFORM_TYPE_CASE
      default:
         ASSERT(false, "Invalid uniform type: %u", type);
         return nullptr;
      }
   }
}

Material::Material(const SPtr<ShaderProgram>& program)
   : shaderProgram(program)
   , textureUnitCounter(0)
{
   ASSERT(shaderProgram);

   for (const auto& pair : shaderProgram->getUniforms())
   {
      UPtr<MaterialParameterBase> parameter = createParameter(pair.first, pair.second->getType());
      if (parameter)
      {
         parameters.emplace(pair.first, std::move(parameter));
      }
   }
}

void Material::commit()
{
   textureUnitCounter = 0;

   for (auto& pair : parameters)
   {
      pair.second->commit(*this);
   }

   shaderProgram->commit();
}
