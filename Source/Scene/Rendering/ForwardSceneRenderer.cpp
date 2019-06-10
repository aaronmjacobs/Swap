#include "Scene/Rendering/ForwardSceneRenderer.h"

#include "Core/Assert.h"
#include "Graphics/DrawingContext.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Texture.h"
#include "Math/MathUtils.h"
#include "Platform/IOUtils.h"
#include "Resources/ResourceManager.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/Lights/DirectionalLightComponent.h"
#include "Scene/Components/Lights/PointLightComponent.h"
#include "Scene/Components/Lights/SpotLightComponent.h"
#include "Scene/Components/ModelComponent.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

namespace
{
   void populateDirectionalLightUniforms(const SceneRenderInfo& sceneRenderInfo, ShaderProgram& program)
   {
      int directionalLightIndex = 0;
      for (const DirectionalLightComponent* directionalLightComponent : sceneRenderInfo.directionalLights)
      {
         std::string directionalLightStr = "uDirectionalLights[" + std::to_string(directionalLightIndex) + "]";

         program.setUniformValue(directionalLightStr + ".color", directionalLightComponent->getColor());
         program.setUniformValue(directionalLightStr + ".direction",
            directionalLightComponent->getAbsoluteTransform().orientation * MathUtils::kForwardVector);

         ++directionalLightIndex;
      }
      program.setUniformValue("uNumDirectionalLights", static_cast<int>(sceneRenderInfo.directionalLights.size()));
   }

   void populatePointLightUniforms(const SceneRenderInfo& sceneRenderInfo, ShaderProgram& program)
   {
      int pointLightIndex = 0;
      for (const PointLightComponent* pointLightComponent : sceneRenderInfo.pointLights)
      {
         std::string pointLightStr = "uPointLights[" + std::to_string(pointLightIndex) + "]";

         Transform transform = pointLightComponent->getAbsoluteTransform();
         float radiusScale = glm::max(transform.scale.x, glm::max(transform.scale.y, transform.scale.z));

         program.setUniformValue(pointLightStr + ".color", pointLightComponent->getColor());
         program.setUniformValue(pointLightStr + ".position", transform.position);
         program.setUniformValue(pointLightStr + ".radius", pointLightComponent->getRadius() * radiusScale);

         ++pointLightIndex;
      }
      program.setUniformValue("uNumPointLights", static_cast<int>(sceneRenderInfo.pointLights.size()));
   }

   void populateSpotLightUniforms(const SceneRenderInfo& sceneRenderInfo, ShaderProgram& program)
   {
      int spotLightIndex = 0;
      for (const SpotLightComponent* spotLightComponent : sceneRenderInfo.spotLights)
      {
         std::string spotLightStr = "uSpotLights[" + std::to_string(spotLightIndex) + "]";

         Transform transform = spotLightComponent->getAbsoluteTransform();
         float radiusScale = glm::max(transform.scale.x, glm::max(transform.scale.y, transform.scale.z));

         program.setUniformValue(spotLightStr + ".color", spotLightComponent->getColor());
         program.setUniformValue(spotLightStr + ".direction", transform.orientation * MathUtils::kForwardVector);
         program.setUniformValue(spotLightStr + ".position", transform.position);
         program.setUniformValue(spotLightStr + ".radius", spotLightComponent->getRadius() * radiusScale);
         program.setUniformValue(spotLightStr + ".beamAngle", glm::radians(spotLightComponent->getBeamAngle()));
         program.setUniformValue(spotLightStr + ".cutoffAngle", glm::radians(spotLightComponent->getCutoffAngle()));

         ++spotLightIndex;
      }
      program.setUniformValue("uNumSpotLights", static_cast<int>(sceneRenderInfo.spotLights.size()));
   }

   void populateLightUniforms(const SceneRenderInfo& sceneRenderInfo, ShaderProgram& program)
   {
      populateDirectionalLightUniforms(sceneRenderInfo, program);
      populatePointLightUniforms(sceneRenderInfo, program);
      populateSpotLightUniforms(sceneRenderInfo, program);
   }
}

ForwardSceneRenderer::ForwardSceneRenderer(int initialWidth, int initialHeight, int numSamples,
   const SPtr<ResourceManager>& inResourceManager)
   : SceneRenderer(initialWidth, initialHeight, inResourceManager, false)
{
   {
      static const std::array<Tex::InternalFormat, 2> kColorAttachmentFormats =
      {
         // Color
         Tex::InternalFormat::RGBA8,

         // Normal
         Tex::InternalFormat::RGB32F
      };

      Fb::Specification specification;
      specification.width = getWidth();
      specification.height = getHeight();
      specification.samples = numSamples;
      specification.depthStencilType = Fb::DepthStencilType::Depth24Stencil8;
      specification.colorAttachmentFormats = kColorAttachmentFormats;

      Fb::Attachments attachments = Fb::generateAttachments(specification);
      ASSERT(attachments.colorAttachments.size() == kColorAttachmentFormats.size());

      depthStencilTexture = std::move(attachments.depthStencilAttachment);
      colorTexture = std::move(attachments.colorAttachments[0]);
      normalTexture = std::move(attachments.colorAttachments[1]);

      Fb::Attachments normalPassAttachments;
      normalPassAttachments.depthStencilAttachment = depthStencilTexture;
      normalPassAttachments.colorAttachments.push_back(normalTexture);
      normalPassFramebuffer.setAttachments(std::move(normalPassAttachments));

      Fb::Attachments mainPassAttachments;
      mainPassAttachments.depthStencilAttachment = depthStencilTexture;
      mainPassAttachments.colorAttachments.push_back(colorTexture);
      mainPassFramebuffer.setAttachments(std::move(mainPassAttachments));
   }

   loadNormalProgramPermutations();
   loadForwardProgramPermutations();

   setPrePassDepthAttachment(depthStencilTexture);
   setSSAOTextures(depthStencilTexture, nullptr, normalTexture);

   forwardMaterial.setParameter("uAmbientOcclusion", getSSAOBlurTexture());
}

void ForwardSceneRenderer::renderScene(const Scene& scene)
{
   // TODO Use uniform buffer objects

   SceneRenderInfo sceneRenderInfo;
   if (!calcSceneRenderInfo(scene, sceneRenderInfo))
   {
      return;
   }

   renderPrePass(sceneRenderInfo);
   renderNormalPass(sceneRenderInfo);
   renderSSAOPass(sceneRenderInfo);
   renderMainPass(sceneRenderInfo);
   renderPostProcessPasses(sceneRenderInfo);
}

void ForwardSceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   SceneRenderer::onFramebufferSizeChanged(newWidth, newHeight);

   depthStencilTexture->updateResolution(getWidth(), getHeight());
   colorTexture->updateResolution(getWidth(), getHeight());
   normalTexture->updateResolution(getWidth(), getHeight());
}

void ForwardSceneRenderer::renderNormalPass(const SceneRenderInfo& sceneRenderInfo)
{
   normalPassFramebuffer.bind();

   for (SPtr<ShaderProgram>& normalProgramPermutation : normalProgramPermutations)
   {
      normalProgramPermutation->setUniformValue(UniformNames::kProjectionMatrix, sceneRenderInfo.perspectiveInfo.projectionMatrix);
      normalProgramPermutation->setUniformValue(UniformNames::kViewMatrix, sceneRenderInfo.perspectiveInfo.viewMatrix);
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
            SPtr<ShaderProgram>& normalProgramPermutation = selectNormalPermutation(material);

            normalProgramPermutation->setUniformValue(UniformNames::kModelMatrix, modelMatrix);
            normalProgramPermutation->setUniformValue(UniformNames::kNormalMatrix, normalMatrix, false);

            DrawingContext context(normalProgramPermutation.get());
            material.apply(context);
            section.draw(context);
         }
      }
   }
}

void ForwardSceneRenderer::renderMainPass(const SceneRenderInfo& sceneRenderInfo)
{
   mainPassFramebuffer.bind();

   for (SPtr<ShaderProgram>& forwardProgramPermutation : forwardProgramPermutations)
   {
      forwardProgramPermutation->setUniformValue(UniformNames::kProjectionMatrix, sceneRenderInfo.perspectiveInfo.projectionMatrix);
      forwardProgramPermutation->setUniformValue(UniformNames::kViewMatrix, sceneRenderInfo.perspectiveInfo.viewMatrix);

      forwardProgramPermutation->setUniformValue(UniformNames::kCameraPos, sceneRenderInfo.perspectiveInfo.cameraPosition);

      glm::vec4 viewport(getWidth(), getHeight(), 1.0f / getWidth(), 1.0f / getHeight());
      forwardProgramPermutation->setUniformValue(UniformNames::kViewport, viewport);

      populateLightUniforms(sceneRenderInfo, *forwardProgramPermutation);
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
            SPtr<ShaderProgram>& forwardProgramPermutation = selectForwardPermutation(material);

            forwardProgramPermutation->setUniformValue(UniformNames::kModelMatrix, modelMatrix);
            forwardProgramPermutation->setUniformValue(UniformNames::kNormalMatrix, normalMatrix, false);

            DrawingContext context(forwardProgramPermutation.get());
            forwardMaterial.apply(context);
            material.apply(context);
            section.draw(context);
         }
      }
   }
}

void ForwardSceneRenderer::renderPostProcessPasses(const SceneRenderInfo& sceneRenderInfo)
{
   // TODO Eventually an actual set of render passes

   glBindFramebuffer(GL_READ_FRAMEBUFFER, mainPassFramebuffer.getId());
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

   glReadBuffer(GL_COLOR_ATTACHMENT0);
   glDrawBuffer(GL_BACK);

   glBlitFramebuffer(0, 0, getWidth(), getHeight(), 0, 0, getWidth(), getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void ForwardSceneRenderer::loadNormalProgramPermutations()
{
   std::vector<ShaderSpecification> shaderSpecifications;
   shaderSpecifications.resize(2);
   shaderSpecifications[0].type = ShaderType::Vertex;
   shaderSpecifications[1].type = ShaderType::Fragment;
   IOUtils::getAbsoluteResourcePath("Normals.vert", shaderSpecifications[0].path);
   IOUtils::getAbsoluteResourcePath("Normals.frag", shaderSpecifications[1].path);

   for (std::size_t i = 0; i < normalProgramPermutations.size(); ++i)
   {
      for (ShaderSpecification& shaderSpecification : shaderSpecifications)
      {
         shaderSpecification.definitions["WITH_NORMAL_TEXTURE"] = i & 0b001 ? "1" : "0";
      }

      normalProgramPermutations[i] = getResourceManager().loadShaderProgram(shaderSpecifications);
   }
}

SPtr<ShaderProgram>& ForwardSceneRenderer::selectNormalPermutation(const Material& material)
{
   int index = material.hasCommonParameter(CommonMaterialParameter::NormalTexture) * 0b001;

   return normalProgramPermutations[index];
}

void ForwardSceneRenderer::loadForwardProgramPermutations()
{
   std::vector<ShaderSpecification> shaderSpecifications;
   shaderSpecifications.resize(2);
   shaderSpecifications[0].type = ShaderType::Vertex;
   shaderSpecifications[1].type = ShaderType::Fragment;
   IOUtils::getAbsoluteResourcePath("Forward.vert", shaderSpecifications[0].path);
   IOUtils::getAbsoluteResourcePath("Forward.frag", shaderSpecifications[1].path);

   for (std::size_t i = 0; i < forwardProgramPermutations.size(); ++i)
   {
      for (ShaderSpecification& shaderSpecification : shaderSpecifications)
      {
         shaderSpecification.definitions["WITH_DIFFUSE_TEXTURE"] = i & 0b001 ? "1" : "0";
         shaderSpecification.definitions["WITH_SPECULAR_TEXTURE"] = i & 0b010 ? "1" : "0";
         shaderSpecification.definitions["WITH_NORMAL_TEXTURE"] = i & 0b100 ? "1" : "0";
      }

      forwardProgramPermutations[i] = getResourceManager().loadShaderProgram(shaderSpecifications);
   }
}

SPtr<ShaderProgram>& ForwardSceneRenderer::selectForwardPermutation(const Material& material)
{
   int index = material.hasCommonParameter(CommonMaterialParameter::DiffuseTexture) * 0b001
      + material.hasCommonParameter(CommonMaterialParameter::SpecularTexture) * 0b010
      + material.hasCommonParameter(CommonMaterialParameter::NormalTexture) * 0b100;

   return forwardProgramPermutations[index];
}
