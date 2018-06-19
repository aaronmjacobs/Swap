#include "Graphics/Uniform.h"

#include "Core/Assert.h"

Uniform::Uniform(const std::string& uniformName, const GLint uniformLocation, const GLenum uniformType, const GLuint program)
   : name(uniformName)
   , location(uniformLocation)
   , type(uniformType)
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

bool Uniform::typeError(const char* typeName)
{
   ASSERT(false, "Trying to set uniform \"%s\" with invalid type (%s, should be %s)", name.c_str(), typeName, getTypeName());
   return false;
}
