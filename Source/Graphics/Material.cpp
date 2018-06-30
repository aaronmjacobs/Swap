#include "Graphics/Material.h"

#include "Graphics/MaterialParameter.h"
#include "Graphics/ShaderProgram.h"

#include <glm/glm.hpp>

Material::Material(const SPtr<ShaderProgram>& program)
   : shaderProgram(program)
   , textureUnitCounter(0)
{
   ASSERT(shaderProgram);
}

void Material::commit()
{
   textureUnitCounter = 0;

   for (auto& pair : parameters)
   {
      pair.second->updateUniformValue(*this);
   }

   shaderProgram->commit();
}

#define DEFINE_SET_PARAMETER_TYPE(class_name_prefix, data_type)\
template<>\
void Material::setParameter(const std::string& name, const data_type& value)\
{\
   auto location = parameters.find(name);\
\
   if (location == parameters.end())\
   {\
      auto pair = parameters.emplace(name, std::make_unique<class_name_prefix##MaterialParameter>(name));\
      location = pair.first;\
   }\
\
   location->second->setValue(value);\
}

DEFINE_SET_PARAMETER_TYPE(Float, float)
DEFINE_SET_PARAMETER_TYPE(Int, int)
DEFINE_SET_PARAMETER_TYPE(UInt, unsigned int)

DEFINE_SET_PARAMETER_TYPE(FVec2, glm::fvec2)
DEFINE_SET_PARAMETER_TYPE(FVec3, glm::fvec3)
DEFINE_SET_PARAMETER_TYPE(FVec4, glm::fvec4)

DEFINE_SET_PARAMETER_TYPE(IVec2, glm::ivec2)
DEFINE_SET_PARAMETER_TYPE(IVec3, glm::ivec3)
DEFINE_SET_PARAMETER_TYPE(IVec4, glm::ivec4)

DEFINE_SET_PARAMETER_TYPE(UVec2, glm::uvec2)
DEFINE_SET_PARAMETER_TYPE(UVec3, glm::uvec3)
DEFINE_SET_PARAMETER_TYPE(UVec4, glm::uvec4)

DEFINE_SET_PARAMETER_TYPE(FMat2x2, glm::fmat2x2)
DEFINE_SET_PARAMETER_TYPE(FMat2x3, glm::fmat2x3)
DEFINE_SET_PARAMETER_TYPE(FMat2x4, glm::fmat2x4)
DEFINE_SET_PARAMETER_TYPE(FMat3x2, glm::fmat3x2)
DEFINE_SET_PARAMETER_TYPE(FMat3x3, glm::fmat3x3)
DEFINE_SET_PARAMETER_TYPE(FMat3x4, glm::fmat3x4)
DEFINE_SET_PARAMETER_TYPE(FMat4x2, glm::fmat4x2)
DEFINE_SET_PARAMETER_TYPE(FMat4x3, glm::fmat4x3)
DEFINE_SET_PARAMETER_TYPE(FMat4x4, glm::fmat4x4)

DEFINE_SET_PARAMETER_TYPE(Texture, SPtr<Texture>)

#undef DEFINE_SET_PARAMETER_TYPE
