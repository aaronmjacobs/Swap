#include "Scene/Rendering/ForwardSceneRenderer.h"

#include "Core/Assert.h"
#include "Graphics/DrawingContext.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Texture.h"
#include "Math/MathUtils.h"
#include "Platform/IOUtils.h"
#include "Resources/ResourceManager.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/ModelComponent.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

ForwardSceneRenderer::ForwardSceneRenderer(int numSamples, const SPtr<ResourceManager>& inResourceManager)
   : SceneRenderer(inResourceManager, false)
{
   Viewport viewport = GraphicsContext::current().getDefaultViewport();

   {
      static const std::array<Tex::InternalFormat, 2> kColorAttachmentFormats =
      {
         // Color
         Tex::InternalFormat::RGBA16F,

         // Normal
         Tex::InternalFormat::RGB32F
      };

      Fb::Specification specification;
      specification.width = viewport.width;
      specification.height = viewport.height;
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

   setPrePassDepthAttachment(depthStencilTexture);
   setSSAOTextures(depthStencilTexture, nullptr, normalTexture);

   setTranslucencyPassAttachments(depthStencilTexture, colorTexture);

   setTonemapTextures(colorTexture, getBloomPassFramebuffer().getColorAttachment(0));
}

void ForwardSceneRenderer::renderScene(const Scene& scene)
{
   SceneRenderInfo sceneRenderInfo;
   if (!calcSceneRenderInfo(scene, sceneRenderInfo))
   {
      return;
   }

   populateViewUniforms(sceneRenderInfo.perspectiveInfo);

   renderPrePass(sceneRenderInfo);
   renderNormalPass(sceneRenderInfo);
   renderSSAOPass(sceneRenderInfo);
   renderMainPass(sceneRenderInfo);
   renderTranslucencyPass(sceneRenderInfo);
   renderPostProcessPasses(sceneRenderInfo);
}

void ForwardSceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   SceneRenderer::onFramebufferSizeChanged(newWidth, newHeight);

   Viewport viewport = GraphicsContext::current().getDefaultViewport();

   depthStencilTexture->updateResolution(viewport.width, viewport.height);
   colorTexture->updateResolution(viewport.width, viewport.height);
   normalTexture->updateResolution(viewport.width, viewport.height);
}

void ForwardSceneRenderer::renderNormalPass(const SceneRenderInfo& sceneRenderInfo)
{
   normalPassFramebuffer.bind();

   glClear(GL_COLOR_BUFFER_BIT);

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
         if (visible && material.getBlendMode() == BlendMode::Opaque)
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

   glClear(GL_COLOR_BUFFER_BIT);

   populateForwardUniforms(sceneRenderInfo);

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
         if (visible && material.getBlendMode() == BlendMode::Opaque)
         {
            SPtr<ShaderProgram>& forwardProgramPermutation = selectForwardPermutation(material);

            forwardProgramPermutation->setUniformValue(UniformNames::kModelMatrix, modelMatrix);
            forwardProgramPermutation->setUniformValue(UniformNames::kNormalMatrix, normalMatrix, false);

            DrawingContext context(forwardProgramPermutation.get());
            getForwardMaterial().apply(context);
            material.apply(context);
            section.draw(context);
         }
      }
   }
}

void ForwardSceneRenderer::renderPostProcessPasses(const SceneRenderInfo& sceneRenderInfo)
{
   renderBloomPass(sceneRenderInfo, mainPassFramebuffer, 0);

   Framebuffer::bindDefault();
   renderTonemapPass(sceneRenderInfo);
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
      normalProgramPermutations[i]->bindUniformBuffer(getViewUniformBuffer());
   }
}

SPtr<ShaderProgram>& ForwardSceneRenderer::selectNormalPermutation(const Material& material)
{
   int index = material.hasCommonParameter(CommonMaterialParameter::NormalTexture) * 0b001;

   return normalProgramPermutations[index];
}
