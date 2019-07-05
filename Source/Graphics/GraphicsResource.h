#pragma once

#include <glad/gl.h>

#include <string>

enum class GraphicsResourceType : GLenum
{
   Buffer = GL_BUFFER,
   Shader = GL_SHADER,
   Program = GL_PROGRAM,
   VertexArray = GL_VERTEX_ARRAY,
   Query = GL_QUERY,
   ProgramPipeline = GL_PROGRAM_PIPELINE,
   TransformFeedback = GL_TRANSFORM_FEEDBACK,
   Sampler = GL_SAMPLER,
   Texture = GL_TEXTURE,
   Renderbuffer = GL_RENDERBUFFER,
   Framebuffer = GL_FRAMEBUFFER
};

class GraphicsResource
{
public:
   GraphicsResource(GraphicsResourceType graphicsResourceType);

protected:
   void move(GraphicsResource&& other);

public:
   GLuint getId() const
   {
      return id;
   }

   const std::string& getLabel() const
   {
      return label;
   }

   void setLabel(std::string newLabel);

protected:
   GLuint id;

private:
   const GraphicsResourceType resourceType;
   std::string label;
};
