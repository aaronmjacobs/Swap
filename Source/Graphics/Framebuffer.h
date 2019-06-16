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

   enum class Target
   {
      Framebuffer = GL_FRAMEBUFFER,
      ReadFramebuffer = GL_READ_FRAMEBUFFER,
      DrawFramebuffer = GL_DRAW_FRAMEBUFFER
   };

   struct Specification
   {
      GLsizei width = 0;
      GLsizei height = 0;

      GLsizei samples = 0;

      DepthStencilType depthStencilType = DepthStencilType::Depth24Stencil8;

      gsl::span<const Tex::InternalFormat> colorAttachmentFormats;
   };

   struct Attachments
   {
      SPtr<Texture> depthStencilAttachment;
      std::vector<SPtr<Texture>> colorAttachments;
   };

   Attachments generateAttachments(const Specification& specification);
}

class Framebuffer
{
public:
   Framebuffer();
   Framebuffer(const Framebuffer& other) = delete;
   Framebuffer(Framebuffer&& other);
   ~Framebuffer();
   Framebuffer& operator=(const Framebuffer& other) = delete;
   Framebuffer& operator=(Framebuffer&& other);

   static void bindDefault(Fb::Target target = Fb::Target::Framebuffer);
   void bind(Fb::Target target = Fb::Target::Framebuffer);

   GLuint getId() const
   {
      return id;
   }

   const Fb::Attachments& getAttachments() const
   {
      return attachments;
   }

   void setAttachments(Fb::Attachments newAttachments);

private:
   void move(Framebuffer&& other);
   void release();

   GLuint id;
   Fb::Attachments attachments;
};
