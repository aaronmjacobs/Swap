#include "Scene/SceneRenderer.h"

#include "Core/Assert.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/ModelComponent.h"
#include "Scene/Scene.h"

#include <algorithm>

namespace
{
   const char* kProjectionMatrix = "uProjectionMatrix";
   const char* kViewMatrix = "uViewMatrix";
   const char* kModelMatrix = "uModelMatrix";
   const char* kNormalMatrix = "uNormalMatrix";

   const char* kCameraPos = "uCameraPos";
}

SceneRenderer::SceneRenderer(int initialWidth, int initialHeight)
   : width(0), height(0), nearPlaneDistance(0.01f), farPlaneDistance(1000.0f)
{
   onFramebufferSizeChanged(initialWidth, initialHeight);
}

void SceneRenderer::renderScene(const Scene& scene)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   const CameraComponent* activeCamera = scene.getActiveCameraComponent();
   if (!activeCamera)
   {
      return;
   }

   ASSERT(width > 0 && height > 0, "Invalid framebuffer size");
   float aspectRatio = static_cast<float>(width) / height;
   glm::mat4 projectionMatrix = glm::perspective(glm::radians(activeCamera->getFieldOfView()), aspectRatio, nearPlaneDistance, farPlaneDistance);

   Transform cameraTransform = activeCamera->getAbsoluteTransform();
   glm::mat4 viewMatrix = glm::lookAt(cameraTransform.position, cameraTransform.position + MathUtils::kForwardVector * cameraTransform.orientation, MathUtils::kUpVector);

   const std::vector<ModelComponent*>& modelComponents = scene.getModelComponents();
   for (const ModelComponent* modelComponent : modelComponents)
   {
      ASSERT(modelComponent);

      if (const SPtr<Model>& model = modelComponent->getModel())
      {
         Transform transform = modelComponent->getAbsoluteTransform();
         glm::mat4 modelMatrix = transform.toMatrix();
         glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));

         model->setMaterialParameter(kProjectionMatrix, projectionMatrix, false);
         model->setMaterialParameter(kViewMatrix, viewMatrix, false);
         model->setMaterialParameter(kModelMatrix, modelMatrix, false);
         model->setMaterialParameter(kNormalMatrix, normalMatrix, false);

         model->setMaterialParameter(kCameraPos, cameraTransform.position);

         model->draw();
      }
   }
}

void SceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   ASSERT(newWidth > 0 && newHeight > 0, "Invalid framebuffer size");

   width = std::max(newWidth, 1);
   height = std::max(newHeight, 1);

   glViewport(0, 0, width, height);
}

void SceneRenderer::setNearPlaneDistance(float newNearPlaneDistance)
{
   ASSERT(newNearPlaneDistance >= MathUtils::kKindaSmallNumber);
   ASSERT(newNearPlaneDistance < farPlaneDistance);

   nearPlaneDistance = std::max(std::min(newNearPlaneDistance, farPlaneDistance - MathUtils::kKindaSmallNumber), MathUtils::kKindaSmallNumber);
}

void SceneRenderer::setFarPlaneDistance(float newFarPlaneDistance)
{
   ASSERT(newFarPlaneDistance > nearPlaneDistance);

   farPlaneDistance = std::max(newFarPlaneDistance, nearPlaneDistance + MathUtils::kKindaSmallNumber);
}
