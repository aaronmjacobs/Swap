#include "Graphics/Framebuffer.h"

#include "Core/Assert.h"
#include "Graphics/Texture.h"

#include <utility>

Framebuffer::Framebuffer(const Fb::Specification& specification)
   : id(0)
{
   glGenFramebuffers(1, &id);
   updateSpecification(specification);
}

Framebuffer::Framebuffer(Framebuffer&& other)
{
   move(std::move(other));
}

Framebuffer::~Framebuffer()
{
   release();
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other)
{
   release();
   move(std::move(other));
   return *this;
}

void Framebuffer::move(Framebuffer&& other)
{
   ASSERT(id == 0);

   id = other.id;
   other.id = 0;

   depthStencilAttachment = std::move(other.depthStencilAttachment);
   colorAttachments = std::move(other.colorAttachments);
}

void Framebuffer::release()
{
   depthStencilAttachment = nullptr;
   colorAttachments.clear();

   if (id != 0)
   {
      glDeleteFramebuffers(1, &id);
      id = 0;
   }
}

// static
void Framebuffer::bindDefault()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind()
{
   ASSERT(id != 0);

   glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void Framebuffer::updateSpecification(const Fb::Specification& framebufferSpecification)
{
   ASSERT(id != 0);

   bind();

   bool isMultisample = framebufferSpecification.samples > 0;
   GLenum target = isMultisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

   depthStencilAttachment = nullptr;
   if (framebufferSpecification.depthStencilType != Fb::DepthStencilType::None)
   {
      Tex::Specification depthStencilSpecification;

      depthStencilSpecification.width = framebufferSpecification.width;
      depthStencilSpecification.height = framebufferSpecification.height;

      if (isMultisample)
      {
         depthStencilSpecification.target = Tex::Target::Texture2D_Multisample;
         depthStencilSpecification.samples = framebufferSpecification.samples;
      }

      bool highPrecision = framebufferSpecification.depthStencilType == Fb::DepthStencilType::Depth32FStencil8;
      depthStencilSpecification.internalFormat = highPrecision ? Tex::InternalFormat::Depth32FStencil8 : Tex::InternalFormat::Depth24Stencil8;
      depthStencilSpecification.providedDataFormat = Tex::ProvidedDataFormat::DepthStencil;
      depthStencilSpecification.providedDataType = Tex::ProvidedDataType::UnsignedInt_24_8;

      depthStencilAttachment = std::make_shared<Texture>(depthStencilSpecification);

      if (!isMultisample)
      {
         depthStencilAttachment->setParam(Tex::IntParam::TextureMinFilter, GL_NEAREST);
         depthStencilAttachment->setParam(Tex::IntParam::TextureMagFilter, GL_NEAREST);
         depthStencilAttachment->setParam(Tex::IntParam::TextureWrapS, GL_CLAMP_TO_EDGE);
         depthStencilAttachment->setParam(Tex::IntParam::TextureWrapT, GL_CLAMP_TO_EDGE);
      }

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, target, depthStencilAttachment->getId(), 0);
   }

   colorAttachments.clear();
   colorAttachments.resize(framebufferSpecification.colorAttachmentFormats.size());
   std::vector<GLenum> drawBuffers(framebufferSpecification.colorAttachmentFormats.size());

   Tex::Specification colorSpecification;
   colorSpecification.width = framebufferSpecification.width;
   colorSpecification.height = framebufferSpecification.height;

   if (isMultisample)
   {
      colorSpecification.target = Tex::Target::Texture2D_Multisample;
      colorSpecification.samples = framebufferSpecification.samples;
   }

   std::size_t attachmentIndex = 0;
   for (SPtr<Texture>& colorAttachment : colorAttachments)
   {
      colorSpecification.internalFormat = framebufferSpecification.colorAttachmentFormats[attachmentIndex];
      colorAttachment = std::make_shared<Texture>(colorSpecification);

      if (!isMultisample)
      {
         colorAttachment->setParam(Tex::IntParam::TextureMinFilter, GL_LINEAR);
         colorAttachment->setParam(Tex::IntParam::TextureMagFilter, GL_LINEAR);
         colorAttachment->setParam(Tex::IntParam::TextureWrapS, GL_CLAMP_TO_EDGE);
         colorAttachment->setParam(Tex::IntParam::TextureWrapT, GL_CLAMP_TO_EDGE);
      }

      drawBuffers[attachmentIndex] = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + attachmentIndex);
      glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffers[attachmentIndex], target, colorAttachment->getId(), 0);
      ++attachmentIndex;
   }
   glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

   ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
   bindDefault();
}

void Framebuffer::updateResolution(GLsizei width, GLsizei height)
{
   ASSERT(id != 0);

   if (depthStencilAttachment)
   {
      depthStencilAttachment->updateResolution(width, height);
   }

   for (SPtr<Texture>& colorAttachment : colorAttachments)
   {
      colorAttachment->updateResolution(width, height);
   }
}
