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
         Texture = 2,
      };
   }
}

Mesh::Mesh()
   : vao(0)
   , ebo(0)
   , vbo(0)
   , numIndices(0)
{
   glGenVertexArrays(1, &vao);
   glGenBuffers(1, &ebo);
   glGenBuffers(1, &vbo);
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
   vao = other.vao;
   other.vao = 0;

   ebo = other.ebo;
   other.ebo = 0;

   vbo = other.vbo;
   other.vbo = 0;
}

void Mesh::release()
{
   if (vbo != 0)
   {
      glDeleteBuffers(1, &vbo);
      vbo = 0;
   }

   if (ebo != 0)
   {
      glDeleteBuffers(1, &ebo);
      ebo = 0;
   }

   if (vao != 0)
   {
      glDeleteVertexArrays(1, &vao);
      vao = 0;
   }
}

void Mesh::setData(gsl::span<GLuint> indices, gsl::span<GLfloat> vertices, int dimensionality)
{
   ASSERT(dimensionality > 0 && dimensionality < 5);
   assertBound();

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size_bytes(), indices.data(), GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, vertices.size_bytes(), vertices.data(), GL_STATIC_DRAW);
   glEnableVertexAttribArray(Attribute::Vertex);
   glVertexAttribPointer(Attribute::Vertex, dimensionality, GL_FLOAT, GL_FALSE, 0, 0);

   numIndices = static_cast<GLsizei>(indices.size());
}

void Mesh::bind()
{
   glBindVertexArray(vao);
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
   ASSERT(vao != 0);

   GLint vaoBinding = 0;
   glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBinding);

   ASSERT(static_cast<GLuint>(vaoBinding) == vao);
#endif // SWAP_DEBUG
}
