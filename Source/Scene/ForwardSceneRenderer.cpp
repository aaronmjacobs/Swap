#include "Scene/ForwardSceneRenderer.h"

#include "Core/Assert.h"
#include "Math/MathUtils.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/Lights/DirectionalLightComponent.h"
#include "Scene/Components/Lights/PointLightComponent.h"
#include "Scene/Components/Lights/SpotLightComponent.h"
#include "Scene/Components/ModelComponent.h"
#include "Scene/Scene.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

namespace
{
   const char* kProjectionMatrix = "uProjectionMatrix";
   const char* kViewMatrix = "uViewMatrix";
   const char* kModelMatrix = "uModelMatrix";
   const char* kNormalMatrix = "uNormalMatrix";

   const char* kCameraPos = "uCameraPos";

   void populateDirectionalLightUniforms(const Scene& scene, Model& model)
   {
      int directionalLightIndex = 0;
      for (const DirectionalLightComponent* directionalLightComponent : scene.getDirectionalLightComponents())
      {
         std::string directionalLightStr = "uDirectionalLights[" + std::to_string(directionalLightIndex) + "]";

         model.setMaterialParameter(directionalLightStr + ".color", directionalLightComponent->getColor());
         model.setMaterialParameter(directionalLightStr + ".direction", directionalLightComponent->getAbsoluteTransform().orientation * MathUtils::kForwardVector);

         ++directionalLightIndex;
      }
      model.setMaterialParameter("uNumDirectionalLights", static_cast<int>(scene.getDirectionalLightComponents().size()));
   }

   void populatePointLightUniforms(const Scene& scene, Model& model)
   {
      int pointLightIndex = 0;
      for (const PointLightComponent* pointLightComponent : scene.getPointLightComponents())
      {
         std::string pointLightStr = "uPointLights[" + std::to_string(pointLightIndex) + "]";

         Transform transform = pointLightComponent->getAbsoluteTransform();
         float radiusScale = glm::max(transform.scale.x, glm::max(transform.scale.y, transform.scale.z));

         model.setMaterialParameter(pointLightStr + ".color", pointLightComponent->getColor());
         model.setMaterialParameter(pointLightStr + ".position", transform.position);
         model.setMaterialParameter(pointLightStr + ".radius", pointLightComponent->getRadius() * radiusScale);

         ++pointLightIndex;
      }
      model.setMaterialParameter("uNumPointLights", static_cast<int>(scene.getPointLightComponents().size()));
   }

   void populateSpotLightUniforms(const Scene& scene, Model& model)
   {
      int spotLightIndex = 0;
      for (const SpotLightComponent* spotLightComponent : scene.getSpotLightComponents())
      {
         std::string spotLightStr = "uSpotLights[" + std::to_string(spotLightIndex) + "]";

         Transform transform = spotLightComponent->getAbsoluteTransform();
         float radiusScale = glm::max(transform.scale.x, glm::max(transform.scale.y, transform.scale.z));

         model.setMaterialParameter(spotLightStr + ".color", spotLightComponent->getColor());
         model.setMaterialParameter(spotLightStr + ".direction", transform.orientation * MathUtils::kForwardVector);
         model.setMaterialParameter(spotLightStr + ".position", transform.position);
         model.setMaterialParameter(spotLightStr + ".radius", spotLightComponent->getRadius() * radiusScale);
         model.setMaterialParameter(spotLightStr + ".beamAngle", glm::radians(spotLightComponent->getBeamAngle()));
         model.setMaterialParameter(spotLightStr + ".cutoffAngle", glm::radians(spotLightComponent->getCutoffAngle()));

         ++spotLightIndex;
      }
      model.setMaterialParameter("uNumSpotLights", static_cast<int>(scene.getSpotLightComponents().size()));
   }

   void populateLightUniforms(const Scene& scene, Model& model)
   {
      populateDirectionalLightUniforms(scene, model);
      populatePointLightUniforms(scene, model);
      populateSpotLightUniforms(scene, model);
   }
}

ForwardSceneRenderer::ForwardSceneRenderer(int initialWidth, int initialHeight)
   : SceneRenderer(initialWidth, initialHeight)
{
}

void ForwardSceneRenderer::renderScene(const Scene& scene)
{
   glEnable(GL_DEPTH_TEST);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   const CameraComponent* activeCamera = scene.getActiveCameraComponent();
   if (!activeCamera)
   {
      return;
   }

   ASSERT(getWidth() > 0 && getHeight() > 0, "Invalid framebuffer size");
   float aspectRatio = static_cast<float>(getWidth()) / getHeight();
   glm::mat4 projectionMatrix = glm::perspective(glm::radians(activeCamera->getFieldOfView()), aspectRatio, getNearPlaneDistance(), getFarPlaneDistance());

   Transform cameraTransform = activeCamera->getAbsoluteTransform();
   glm::mat4 viewMatrix = glm::lookAt(cameraTransform.position, cameraTransform.position + MathUtils::kForwardVector * cameraTransform.orientation, MathUtils::kUpVector);

   // TODO Use uniform buffer objects

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

         populateLightUniforms(scene, *model);

         model->draw();
      }
   }
}
