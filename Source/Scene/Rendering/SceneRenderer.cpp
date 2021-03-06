#include "Scene/Rendering/SceneRenderer.h"

#include "Core/Assert.h"
#include "Graphics/DrawingContext.h"
#include "Graphics/GraphicsContext.h"
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

#include <glad/gl.h>
#include <glm/gtx/compatibility.hpp>

#include <array>
#include <random>

namespace UniformNames
{
   const char* kLocalToWorld = "uLocalToWorld";
   const char* kLocalToNormal = "uLocalToNormal";
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

      glm::vec3  // uCameraPosition
   >;

   ViewUniforms calcViewUniforms(const ViewInfo& viewInfo)
   {
      ViewUniforms viewUniforms;

      std::get<0>(viewUniforms) = viewInfo.getWorldToView();
      std::get<1>(viewUniforms) = glm::inverse(std::get<0>(viewUniforms));

      std::get<2>(viewUniforms) = viewInfo.getViewToClip();
      std::get<3>(viewUniforms) = glm::inverse(std::get<2>(viewUniforms));

      std::get<4>(viewUniforms) = viewInfo.getWorldToClip();
      std::get<5>(viewUniforms) = glm::inverse(std::get<4>(viewUniforms));

      std::get<6>(viewUniforms) = viewInfo.getViewOrigin();

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
      meshSection.setLabel("Screen Mesh");

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

   const float kLightNearPlane = 0.1f;
   const int kMaxDirectionalLights = 2;
   const int kMaxPointLights = 8;
   const int kMaxSpotLights = 8;

   void populateDirectionalLightUniforms(const SceneRenderInfo& sceneRenderInfo, DrawingContext& context, const SPtr<Texture>& dummyShadowMap)
   {
      ASSERT(context.program);
      ASSERT(dummyShadowMap);

      ShaderProgram& program = *context.program;

      for (int directionalLightIndex = 0; directionalLightIndex < kMaxDirectionalLights; ++directionalLightIndex)
      {
         DirectionalLightUniformData uniformData;
         if (directionalLightIndex < sceneRenderInfo.directionalLights.size())
         {
            uniformData = sceneRenderInfo.directionalLights[directionalLightIndex].getUniformData();
         }

         std::string directionalLightStr = "uDirectionalLights[" + std::to_string(directionalLightIndex) + "]";

         program.setUniformValue(directionalLightStr + ".color", uniformData.color);
         program.setUniformValue(directionalLightStr + ".direction", uniformData.direction);

         program.setUniformValue(directionalLightStr + ".castShadows", uniformData.castShadows);
         program.setUniformValue(directionalLightStr + ".worldToShadow", uniformData.worldToShadow);
         program.setUniformValue(directionalLightStr + ".shadowBias", uniformData.shadowBias);

         const SPtr<Texture>& shadowMap = uniformData.shadowMap ? uniformData.shadowMap : dummyShadowMap;
         GLint shadowMapTextureUnit = shadowMap->activateAndBind(context);
         program.setUniformValue(directionalLightStr + ".shadowMap", shadowMapTextureUnit);
      }

      program.setUniformValue("uNumDirectionalLights", static_cast<int>(sceneRenderInfo.directionalLights.size()));
   }

   void populatePointLightUniforms(const SceneRenderInfo& sceneRenderInfo, DrawingContext& context, const SPtr<Texture>& dummyShadowCubeMap)
   {
      ASSERT(context.program);
      ASSERT(dummyShadowCubeMap);

      ShaderProgram& program = *context.program;

      for (int pointLightIndex = 0; pointLightIndex < kMaxPointLights; ++pointLightIndex)
      {
         PointLightUniformData uniformData;
         if (pointLightIndex < sceneRenderInfo.pointLights.size())
         {
            uniformData = sceneRenderInfo.pointLights[pointLightIndex].getUniformData();
         }

         std::string pointLightStr = "uPointLights[" + std::to_string(pointLightIndex) + "]";

         program.setUniformValue(pointLightStr + ".color", uniformData.color);
         program.setUniformValue(pointLightStr + ".position", uniformData.position);
         program.setUniformValue(pointLightStr + ".radius", uniformData.radius);

         program.setUniformValue(pointLightStr + ".castShadows", uniformData.castShadows);
         program.setUniformValue(pointLightStr + ".nearFar", uniformData.nearFar);
         program.setUniformValue(pointLightStr + ".shadowBias", uniformData.shadowBias);

         const SPtr<Texture>& shadowMap = uniformData.shadowMap ? uniformData.shadowMap : dummyShadowCubeMap;
         GLint shadowMapTextureUnit = shadowMap->activateAndBind(context);
         program.setUniformValue(pointLightStr + ".shadowMap", shadowMapTextureUnit);
      }

      program.setUniformValue("uNumPointLights", static_cast<int>(sceneRenderInfo.pointLights.size()));
   }

   void populateSpotLightUniforms(const SceneRenderInfo& sceneRenderInfo, DrawingContext& context, const SPtr<Texture>& dummyShadowMap)
   {
      ASSERT(context.program);
      ASSERT(dummyShadowMap);

      ShaderProgram& program = *context.program;

      for (int spotLightIndex = 0; spotLightIndex < kMaxSpotLights; ++spotLightIndex)
      {
         SpotLightUniformData uniformData;
         if (spotLightIndex < sceneRenderInfo.spotLights.size())
         {
            uniformData = sceneRenderInfo.spotLights[spotLightIndex].getUniformData();
         }

         std::string spotLightStr = "uSpotLights[" + std::to_string(spotLightIndex) + "]";

         program.setUniformValue(spotLightStr + ".color", uniformData.color);
         program.setUniformValue(spotLightStr + ".direction", uniformData.direction);
         program.setUniformValue(spotLightStr + ".position", uniformData.position);
         program.setUniformValue(spotLightStr + ".radius", uniformData.radius);
         program.setUniformValue(spotLightStr + ".beamAngle", uniformData.beamAngle);
         program.setUniformValue(spotLightStr + ".cutoffAngle", uniformData.cutoffAngle);

         program.setUniformValue(spotLightStr + ".castShadows", uniformData.castShadows);
         program.setUniformValue(spotLightStr + ".worldToShadow", uniformData.worldToShadow);
         program.setUniformValue(spotLightStr + ".shadowBias", uniformData.shadowBias);

         const SPtr<Texture>& shadowMap = uniformData.shadowMap ? uniformData.shadowMap : dummyShadowMap;
         GLint shadowMapTextureUnit = shadowMap->activateAndBind(context);
         program.setUniformValue(spotLightStr + ".shadowMap", shadowMapTextureUnit);
      }

      program.setUniformValue("uNumSpotLights", static_cast<int>(sceneRenderInfo.spotLights.size()));
   }

   void populateLightUniforms(const SceneRenderInfo& sceneRenderInfo, DrawingContext& context, const SPtr<Texture>& dummyShadowMap, const SPtr<Texture>& dummyShadowCubeMap)
   {
      populateDirectionalLightUniforms(sceneRenderInfo, context, dummyShadowMap);
      populatePointLightUniforms(sceneRenderInfo, context, dummyShadowCubeMap);
      populateSpotLightUniforms(sceneRenderInfo, context, dummyShadowMap);
   }

   ViewInfo getShadowViewInfo(const DirectionalLightComponent& directionalLight, const CameraComponent& camera)
   {
      const Bounds& clipBounds = directionalLight.getShadowClipBounds();
      glm::vec3 clipBoundsMin = clipBounds.getMin();
      glm::vec3 clipBoundsMax = clipBounds.getMax();
      glm::mat4 viewToClip = glm::ortho(clipBoundsMin.x, clipBoundsMax.x, clipBoundsMin.y, clipBoundsMax.y, clipBoundsMin.z, clipBoundsMax.z);

      Transform lightTransform = directionalLight.getAbsoluteTransform();
      glm::vec3 lightDirection = lightTransform.transformVector(MathUtils::kForwardVector);
      glm::mat4 worldToView = glm::lookAt(lightTransform.position, lightTransform.position + lightDirection, MathUtils::kUpVector);

      ViewInfo viewInfo;
      viewInfo.init(worldToView, viewToClip);
      return viewInfo;
   }

   glm::mat4 getCubeShadowViewToClip(float nearPlane, float farPlane)
   {
      float fovY = glm::radians(90.0f);
      float aspectRatio = 1.0f;

      return glm::perspective(fovY, aspectRatio, nearPlane, farPlane);
   }

   glm::mat4 getCubeShadowWorldToView(const glm::vec3& lightPosition, Fb::CubeFace cubeFace)
   {
      glm::vec3 forward = MathUtils::kForwardVector;
      glm::vec3 up = MathUtils::kUpVector;

      /*
       * major axis
       * direction     target                                sc    tc   ma
       * ----------    ----------------------------------   ---   ---   --
       *  +rx          GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT   -rz   -ry   rx
       *  -rx          GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT   +rz   -ry   rx
       *  +ry          GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT   +rx   +rz   ry
       *  -ry          GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT   +rx   -rz   ry
       *  +rz          GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT   +rx   -ry   rz
       *  -rz          GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT   -rx   -ry   rz
       */
      switch (cubeFace)
      {
      case Fb::CubeFace::Front:
         forward = MathUtils::kForwardVector;
         up = -MathUtils::kUpVector;
         break;
      case Fb::CubeFace::Back:
         forward = -MathUtils::kForwardVector;
         up = -MathUtils::kUpVector;
         break;
      case Fb::CubeFace::Top:
         forward = MathUtils::kUpVector;
         up = -MathUtils::kForwardVector;
         break;
      case Fb::CubeFace::Bottom:
         forward = -MathUtils::kUpVector;
         up = MathUtils::kForwardVector;
         break;
      case Fb::CubeFace::Left:
         forward = -MathUtils::kRightVector;
         up = -MathUtils::kUpVector;
         break;
      case Fb::CubeFace::Right:
         forward = MathUtils::kRightVector;
         up = -MathUtils::kUpVector;
         break;
      default:
         ASSERT(false);
         break;
      }

      return glm::lookAt(lightPosition, lightPosition + forward, up);
   }

   ViewInfo getShadowViewInfo(const SpotLightComponent& spotLight)
   {
      float fovY = glm::radians(spotLight.getCutoffAngle() * 2.0f);
      float aspectRatio = 1.0f;
      float zNear = kLightNearPlane;
      float zFar = spotLight.getScaledRadius();
      glm::mat4 viewToClip = glm::perspective(fovY, aspectRatio, zNear, zFar);

      Transform lightTransform = spotLight.getAbsoluteTransform();
      glm::vec3 viewTarget = lightTransform.transformPosition(MathUtils::kForwardVector);
      glm::mat4 worldToView = glm::lookAt(lightTransform.position, viewTarget, MathUtils::kUpVector);

      ViewInfo viewInfo;
      viewInfo.init(worldToView, viewToClip);
      return viewInfo;
   }

   void prepareShadowMap(Texture& shadowMap)
   {
      shadowMap.bind();

      shadowMap.setParam(Tex::IntParam::TextureCompareFunc, GL_LEQUAL);
      shadowMap.setParam(Tex::IntParam::TextureCompareMode, GL_COMPARE_REF_TO_TEXTURE);
      shadowMap.setParam(Tex::IntParam::TextureMinFilter, static_cast<GLint>(Tex::MinFilter::Linear));
      shadowMap.setParam(Tex::IntParam::TextureMagFilter, static_cast<GLint>(Tex::MagFilter::Linear));

      if (shadowMap.isCubemap())
      {
         shadowMap.setParam(Tex::IntParam::TextureWrapS, static_cast<GLint>(Tex::Wrap::ClampToEdge));
         shadowMap.setParam(Tex::IntParam::TextureWrapT, static_cast<GLint>(Tex::Wrap::ClampToEdge));
         shadowMap.setParam(Tex::IntParam::TextureWrapR, static_cast<GLint>(Tex::Wrap::ClampToEdge));
      }
      else
      {
         shadowMap.setParam(Tex::IntParam::TextureWrapS, static_cast<GLint>(Tex::Wrap::ClampToBorder));
         shadowMap.setParam(Tex::IntParam::TextureWrapT, static_cast<GLint>(Tex::Wrap::ClampToBorder));
         shadowMap.setParam(Tex::FloatArrayParam::TextureBorderColor, glm::vec4(1.0f));
      }
   }
}

DirectionalLightUniformData DirectionalLightRenderInfo::getUniformData() const
{
   ASSERT(component);

   DirectionalLightUniformData uniformData;

   Transform transform = component->getAbsoluteTransform();

   uniformData.color = component->getColor();
   uniformData.direction = transform.rotateVector(MathUtils::kForwardVector);

   uniformData.shadowMap = shadowMapFramebuffer ? shadowMapFramebuffer->getDepthStencilAttachment() : nullptr;
   uniformData.castShadows = uniformData.shadowMap != nullptr;
   uniformData.worldToShadow = shadowViewInfo.getWorldToClip();
   uniformData.shadowBias = component->getShadowBias();

   return uniformData;
}

PointLightUniformData PointLightRenderInfo::getUniformData() const
{
   ASSERT(component);

   PointLightUniformData uniformData;

   Transform transform = component->getAbsoluteTransform();
   float radiusScale = glm::max(transform.scale.x, glm::max(transform.scale.y, transform.scale.z));

   uniformData.color = component->getColor();
   uniformData.position = transform.position;
   uniformData.radius = component->getRadius() * radiusScale;

   uniformData.shadowMap = shadowMapFramebuffer ? shadowMapFramebuffer->getDepthStencilAttachment() : nullptr;
   uniformData.castShadows = uniformData.shadowMap != nullptr;
   uniformData.nearFar = glm::vec2(nearPlane, farPlane);
   uniformData.shadowBias = component->getShadowBias();

   return uniformData;
}

SpotLightUniformData SpotLightRenderInfo::getUniformData() const
{
   ASSERT(component);

   SpotLightUniformData uniformData;

   Transform transform = component->getAbsoluteTransform();
   float radiusScale = glm::max(transform.scale.x, glm::max(transform.scale.y, transform.scale.z));

   uniformData.color = component->getColor();
   uniformData.direction = transform.rotateVector(MathUtils::kForwardVector);
   uniformData.position = transform.position;
   uniformData.radius = component->getRadius() * radiusScale;
   uniformData.beamAngle = glm::radians(component->getBeamAngle());
   uniformData.cutoffAngle = glm::radians(component->getCutoffAngle());

   uniformData.shadowMap = shadowMapFramebuffer ? shadowMapFramebuffer->getDepthStencilAttachment() : nullptr;
   uniformData.castShadows = uniformData.shadowMap != nullptr;
   uniformData.worldToShadow = shadowViewInfo.getWorldToClip();
   uniformData.shadowBias = component->getShadowBias();

   return uniformData;
}

SceneRenderer::SceneRenderer(const SPtr<ResourceManager>& inResourceManager, bool hasPositionBuffer)
   : nearPlaneDistance(0.01f)
   , farPlaneDistance(1000.0f)
   , resourceManager(inResourceManager)
   , screenMesh(generateScreenMesh())
{
   ASSERT(resourceManager);

   {
      shadowMapPool.bindOnResourceCreated([](Framebuffer& shadowMapFramebuffer)
      {
         ASSERT(shadowMapFramebuffer.getAttachments().colorAttachments.size() == 0 && shadowMapFramebuffer.getDepthStencilAttachment() != nullptr);

         const SPtr<Texture>& shadowMap = shadowMapFramebuffer.getDepthStencilAttachment();
         prepareShadowMap(*shadowMap);

         shadowMap->setLabel(shadowMapFramebuffer.getLabel() + " | Depth");
      });
   }

   {
      viewUniformBuffer = std::make_shared<UniformBufferObject>("View");

      ViewUniforms viewUniforms;
      viewUniformBuffer->setData(viewUniforms);
      viewUniformBuffer->bindTo(UniformBufferObjectIndex::View);
      viewUniformBuffer->setLabel("View Uniform Buffer");
   }

   {
      Tex::Specification dummyShadowMapSpec;
      dummyShadowMapSpec.internalFormat = Tex::InternalFormat::Depth24Stencil8;
      dummyShadowMapSpec.width = 1;
      dummyShadowMapSpec.height = 1;
      dummyShadowMapSpec.providedDataFormat = Tex::ProvidedDataFormat::DepthStencil;
      dummyShadowMapSpec.providedDataType = Tex::ProvidedDataType::UnsignedInt_24_8;
      uint32_t data = 0;
      dummyShadowMapSpec.providedData = &data;

      dummyShadowMap = std::make_shared<Texture>(dummyShadowMapSpec);
      dummyShadowMap->setLabel("Dummy Shadow Map");

      prepareShadowMap(*dummyShadowMap);
   }

   {
      Tex::Specification dummyShadowCubeMapSpec;
      dummyShadowCubeMapSpec.target = Tex::Target::TextureCubeMap;
      dummyShadowCubeMapSpec.internalFormat = Tex::InternalFormat::Depth24Stencil8;
      dummyShadowCubeMapSpec.width = 1;
      dummyShadowCubeMapSpec.height = 1;
      dummyShadowCubeMapSpec.providedDataFormat = Tex::ProvidedDataFormat::DepthStencil;
      dummyShadowCubeMapSpec.providedDataType = Tex::ProvidedDataType::UnsignedInt_24_8;
      uint32_t data = 0;
      dummyShadowCubeMapSpec.positiveXData = &data;
      dummyShadowCubeMapSpec.negativeXData = &data;
      dummyShadowCubeMapSpec.positiveYData = &data;
      dummyShadowCubeMapSpec.negativeYData = &data;
      dummyShadowCubeMapSpec.positiveZData = &data;
      dummyShadowCubeMapSpec.negativeZData = &data;

      dummyShadowCubeMap = std::make_shared<Texture>(dummyShadowCubeMapSpec);
      dummyShadowCubeMap->setLabel("Dummy Shadow Cube Map");

      prepareShadowMap(*dummyShadowCubeMap);
   }

   Viewport viewport = GraphicsContext::current().getDefaultViewport();

   {
      std::vector<ShaderSpecification> depthShaderSpecifications;
      depthShaderSpecifications.resize(2);
      depthShaderSpecifications[0].type = ShaderType::Vertex;
      depthShaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("Shaders/DepthOnly.vert", depthShaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("Shaders/DepthOnly.frag", depthShaderSpecifications[1].path);
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
      specification.width = viewport.width;
      specification.height = viewport.height;
      specification.depthStencilType = Fb::DepthStencilType::None;
      specification.colorAttachmentFormats = kColorAttachmentFormats;

      Fb::Attachments attachments = Fb::generateAttachments(specification);
      ASSERT(attachments.colorAttachments.size() == kColorAttachmentFormats.size());

      ssaoUnfilteredTexture = attachments.colorAttachments[0];
      ssaoUnfilteredTexture->setLabel("SSAO Unfiltered");
      ssaoTexture = attachments.colorAttachments[1];
      ssaoTexture->setLabel("SSAO");

      Fb::Attachments ssaoAttachments;
      ssaoAttachments.colorAttachments.push_back(ssaoUnfilteredTexture);
      ssaoBuffer.setAttachments(std::move(ssaoAttachments));
      ssaoBuffer.setLabel("SSAO Framebuffer");

      Fb::Attachments ssaoBlurAttachments;
      ssaoBlurAttachments.colorAttachments.push_back(ssaoTexture);
      ssaoBlurBuffer.setAttachments(std::move(ssaoBlurAttachments));
      ssaoBlurBuffer.setLabel("SSAO Blur Framebuffer");
   }

   {
      static const int kNumSamples = 16;

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
      ssaoNoiseTexture->setLabel("SSAO Noise");

      std::vector<ShaderSpecification> shaderSpecifications;
      shaderSpecifications.resize(2);
      shaderSpecifications[0].type = ShaderType::Vertex;
      shaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("Shaders/Screen.vert", shaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("Shaders/SSAO.frag", shaderSpecifications[1].path);

      shaderSpecifications[1].definitions["WITH_POSITION_BUFFER"] = hasPositionBuffer ? "1" : "0";
      shaderSpecifications[1].definitions["SSAO_NUM_SAMPLES"] = std::to_string(kNumSamples);

      ssaoProgram = resourceManager->loadShaderProgram(shaderSpecifications);
      ssaoProgram->bindUniformBuffer(GraphicsContext::current().getFramebufferUniformBuffer());
      ssaoProgram->bindUniformBuffer(viewUniformBuffer);

      {
         ssaoMaterial.setParameter("uNoise", ssaoNoiseTexture);
         ssaoMaterial.setParameter("uDepth", ssaoNoiseTexture);
         ssaoMaterial.setParameter("uPosition", ssaoNoiseTexture);
         ssaoMaterial.setParameter("uNormal", ssaoNoiseTexture);

         std::uniform_real_distribution<GLfloat> distribution(0.0f, 1.0f);
         std::default_random_engine generator;
         for (unsigned int i = 0; i < kNumSamples; ++i)
         {
            glm::vec3 sample(distribution(generator) * 2.0f - 1.0f, distribution(generator) * 2.0f - 1.0f, distribution(generator));
            sample = glm::normalize(sample);
            sample *= distribution(generator);
            float scale = i / static_cast<float>(kNumSamples);

            // scale samples s.t. they're more aligned to center of kernel
            scale = glm::lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;

            ssaoMaterial.setParameter("uSamples[" + std::to_string(i) + "]", sample);
         }
      }

      IOUtils::getAbsoluteResourcePath("Shaders/SSAOBlur.frag", shaderSpecifications[1].path);
      ssaoBlurProgram = resourceManager->loadShaderProgram(shaderSpecifications);
      ssaoBlurMaterial.setParameter("uAmbientOcclusion", ssaoUnfilteredTexture);
   }

   {
      forwardMaterial.setParameter("uAmbientOcclusion", ssaoTexture);

      loadForwardProgramPermutations();
   }

   {
      std::vector<ShaderSpecification> shaderSpecifications;
      shaderSpecifications.resize(2);
      shaderSpecifications[0].type = ShaderType::Vertex;
      shaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("Shaders/Screen.vert", shaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("Shaders/Threshold.frag", shaderSpecifications[1].path);

      thresholdProgram = getResourceManager().loadShaderProgram(shaderSpecifications);
   }

   {
      std::vector<ShaderSpecification> shaderSpecifications;
      shaderSpecifications.resize(2);
      shaderSpecifications[0].type = ShaderType::Vertex;
      shaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("Shaders/Screen.vert", shaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("Shaders/Blur.frag", shaderSpecifications[1].path);

      shaderSpecifications[1].definitions["HORIZONTAL"] = "1";
      horizontalBlurProgram = getResourceManager().loadShaderProgram(shaderSpecifications);

      shaderSpecifications[1].definitions["HORIZONTAL"] = "0";
      verticalBlurProgram = getResourceManager().loadShaderProgram(shaderSpecifications);
   }

   {
      std::array<Tex::InternalFormat, 1> kColorFormats = { Tex::InternalFormat::RGB16F };

      Fb::Specification specification;
      specification.width = viewport.width / 4;
      specification.height = viewport.height / 4;
      specification.depthStencilType = Fb::DepthStencilType::None;
      specification.colorAttachmentFormats = kColorFormats;

      downsampledColorFramebuffer.setAttachments(Fb::generateAttachments(specification));
      downsampledColorFramebuffer.setLabel("Downsample Framebuffer");
      downsampledColorFramebuffer.getColorAttachment(0)->setLabel("Downsample");
      bloomPassFramebuffer.setAttachments(Fb::generateAttachments(specification));
      bloomPassFramebuffer.setLabel("Bloom Framebuffer");
      bloomPassFramebuffer.getColorAttachment(0)->setLabel("Bloom");

      blurFramebuffer.setAttachments(Fb::generateAttachments(specification));
      blurFramebuffer.setLabel("Blur Framebuffer");
      blurFramebuffer.getColorAttachment(0)->setLabel("Blur");
   }

   {
      std::vector<ShaderSpecification> shaderSpecifications;
      shaderSpecifications.resize(2);
      shaderSpecifications[0].type = ShaderType::Vertex;
      shaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("Shaders/Screen.vert", shaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("Shaders/Tonemap.frag", shaderSpecifications[1].path);

      tonemapProgram = getResourceManager().loadShaderProgram(shaderSpecifications);
   }
}

void SceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   ASSERT(newWidth > 0 && newHeight > 0, "Invalid framebuffer size");

   Viewport newViewport(glm::max(newWidth, 1), glm::max(newHeight, 1));
   GraphicsContext::current().setDefaultViewport(newViewport);

   ssaoUnfilteredTexture->updateResolution(newViewport.width, newViewport.height);
   ssaoTexture->updateResolution(newViewport.width, newViewport.height);
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

bool SceneRenderer::getViewInfo(const Scene& scene, ViewInfo& viewInfo) const
{
   const CameraComponent* activeCamera = scene.getActiveCameraComponent();
   if (!activeCamera)
   {
      return false;
   }

   Transform cameraTransform = activeCamera->getAbsoluteTransform();
   glm::vec3 viewTarget = cameraTransform.transformPosition(MathUtils::kForwardVector);
   glm::mat4 worldToView = glm::lookAt(cameraTransform.position, viewTarget, MathUtils::kUpVector);

   Viewport viewport = GraphicsContext::current().getDefaultViewport();
   float fovY = glm::radians(activeCamera->getFieldOfView());
   float aspectRatio = static_cast<float>(viewport.width) / viewport.height;
   float zNear = getNearPlaneDistance();
   float zFar = getFarPlaneDistance();
   glm::mat4 viewToClip = glm::perspective(fovY, aspectRatio, zNear, zFar);

   viewInfo.init(worldToView, viewToClip);

   return true;
}

SceneRenderInfo SceneRenderer::calcSceneRenderInfo(const Scene& scene, const ViewInfo& viewInfo, bool includeLights) const
{
   SceneRenderInfo sceneRenderInfo;
   sceneRenderInfo.viewInfo = viewInfo;

   std::array<glm::vec4, 6> frustumPlanes = computeFrustumPlanes(viewInfo.getWorldToClip());

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
      [cameraPosition = viewInfo.getViewOrigin()](const ModelRenderInfo& first, const ModelRenderInfo& second)
   {
      return glm::distance2(first.localToWorld.position, cameraPosition) > glm::distance2(second.localToWorld.position, cameraPosition);
   });

   if (includeLights)
   {
      // Everything the light touches is our kingdom
      sceneRenderInfo.directionalLights.reserve(scene.getDirectionalLightComponents().size());
      for (DirectionalLightComponent* directionalLight : scene.getDirectionalLightComponents())
      {
         ASSERT(directionalLight);

         DirectionalLightRenderInfo directionalLightRenderInfo;
         directionalLightRenderInfo.component = directionalLight;
         sceneRenderInfo.directionalLights.push_back(directionalLightRenderInfo);
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
            PointLightRenderInfo pointLightRenderInfo;
            pointLightRenderInfo.component = pointLight;
            sceneRenderInfo.pointLights.push_back(pointLightRenderInfo);
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
         worldBounds.center = localToWorld.position + localToWorld.rotateVector(MathUtils::kForwardVector) * worldBounds.radius;

         bool visible = !frustumCull(worldBounds, frustumPlanes);
         if (visible)
         {
            SpotLightRenderInfo spotLightRenderInfo;
            spotLightRenderInfo.component = spotLight;
            sceneRenderInfo.spotLights.push_back(spotLightRenderInfo);
         }
      }
   }

   return sceneRenderInfo;
}

void SceneRenderer::setView(const ViewInfo& viewInfo)
{
   viewUniformBuffer->updateData(calcViewUniforms(viewInfo));
}

void SceneRenderer::renderDepthPass(const SceneRenderInfo& sceneRenderInfo, Framebuffer& framebuffer)
{
   framebuffer.bind();

   RasterizerState rasterizerState;
   RasterizerStateScope rasterizerStateScope(rasterizerState);

   glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   for (const ModelRenderInfo& modelRenderInfo : sceneRenderInfo.modelRenderInfo)
   {
      ASSERT(modelRenderInfo.model);

      glm::mat4 localToWorld = modelRenderInfo.localToWorld.toMatrix();
      depthOnlyProgram->setUniformValue(UniformNames::kLocalToWorld, localToWorld);

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
}

void SceneRenderer::renderPrePass(const SceneRenderInfo& sceneRenderInfo)
{
   renderDepthPass(sceneRenderInfo, prePassFramebuffer);
}

void SceneRenderer::setPrePassDepthAttachment(const SPtr<Texture>& depthAttachment)
{
   Fb::Attachments attachments;
   attachments.depthStencilAttachment = depthAttachment;

   prePassFramebuffer.setAttachments(std::move(attachments));
   prePassFramebuffer.setLabel("Pre Pass Framebuffer");
}

void SceneRenderer::renderSSAOPass(const SceneRenderInfo& sceneRenderInfo)
{
   ssaoBuffer.bind();

   RasterizerState rasterizerState;
   rasterizerState.enableDepthTest = false;
   RasterizerStateScope rasterizerStateScope(rasterizerState);

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

SPtr<Framebuffer> SceneRenderer::renderShadowMap(const Scene& scene, const DirectionalLightComponent& directionalLight, ViewInfo& viewInfo)
{
   static const int kShadowMapRes = 2048;

   ASSERT(directionalLight.getCastShadows());

   const CameraComponent* camera = scene.getActiveCameraComponent();
   if (!camera)
   {
      return nullptr;
   }

   viewInfo = getShadowViewInfo(directionalLight, *camera);

   SceneRenderInfo shadowSceneRenderInfo = calcSceneRenderInfo(scene, viewInfo, false);
   setView(viewInfo);

   SPtr<Framebuffer> shadowFramebuffer = obtainShadowMap(kShadowMapRes, kShadowMapRes);
   renderDepthPass(shadowSceneRenderInfo, *shadowFramebuffer);

   return shadowFramebuffer;
}

SPtr<Framebuffer> SceneRenderer::renderShadowMap(const Scene& scene, const PointLightComponent& pointLight, float& nearPlane, float& farPlane)
{
   static const int kCubeShadowMapRes = 1024;

   ASSERT(pointLight.getCastShadows());

   nearPlane = kLightNearPlane;
   farPlane = pointLight.getScaledRadius();

   glm::vec3 lightPosition = pointLight.getAbsolutePosition();
   glm::mat4 viewToClip = getCubeShadowViewToClip(nearPlane, farPlane);

   SPtr<Framebuffer> cubeShadowFramebuffer = obtainCubeShadowMap(kCubeShadowMapRes);

   auto renderFaceDepthPass = [&](Fb::CubeFace face)
   {
      glm::mat4 worldToView = getCubeShadowWorldToView(lightPosition, face);

      ViewInfo viewInfo;
      viewInfo.init(worldToView, viewToClip);

      SceneRenderInfo sceneRenderInfo = calcSceneRenderInfo(scene, viewInfo, false);
      setView(viewInfo);

      cubeShadowFramebuffer->bind();
      cubeShadowFramebuffer->setActiveFace(face);
      renderDepthPass(sceneRenderInfo, *cubeShadowFramebuffer);
   };

   renderFaceDepthPass(Fb::CubeFace::Front);
   renderFaceDepthPass(Fb::CubeFace::Back);
   renderFaceDepthPass(Fb::CubeFace::Top);
   renderFaceDepthPass(Fb::CubeFace::Bottom);
   renderFaceDepthPass(Fb::CubeFace::Left);
   renderFaceDepthPass(Fb::CubeFace::Right);

   return cubeShadowFramebuffer;
}

SPtr<Framebuffer> SceneRenderer::renderShadowMap(const Scene& scene, const SpotLightComponent& spotLight, ViewInfo& viewInfo)
{
   static const int kShadowMapRes = 1024;

   ASSERT(spotLight.getCastShadows());

   viewInfo = getShadowViewInfo(spotLight);

   SceneRenderInfo shadowSceneRenderInfo = calcSceneRenderInfo(scene, viewInfo, false);
   setView(viewInfo);

   SPtr<Framebuffer> shadowFramebuffer = obtainShadowMap(kShadowMapRes, kShadowMapRes);
   renderDepthPass(shadowSceneRenderInfo, *shadowFramebuffer);

   return shadowFramebuffer;
}

void SceneRenderer::renderShadowMaps(const Scene& scene, SceneRenderInfo& sceneRenderInfo)
{
   bool anyShadowMapsRendered = false;

   for (DirectionalLightRenderInfo& directionalLightRenderInfo : sceneRenderInfo.directionalLights)
   {
      const DirectionalLightComponent* component = directionalLightRenderInfo.component;

      if (component->getCastShadows())
      {
         directionalLightRenderInfo.shadowMapFramebuffer = renderShadowMap(scene, *component, directionalLightRenderInfo.shadowViewInfo);
         anyShadowMapsRendered = true;
      }
   }

   for (PointLightRenderInfo& pointLightRenderInfo : sceneRenderInfo.pointLights)
   {
      const PointLightComponent* component = pointLightRenderInfo.component;

      if (component->getCastShadows())
      {
         pointLightRenderInfo.shadowMapFramebuffer = renderShadowMap(scene, *component, pointLightRenderInfo.nearPlane, pointLightRenderInfo.farPlane);
         anyShadowMapsRendered = true;
      }
   }

   for (SpotLightRenderInfo& spotLightRenderInfo : sceneRenderInfo.spotLights)
   {
      const SpotLightComponent* component = spotLightRenderInfo.component;

      if (component->getCastShadows())
      {
         spotLightRenderInfo.shadowMapFramebuffer = renderShadowMap(scene, *component, spotLightRenderInfo.shadowViewInfo);
         anyShadowMapsRendered = true;
      }
   }

   if (anyShadowMapsRendered)
   {
      setView(sceneRenderInfo.viewInfo);
   }
}

void SceneRenderer::renderTranslucencyPass(const SceneRenderInfo& sceneRenderInfo)
{
   translucencyPassFramebuffer.bind();

   RasterizerState rasterizerState;
   rasterizerState.enableDepthWriting = false;
   rasterizerState.enableBlending = true;
   rasterizerState.sourceBlendFactor = BlendFactor::SourceAlpha;
   rasterizerState.destinationBlendFactor = BlendFactor::OneMinusSourceAlpha;
   RasterizerStateScope rasterizerStateScope(rasterizerState);

   std::array<DrawingContext, 8> contexts;
   populateForwardUniforms(sceneRenderInfo, contexts);

   for (const ModelRenderInfo& modelRenderInfo : sceneRenderInfo.modelRenderInfo)
   {
      ASSERT(modelRenderInfo.model);

      glm::mat4 localToWorld = modelRenderInfo.localToWorld.toMatrix();
      glm::mat4 localToNormal = glm::transpose(glm::inverse(localToWorld));

      for (std::size_t i = 0; i < modelRenderInfo.model->getNumMeshSections(); ++i)
      {
         const MeshSection& section = modelRenderInfo.model->getMeshSection(i);
         const Material& material = modelRenderInfo.model->getMaterial(i);

         bool visible = i >= modelRenderInfo.visibilityMask.size() || modelRenderInfo.visibilityMask[i];
         if (visible && material.getBlendMode() == BlendMode::Translucent)
         {
            int permutationIndex = selectForwardPermutation(material);
            DrawingContext& permutationContext = contexts[permutationIndex];

            permutationContext.program->setUniformValue(UniformNames::kLocalToWorld, localToWorld);
            permutationContext.program->setUniformValue(UniformNames::kLocalToNormal, localToNormal, false);

            DrawingContext localContext = permutationContext;
            forwardMaterial.apply(localContext);
            material.apply(localContext);
            section.draw(localContext);
         }
      }
   }
}

void SceneRenderer::setTranslucencyPassAttachments(const SPtr<Texture>& depthAttachment, const SPtr<Texture>& colorAttachment)
{
   Fb::Attachments attachments;
   attachments.depthStencilAttachment = depthAttachment;
   attachments.colorAttachments.push_back(colorAttachment);

   translucencyPassFramebuffer.setAttachments(std::move(attachments));
   translucencyPassFramebuffer.setLabel("Translucency Pass Framebuffer");
}

void SceneRenderer::renderBloomPass(const SceneRenderInfo& sceneRenderInfo, Framebuffer& lightingFramebuffer, int lightingBufferAttachmentIndex)
{
   RasterizerState rasterizerState;
   rasterizerState.enableDepthTest = false;
   RasterizerStateScope rasterizerStateScope(rasterizerState);

   Framebuffer::blit(lightingFramebuffer, downsampledColorFramebuffer, GL_COLOR_ATTACHMENT0 + lightingBufferAttachmentIndex, GL_COLOR_ATTACHMENT0, GL_COLOR_BUFFER_BIT, GL_LINEAR);

   bloomPassFramebuffer.bind();
   DrawingContext thresholdContext(thresholdProgram.get());
   thresholdMaterial.setParameter("uTexture", downsampledColorFramebuffer.getColorAttachment(0));
   thresholdMaterial.apply(thresholdContext);
   getScreenMesh().draw(thresholdContext);

   renderBlurPass(sceneRenderInfo, bloomPassFramebuffer.getColorAttachment(0), bloomPassFramebuffer, 2);
}

void SceneRenderer::renderBlurPass(const SceneRenderInfo& sceneRenderInfo, const SPtr<Texture>& inputTexture, Framebuffer& resultFramebuffer, int iterations)
{
   RasterizerState rasterizerState;
   rasterizerState.enableDepthTest = false;
   RasterizerStateScope rasterizerStateScope(rasterizerState);

   horizontalBlurMaterial.setParameter("uTexture", inputTexture);

   for (int i = 0; i < iterations; ++i)
   {
      blurFramebuffer.bind();
      DrawingContext horizontalBlurContext(horizontalBlurProgram.get());
      horizontalBlurMaterial.apply(horizontalBlurContext);
      getScreenMesh().draw(horizontalBlurContext);

      verticalBlurMaterial.setParameter("uTexture", blurFramebuffer.getColorAttachment(0));

      resultFramebuffer.bind();
      DrawingContext verticalBlurContext(verticalBlurProgram.get());
      verticalBlurMaterial.apply(verticalBlurContext);
      getScreenMesh().draw(verticalBlurContext);

      horizontalBlurMaterial.setParameter("uTexture", resultFramebuffer.getColorAttachment(0));
   }
}

void SceneRenderer::renderTonemapPass(const SceneRenderInfo& sceneRenderInfo)
{
   RasterizerState rasterizerState;
   rasterizerState.enableDepthTest = false;
   RasterizerStateScope rasterizerStateScope(rasterizerState);

   DrawingContext tonemapContext(tonemapProgram.get());
   tonemapMaterial.apply(tonemapContext);
   getScreenMesh().draw(tonemapContext);
}

void SceneRenderer::setTonemapTextures(const SPtr<Texture>& hdrColorTexture, const SPtr<Texture>& bloomTexture)
{
   tonemapMaterial.setParameter("uColorHDR", hdrColorTexture);
   tonemapMaterial.setParameter("uBloom", bloomTexture);
}

void SceneRenderer::loadForwardProgramPermutations()
{
   std::vector<ShaderSpecification> shaderSpecifications;
   shaderSpecifications.resize(2);
   shaderSpecifications[0].type = ShaderType::Vertex;
   shaderSpecifications[1].type = ShaderType::Fragment;
   IOUtils::getAbsoluteResourcePath("Shaders/Forward.vert", shaderSpecifications[0].path);
   IOUtils::getAbsoluteResourcePath("Shaders/Forward.frag", shaderSpecifications[1].path);

   for (std::size_t i = 0; i < forwardProgramPermutations.size(); ++i)
   {
      for (ShaderSpecification& shaderSpecification : shaderSpecifications)
      {
         shaderSpecification.definitions["WITH_DIFFUSE_TEXTURE"] = i & 0b001 ? "1" : "0";
         shaderSpecification.definitions["WITH_SPECULAR_TEXTURE"] = i & 0b010 ? "1" : "0";
         shaderSpecification.definitions["WITH_NORMAL_TEXTURE"] = i & 0b100 ? "1" : "0";

         shaderSpecification.definitions["MAX_DIRECTIONAL_LIGHTS"] = std::to_string(kMaxDirectionalLights);
         shaderSpecification.definitions["MAX_POINT_LIGHTS"] = std::to_string(kMaxPointLights);
         shaderSpecification.definitions["MAX_SPOT_LIGHTS"] = std::to_string(kMaxSpotLights);
      }

      forwardProgramPermutations[i] = getResourceManager().loadShaderProgram(shaderSpecifications);
      forwardProgramPermutations[i]->bindUniformBuffer(GraphicsContext::current().getFramebufferUniformBuffer());
      forwardProgramPermutations[i]->bindUniformBuffer(viewUniformBuffer);
   }
}

int SceneRenderer::selectForwardPermutation(const Material& material)
{
   int index = material.hasCommonParameter(CommonMaterialParameter::DiffuseTexture) * 0b001
      + material.hasCommonParameter(CommonMaterialParameter::SpecularTexture) * 0b010
      + material.hasCommonParameter(CommonMaterialParameter::NormalTexture) * 0b100;

   return index;
}

void SceneRenderer::populateForwardUniforms(const SceneRenderInfo& sceneRenderInfo, std::array<DrawingContext, 8>& contexts)
{
   ASSERT(contexts.size() == forwardProgramPermutations.size());

   int index = 0;
   for (SPtr<ShaderProgram>& forwardProgramPermutation : forwardProgramPermutations)
   {
      contexts[index].program = forwardProgramPermutation.get();
      populateLightUniforms(sceneRenderInfo, contexts[index], dummyShadowMap, dummyShadowCubeMap);

      ++index;
   }
}

SPtr<Framebuffer> SceneRenderer::obtainShadowMap(int width, int height)
{
   Fb::Specification shadowMapSpecification;
   shadowMapSpecification.width = width;
   shadowMapSpecification.height = height;
   shadowMapSpecification.depthStencilType = Fb::DepthStencilType::Depth24Stencil8;

   return shadowMapPool.obtain(shadowMapSpecification);
}

SPtr<Framebuffer> SceneRenderer::obtainCubeShadowMap(int size)
{
   Fb::Specification cubeShadowMapSpecification;
   cubeShadowMapSpecification.width = size;
   cubeShadowMapSpecification.height = size;
   cubeShadowMapSpecification.cubeMap = true;
   cubeShadowMapSpecification.depthStencilType = Fb::DepthStencilType::Depth24Stencil8;

   return shadowMapPool.obtain(cubeShadowMapSpecification);
}
