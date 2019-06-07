#include "Graphics/MaterialParameter.h"

#include "Graphics/DrawingContext.h"
#include "Graphics/Material.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Texture.h"

bool MaterialParameterBase::typeError(const char* dataTypeName)
{
   ASSERT(false, "Trying to set %s material parameter \"%s\" with invalid data type (%s, should be %s)", getUniformTypeName(), name.c_str(), dataTypeName, getDataTypeName());
   return false;
}

#define DEFINE_MATERIAL_PARAM_TYPE(uniform_type, data_type, param_type)\
void uniform_type##MaterialParameter::apply(DrawingContext& context)\
{\
   ASSERT(context.program);\
\
   if (enabled)\
   {\
      context.program->setUniformValue(name, value, false);\
   }\
}\

#define FOR_EACH_UNIFORM_TYPE DEFINE_MATERIAL_PARAM_TYPE
#define FOR_EACH_UNIFORM_TYPE_NO_COMPLEX
#include "ForEachUniformType.inl"
#undef FOR_EACH_UNIFORM_TYPE_NO_COMPLEX
#undef FOR_EACH_UNIFORM_TYPE

#undef DEFINE_MATERIAL_PARAM_TYPE

void TextureMaterialParameter::apply(DrawingContext& context)
{
   ASSERT(context.program);

   if (enabled && value)
   {
      GLint textureUnit = value->activateAndBind(context);
      context.program->setUniformValue(name, textureUnit, false);
   }
}
