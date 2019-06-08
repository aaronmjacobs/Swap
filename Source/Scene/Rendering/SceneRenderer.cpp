#include "Scene/Rendering/SceneRenderer.h"

#include "Core/Assert.h"
#include "Graphics/DrawingContext.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Texture.h"
#include "Math/MathUtils.h"
#include "Platform/IOUtils.h"
#include "Resources/ResourceManager.h"
#include "Scene/Components/CameraComponent.h"
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
   const char* kProjectionMatrix = "uProjectionMatrix";
   const char* kViewMatrix = "uViewMatrix";
   const char* kModelMatrix = "uModelMatrix";
   const char* kNormalMatrix = "uNormalMatrix";

   const char* kCameraPos = "uCameraPos";
   const char* kViewport = "uViewport";
}

namespace
{
   Fb::Specification getSSAOBufferSpecification(int width, int height)
   {
      static const std::array<Tex::InternalFormat, 1> kColorAttachmentFormats =
      {
         Tex::InternalFormat::R8
      };

      Fb::Specification specification;

      specification.width = width;
      specification.height = height;
      specification.depthStencilType = Fb::DepthStencilType::None;
      specification.colorAttachmentFormats = kColorAttachmentFormats;

      return specification;
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
}

SceneRenderer::SceneRenderer(int initialWidth, int initialHeight, const SPtr<ResourceManager>& inResourceManager, bool hasPositionBuffer)
   : width(glm::max(initialWidth, 1))
   , height(glm::max(initialHeight, 1))
   , nearPlaneDistance(0.01f)
   , farPlaneDistance(1000.0f)
   , resourceManager(inResourceManager)
   , screenMesh(generateScreenMesh())
   , ssaoBuffer(getSSAOBufferSpecification(getWidth(), getHeight()))
   , ssaoBlurBuffer(getSSAOBufferSpecification(getWidth(), getHeight()))
{
   ASSERT(initialWidth > 0 && initialHeight > 0, "Invalid framebuffer size");
   ASSERT(resourceManager);

   glViewport(0, 0, width, height);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

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
      ssaoBlurMaterial.setParameter("uAmbientOcclusion", ssaoBuffer.getColorAttachments()[0]);
   }
}

void SceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   ASSERT(newWidth > 0 && newHeight > 0, "Invalid framebuffer size");

   width = glm::max(newWidth, 1);
   height = glm::max(newHeight, 1);

   glViewport(0, 0, width, height);

   ssaoBuffer.updateResolution(width, height);
   ssaoBlurBuffer.updateResolution(width, height);
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

   return true;
}

void SceneRenderer::renderSSAOPass(const SceneRenderInfo& sceneRenderInfo)
{
   ssaoBuffer.bind();

   glDisable(GL_DEPTH_TEST);

   glm::vec4 viewport(getWidth(), getHeight(), 1.0f / getWidth(), 1.0f / getHeight());
   ssaoProgram->setUniformValue(UniformNames::kViewport, viewport);
   ssaoProgram->setUniformValue(UniformNames::kProjectionMatrix, sceneRenderInfo.perspectiveInfo.projectionMatrix);
   ssaoProgram->setUniformValue(UniformNames::kViewMatrix, sceneRenderInfo.perspectiveInfo.viewMatrix);
   ssaoProgram->setUniformValue("uInverseProjectionMatrix", glm::inverse(sceneRenderInfo.perspectiveInfo.projectionMatrix), false);
   ssaoProgram->setUniformValue("uInverseViewMatrix", glm::inverse(sceneRenderInfo.perspectiveInfo.viewMatrix), false);

   DrawingContext ssaoContext(ssaoProgram.get());
   ssaoMaterial.apply(ssaoContext);
   getScreenMesh().draw(ssaoContext);

   ssaoBlurBuffer.bind();

   DrawingContext blurContext(ssaoBlurProgram.get());
   ssaoBlurMaterial.apply(blurContext);
   getScreenMesh().draw(blurContext);
}

void SceneRenderer::setSSAOTextures(const SPtr<Texture>& depthTexture, const SPtr<Texture>& positionTexture, const SPtr<Texture>& normalTexture)
{
   ssaoMaterial.setParameter("uDepth", depthTexture);
   ssaoMaterial.setParameter("uPosition", positionTexture);
   ssaoMaterial.setParameter("uNormal", normalTexture);
}

const SPtr<Texture>& SceneRenderer::getSSAOBlurTexture() const
{
   ASSERT(ssaoBlurBuffer.getColorAttachments().size() == 1);

   return ssaoBlurBuffer.getColorAttachments()[0];
}
