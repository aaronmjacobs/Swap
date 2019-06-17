#include "Scene/Rendering/SceneRenderer.h"

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
#include <glm/gtx/compatibility.hpp>

#include <array>
#include <random>

namespace UniformNames
{
   const char* kModelMatrix = "uModelMatrix";
   const char* kNormalMatrix = "uNormalMatrix";
}

namespace
{
   using ViewUniforms = std::tuple<
      glm::mat4, // uWorldToView
      glm::mat4, // uViewToWorld

      glm::mat4, // uViewToClip
      glm::mat4, // uClipToView

      glm::mat4, // uWorldToClip
      glm::mat4, // uClipToWorld

      glm::vec4, // uViewport

      glm::vec3  // uCameraPosition
   >;

   ViewUniforms calcViewUniforms(const PerspectiveInfo& perspectiveInfo)
   {
      ViewUniforms viewUniforms;

      std::get<0>(viewUniforms) = perspectiveInfo.viewMatrix;
      std::get<1>(viewUniforms) = glm::inverse(std::get<0>(viewUniforms));

      std::get<2>(viewUniforms) = perspectiveInfo.projectionMatrix;
      std::get<3>(viewUniforms) = glm::inverse(std::get<2>(viewUniforms));

      std::get<4>(viewUniforms) = perspectiveInfo.projectionMatrix * perspectiveInfo.viewMatrix;
      std::get<5>(viewUniforms) = glm::inverse(std::get<4>(viewUniforms));

      std::get<6>(viewUniforms) = perspectiveInfo.viewport;

      std::get<7>(viewUniforms) = perspectiveInfo.cameraPosition;

      return viewUniforms;
   }

   Mesh generateScreenMesh()
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

      MeshSection meshSection;
      meshSection.setData(screenMeshData);

      std::vector<MeshSection> sections;
      sections.push_back(std::move(meshSection));
      return Mesh(std::move(sections));
   }

   std::array<glm::vec4, 6> computeFrustumPlanes(const glm::mat4& worldToClip)
   {
      std::array<glm::vec4, 6> frustumPlanes;

      for (int i = 0; i < frustumPlanes.size(); ++i)
      {
         glm::vec4& frustumPlane = frustumPlanes[i];

         int row = i / 2;
         int sign = (i % 2) == 0 ? 1 : -1;

         frustumPlane.x = worldToClip[0][3] + sign * worldToClip[0][row];
         frustumPlane.y = worldToClip[1][3] + sign * worldToClip[1][row];
         frustumPlane.z = worldToClip[2][3] + sign * worldToClip[2][row];
         frustumPlane.w = worldToClip[3][3] + sign * worldToClip[3][row];

         frustumPlane /= glm::length(glm::vec3(frustumPlane.x, frustumPlane.y, frustumPlane.z));
      }

      return frustumPlanes;
   }

   float signedPlaneDist(const glm::vec3& point, const glm::vec4& plane)
   {
      return plane.x * point.x + plane.y * point.y + plane.z * point.z + plane.w;
   }

   bool outside(const std::array<glm::vec3, 8>& points, const glm::vec4& plane)
   {
      for (const glm::vec3& point : points)
      {
         if (signedPlaneDist(point, plane) >= 0.0f)
         {
            return false;
         }
      }

      return true;
   }

   bool frustumCull(const Bounds& bounds, const std::array<glm::vec4, 6>& frustumPlanes)
   {
      // First check if the bounding sphere is completely outside of any of the planes
      for (const glm::vec4& plane : frustumPlanes)
      {
         if (signedPlaneDist(bounds.center, plane) < -bounds.radius)
         {
            return true;
         }
      }

      // Next, check the bounding box
      glm::vec3 min = bounds.getMin();
      glm::vec3 max = bounds.getMax();
      std::array<glm::vec3, 8> corners =
      {
         glm::vec3(min.x, min.y, min.z),
         glm::vec3(min.x, min.y, max.z),
         glm::vec3(min.x, max.y, min.z),
         glm::vec3(min.x, max.y, max.z),
         glm::vec3(max.x, min.y, min.z),
         glm::vec3(max.x, min.y, max.z),
         glm::vec3(max.x, max.y, min.z),
         glm::vec3(max.x, max.y, max.z)
      };
      for (const glm::vec4& plane : frustumPlanes)
      {
         if (outside(corners, plane))
         {
            return true;
         }
      }

      return false;
   }

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

SceneRenderer::SceneRenderer(int initialWidth, int initialHeight, const SPtr<ResourceManager>& inResourceManager, bool hasPositionBuffer)
   : width(glm::max(initialWidth, 1))
   , height(glm::max(initialHeight, 1))
   , nearPlaneDistance(0.01f)
   , farPlaneDistance(1000.0f)
   , resourceManager(inResourceManager)
   , screenMesh(generateScreenMesh())
   , viewUniformBuffer("View")
{
   ASSERT(initialWidth > 0 && initialHeight > 0, "Invalid framebuffer size");
   ASSERT(resourceManager);

   glViewport(0, 0, width, height);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   {
      ViewUniforms viewUniforms;
      viewUniformBuffer.setData(viewUniforms);
      viewUniformBuffer.bindTo(0);
   }

   {
      std::vector<ShaderSpecification> depthShaderSpecifications;
      depthShaderSpecifications.resize(2);
      depthShaderSpecifications[0].type = ShaderType::Vertex;
      depthShaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("DepthOnly.vert", depthShaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("DepthOnly.frag", depthShaderSpecifications[1].path);
      depthOnlyProgram = getResourceManager().loadShaderProgram(depthShaderSpecifications);
      depthOnlyProgram->bindUniformBuffer(viewUniformBuffer);
   }

   {
      static const std::array<Tex::InternalFormat, 2> kColorAttachmentFormats =
      {
         // SSAO
         Tex::InternalFormat::R8,

         // SSAO blur
         Tex::InternalFormat::R8
      };

      Fb::Specification specification;
      specification.width = getWidth();
      specification.height = getHeight();
      specification.depthStencilType = Fb::DepthStencilType::None;
      specification.colorAttachmentFormats = kColorAttachmentFormats;

      Fb::Attachments attachments = Fb::generateAttachments(specification);
      ASSERT(attachments.colorAttachments.size() == kColorAttachmentFormats.size());

      ssaoTexture = attachments.colorAttachments[0];
      ssaoBlurTexture = attachments.colorAttachments[1];

      Fb::Attachments ssaoAttachments;
      ssaoAttachments.colorAttachments.push_back(ssaoTexture);
      ssaoBuffer.setAttachments(std::move(ssaoAttachments));

      Fb::Attachments ssaoBlurAttachments;
      ssaoBlurAttachments.colorAttachments.push_back(ssaoBlurTexture);
      ssaoBlurBuffer.setAttachments(std::move(ssaoBlurAttachments));
   }

   {
      std::uniform_real_distribution<GLfloat> distribution(0.0f, 1.0f);
      std::default_random_engine generator;

      std::array<glm::vec3, 16> ssaoNoise;
      for (glm::vec3& noiseValue : ssaoNoise)
      {
         noiseValue = glm::vec3(distribution(generator) * 2.0f - 1.0f, distribution(generator) * 2.0f - 1.0f, 0.0f);
      }

      Tex::Specification textureSpecification;
      textureSpecification.internalFormat = Tex::InternalFormat::RGB16F;
      textureSpecification.width = 4;
      textureSpecification.height = 4;
      textureSpecification.providedDataType = Tex::ProvidedDataType::Float;
      textureSpecification.providedData = ssaoNoise.data();
      ssaoNoiseTexture = std::make_shared<Texture>(textureSpecification);
      ssaoNoiseTexture->setParam(Tex::IntParam::TextureWrapS, static_cast<GLint>(Tex::Wrap::Repeat));
      ssaoNoiseTexture->setParam(Tex::IntParam::TextureWrapT, static_cast<GLint>(Tex::Wrap::Repeat));
      ssaoNoiseTexture->setParam(Tex::IntParam::TextureMinFilter, static_cast<GLint>(Tex::MinFilter::Nearest));
      ssaoNoiseTexture->setParam(Tex::IntParam::TextureMagFilter, static_cast<GLint>(Tex::MinFilter::Nearest));

      std::vector<ShaderSpecification> shaderSpecifications;
      shaderSpecifications.resize(2);
      shaderSpecifications[0].type = ShaderType::Vertex;
      shaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("Screen.vert", shaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("SSAO.frag", shaderSpecifications[1].path);

      shaderSpecifications[1].definitions["WITH_POSITION_BUFFER"] = hasPositionBuffer ? "1" : "0";

      ssaoProgram = resourceManager->loadShaderProgram(shaderSpecifications);
      ssaoProgram->bindUniformBuffer(viewUniformBuffer);

      {
         ssaoMaterial.setParameter("uNoise", ssaoNoiseTexture);
         ssaoMaterial.setParameter("uDepth", ssaoNoiseTexture);
         ssaoMaterial.setParameter("uPosition", ssaoNoiseTexture);
         ssaoMaterial.setParameter("uNormal", ssaoNoiseTexture);

         std::uniform_real_distribution<GLfloat> distribution(0.0f, 1.0f);
         std::default_random_engine generator;
         for (unsigned int i = 0; i < 64; ++i)
         {
            glm::vec3 sample(distribution(generator) * 2.0f - 1.0f, distribution(generator) * 2.0f - 1.0f, distribution(generator));
            sample = glm::normalize(sample);
            sample *= distribution(generator);
            float scale = i / 64.0f;

            // scale samples s.t. they're more aligned to center of kernel
            scale = glm::lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;

            ssaoMaterial.setParameter("uSamples[" + std::to_string(i) + "]", sample);
         }
      }

      IOUtils::getAbsoluteResourcePath("SSAOBlur.frag", shaderSpecifications[1].path);
      ssaoBlurProgram = resourceManager->loadShaderProgram(shaderSpecifications);
      ssaoBlurMaterial.setParameter("uAmbientOcclusion", ssaoTexture);
   }

   {
      forwardMaterial.setParameter("uAmbientOcclusion", ssaoBlurTexture);

      loadForwardProgramPermutations();
   }
}

void SceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   ASSERT(newWidth > 0 && newHeight > 0, "Invalid framebuffer size");

   width = glm::max(newWidth, 1);
   height = glm::max(newHeight, 1);

   glViewport(0, 0, width, height);

   ssaoTexture->updateResolution(width, height);
   ssaoBlurTexture->updateResolution(width, height);
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

bool SceneRenderer::calcSceneRenderInfo(const Scene& scene, SceneRenderInfo& sceneRenderInfo) const
{
   if (!getPerspectiveInfo(scene, sceneRenderInfo.perspectiveInfo))
   {
      return false;
   }

   glm::mat4 worldToClip = sceneRenderInfo.perspectiveInfo.projectionMatrix * sceneRenderInfo.perspectiveInfo.viewMatrix;
   std::array<glm::vec4, 6> frustumPlanes = computeFrustumPlanes(worldToClip);

   for (const ModelComponent* modelComponent : scene.getModelComponents())
   {
      ASSERT(modelComponent);

      bool anySectionVisible = false;

      ModelRenderInfo modelRenderInfo;
      modelRenderInfo.model = &modelComponent->getModel();
      modelRenderInfo.localToWorld = modelComponent->getAbsoluteTransform();

      const Model& model = modelComponent->getModel();
      if (const SPtr<Mesh>& mesh = model.getMesh())
      {
         for (std::size_t i = 0; i < model.getNumMeshSections(); ++i)
         {
            const MeshSection& section = model.getMeshSection(i);
            const Bounds& localBounds = section.getBounds();

            Bounds worldBounds;
            worldBounds.center = modelRenderInfo.localToWorld.transformPosition(localBounds.center);
            worldBounds.extent = modelRenderInfo.localToWorld.transformVector(localBounds.extent);
            worldBounds.radius = glm::max(glm::max(modelRenderInfo.localToWorld.scale.x, modelRenderInfo.localToWorld.scale.y), modelRenderInfo.localToWorld.scale.z) * localBounds.radius;

            bool visible = !frustumCull(worldBounds, frustumPlanes);

            if (model.getNumMeshSections() > 1)
            {
               modelRenderInfo.visibilityMask.push_back(visible);
            }

            anySectionVisible |= visible;
         }
      }

      if (anySectionVisible)
      {
         sceneRenderInfo.modelRenderInfo.push_back(modelRenderInfo);
      }
   }

   // Sort back-to-front
   std::sort(sceneRenderInfo.modelRenderInfo.begin(), sceneRenderInfo.modelRenderInfo.end(),
      [cameraPosition = sceneRenderInfo.perspectiveInfo.cameraPosition](const ModelRenderInfo& first, const ModelRenderInfo& second)
   {
      return glm::distance2(first.localToWorld.position, cameraPosition) > glm::distance2(second.localToWorld.position, cameraPosition);
   });

   // Everything the light touches is our kingdom
   sceneRenderInfo.directionalLights.reserve(scene.getDirectionalLightComponents().size());
   for (DirectionalLightComponent* directionalLight : scene.getDirectionalLightComponents())
   {
      ASSERT(directionalLight);

      sceneRenderInfo.directionalLights.push_back(directionalLight);
   }

   for (PointLightComponent* pointLight : scene.getPointLightComponents())
   {
      ASSERT(pointLight);

      Transform localToWorld = pointLight->getAbsoluteTransform();

      Bounds worldBounds;
      worldBounds.center = localToWorld.position;
      worldBounds.radius = pointLight->getScaledRadius();
      worldBounds.extent = glm::vec3(worldBounds.radius);

      bool visible = !frustumCull(worldBounds, frustumPlanes);
      if (visible)
      {
         sceneRenderInfo.pointLights.push_back(pointLight);
      }
   }

   sceneRenderInfo.spotLights.reserve(scene.getSpotLightComponents().size());
   for (SpotLightComponent* spotLight : scene.getSpotLightComponents())
   {
      ASSERT(spotLight);

      Transform localToWorld = spotLight->getAbsoluteTransform();

      // Set the radius to half that of the light, and center the bounds on the center of the light (not the origin)
      Bounds worldBounds;
      worldBounds.radius = spotLight->getScaledRadius() * 0.5f;
      worldBounds.extent = glm::vec3(worldBounds.radius);
      worldBounds.center = localToWorld.position + localToWorld.orientation * MathUtils::kForwardVector * worldBounds.radius;

      bool visible = !frustumCull(worldBounds, frustumPlanes);
      if (visible)
      {
         sceneRenderInfo.spotLights.push_back(spotLight);
      }
   }

   return true;
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

   perspectiveInfo.viewport = glm::vec4(getWidth(), getHeight(), 1.0f / getWidth(), 1.0f / getHeight());

   return true;
}

void SceneRenderer::populateViewUniforms(const PerspectiveInfo& perspectiveInfo)
{
   viewUniformBuffer.updateData(calcViewUniforms(perspectiveInfo));
}

void SceneRenderer::renderPrePass(const SceneRenderInfo& sceneRenderInfo)
{
   prePassFramebuffer.bind();

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   glDepthMask(GL_TRUE);

   glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   for (const ModelRenderInfo& modelRenderInfo : sceneRenderInfo.modelRenderInfo)
   {
      ASSERT(modelRenderInfo.model);

      glm::mat4 modelMatrix = modelRenderInfo.localToWorld.toMatrix();
      depthOnlyProgram->setUniformValue(UniformNames::kModelMatrix, modelMatrix);

      for (std::size_t i = 0; i < modelRenderInfo.model->getNumMeshSections(); ++i)
      {
         const MeshSection& section = modelRenderInfo.model->getMeshSection(i);
         const Material& material = modelRenderInfo.model->getMaterial(i);

         bool visible = i >= modelRenderInfo.visibilityMask.size() || modelRenderInfo.visibilityMask[i];
         if (visible && material.getBlendMode() == BlendMode::Opaque)
         {
            DrawingContext context(depthOnlyProgram.get());
            section.draw(context);
         }
      }
   }

   glDepthMask(GL_FALSE);
   glDepthFunc(GL_LEQUAL);
}

void SceneRenderer::setPrePassDepthAttachment(const SPtr<Texture>& depthAttachment)
{
   Fb::Attachments attachments;
   attachments.depthStencilAttachment = depthAttachment;

   prePassFramebuffer.setAttachments(std::move(attachments));
}

void SceneRenderer::renderSSAOPass(const SceneRenderInfo& sceneRenderInfo)
{
   ssaoBuffer.bind();

   glDisable(GL_DEPTH_TEST);

   glm::vec4 viewport(getWidth(), getHeight(), 1.0f / getWidth(), 1.0f / getHeight());
   ssaoProgram->setUniformValue("uInverseProjectionMatrix", glm::inverse(sceneRenderInfo.perspectiveInfo.projectionMatrix), false);
   ssaoProgram->setUniformValue("uInverseViewMatrix", glm::inverse(sceneRenderInfo.perspectiveInfo.viewMatrix), false);

   DrawingContext ssaoContext(ssaoProgram.get());
   ssaoMaterial.apply(ssaoContext);
   getScreenMesh().draw(ssaoContext);

   ssaoBlurBuffer.bind();

   DrawingContext blurContext(ssaoBlurProgram.get());
   ssaoBlurMaterial.apply(blurContext);
   getScreenMesh().draw(blurContext);

   glEnable(GL_DEPTH_TEST);
}

void SceneRenderer::setSSAOTextures(const SPtr<Texture>& depthTexture, const SPtr<Texture>& positionTexture, const SPtr<Texture>& normalTexture)
{
   ssaoMaterial.setParameter("uDepth", depthTexture);
   ssaoMaterial.setParameter("uPosition", positionTexture);
   ssaoMaterial.setParameter("uNormal", normalTexture);
}

void SceneRenderer::renderTranslucencyPass(const SceneRenderInfo& sceneRenderInfo)
{
   translucencyPassFramebuffer.bind();

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
         if (visible && material.getBlendMode() == BlendMode::Translucent)
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

   glDisable(GL_BLEND);
}

void SceneRenderer::setTranslucencyPassAttachments(const SPtr<Texture>& depthAttachment, const SPtr<Texture>& colorAttachment)
{
   Fb::Attachments attachments;
   attachments.depthStencilAttachment = depthAttachment;
   attachments.colorAttachments.push_back(colorAttachment);

   translucencyPassFramebuffer.setAttachments(std::move(attachments));
}

void SceneRenderer::loadForwardProgramPermutations()
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
      forwardProgramPermutations[i]->bindUniformBuffer(viewUniformBuffer);
   }
}

SPtr<ShaderProgram>& SceneRenderer::selectForwardPermutation(const Material& material)
{
   int index = material.hasCommonParameter(CommonMaterialParameter::DiffuseTexture) * 0b001
      + material.hasCommonParameter(CommonMaterialParameter::SpecularTexture) * 0b010
      + material.hasCommonParameter(CommonMaterialParameter::NormalTexture) * 0b100;

   return forwardProgramPermutations[index];
}

void SceneRenderer::populateForwardUniforms(const SceneRenderInfo& sceneRenderInfo)
{
   for (SPtr<ShaderProgram>& forwardProgramPermutation : forwardProgramPermutations)
   {
      populateLightUniforms(sceneRenderInfo, *forwardProgramPermutation);
   }
}
