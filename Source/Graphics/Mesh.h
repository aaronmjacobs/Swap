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
   void setData(gsl::span<GLuint> indices, gsl::span<GLfloat> vertices, int dimensionality = 3);

   void bind();
   void draw();

private:
   void assertBound() const;

   GLuint vao;
   GLuint ebo;
   GLuint vbo;

   GLsizei numIndices;
};
