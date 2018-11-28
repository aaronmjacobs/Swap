#include "Scene/SceneRenderer.h"

#include "Scene/CameraComponent.h"
#include "Scene/ModelComponent.h"
#include "Scene/Scene.h"

namespace
{
   const char* kProjectionMatrix = "uProjectionMatrix";
   const char* kViewMatrix = "uViewMatrix";
   const char* kModelMatrix = "uModelMatrix";
   const char* kNormalMatrix = "uNormalMatrix";

   const char* kCameraPos = "uCameraPos";
}

void SceneRenderer::renderScene(const Scene& scene)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   const CameraComponent* activeCamera = scene.getActiveCameraComponent();
   if (!activeCamera)
   {
      return;
   }

   Transform cameraTransform = activeCamera->getAbsoluteTransform();
   glm::mat4 projectionMatrix = glm::perspective(activeCamera->getFieldOfView(), 1280.0f / 720.0f, 0.1f, 1000.0f); // TODO
   glm::mat4 viewMatrix = glm::lookAt(cameraTransform.position, cameraTransform.position + MathUtils::kForwardVector * cameraTransform.orientation, MathUtils::kUpVector);

   const std::vector<ModelComponent*> modelComponents = scene.getModelComponents();
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
