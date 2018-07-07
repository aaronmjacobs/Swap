#pragma once

#include <glad/glad.h>

enum class BufferBindingTarget : GLenum
{
   Array = GL_ARRAY_BUFFER,
   CopyRead = GL_COPY_READ_BUFFER,
   CopyWrite = GL_COPY_WRITE_BUFFER,
   DrawIndirect = GL_DRAW_INDIRECT_BUFFER,
   ElementArray = GL_ELEMENT_ARRAY_BUFFER,
   PixelPack = GL_PIXEL_PACK_BUFFER,
   PixelUnpack = GL_PIXEL_UNPACK_BUFFER,
   Texture = GL_TEXTURE_BUFFER,
   TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
   Uniform = GL_UNIFORM_BUFFER
};

enum class BufferUsage : GLenum
{
   StreamDraw = GL_STREAM_DRAW,
   StreamRead = GL_STREAM_READ,
   StreamCopy = GL_STREAM_COPY,
   StaticDraw = GL_STATIC_DRAW,
   StaticRead = GL_STATIC_READ,
   StaticCopy = GL_STATIC_COPY,
   DynamicDraw = GL_DYNAMIC_DRAW,
   DynamicRead = GL_DYNAMIC_READ,
   DynamicCopy = GL_DYNAMIC_COPY
};

class BufferObject
{
public:
   BufferObject();
   BufferObject(const BufferObject& other) = delete;
   BufferObject(BufferObject&& other);
   ~BufferObject();
   BufferObject& operator=(const BufferObject& other) = delete;
   BufferObject& operator=(BufferObject&& other);

protected:
   void move(BufferObject&& other);

public:
   void release();

   void setData(BufferBindingTarget target, GLsizeiptr size, const GLvoid* data, BufferUsage usage);

private:
   GLuint id;
};

enum class VertexAttribute : GLuint
{
   Position = 0,
   Normal = 1,
   TexCoord = 2,
   Tangent = 3,
   Bitangent = 4,
   Color = 5
};

class VertexBufferObject : public BufferObject
{
public:
   VertexBufferObject(VertexAttribute vertexAttribute);
   VertexBufferObject(const VertexBufferObject& other) = delete;
   VertexBufferObject(VertexBufferObject&& other);
   ~VertexBufferObject() = default;
   VertexBufferObject& operator=(const VertexBufferObject& other) = delete;
   VertexBufferObject& operator=(VertexBufferObject&& other);

protected:
   void move(VertexBufferObject&& other);

public:
   void setData(GLsizeiptr size, const GLvoid* data, BufferUsage usage, GLint attributeSize);

private:
   VertexAttribute attribute;
};
