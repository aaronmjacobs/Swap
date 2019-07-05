#include "Graphics/BufferObject.h"

#include "Core/Assert.h"

#include <utility>

BufferObject::BufferObject()
   : GraphicsResource(GraphicsResourceType::Buffer)
{
}

BufferObject::BufferObject(BufferObject&& other)
   : GraphicsResource(GraphicsResourceType::Buffer)
{
   move(std::move(other));
}

BufferObject::~BufferObject()
{
   release();
}

BufferObject& BufferObject::operator=(BufferObject&& other)
{
   release();
   move(std::move(other));
   return *this;
}

void BufferObject::release()
{
   if (id != 0)
   {
      glDeleteBuffers(1, &id);
      id = 0;
   }
}

void BufferObject::setData(BufferBindingTarget target, GLsizeiptr size, const GLvoid* data, BufferUsage usage)
{
   if (size > 0)
   {
      if (id == 0)
      {
         glGenBuffers(1, &id);
      }

      glBindBuffer(static_cast<GLenum>(target), id);
      glBufferData(static_cast<GLenum>(target), size, data, static_cast<GLenum>(usage));
   }
   else
   {
      release();
   }
}

void BufferObject::updateData(BufferBindingTarget target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
   ASSERT(id != 0);
   ASSERT(size > 0);

   glBindBuffer(static_cast<GLenum>(target), id);
   glBufferSubData(static_cast<GLenum>(target), offset, size, data);
}

VertexBufferObject::VertexBufferObject(VertexAttribute vertexAttribute)
   : attribute(vertexAttribute)
{
}

VertexBufferObject::VertexBufferObject(VertexBufferObject&& other)
{
   move(std::move(other));
}

VertexBufferObject& VertexBufferObject::operator=(VertexBufferObject&& other)
{
   release();
   move(std::move(other));
   return *this;
}

void VertexBufferObject::move(VertexBufferObject&& other)
{
   attribute = other.attribute;

   BufferObject::move(std::move(other));
}

void VertexBufferObject::setData(GLsizeiptr size, const GLvoid* data, BufferUsage usage, GLint attributeSize)
{
   BufferObject::setData(BufferBindingTarget::Array, size, data, usage);

   if (size > 0)
   {
      glEnableVertexAttribArray(static_cast<GLuint>(attribute));
      glVertexAttribPointer(static_cast<GLuint>(attribute), attributeSize, GL_FLOAT, GL_FALSE, 0, nullptr);
   }
   else
   {
      glDisableVertexAttribArray(static_cast<GLuint>(attribute));
   }
}
