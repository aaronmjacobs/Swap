#include "Graphics/GraphicsResource.h"

#include "Core/Assert.h"
#include "Graphics/GraphicsDefines.h"

GraphicsResource::GraphicsResource(GraphicsResourceType graphicsResourceType)
   : id(0)
   , resourceType(graphicsResourceType)
{
}

void GraphicsResource::move(GraphicsResource&& other)
{
   ASSERT(id == 0);

   id = other.id;
   other.id = 0;

   label = std::move(other.label);
}

void GraphicsResource::setLabel(std::string newLabel)
{
   label = std::move(newLabel);

#if SWAP_GL_OBJECT_LABEL_SUPPORTED
   glObjectLabel(static_cast<GLenum>(resourceType), id, static_cast<GLsizei>(label.size()), label.data());
#endif // SWAP_GL_OBJECT_LABEL_SUPPORTED
}
