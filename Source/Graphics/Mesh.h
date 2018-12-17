#pragma once

#include "Graphics/BufferObject.h"

#include <glad/glad.h>
#include <gsl/span>

template<typename T>
struct MeshAttributeData
{
   gsl::span<T> values;
   GLint valueSize = 0;
};

struct MeshData
{
   gsl::span<GLuint> indices;
   MeshAttributeData<GLfloat> positions;
   MeshAttributeData<GLfloat> normals;
   MeshAttributeData<GLfloat> texCoords;
   MeshAttributeData<GLfloat> tangents;
   MeshAttributeData<GLfloat> bitangents;
   MeshAttributeData<GLfloat> colors;
};

class Mesh
{
public:
   Mesh();
   Mesh(const Mesh& other) = delete;
   Mesh(Mesh&& other);
   ~Mesh();
   Mesh& operator=(const Mesh& other) = delete;
   Mesh& operator=(Mesh&& other);

private:
   void move(Mesh&& other);
   void release();

public:
   void setData(const MeshData& data);
   void draw() const;

private:
   void bind() const;
   void unbind() const;

   GLuint vertexArrayObject;

   BufferObject elementBufferObject;
   VertexBufferObject positionBufferObject;
   VertexBufferObject normalBufferObject;
   VertexBufferObject texCoordBufferObject;
   VertexBufferObject tangentBufferObject;
   VertexBufferObject bitangentBufferObject;
   VertexBufferObject colorBufferObject;

   GLsizei numIndices;
};
