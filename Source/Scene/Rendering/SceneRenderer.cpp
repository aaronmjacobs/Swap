#include "Scene/Rendering/SceneRenderer.h"

#include "Core/Assert.h"
#include "Graphics/DrawingContext.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Texture.h"
#include "Math/MathUtils.h"
#include "Platform/IOUtils.h"
#include "Resources/ResourceManager.h"
#include "Scene/Components/CameraComponent.h"
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

   void setScreenMeshData(Mesh& screenMesh)
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

      screenMesh.setData(screenMeshData);
   }
}

SceneRenderer::SceneRenderer(int initialWidth, int initialHeight, const SPtr<ResourceManager>& inResourceManager, bool hasPositionBuffer)
   : width(glm::max(initialWidth, 1))
   , height(glm::max(initialHeight, 1))
   , nearPlaneDistance(0.01f)
   , farPlaneDistance(1000.0f)
   , resourceManager(inResourceManager)
   , ssaoBuffer(getSSAOBufferSpecification(getWidth(), getHeight()))
   , ssaoBlurBuffer(getSSAOBufferSpecification(getWidth(), getHeight()))
{
   ASSERT(initialWidth > 0 && initialHeight > 0, "Invalid framebuffer size");
   ASSERT(resourceManager);

   glViewport(0, 0, width, height);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   setScreenMeshData(screenMesh);

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

void SceneRenderer::renderSSAOPass(const PerspectiveInfo& perspectiveInfo)
{
   ssaoBuffer.bind();

   glDisable(GL_DEPTH_TEST);

   glm::vec4 viewport(getWidth(), getHeight(), 1.0f / getWidth(), 1.0f / getHeight());
   ssaoProgram->setUniformValue(UniformNames::kViewport, viewport);
   ssaoProgram->setUniformValue(UniformNames::kProjectionMatrix, perspectiveInfo.projectionMatrix);
   ssaoProgram->setUniformValue(UniformNames::kViewMatrix, perspectiveInfo.viewMatrix);
   ssaoProgram->setUniformValue("uInverseProjectionMatrix", glm::inverse(perspectiveInfo.projectionMatrix), false);
   ssaoProgram->setUniformValue("uInverseViewMatrix", glm::inverse(perspectiveInfo.viewMatrix), false);

   DrawingContext ssaoContext(ssaoProgram.get());
   ssaoMaterial.apply(ssaoContext);
   ssaoProgram->commit();
   getScreenMesh().draw();

   ssaoBlurBuffer.bind();

   DrawingContext blurContext(ssaoBlurProgram.get());
   ssaoBlurMaterial.apply(blurContext);
   ssaoBlurProgram->commit();
   getScreenMesh().draw();
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
