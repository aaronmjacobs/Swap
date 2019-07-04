#pragma once

#include "Graphics/BufferObject.h"
#include "Math/Bounds.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <gsl/span>

#include <vector>

struct DrawingContext;

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

class MeshSection
{
public:
   MeshSection();
   MeshSection(const MeshSection& other) = delete;
   MeshSection(MeshSection&& other);
   ~MeshSection();
   MeshSection& operator=(const MeshSection& other) = delete;
   MeshSection& operator=(MeshSection&& other);

private:
   void move(MeshSection&& other);
   void release();

public:
   void setData(const MeshData& data);
   void draw(const DrawingContext& context) const;

   const Bounds& getBounds() const
   {
      return bounds;
   }

private:
   void bind() const;

   GLuint vertexArrayObject;

   BufferObject elementBufferObject;
   VertexBufferObject positionBufferObject;
   VertexBufferObject normalBufferObject;
   VertexBufferObject texCoordBufferObject;
   VertexBufferObject tangentBufferObject;
   VertexBufferObject bitangentBufferObject;
   VertexBufferObject colorBufferObject;

   GLsizei numIndices;

   Bounds bounds;
};

class Mesh
{
public:
   Mesh(std::vector<MeshSection>&& meshSections);

   void draw(const DrawingContext& context) const;

   const std::vector<MeshSection>& getSections() const
   {
      return sections;
   }

private:
   std::vector<MeshSection> sections;
};
