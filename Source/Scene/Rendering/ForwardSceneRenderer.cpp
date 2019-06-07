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
#include "Scene/Scene.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

namespace
{
   Fb::Specification getMainPassFramebufferSpecification(int width, int height, int numSamples)
   {
      static const std::array<Tex::InternalFormat, 2> kColorAttachmentFormats =
      {
         // Normal
         Tex::InternalFormat::RGB32F,

         // Color
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

   void populateDirectionalLightUniforms(const Scene& scene, ShaderProgram& program)
   {
      int directionalLightIndex = 0;
      for (const DirectionalLightComponent* directionalLightComponent : scene.getDirectionalLightComponents())
      {
         std::string directionalLightStr = "uDirectionalLights[" + std::to_string(directionalLightIndex) + "]";

         program.setUniformValue(directionalLightStr + ".color", directionalLightComponent->getColor());
         program.setUniformValue(directionalLightStr + ".direction",
            directionalLightComponent->getAbsoluteTransform().orientation * MathUtils::kForwardVector);

         ++directionalLightIndex;
      }
      program.setUniformValue("uNumDirectionalLights", static_cast<int>(scene.getDirectionalLightComponents().size()));
   }

   void populatePointLightUniforms(const Scene& scene, ShaderProgram& program)
   {
      int pointLightIndex = 0;
      for (const PointLightComponent* pointLightComponent : scene.getPointLightComponents())
      {
         std::string pointLightStr = "uPointLights[" + std::to_string(pointLightIndex) + "]";

         Transform transform = pointLightComponent->getAbsoluteTransform();
         float radiusScale = glm::max(transform.scale.x, glm::max(transform.scale.y, transform.scale.z));

         program.setUniformValue(pointLightStr + ".color", pointLightComponent->getColor());
         program.setUniformValue(pointLightStr + ".position", transform.position);
         program.setUniformValue(pointLightStr + ".radius", pointLightComponent->getRadius() * radiusScale);

         ++pointLightIndex;
      }
      program.setUniformValue("uNumPointLights", static_cast<int>(scene.getPointLightComponents().size()));
   }

   void populateSpotLightUniforms(const Scene& scene, ShaderProgram& program)
   {
      int spotLightIndex = 0;
      for (const SpotLightComponent* spotLightComponent : scene.getSpotLightComponents())
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
      program.setUniformValue("uNumSpotLights", static_cast<int>(scene.getSpotLightComponents().size()));
   }

   void populateLightUniforms(const Scene& scene, ShaderProgram& program)
   {
      populateDirectionalLightUniforms(scene, program);
      populatePointLightUniforms(scene, program);
      populateSpotLightUniforms(scene, program);
   }
}

ForwardSceneRenderer::ForwardSceneRenderer(int initialWidth, int initialHeight, int numSamples,
   const SPtr<ResourceManager>& inResourceManager)
   : SceneRenderer(initialWidth, initialHeight, inResourceManager, false)
   , mainPassFramebuffer(getMainPassFramebufferSpecification(getWidth(), getHeight(), numSamples))
{
   std::vector<ShaderSpecification> depthShaderSpecifications;
   depthShaderSpecifications.resize(2);
   depthShaderSpecifications[0].type = ShaderType::Vertex;
   depthShaderSpecifications[1].type = ShaderType::Fragment;
   IOUtils::getAbsoluteResourcePath("DepthOnly.vert", depthShaderSpecifications[0].path);
   IOUtils::getAbsoluteResourcePath("DepthOnly.frag", depthShaderSpecifications[1].path);
   depthOnlyProgram = getResourceManager().loadShaderProgram(depthShaderSpecifications);

   loadNormalProgramPermutations();
   loadForwardProgramPermutations();

   setSSAOTextures(mainPassFramebuffer.getDepthStencilAttachment(), nullptr, mainPassFramebuffer.getColorAttachments()[0]);

   forwardMaterial.setParameter("uAmbientOcclusion", getSSAOBlurTexture());
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
   renderNormalPass(scene, perspectiveInfo);
   renderSSAOPass(perspectiveInfo);
   renderMainPass(scene, perspectiveInfo);
   renderPostProcessPasses(scene, perspectiveInfo);
}

void ForwardSceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   SceneRenderer::onFramebufferSizeChanged(newWidth, newHeight);

   mainPassFramebuffer.updateResolution(getWidth(), getHeight());
}

void ForwardSceneRenderer::renderPrePass(const Scene& scene, const PerspectiveInfo& perspectiveInfo)
{
   mainPassFramebuffer.bind();

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   mainPassFramebuffer.setColorAttachmentsEnabled(false);

   depthOnlyProgram->setUniformValue(UniformNames::kProjectionMatrix, perspectiveInfo.projectionMatrix);
   depthOnlyProgram->setUniformValue(UniformNames::kViewMatrix, perspectiveInfo.viewMatrix);

   for (const ModelComponent* modelComponent : scene.getModelComponents())
   {
      ASSERT(modelComponent);

      if (const SPtr<Model>& model = modelComponent->getModel())
      {
         Transform transform = modelComponent->getAbsoluteTransform();
         glm::mat4 modelMatrix = transform.toMatrix();

         depthOnlyProgram->setUniformValue(UniformNames::kModelMatrix, modelMatrix);

         DrawingContext context(depthOnlyProgram.get());
         model->draw(context, false);
      }
   }
}

void ForwardSceneRenderer::renderNormalPass(const Scene& scene, const PerspectiveInfo& perspectiveInfo)
{
   mainPassFramebuffer.setColorAttachmentsEnabled(true);

   glDepthFunc(GL_LEQUAL);

   for (SPtr<ShaderProgram>& normalProgramPermutation : normalProgramPermutations)
   {
      normalProgramPermutation->setUniformValue(UniformNames::kProjectionMatrix, perspectiveInfo.projectionMatrix);
      normalProgramPermutation->setUniformValue(UniformNames::kViewMatrix, perspectiveInfo.viewMatrix);
   }

   for (const ModelComponent* modelComponent : scene.getModelComponents())
   {
      ASSERT(modelComponent);

      if (const SPtr<Model>& model = modelComponent->getModel())
      {
         Transform transform = modelComponent->getAbsoluteTransform();
         glm::mat4 modelMatrix = transform.toMatrix();
         glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));

         for (ModelSection& section : model->getSections())
         {
            SPtr<ShaderProgram>& normalProgramPermutation = selectNormalPermutation(section.material);

            normalProgramPermutation->setUniformValue(UniformNames::kModelMatrix, modelMatrix);
            normalProgramPermutation->setUniformValue(UniformNames::kNormalMatrix, normalMatrix, false);

            DrawingContext context(normalProgramPermutation.get());
            section.draw(context, true);
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

   for (SPtr<ShaderProgram>& forwardProgramPermutation : forwardProgramPermutations)
   {
      forwardProgramPermutation->setUniformValue(UniformNames::kProjectionMatrix, perspectiveInfo.projectionMatrix);
      forwardProgramPermutation->setUniformValue(UniformNames::kViewMatrix, perspectiveInfo.viewMatrix);

      forwardProgramPermutation->setUniformValue(UniformNames::kCameraPos, perspectiveInfo.cameraPosition);

      glm::vec4 viewport(getWidth(), getHeight(), 1.0f / getWidth(), 1.0f / getHeight());
      forwardProgramPermutation->setUniformValue(UniformNames::kViewport, viewport);

      populateLightUniforms(scene, *forwardProgramPermutation);
   }

   for (const ModelComponent* modelComponent : scene.getModelComponents())
   {
      ASSERT(modelComponent);

      if (const SPtr<Model>& model = modelComponent->getModel())
      {
         Transform transform = modelComponent->getAbsoluteTransform();
         glm::mat4 modelMatrix = transform.toMatrix();
         glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));

         for (ModelSection& section : model->getSections())
         {
            SPtr<ShaderProgram>& forwardProgramPermutation = selectForwardPermutation(section.material);

            forwardProgramPermutation->setUniformValue(UniformNames::kModelMatrix, modelMatrix);
            forwardProgramPermutation->setUniformValue(UniformNames::kNormalMatrix, normalMatrix, false);

            DrawingContext context(forwardProgramPermutation.get());
            forwardMaterial.apply(context);
            section.draw(context, true);
         }
      }
   }
}

void ForwardSceneRenderer::renderPostProcessPasses(const Scene& scene, const PerspectiveInfo& perspectiveInfo)
{
   // TODO Eventually an actual set of render passes

   glBindFramebuffer(GL_READ_FRAMEBUFFER, mainPassFramebuffer.getId());
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

   glReadBuffer(GL_COLOR_ATTACHMENT1);
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

   for (int i = 0; i < normalProgramPermutations.size(); ++i)
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

   for (int i = 0; i < forwardProgramPermutations.size(); ++i)
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
