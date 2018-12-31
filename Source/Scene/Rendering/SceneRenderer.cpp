#include "Scene/Rendering/SceneRenderer.h"

#include "Core/Assert.h"
#include "Math/MathUtils.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Scene.h"

#include <glad/glad.h>

#include <array>

namespace
{
   void setScreenMeshData(Mesh& screenMesh)
   {
      MeshData screenMeshData;

      std::array<GLuint, 6> indices =
      {
         0, 1, 3,
         1, 2, 3
      };
      screenMeshData.indices = indices;

      std::array<GLfloat, 12> positions =
      {
         -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f, 1.0f, 0.0f,
         -1.0f, 1.0f, 0.0f
      };
      screenMeshData.positions.values = positions;
      screenMeshData.positions.valueSize = 3;

      std::array<GLfloat, 8> texCoords =
      {
         0.0f, 0.0f,
         1.0f, 0.0f,
         1.0f, 1.0f,
         0.0f, 1.0f
      };
      screenMeshData.texCoords.values = texCoords;
      screenMeshData.texCoords.valueSize = 2;

      screenMesh.setData(screenMeshData);
   }
}

SceneRenderer::SceneRenderer(int initialWidth, int initialHeight)
   : width(glm::max(initialWidth, 1))
   , height(glm::max(initialHeight, 1))
   , nearPlaneDistance(0.01f)
   , farPlaneDistance(1000.0f)
{
   ASSERT(initialWidth > 0 && initialHeight > 0, "Invalid framebuffer size");

   glViewport(0, 0, width, height);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   setScreenMeshData(screenMesh);
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

   nearPlaneDistance = glm::clamp(newNearPlaneDistance,
                                  MathUtils::kKindaSmallNumber,
                                  farPlaneDistance - MathUtils::kKindaSmallNumber);
}

void SceneRenderer::setFarPlaneDistance(float newFarPlaneDistance)
{
   ASSERT(newFarPlaneDistance > nearPlaneDistance);

   farPlaneDistance = glm::max(newFarPlaneDistance, nearPlaneDistance + MathUtils::kKindaSmallNumber);
}

bool SceneRenderer::getPerspectiveInfo(const Scene& scene, PerspectiveInfo& perspectiveInfo) const
{
   const CameraComponent* activeCamera = scene.getActiveCameraComponent();
   if (!activeCamera)
   {
      return false;
   }

   ASSERT(getWidth() > 0 && getHeight() > 0, "Invalid framebuffer size");
   float aspectRatio = static_cast<float>(getWidth()) / getHeight();
   perspectiveInfo.projectionMatrix = glm::perspective(glm::radians(activeCamera->getFieldOfView()), aspectRatio,
      getNearPlaneDistance(), getFarPlaneDistance());

   Transform cameraTransform = activeCamera->getAbsoluteTransform();
   perspectiveInfo.viewMatrix = glm::lookAt(cameraTransform.position,
      cameraTransform.position + MathUtils::kForwardVector * cameraTransform.orientation, MathUtils::kUpVector);

   perspectiveInfo.cameraPosition = cameraTransform.position;

   return true;
}
