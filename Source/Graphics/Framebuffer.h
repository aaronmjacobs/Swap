#pragma once

#include "Core/Pointers.h"
#include "Graphics/TextureInfo.h"

#include <glad/glad.h>
#include <gsl/span>

#include <vector>

class Texture;

namespace Fb
{
   enum class DepthStencilType
   {
      None,
      Depth24Stencil8,
      Depth32FStencil8
   };

   struct Specification
   {
      GLsizei width = 0;
      GLsizei height = 0;

      GLsizei samples = 0;

      DepthStencilType depthStencilType = DepthStencilType::Depth24Stencil8;

      gsl::span<const Tex::InternalFormat> colorAttachmentFormats;
   };
}

class Framebuffer
{
public:
   Framebuffer(const Fb::Specification& specification);
   Framebuffer(const Framebuffer& other) = delete;
   Framebuffer(Framebuffer&& other);
   ~Framebuffer();
   Framebuffer& operator=(const Framebuffer& other) = delete;
   Framebuffer& operator=(Framebuffer&& other);

   static void bindDefault();
   void bind();

   void updateSpecification(const Fb::Specification& framebufferSpecification);
   void updateResolution(GLsizei width, GLsizei height);

   const SPtr<Texture>& getDepthStencilAttachment() const
   {
      return depthStencilAttachment;
   }

   const std::vector<SPtr<Texture>>& getColorAttachments() const
   {
      return colorAttachments;
   }

private:
   void move(Framebuffer&& other);
   void release();

   GLuint id;

   SPtr<Texture> depthStencilAttachment;
   std::vector<SPtr<Texture>> colorAttachments;
};
