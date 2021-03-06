#pragma once

#include "Graphics/GraphicsResource.h"
#include "Graphics/TextureInfo.h"

#include <glad/gl.h>
#include <glm/glm.hpp>

struct DrawingContext;

class Texture : public GraphicsResource
{
public:
   Texture(const Tex::Specification& textureSpecification);
   Texture(const Texture& other) = delete;
   Texture(Texture&& other);
   ~Texture();
   Texture& operator=(const Texture& other) = delete;
   Texture& operator=(Texture&& other);

private:
   void move(Texture&& other);
   void release();

public:
   int activateAndBind(DrawingContext& context) const;
   void bind() const;

   void updateSpecification(const Tex::Specification& textureSpecification);
   void updateResolution(GLsizei width = -1, GLsizei height = -1, GLsizei depth = -1);

   void setParam(Tex::FloatParam param, GLfloat value);
   void setParam(Tex::IntParam param, GLint value);
   void setParam(Tex::FloatArrayParam param, const glm::vec4& value);
   void setParam(Tex::IntArrayParam param, const glm::ivec4& value);
   void setParam(Tex::InternalIntArrayParam param, const glm::ivec4& value);
   void setParam(Tex::InternalUintArrayParam param, const glm::uvec4& value);

   void generateMipMaps();

   const Tex::Specification& getSpecification() const
   {
      return specification;
   }

   bool hasAlpha() const;
   bool isMultisample() const;
   bool isCubemap() const;

private:
   void assertBound() const;

   Tex::Specification specification;
};
