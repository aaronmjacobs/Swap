#include "Graphics/Mesh.h"

#include "Core/Assert.h"

#include <utility>

Mesh::Mesh()
   : vertexArrayObject(0)
   , positionBufferObject(VertexAttribute::Position)
   , normalBufferObject(VertexAttribute::Normal)
   , texCoordBufferObject(VertexAttribute::TexCoord)
   , tangentBufferObject(VertexAttribute::Tangent)
   , bitangentBufferObject(VertexAttribute::Bitangent)
   , colorBufferObject(VertexAttribute::Color)
   , numIndices(0)
{
   glGenVertexArrays(1, &vertexArrayObject);
}

Mesh::Mesh(Mesh&& other)
   : vertexArrayObject(0)
   , positionBufferObject(VertexAttribute::Position)
   , normalBufferObject(VertexAttribute::Normal)
   , texCoordBufferObject(VertexAttribute::TexCoord)
   , tangentBufferObject(VertexAttribute::Tangent)
   , bitangentBufferObject(VertexAttribute::Bitangent)
   , colorBufferObject(VertexAttribute::Color)
   , numIndices(0)
{
   move(std::move(other));
}

Mesh::~Mesh()
{
   release();
}

Mesh& Mesh::operator=(Mesh&& other)
{
   release();
   move(std::move(other));
   return *this;
}

void Mesh::move(Mesh&& other)
{
   vertexArrayObject = other.vertexArrayObject;
   other.vertexArrayObject = 0;

   elementBufferObject = std::move(other.elementBufferObject);
   positionBufferObject = std::move(other.positionBufferObject);
   normalBufferObject = std::move(other.normalBufferObject);
   texCoordBufferObject = std::move(other.texCoordBufferObject);
   tangentBufferObject = std::move(other.tangentBufferObject);
   bitangentBufferObject = std::move(other.bitangentBufferObject);
   colorBufferObject = std::move(other.colorBufferObject);

   numIndices = other.numIndices;
   other.numIndices = 0;
}

void Mesh::release()
{
   elementBufferObject.release();
   positionBufferObject.release();
   normalBufferObject.release();
   texCoordBufferObject.release();
   tangentBufferObject.release();
   bitangentBufferObject.release();
   colorBufferObject.release();

   if (vertexArrayObject != 0)
   {
      glDeleteVertexArrays(1, &vertexArrayObject);
      vertexArrayObject = 0;
   }

   numIndices = 0;
}

void Mesh::setData(const MeshData& data)
{
   ASSERT(data.indices.size() % 3 == 0);
   ASSERT(data.positions.valueSize >= 0 && data.positions.valueSize < 5
      && data.normals.valueSize >= 0 && data.normals.valueSize < 5
      && data.texCoords.valueSize >= 0 && data.texCoords.valueSize < 5
      && data.tangents.valueSize >= 0 && data.tangents.valueSize < 5
      && data.bitangents.valueSize >= 0 && data.bitangents.valueSize < 5
      && data.colors.valueSize >= 0 && data.colors.valueSize < 5);
   ASSERT(data.positions.values.size() == 0 || (data.positions.values.size() % data.positions.valueSize == 0)
      && data.normals.values.size() == 0 || (data.normals.values.size() % data.normals.valueSize == 0)
      && data.texCoords.values.size() == 0 || (data.texCoords.values.size() % data.texCoords.valueSize == 0)
      && data.tangents.values.size() == 0 || (data.tangents.values.size() % data.tangents.valueSize == 0)
      && data.bitangents.values.size() == 0 || (data.bitangents.values.size() % data.bitangents.valueSize == 0)
      && data.colors.values.size() == 0 || (data.colors.values.size() % data.colors.valueSize == 0));

   bind();

   elementBufferObject.setData(BufferBindingTarget::ElementArray, data.indices.size_bytes(), data.indices.data(),
      BufferUsage::StaticDraw);

   positionBufferObject.setData(data.positions.values.size_bytes(), data.positions.values.data(),
      BufferUsage::StaticDraw, data.positions.valueSize);

   normalBufferObject.setData(data.normals.values.size_bytes(), data.normals.values.data(), BufferUsage::StaticDraw,
      data.normals.valueSize);

   texCoordBufferObject.setData(data.texCoords.values.size_bytes(), data.texCoords.values.data(),
      BufferUsage::StaticDraw, data.texCoords.valueSize);

   tangentBufferObject.setData(data.tangents.values.size_bytes(), data.tangents.values.data(), BufferUsage::StaticDraw,
      data.tangents.valueSize);

   bitangentBufferObject.setData(data.bitangents.values.size_bytes(), data.bitangents.values.data(),
      BufferUsage::StaticDraw, data.bitangents.valueSize);

   colorBufferObject.setData(data.colors.values.size_bytes(), data.colors.values.data(), BufferUsage::StaticDraw,
      data.colors.valueSize);

   unbind();

   numIndices = static_cast<GLsizei>(data.indices.size());
}

void Mesh::draw() const
{
   ASSERT(numIndices > 0);

   bind();
   glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
   unbind();
}

void Mesh::bind() const
{
   ASSERT(vertexArrayObject != 0);

   glBindVertexArray(vertexArrayObject);
}

void Mesh::unbind() const
{
   glBindVertexArray(0);
}
