#include "Scene/SceneRenderer.h"

#include "Core/Assert.h"
#include "Math/MathUtils.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

SceneRenderer::SceneRenderer(int initialWidth, int initialHeight)
   : width(glm::max(initialWidth, 1)), height(glm::max(initialHeight, 1)), nearPlaneDistance(0.01f), farPlaneDistance(1000.0f)
{
   ASSERT(initialWidth > 0 && initialHeight > 0, "Invalid framebuffer size");

   glViewport(0, 0, width, height);
}

void SceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   ASSERT(newWidth > 0 && newHeight > 0, "Invalid framebuffer size");

   width = glm::max(newWidth, 1);
   height = glm::max(newHeight, 1);

   glViewport(0, 0, width, height);
}

void SceneRenderer::setNearPlaneDistance(float newNearPlaneDistance)
{
   ASSERT(newNearPlaneDistance >= MathUtils::kKindaSmallNumber);
   ASSERT(newNearPlaneDistance < farPlaneDistance);

   nearPlaneDistance = glm::clamp(newNearPlaneDistance, MathUtils::kKindaSmallNumber, farPlaneDistance - MathUtils::kKindaSmallNumber);
}

void SceneRenderer::setFarPlaneDistance(float newFarPlaneDistance)
{
   ASSERT(newFarPlaneDistance > nearPlaneDistance);

   farPlaneDistance = glm::max(newFarPlaneDistance, nearPlaneDistance + MathUtils::kKindaSmallNumber);
}
