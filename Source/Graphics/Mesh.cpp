#include "Graphics/Mesh.h"

#include "Core/Assert.h"

#include <utility>

namespace
{
   namespace Attribute
   {
      enum Enum
      {
         Vertex = 0,
         Normal = 1,
         TexCoord = 2,
      };
   }
}

Mesh::Mesh()
   : vertexArrayObject(0)
   , elementBufferObject(0)
   , positionBufferObject(0)
   , normalBufferObject(0)
   , texCoordBufferObject(0)
   , numIndices(0)
{
   glGenVertexArrays(1, &vertexArrayObject);
   glGenBuffers(1, &elementBufferObject);
   glGenBuffers(1, &positionBufferObject);
   glGenBuffers(1, &normalBufferObject);
   glGenBuffers(1, &texCoordBufferObject);
}

Mesh::Mesh(Mesh&& other)
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

   elementBufferObject = other.elementBufferObject;
   other.elementBufferObject = 0;

   positionBufferObject = other.positionBufferObject;
   other.positionBufferObject = 0;

   normalBufferObject = other.normalBufferObject;
   other.normalBufferObject = 0;

   texCoordBufferObject = other.texCoordBufferObject;
   other.texCoordBufferObject = 0;

   numIndices = other.numIndices;
   other.numIndices = 0;
}

void Mesh::release()
{
   static const auto releaseBuffer = [](GLuint& buffer)
   {
      if (buffer != 0)
      {
         glDeleteBuffers(1, &buffer);
         buffer = 0;
      }
   };

   releaseBuffer(elementBufferObject);
   releaseBuffer(positionBufferObject);
   releaseBuffer(normalBufferObject);
   releaseBuffer(texCoordBufferObject);

   if (vertexArrayObject != 0)
   {
      glDeleteVertexArrays(1, &vertexArrayObject);
      vertexArrayObject = 0;
   }

   numIndices = 0;
}

void Mesh::setData(gsl::span<GLuint> indices, gsl::span<GLfloat> vertices, gsl::span<GLfloat> normals/* = {}*/, gsl::span<GLfloat> texCoords/* = {}*/, int dimensionality/* = 3*/)
{
   ASSERT(indices.size() % 3 == 0);
   ASSERT(dimensionality > 0 && dimensionality < 5);
   ASSERT((vertices.size() % dimensionality == 0) && (normals.size() % dimensionality == 0) && (texCoords.size() % 2 == 0));
   assertBound();

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size_bytes(), indices.data(), GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
   glBufferData(GL_ARRAY_BUFFER, vertices.size_bytes(), vertices.data(), GL_STATIC_DRAW);
   glVertexAttribPointer(Attribute::Vertex, dimensionality, GL_FLOAT, GL_FALSE, 0, 0);
   if (vertices.empty())
   {
      glDisableVertexAttribArray(Attribute::Vertex);
   }
   else
   {
      glEnableVertexAttribArray(Attribute::Vertex);
   }

   glBindBuffer(GL_ARRAY_BUFFER, normalBufferObject);
   glBufferData(GL_ARRAY_BUFFER, normals.size_bytes(), normals.data(), GL_STATIC_DRAW);
   glVertexAttribPointer(Attribute::Normal, dimensionality, GL_FLOAT, GL_FALSE, 0, 0);
   if (normals.empty())
   {
      glDisableVertexAttribArray(Attribute::Normal);
   }
   else
   {
      glEnableVertexAttribArray(Attribute::Normal);
   }

   glBindBuffer(GL_ARRAY_BUFFER, texCoordBufferObject);
   glBufferData(GL_ARRAY_BUFFER, texCoords.size_bytes(), texCoords.data(), GL_STATIC_DRAW);
   glVertexAttribPointer(Attribute::TexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
   if (texCoords.empty())
   {
      glDisableVertexAttribArray(Attribute::TexCoord);
   }
   else
   {
      glEnableVertexAttribArray(Attribute::TexCoord);
   }

   numIndices = static_cast<GLsizei>(indices.size());
}

void Mesh::bind()
{
   glBindVertexArray(vertexArrayObject);
}

void Mesh::draw()
{
   ASSERT(numIndices > 0);
   assertBound();

   glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
}

void Mesh::assertBound() const
{
#if SWAP_DEBUG
   ASSERT(vertexArrayObject != 0);

   GLint vertexArrayBinding = 0;
   glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArrayBinding);

   ASSERT(static_cast<GLuint>(vertexArrayBinding) == vertexArrayObject);
#endif // SWAP_DEBUG
}
