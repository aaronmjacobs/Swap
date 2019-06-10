#include "Scene/Rendering/DeferredSceneRenderer.h"

#include "Core/Assert.h"
#include "Graphics/DrawingContext.h"
#include "Graphics/Material.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Texture.h"
#include "Platform/IOUtils.h"
#include "Resources/ResourceManager.h"
#include "Scene/Components/Lights/DirectionalLightComponent.h"
#include "Scene/Components/Lights/PointLightComponent.h"
#include "Scene/Components/Lights/SpotLightComponent.h"
#include "Scene/Components/ModelComponent.h"

#include <vector>

DeferredSceneRenderer::DeferredSceneRenderer(int initialWidth, int initialHeight,
   const SPtr<ResourceManager>& inResourceManager)
   : SceneRenderer(initialWidth, initialHeight, inResourceManager, true)
{
   {
      static const std::array<Tex::InternalFormat, 6> kColorAttachmentFormats =
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
         Tex::InternalFormat::RGB8,

         // Color
         Tex::InternalFormat::RGB8
      };

      Fb::Specification specification;
      specification.width = getWidth();
      specification.height = getHeight();
      specification.depthStencilType = Fb::DepthStencilType::Depth24Stencil8;
      specification.colorAttachmentFormats = kColorAttachmentFormats;

      Fb::Attachments attachments = Fb::generateAttachments(specification);
      ASSERT(attachments.colorAttachments.size() == kColorAttachmentFormats.size());

      depthStencilTexture = attachments.depthStencilAttachment;
      positionTexture = attachments.colorAttachments[0];
      normalShininessTexture = attachments.colorAttachments[1];
      albedoTexture = attachments.colorAttachments[2];
      specularTexture = attachments.colorAttachments[3];
      emissiveTexture = attachments.colorAttachments[4];
      colorTexture = attachments.colorAttachments[5];

      Fb::Attachments basePassAttachments;
      basePassAttachments.depthStencilAttachment = depthStencilTexture;
      basePassAttachments.colorAttachments.push_back(positionTexture);
      basePassAttachments.colorAttachments.push_back(normalShininessTexture);
      basePassAttachments.colorAttachments.push_back(albedoTexture);
      basePassAttachments.colorAttachments.push_back(specularTexture);
      basePassAttachments.colorAttachments.push_back(emissiveTexture);
      basePassFramebuffer.setAttachments(std::move(basePassAttachments));

      Fb::Attachments lightingPassAttachments;
      lightingPassAttachments.colorAttachments.push_back(colorTexture);
      lightingPassFramebuffer.setAttachments(std::move(lightingPassAttachments));
   }

   loadGBufferProgramPermutations();

   {
      std::vector<ShaderSpecification> shaderSpecifications;
      shaderSpecifications.resize(2);
      shaderSpecifications[0].type = ShaderType::Vertex;
      shaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("DeferredLighting.vert", shaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("DeferredLighting.frag", shaderSpecifications[1].path);

      shaderSpecifications[0].definitions["LIGHT_TYPE"] = "DIRECTIONAL_LIGHT";
      shaderSpecifications[1].definitions["LIGHT_TYPE"] = "DIRECTIONAL_LIGHT";
      directionalLightingProgram = getResourceManager().loadShaderProgram(shaderSpecifications);

      shaderSpecifications[0].definitions["LIGHT_TYPE"] = "POINT_LIGHT";
      shaderSpecifications[1].definitions["LIGHT_TYPE"] = "POINT_LIGHT";
      pointLightingProgram = getResourceManager().loadShaderProgram(shaderSpecifications);

      shaderSpecifications[0].definitions["LIGHT_TYPE"] = "SPOT_LIGHT";
      shaderSpecifications[1].definitions["LIGHT_TYPE"] = "SPOT_LIGHT";
      spotLightingProgram = getResourceManager().loadShaderProgram(shaderSpecifications);
   }

   {
      lightingMaterial.setParameter("uPosition", positionTexture);
      lightingMaterial.setParameter("uNormalShininess", normalShininessTexture);
      lightingMaterial.setParameter("uAlbedo", albedoTexture);
      lightingMaterial.setParameter("uSpecular", specularTexture);

      lightingMaterial.setParameter("uAmbientOcclusion", getSSAOBlurTexture());
   }

   {
      ModelSpecification sphereSpecification;
      IOUtils::getAbsoluteResourcePath("Sphere.obj", sphereSpecification.path);
      sphereSpecification.normalGenerationMode = NormalGenerationMode::None;
      sphereSpecification.cache = false;
      sphereSpecification.cacheTextures = false;

      sphereMesh = getResourceManager().loadModel(sphereSpecification).getMesh();
   }

   {
      ModelSpecification coneSpecification;
      IOUtils::getAbsoluteResourcePath("Cone.obj", coneSpecification.path);
      coneSpecification.normalGenerationMode = NormalGenerationMode::None;
      coneSpecification.cache = false;
      coneSpecification.cacheTextures = false;

      coneMesh = getResourceManager().loadModel(coneSpecification).getMesh();
   }

   setPrePassDepthAttachment(depthStencilTexture);
   setSSAOTextures(nullptr, positionTexture, normalShininessTexture);
}

void DeferredSceneRenderer::renderScene(const Scene& scene)
{
   SceneRenderInfo sceneRenderInfo;
   if (!calcSceneRenderInfo(scene, sceneRenderInfo))
   {
      return;
   }

   renderPrePass(sceneRenderInfo);
   renderBasePass(sceneRenderInfo);
   renderSSAOPass(sceneRenderInfo);
   renderLightingPass(sceneRenderInfo);
   renderPostProcessPasses(sceneRenderInfo);
}

void DeferredSceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   SceneRenderer::onFramebufferSizeChanged(newWidth, newHeight);

   depthStencilTexture->updateResolution(getWidth(), getHeight());
   positionTexture->updateResolution(getWidth(), getHeight());
   normalShininessTexture->updateResolution(getWidth(), getHeight());
   albedoTexture->updateResolution(getWidth(), getHeight());
   specularTexture->updateResolution(getWidth(), getHeight());
   emissiveTexture->updateResolution(getWidth(), getHeight());
   colorTexture->updateResolution(getWidth(), getHeight());
}

void DeferredSceneRenderer::renderBasePass(const SceneRenderInfo& sceneRenderInfo)
{
   basePassFramebuffer.bind();

   for (SPtr<ShaderProgram>& gBufferProgramPermutation : gBufferProgramPermutations)
   {
      gBufferProgramPermutation->setUniformValue(UniformNames::kProjectionMatrix, sceneRenderInfo.perspectiveInfo.projectionMatrix);
      gBufferProgramPermutation->setUniformValue(UniformNames::kViewMatrix, sceneRenderInfo.perspectiveInfo.viewMatrix);
   }

   for (const ModelRenderInfo& modelRenderInfo : sceneRenderInfo.modelRenderInfo)
   {
      ASSERT(modelRenderInfo.model);

      glm::mat4 modelMatrix = modelRenderInfo.localToWorld.toMatrix();
      glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));

      for (std::size_t i = 0; i < modelRenderInfo.model->getNumMeshSections(); ++i)
      {
         const MeshSection& section = modelRenderInfo.model->getMeshSection(i);
         const Material& material = modelRenderInfo.model->getMaterial(i);

         bool visible = i >= modelRenderInfo.visibilityMask.size() || modelRenderInfo.visibilityMask[i];
         if (visible)
         {
            SPtr<ShaderProgram>& gBufferProgramPermutation = selectGBufferPermutation(material);

            gBufferProgramPermutation->setUniformValue(UniformNames::kModelMatrix, modelMatrix);
            gBufferProgramPermutation->setUniformValue(UniformNames::kNormalMatrix, normalMatrix, false);

            DrawingContext context(gBufferProgramPermutation.get());
            material.apply(context);
            section.draw(context);
         }
      }
   }
}

void DeferredSceneRenderer::renderLightingPass(const SceneRenderInfo& sceneRenderInfo)
{
   lightingPassFramebuffer.bind();

   // Blit the emissive color
   glBindFramebuffer(GL_READ_FRAMEBUFFER, basePassFramebuffer.getId());
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightingPassFramebuffer.getId());
   glReadBuffer(GL_COLOR_ATTACHMENT4);
   glDrawBuffer(GL_COLOR_ATTACHMENT0);
   glBlitFramebuffer(0, 0, getWidth(), getHeight(), 0, 0, getWidth(), getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

   glDisable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE);

   glm::vec4 viewport(getWidth(), getHeight(), 1.0f / getWidth(), 1.0f / getHeight());

   directionalLightingProgram->setUniformValue(UniformNames::kCameraPos, sceneRenderInfo.perspectiveInfo.cameraPosition);
   directionalLightingProgram->setUniformValue(UniformNames::kViewport, viewport);
   for (const DirectionalLightComponent* directionalLightComponent : sceneRenderInfo.directionalLights)
   {
      directionalLightingProgram->setUniformValue("uDirectionalLight.color", directionalLightComponent->getColor());
      directionalLightingProgram->setUniformValue("uDirectionalLight.direction",
         directionalLightComponent->getAbsoluteTransform().orientation * MathUtils::kForwardVector);

      DrawingContext context(directionalLightingProgram.get());
      lightingMaterial.apply(context);
      getScreenMesh().draw(context);
   }

   glCullFace(GL_FRONT);

   pointLightingProgram->setUniformValue(UniformNames::kCameraPos, sceneRenderInfo.perspectiveInfo.cameraPosition);
   pointLightingProgram->setUniformValue(UniformNames::kViewport, viewport);
   for (const PointLightComponent* pointLightComponent : sceneRenderInfo.pointLights)
   {
      Transform transform = pointLightComponent->getAbsoluteTransform();

      float scaledRadius = pointLightComponent->getScaledRadius();
      transform.scale = glm::vec3(scaledRadius);

      pointLightingProgram->setUniformValue("uModelViewProjectionMatrix",
         sceneRenderInfo.perspectiveInfo.projectionMatrix * sceneRenderInfo.perspectiveInfo.viewMatrix * transform.toMatrix());

      pointLightingProgram->setUniformValue("uPointLight.color", pointLightComponent->getColor());
      pointLightingProgram->setUniformValue("uPointLight.position", transform.position);
      pointLightingProgram->setUniformValue("uPointLight.radius", scaledRadius);

      DrawingContext context(pointLightingProgram.get());
      lightingMaterial.apply(context);
      sphereMesh->draw(context);
   }

   spotLightingProgram->setUniformValue(UniformNames::kCameraPos, sceneRenderInfo.perspectiveInfo.cameraPosition);
   spotLightingProgram->setUniformValue(UniformNames::kViewport, viewport);
   for (const SpotLightComponent* spotLightComponent : sceneRenderInfo.spotLights)
   {
      Transform transform = spotLightComponent->getAbsoluteTransform();

      float beamAngle = glm::radians(spotLightComponent->getBeamAngle());
      float cutoffAngle = glm::radians(spotLightComponent->getCutoffAngle());

      float scaledRadius = spotLightComponent->getScaledRadius();
      float widthScale = glm::tan(cutoffAngle) * scaledRadius * 2.0f;
      transform.scale = glm::vec3(widthScale, widthScale, scaledRadius);

      spotLightingProgram->setUniformValue("uModelViewProjectionMatrix",
         sceneRenderInfo.perspectiveInfo.projectionMatrix * sceneRenderInfo.perspectiveInfo.viewMatrix * transform.toMatrix());

      spotLightingProgram->setUniformValue("uSpotLight.color", spotLightComponent->getColor());
      spotLightingProgram->setUniformValue("uSpotLight.direction", transform.orientation * MathUtils::kForwardVector);
      spotLightingProgram->setUniformValue("uSpotLight.position", transform.position);
      spotLightingProgram->setUniformValue("uSpotLight.radius", scaledRadius);
      spotLightingProgram->setUniformValue("uSpotLight.beamAngle", beamAngle);
      spotLightingProgram->setUniformValue("uSpotLight.cutoffAngle", cutoffAngle);

      DrawingContext context(spotLightingProgram.get());
      lightingMaterial.apply(context);
      coneMesh->draw(context);
   }

   glCullFace(GL_BACK);
   glDisable(GL_BLEND);
   glEnable(GL_DEPTH_TEST);
}

void DeferredSceneRenderer::renderPostProcessPasses(const SceneRenderInfo& sceneRenderInfo)
{
   Framebuffer::bindDefault();

   // TODO Eventually an actual set of render passes

   glBindFramebuffer(GL_READ_FRAMEBUFFER, lightingPassFramebuffer.getId());
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

   glReadBuffer(GL_COLOR_ATTACHMENT0);
   glDrawBuffer(GL_BACK);

   glBlitFramebuffer(0, 0, getWidth(), getHeight(), 0, 0, getWidth(), getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void DeferredSceneRenderer::loadGBufferProgramPermutations()
{
   std::vector<ShaderSpecification> shaderSpecifications;
   shaderSpecifications.resize(2);
   shaderSpecifications[0].type = ShaderType::Vertex;
   shaderSpecifications[1].type = ShaderType::Fragment;
   IOUtils::getAbsoluteResourcePath("GBuffer.vert", shaderSpecifications[0].path);
   IOUtils::getAbsoluteResourcePath("GBuffer.frag", shaderSpecifications[1].path);

   for (std::size_t i = 0; i < gBufferProgramPermutations.size(); ++i)
   {
      for (ShaderSpecification& shaderSpecification : shaderSpecifications)
      {
         shaderSpecification.definitions["WITH_DIFFUSE_TEXTURE"] = i & 0b001 ? "1" : "0";
         shaderSpecification.definitions["WITH_SPECULAR_TEXTURE"] = i & 0b010 ? "1" : "0";
         shaderSpecification.definitions["WITH_NORMAL_TEXTURE"] = i & 0b100 ? "1" : "0";
      }

      gBufferProgramPermutations[i] = getResourceManager().loadShaderProgram(shaderSpecifications);
   }
}

SPtr<ShaderProgram>& DeferredSceneRenderer::selectGBufferPermutation(const Material& material)
{
   int index = material.hasCommonParameter(CommonMaterialParameter::DiffuseTexture) * 0b001
      + material.hasCommonParameter(CommonMaterialParameter::SpecularTexture) * 0b010
      + material.hasCommonParameter(CommonMaterialParameter::NormalTexture) * 0b100;

   return gBufferProgramPermutations[index];
}
