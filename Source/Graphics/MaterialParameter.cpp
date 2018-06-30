#include "Graphics/MaterialParameter.h"

#include "Graphics/Material.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Texture.h"

#define DEFINE_MATERIAL_PARAM_TYPE(class_name_prefix)\
void class_name_prefix##MaterialParameter::updateUniformValue(Material& owningMaterial)\
{\
   owningMaterial.getShaderProgram().setUniformValue(name, value);\
}\

DEFINE_MATERIAL_PARAM_TYPE(Float)
DEFINE_MATERIAL_PARAM_TYPE(Int)
DEFINE_MATERIAL_PARAM_TYPE(UInt)

DEFINE_MATERIAL_PARAM_TYPE(FVec2)
DEFINE_MATERIAL_PARAM_TYPE(FVec3)
DEFINE_MATERIAL_PARAM_TYPE(FVec4)

DEFINE_MATERIAL_PARAM_TYPE(IVec2)
DEFINE_MATERIAL_PARAM_TYPE(IVec3)
DEFINE_MATERIAL_PARAM_TYPE(IVec4)

DEFINE_MATERIAL_PARAM_TYPE(UVec2)
DEFINE_MATERIAL_PARAM_TYPE(UVec3)
DEFINE_MATERIAL_PARAM_TYPE(UVec4)

DEFINE_MATERIAL_PARAM_TYPE(FMat22)
DEFINE_MATERIAL_PARAM_TYPE(FMat23)
DEFINE_MATERIAL_PARAM_TYPE(FMat24)
DEFINE_MATERIAL_PARAM_TYPE(FMat32)
DEFINE_MATERIAL_PARAM_TYPE(FMat33)
DEFINE_MATERIAL_PARAM_TYPE(FMat34)
DEFINE_MATERIAL_PARAM_TYPE(FMat42)
DEFINE_MATERIAL_PARAM_TYPE(FMat43)
DEFINE_MATERIAL_PARAM_TYPE(FMat44)

void TextureMaterialParameter::updateUniformValue(Material& owningMaterial)
{
   GLint textureUnit = owningMaterial.consumeTextureUnit();
   owningMaterial.getShaderProgram().setUniformValue(name, textureUnit);

   glActiveTexture(GL_TEXTURE0 + textureUnit);
   value->bind();
}

#undef DEFINE_MATERIAL_PARAM_TYPE