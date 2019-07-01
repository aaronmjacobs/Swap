#include "Graphics/UniformBufferObject.h"

#include <utility>

UniformBufferObject::UniformBufferObject(std::string name)
   : blockName(std::move(name))
   , boundIndex(UniformBufferObjectIndex::Invalid)
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
   other.boundIndex = UniformBufferObjectIndex::Invalid;

   BufferObject::move(std::move(other));
}

void UniformBufferObject::bindTo(UniformBufferObjectIndex index)
{
   glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(index), getId());
   boundIndex = index;
}
