#include "Scene/Rendering/DeferredSceneRenderer.h"

#include "Core/Assert.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Texture.h"
#include "Platform/IOUtils.h"
#include "Resources/ResourceManager.h"
#include "Scene/Components/Lights/DirectionalLightComponent.h"
#include "Scene/Components/Lights/PointLightComponent.h"
#include "Scene/Components/Lights/SpotLightComponent.h"
#include "Scene/Components/ModelComponent.h"
#include "Scene/Scene.h"

#include <vector>

namespace
{
   const char* kProjectionMatrix = "uProjectionMatrix";
   const char* kViewMatrix = "uViewMatrix";
   const char* kModelMatrix = "uModelMatrix";
   const char* kNormalMatrix = "uNormalMatrix";

   const char* kCameraPos = "uCameraPos";
   const char* kViewport = "uViewport";

   Fb::Specification getGBufferSpecification(int width, int height)
   {
      static const std::array<Tex::InternalFormat, 5> kColorAttachmentFormats =
      {
         // Position
         Tex::InternalFormat::RGB32F,

         // Normal + shininess
         Tex::InternalFormat::RGBA32F,

         // Albedo
         Tex::InternalFormat::RGBA8,

         // Specular
         Tex::InternalFormat::RGBA8,

         // Emissive
         Tex::InternalFormat::RGB8
      };

      Fb::Specification specification;

      specification.width = width;
      specification.height = height;
      specification.depthStencilType = Fb::DepthStencilType::Depth24Stencil8;
      specification.colorAttachmentFormats = kColorAttachmentFormats;

      return specification;
   }

   void setGBufferUniforms(ShaderProgram& shaderProgram)
   {
      shaderProgram.setUniformValue("uPosition", 0);
      shaderProgram.setUniformValue("uNormalShininess", 1);
      shaderProgram.setUniformValue("uAlbedo", 2);
      shaderProgram.setUniformValue("uSpecular", 3);
   }
}

DeferredSceneRenderer::DeferredSceneRenderer(int initialWidth, int initialHeight,
   const SPtr<ResourceManager>& inResourceManager)
   : SceneRenderer(initialWidth, initialHeight)
   , resourceManager(inResourceManager)
   , gBuffer(getGBufferSpecification(getWidth(), getHeight()))
{
   ASSERT(resourceManager);

   {
      ModelSpecification sphereSpecification;
      IOUtils::getAbsoluteResourcePath("Sphere.obj", sphereSpecification.path);
      sphereSpecification.normalGenerationMode = NormalGenerationMode::None;
      sphereSpecification.cache = false;
      sphereSpecification.cacheTextures = false;

      SPtr<Model> sphereModel = resourceManager->loadModel(sphereSpecification);
      if (sphereModel->getSections().size() > 0)
      {
         sphereMesh = std::move(sphereModel->getSections()[0].mesh);
      }
   }

   {
      ModelSpecification coneSpecification;
      IOUtils::getAbsoluteResourcePath("Cone.obj", coneSpecification.path);
      coneSpecification.normalGenerationMode = NormalGenerationMode::None;
      coneSpecification.cache = false;
      coneSpecification.cacheTextures = false;

      SPtr<Model> coneModel = resourceManager->loadModel(coneSpecification);
      if (coneModel->getSections().size() > 0)
      {
         coneMesh = std::move(coneModel->getSections()[0].mesh);
      }
   }

   {
      std::vector<ShaderSpecification> shaderSpecifications;
      shaderSpecifications.resize(2);
      shaderSpecifications[0].type = ShaderType::Vertex;
      shaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("DepthOnly.vert", shaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("DepthOnly.frag", shaderSpecifications[1].path);

      depthOnlyProgram = resourceManager->loadShaderProgram(shaderSpecifications);
   }

   {
      std::vector<ShaderSpecification> shaderSpecifications;
      shaderSpecifications.resize(2);
      shaderSpecifications[0].type = ShaderType::Vertex;
      shaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("DeferredLighting.vert", shaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("DeferredLighting.frag", shaderSpecifications[1].path);

      shaderSpecifications[0].definitions["LIGHT_TYPE"] = "DIRECTIONAL_LIGHT";
      shaderSpecifications[1].definitions["LIGHT_TYPE"] = "DIRECTIONAL_LIGHT";
      directionalLightingProgram = resourceManager->loadShaderProgram(shaderSpecifications);
      setGBufferUniforms(*directionalLightingProgram);

      shaderSpecifications[0].definitions["LIGHT_TYPE"] = "POINT_LIGHT";
      shaderSpecifications[1].definitions["LIGHT_TYPE"] = "POINT_LIGHT";
      pointLightingProgram = resourceManager->loadShaderProgram(shaderSpecifications);
      setGBufferUniforms(*pointLightingProgram);

      shaderSpecifications[0].definitions["LIGHT_TYPE"] = "SPOT_LIGHT";
      shaderSpecifications[1].definitions["LIGHT_TYPE"] = "SPOT_LIGHT";
      spotLightingProgram = resourceManager->loadShaderProgram(shaderSpecifications);
      setGBufferUniforms(*spotLightingProgram);
   }
}

void DeferredSceneRenderer::renderScene(const Scene& scene)
{
   PerspectiveInfo perspectiveInfo;
   if (!getPerspectiveInfo(scene, perspectiveInfo))
   {
      return;
   }

   renderPrePass(scene, perspectiveInfo);
   renderBasePass(scene, perspectiveInfo);
   renderLightingPass(scene, perspectiveInfo);
   renderPostProcessPasses(scene, perspectiveInfo);
}

void DeferredSceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   SceneRenderer::onFramebufferSizeChanged(newWidth, newHeight);

   gBuffer.updateResolution(getWidth(), getHeight());
}

void DeferredSceneRenderer::renderPrePass(const Scene& scene, const PerspectiveInfo& perspectiveInfo)
{
   gBuffer.bind();

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   gBuffer.setColorAttachmentsEnabled(false);

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

void DeferredSceneRenderer::renderBasePass(const Scene& scene, const PerspectiveInfo& perspectiveInfo)
{
   glDepthFunc(GL_LEQUAL);

   gBuffer.setColorAttachmentsEnabled(true);

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

         model->draw();
      }
   }
}

void DeferredSceneRenderer::renderLightingPass(const Scene& scene, const PerspectiveInfo& perspectiveInfo)
{
   Framebuffer::bindDefault();

   glDisable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Blit the emissive color
   glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.getId());
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
   glReadBuffer(GL_COLOR_ATTACHMENT4);
   glDrawBuffer(GL_BACK);
   glBlitFramebuffer(0, 0, getWidth(), getHeight(), 0, 0, getWidth(), getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

   GLint attachmentIndex = 0;
   for (const SPtr<Texture>& colorAttachment : gBuffer.getColorAttachments())
   {
      glActiveTexture(GL_TEXTURE0 + attachmentIndex);
      colorAttachment->bind();

      ++attachmentIndex;
   }

#if SWAP_DEBUG
   setGBufferUniforms(*directionalLightingProgram);
   setGBufferUniforms(*pointLightingProgram);
   setGBufferUniforms(*spotLightingProgram);
#endif

   glm::vec4 viewport(getWidth(), getHeight(), 1.0f / getWidth(), 1.0f / getHeight());

   directionalLightingProgram->setUniformValue(kCameraPos, perspectiveInfo.cameraPosition);
   directionalLightingProgram->setUniformValue(kViewport, viewport);
   for (const DirectionalLightComponent* directionalLightComponent : scene.getDirectionalLightComponents())
   {
      directionalLightingProgram->setUniformValue("uDirectionalLight.color", directionalLightComponent->getColor());
      directionalLightingProgram->setUniformValue("uDirectionalLight.direction",
         directionalLightComponent->getAbsoluteTransform().orientation * MathUtils::kForwardVector);

      directionalLightingProgram->commit();
      getScreenMesh().draw();
   }
   
   glCullFace(GL_FRONT);

   pointLightingProgram->setUniformValue(kCameraPos, perspectiveInfo.cameraPosition);
   pointLightingProgram->setUniformValue(kViewport, viewport);
   for (const PointLightComponent* pointLightComponent : scene.getPointLightComponents())
   {
      Transform transform = pointLightComponent->getAbsoluteTransform();

      float maxScale = glm::max(transform.scale.x, glm::max(transform.scale.y, transform.scale.z));
      float radiusScale = pointLightComponent->getRadius() * maxScale;
      transform.scale = glm::vec3(radiusScale);

      pointLightingProgram->setUniformValue("uModelViewProjectionMatrix",
         perspectiveInfo.projectionMatrix * perspectiveInfo.viewMatrix * transform.toMatrix());

      pointLightingProgram->setUniformValue("uPointLight.color", pointLightComponent->getColor());
      pointLightingProgram->setUniformValue("uPointLight.position", transform.position);
      pointLightingProgram->setUniformValue("uPointLight.radius", radiusScale);

      pointLightingProgram->commit();
      sphereMesh.draw();
   }

   spotLightingProgram->setUniformValue(kCameraPos, perspectiveInfo.cameraPosition);
   spotLightingProgram->setUniformValue(kViewport, viewport);
   for (const SpotLightComponent* spotLightComponent : scene.getSpotLightComponents())
   {
      Transform transform = spotLightComponent->getAbsoluteTransform();

      float beamAngle = glm::radians(spotLightComponent->getBeamAngle());
      float cutoffAngle = glm::radians(spotLightComponent->getCutoffAngle());

      float radiusScale = spotLightComponent->getRadius() * transform.scale.z;
      float widthScale = glm::tan(cutoffAngle) * radiusScale * 2.0f;
      transform.scale = glm::vec3(widthScale, widthScale, radiusScale);

      spotLightingProgram->setUniformValue("uModelViewProjectionMatrix",
         perspectiveInfo.projectionMatrix * perspectiveInfo.viewMatrix * transform.toMatrix());

      spotLightingProgram->setUniformValue("uSpotLight.color", spotLightComponent->getColor());
      spotLightingProgram->setUniformValue("uSpotLight.direction", transform.orientation * MathUtils::kForwardVector);
      spotLightingProgram->setUniformValue("uSpotLight.position", transform.position);
      spotLightingProgram->setUniformValue("uSpotLight.radius", radiusScale);
      spotLightingProgram->setUniformValue("uSpotLight.beamAngle", beamAngle);
      spotLightingProgram->setUniformValue("uSpotLight.cutoffAngle", cutoffAngle);

      spotLightingProgram->commit();
      coneMesh.draw();
   }

   glCullFace(GL_BACK);
   glDisable(GL_BLEND);
}

void DeferredSceneRenderer::renderPostProcessPasses(const Scene& scene, const PerspectiveInfo& perspectiveInfo)
{
   /*glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.getId());
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

   glBlitFramebuffer(0, 0, getWidth(), getHeight(), 0, 0, getWidth(), getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);*/
}
