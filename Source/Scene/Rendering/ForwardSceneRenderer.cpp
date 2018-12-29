#include "Scene/Rendering/ForwardSceneRenderer.h"

#include "Core/Assert.h"
#include "Graphics/Texture.h"
#include "Math/MathUtils.h"
#include "Resources/ResourceManager.h"
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

   Fb::Specification getMainPassFramebufferSpecification(int width, int height, int numSamples)
   {
      static const std::array<Tex::InternalFormat, 1> kColorAttachmentFormats =
      {
         Tex::InternalFormat::RGBA8
      };

      Fb::Specification specification;

      specification.width = width;
      specification.height = height;
      specification.samples = numSamples;
      specification.depthStencilType = Fb::DepthStencilType::Depth24Stencil8;
      specification.colorAttachmentFormats = kColorAttachmentFormats;

      return specification;
   }

   void populateDirectionalLightUniforms(const Scene& scene, Model& model)
   {
      int directionalLightIndex = 0;
      for (const DirectionalLightComponent* directionalLightComponent : scene.getDirectionalLightComponents())
      {
         std::string directionalLightStr = "uDirectionalLights[" + std::to_string(directionalLightIndex) + "]";

         model.setMaterialParameter(directionalLightStr + ".color", directionalLightComponent->getColor());
         model.setMaterialParameter(directionalLightStr + ".direction",
            directionalLightComponent->getAbsoluteTransform().orientation * MathUtils::kForwardVector);

         ++directionalLightIndex;
      }
      model.setMaterialParameter("uNumDirectionalLights",
         static_cast<int>(scene.getDirectionalLightComponents().size()));
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

#include "Graphics/ShaderProgram.h"
#include "Platform/IOUtils.h"

ForwardSceneRenderer::ForwardSceneRenderer(int initialWidth, int initialHeight, int numSamples,
   const SPtr<ResourceManager>& inResourceManager)
   : SceneRenderer(initialWidth, initialHeight)
   , mainPassFramebuffer(getMainPassFramebufferSpecification(getWidth(), getHeight(), numSamples))
   , resourceManager(inResourceManager)
{
   ASSERT(resourceManager);

   std::vector<ShaderSpecification> shaderSpecifications;
   shaderSpecifications.resize(2);
   shaderSpecifications[0].type = ShaderType::Vertex;
   shaderSpecifications[1].type = ShaderType::Fragment;
   IOUtils::getAbsoluteResourcePath("DepthOnly.vert", shaderSpecifications[0].path);
   IOUtils::getAbsoluteResourcePath("DepthOnly.frag", shaderSpecifications[1].path);

   depthOnlyProgram = resourceManager->loadShaderProgram(shaderSpecifications);
}

void ForwardSceneRenderer::renderScene(const Scene& scene)
{
   // TODO Use uniform buffer objects

   PerspectiveInfo perspectiveInfo;
   if (!getPerspectiveInfo(scene, perspectiveInfo))
   {
      return;
   }

   renderPrePass(scene, perspectiveInfo);
   renderMainPass(scene, perspectiveInfo);
   renderPostProcessPasses(scene, perspectiveInfo);
}

void ForwardSceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   SceneRenderer::onFramebufferSizeChanged(newWidth, newHeight);

   mainPassFramebuffer.updateResolution(getWidth(), getHeight());
}

bool ForwardSceneRenderer::getPerspectiveInfo(const Scene& scene, PerspectiveInfo& perspectiveInfo) const
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

void ForwardSceneRenderer::renderPrePass(const Scene& scene, const PerspectiveInfo& perspectiveInfo)
{
   mainPassFramebuffer.bind();

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   mainPassFramebuffer.setColorAttachmentsEnabled(false);

   depthOnlyProgram->setUniformValue(kProjectionMatrix, perspectiveInfo.projectionMatrix);
   depthOnlyProgram->setUniformValue(kViewMatrix, perspectiveInfo.viewMatrix);

   for (const ModelComponent* modelComponent : scene.getModelComponents())
   {
      ASSERT(modelComponent);

      if (const SPtr<Model>& model = modelComponent->getModel())
      {
         Transform transform = modelComponent->getAbsoluteTransform();
         glm::mat4 modelMatrix = transform.toMatrix();

         depthOnlyProgram->setUniformValue(kModelMatrix, modelMatrix);
         depthOnlyProgram->commit();

         for (ModelSection& modelSection : model->getSections())
         {
            modelSection.mesh.draw();
         }
      }
   }
}

void ForwardSceneRenderer::renderMainPass(const Scene& scene, const PerspectiveInfo& perspectiveInfo)
{
   mainPassFramebuffer.bind();
   mainPassFramebuffer.setColorAttachmentsEnabled(true);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);

   for (const ModelComponent* modelComponent : scene.getModelComponents())
   {
      ASSERT(modelComponent);

      if (const SPtr<Model>& model = modelComponent->getModel())
      {
         Transform transform = modelComponent->getAbsoluteTransform();
         glm::mat4 modelMatrix = transform.toMatrix();
         glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));

         model->setMaterialParameter(kProjectionMatrix, perspectiveInfo.projectionMatrix, false);
         model->setMaterialParameter(kViewMatrix, perspectiveInfo.viewMatrix, false);
         model->setMaterialParameter(kModelMatrix, modelMatrix, false);
         model->setMaterialParameter(kNormalMatrix, normalMatrix, false);

         model->setMaterialParameter(kCameraPos, perspectiveInfo.cameraPosition);

         populateLightUniforms(scene, *model);

         model->draw();
      }
   }
}

void ForwardSceneRenderer::renderPostProcessPasses(const Scene& scene, const PerspectiveInfo& perspectiveInfo)
{
   // TODO Eventually an actual set of render passes

   glBindFramebuffer(GL_READ_FRAMEBUFFER, mainPassFramebuffer.getId());
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

   glBlitFramebuffer(0, 0, getWidth(), getHeight(), 0, 0, getWidth(), getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
