#include "Graphics/UniformBufferObject.h"

#include <utility>

UniformBufferObject::UniformBufferObject(std::string name)
   : blockName(std::move(name))
   , boundIndex(GL_INVALID_INDEX)
{
}

UniformBufferObject::UniformBufferObject(UniformBufferObject&& other)
{
   move(std::move(other));
}

UniformBufferObject& UniformBufferObject::operator=(UniformBufferObject&& other)
{
   release();
   move(std::move(other));
   return *this;
}

void UniformBufferObject::move(UniformBufferObject&& other)
{
   boundIndex = other.boundIndex;
   other.boundIndex = GL_INVALID_INDEX;

   BufferObject::move(std::move(other));
}

void UniformBufferObject::bindTo(GLuint index)
{
   glBindBufferBase(GL_UNIFORM_BUFFER, index, getId());
   boundIndex = index;
}
