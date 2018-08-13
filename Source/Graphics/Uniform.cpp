#include "Graphics/Uniform.h"

#include "Core/Assert.h"

Uniform::Uniform(const std::string& uniformName, const GLint uniformLocation, const GLuint program)
   : name(uniformName)
   , location(uniformLocation)
   , dirty(false)
{
   ASSERT(program != 0);
}

void Uniform::commit()
{
   if (dirty)
   {
      commitData();
      dirty = false;
   }
}

bool Uniform::typeError(const char* dataTypeName)
{
   ASSERT(false, "Trying to set %s uniform \"%s\" with invalid data type (%s, should be %s)", getUniformTypeName(), name.c_str(), dataTypeName, getDataTypeName());
   return false;
}
