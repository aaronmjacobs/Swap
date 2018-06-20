#pragma once

#include <glad/glad.h>
#include <gsl/span>

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
   void setData(gsl::span<GLuint> indices, gsl::span<GLfloat> vertices, gsl::span<GLfloat> normals = {}, gsl::span<GLfloat> texCoords = {}, int dimensionality = 3);

   void bind();
   void draw();

private:
   void assertBound() const;

   GLuint vertexArrayObject;
   GLuint elementBufferObject;
   GLuint positionBufferObject;
   GLuint normalBufferObject;
   GLuint texCoordBufferObject;

   GLsizei numIndices;
};
