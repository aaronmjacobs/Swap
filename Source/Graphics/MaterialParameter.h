#pragma once

#include "Core/Assert.h"
#include "Core/Pointers.h"

#include <glm/glm.hpp>

#include <string>

class Material;
class Texture;

class MaterialParameterBase
{
public:
   MaterialParameterBase(const std::string& paramName)
      : name(paramName)
   {
   }

   virtual void updateUniformValue(Material& owningMaterial) = 0;

#define DECLARE_SET_VALUE_VAL(param_type) virtual void setValue(param_type newValue) { ASSERT(false, "Trying to call MaterialParameterBase::setValue() with wrong data type"); }
#define DECLARE_SET_VALUE_REF(param_type) DECLARE_SET_VALUE_VAL(const param_type&)

   DECLARE_SET_VALUE_VAL(float)
   DECLARE_SET_VALUE_VAL(int)
   DECLARE_SET_VALUE_VAL(unsigned int)

   DECLARE_SET_VALUE_REF(glm::fvec2)
   DECLARE_SET_VALUE_REF(glm::fvec3)
   DECLARE_SET_VALUE_REF(glm::fvec4)

   DECLARE_SET_VALUE_REF(glm::ivec2)
   DECLARE_SET_VALUE_REF(glm::ivec3)
   DECLARE_SET_VALUE_REF(glm::ivec4)

   DECLARE_SET_VALUE_REF(glm::uvec2)
   DECLARE_SET_VALUE_REF(glm::uvec3)
   DECLARE_SET_VALUE_REF(glm::uvec4)

   DECLARE_SET_VALUE_REF(glm::fmat2x2)
   DECLARE_SET_VALUE_REF(glm::fmat2x3)
   DECLARE_SET_VALUE_REF(glm::fmat2x4)
   DECLARE_SET_VALUE_REF(glm::fmat3x2)
   DECLARE_SET_VALUE_REF(glm::fmat3x3)
   DECLARE_SET_VALUE_REF(glm::fmat3x4)
   DECLARE_SET_VALUE_REF(glm::fmat4x2)
   DECLARE_SET_VALUE_REF(glm::fmat4x3)
   DECLARE_SET_VALUE_REF(glm::fmat4x4)

   DECLARE_SET_VALUE_REF(SPtr<Texture>)

#undef DECLARE_SET_VALUE_VAL
#undef DECLARE_SET_VALUE_REF

protected:
   const std::string name;
};

#define DECLARE_MATERIAL_PARAM_TYPE(class_name_prefix, data_type, param_type)\
class class_name_prefix##MaterialParameter : public MaterialParameterBase\
{\
public:\
   class_name_prefix##MaterialParameter(const std::string& paramName)\
      : MaterialParameterBase(paramName)\
   {\
   }\
\
   void updateUniformValue(Material& owningMaterial) override;\
\
   void setValue(param_type newValue) override\
   {\
      value = newValue;\
   }\
\
private:\
   data_type value;\
};
#define DECLARE_MATERIAL_PARAM_TYPE_VAL(class_name_prefix, data_type) DECLARE_MATERIAL_PARAM_TYPE(class_name_prefix, data_type, data_type)
#define DECLARE_MATERIAL_PARAM_TYPE_REF(class_name_prefix, data_type) DECLARE_MATERIAL_PARAM_TYPE(class_name_prefix, data_type, const data_type&)

DECLARE_MATERIAL_PARAM_TYPE_VAL(Float, float)
DECLARE_MATERIAL_PARAM_TYPE_VAL(Int, int)
DECLARE_MATERIAL_PARAM_TYPE_VAL(UInt, unsigned int)

DECLARE_MATERIAL_PARAM_TYPE_REF(FVec2, glm::fvec2)
DECLARE_MATERIAL_PARAM_TYPE_REF(FVec3, glm::fvec3)
DECLARE_MATERIAL_PARAM_TYPE_REF(FVec4, glm::fvec4)

DECLARE_MATERIAL_PARAM_TYPE_REF(IVec2, glm::ivec2)
DECLARE_MATERIAL_PARAM_TYPE_REF(IVec3, glm::ivec3)
DECLARE_MATERIAL_PARAM_TYPE_REF(IVec4, glm::ivec4)

DECLARE_MATERIAL_PARAM_TYPE_REF(UVec2, glm::uvec2)
DECLARE_MATERIAL_PARAM_TYPE_REF(UVec3, glm::uvec3)
DECLARE_MATERIAL_PARAM_TYPE_REF(UVec4, glm::uvec4)

DECLARE_MATERIAL_PARAM_TYPE_REF(FMat22, glm::fmat2x2)
DECLARE_MATERIAL_PARAM_TYPE_REF(FMat23, glm::fmat2x3)
DECLARE_MATERIAL_PARAM_TYPE_REF(FMat24, glm::fmat2x4)
DECLARE_MATERIAL_PARAM_TYPE_REF(FMat32, glm::fmat3x2)
DECLARE_MATERIAL_PARAM_TYPE_REF(FMat33, glm::fmat3x3)
DECLARE_MATERIAL_PARAM_TYPE_REF(FMat34, glm::fmat3x4)
DECLARE_MATERIAL_PARAM_TYPE_REF(FMat42, glm::fmat4x2)
DECLARE_MATERIAL_PARAM_TYPE_REF(FMat43, glm::fmat4x3)
DECLARE_MATERIAL_PARAM_TYPE_REF(FMat44, glm::fmat4x4)

DECLARE_MATERIAL_PARAM_TYPE_REF(Texture, SPtr<Texture>)

#undef DECLARE_MATERIAL_PARAM_TYPE
#undef DECLARE_MATERIAL_PARAM_TYPE_VAL
#undef DECLARE_MATERIAL_PARAM_TYPE_REF
