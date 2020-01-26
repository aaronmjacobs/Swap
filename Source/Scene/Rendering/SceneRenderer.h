#pragma once

#include "Core/Assert.h"
#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/ResourcePool.h"
#include "Graphics/UniformBufferObject.h"
#include "Math/Transform.h"

#include <glm/glm.hpp>
#include <vector>

class DirectionalLightComponent;
class Model;
class ModelComponent;
class PointLightComponent;
class ResourceManager;
class Scene;
class ShaderProgram;
class SpotLightComponent;
class Texture;
struct DrawingContext;

namespace UniformNames
{
   extern const char* kLocalToWorld;
   extern const char* kLocalToNormal;
}

class ViewInfo
{
public:
   void init(const glm::mat4& worldToViewMatrix, const glm::mat4& viewToClipMatrix)
   {
      worldToView = worldToViewMatrix;
      viewToClip = viewToClipMatrix;
      worldToClip = viewToClip * worldToView;
   }

   const glm::mat4& getWorldToView() const
   {
      return worldToView;
   }

   const glm::mat4& getViewToClip() const
   {
      return viewToClip;
   }

   const glm::mat4& getWorldToClip() const
   {
      return worldToClip;
   }

   glm::vec3 getViewOrigin() const
   {
      return glm::vec3(-worldToView[3][0], -worldToView[3][1], -worldToView[3][2]);
   }

private:
   glm::mat4 worldToView;
   glm::mat4 viewToClip;
   glm::mat4 worldToClip;
};

struct ModelRenderInfo
{
   Transform localToWorld;
   std::vector<bool> visibilityMask;
   const Model* model = nullptr;
};

struct DirectionalLightUniformData
{
   glm::vec3 color = glm::vec3(0.0f);
   glm::vec3 direction = glm::vec3(0.0f);

   bool castShadows = false;
   glm::mat4 worldToShadow = glm::mat4(0.0f);
   float shadowBias = 0.0f;
   SPtr<Texture> shadowMap = nullptr;
};

struct PointLightUniformData
{
   glm::vec3 color = glm::vec3(0.0f);
   glm::vec3 position = glm::vec3(0.0f);
   float radius = 0.0f;

   bool castShadows = false;
   glm::vec2 nearFar = glm::vec2(0.0f);
   float shadowBias = 0.0f;
   SPtr<Texture> shadowMap = nullptr;
};

struct SpotLightUniformData
{
   glm::vec3 color = glm::vec3(0.0f);
   glm::vec3 direction = glm::vec3(0.0f);
   glm::vec3 position = glm::vec3(0.0f);
   float radius = 0.0f;
   float beamAngle = 0.0f;
   float cutoffAngle = 0.0f;

   bool castShadows = false;
   glm::mat4 worldToShadow = glm::mat4(0.0f);
   float shadowBias = 0.0f;
   SPtr<Texture> shadowMap = nullptr;
};

struct LightRenderInfo
{
   SPtr<Framebuffer> shadowMapFramebuffer;
};

struct DirectionalLightRenderInfo : public LightRenderInfo
{
   ViewInfo shadowViewInfo;
   const DirectionalLightComponent* component = nullptr;

   DirectionalLightUniformData getUniformData() const;
};

struct PointLightRenderInfo : public LightRenderInfo
{
   float nearPlane = 0.1f;
   float farPlane = 1.0f;
   const PointLightComponent* component = nullptr;

   PointLightUniformData getUniformData() const;
};

struct SpotLightRenderInfo : public LightRenderInfo
{
   ViewInfo shadowViewInfo;
   const SpotLightComponent* component = nullptr;

   SpotLightUniformData getUniformData() const;
};

struct SceneRenderInfo
{
   ViewInfo viewInfo;
   std::vector<ModelRenderInfo> modelRenderInfo;
   std::vector<DirectionalLightRenderInfo> directionalLights;
   std::vector<PointLightRenderInfo> pointLights;
   std::vector<SpotLightRenderInfo> spotLights;
};

class SceneRenderer
{
public:
   SceneRenderer(const SPtr<ResourceManager>& inResourceManager, bool hasPositionBuffer);
   virtual ~SceneRenderer() = default;

   virtual void renderScene(const Scene& scene) = 0;

   virtual void onFramebufferSizeChanged(int newWidth, int newHeight);

   void setNearPlaneDistance(float newNearPlaneDistance);
   void setFarPlaneDistance(float newFarPlaneDistance);

protected:
   ResourceManager& getResourceManager() const
   {
      ASSERT(resourceManager);

      return *resourceManager;
   }

   const SPtr<Texture>& getDummyShadowMap() const
   {
      return dummyShadowMap;
   }

   const SPtr<Texture>& getDummyShadowCubeMap() const
   {
      return dummyShadowCubeMap;
   }

   bool getViewInfo(const Scene& scene, ViewInfo& viewInfo) const;
   SceneRenderInfo calcSceneRenderInfo(const Scene& scene, const ViewInfo& viewInfo, bool includeLights) const;

   void setView(const ViewInfo& viewInfo);

   void renderDepthPass(const SceneRenderInfo& sceneRenderInfo, Framebuffer& framebuffer);

   void renderPrePass(const SceneRenderInfo& sceneRenderInfo);
   void setPrePassDepthAttachment(const SPtr<Texture>& depthAttachment);

   void renderSSAOPass(const SceneRenderInfo& sceneRenderInfo);
   void setSSAOTextures(const SPtr<Texture>& depthTexture, const SPtr<Texture>& positionTexture, const SPtr<Texture>& normalTexture);

   SPtr<Framebuffer> renderShadowMap(const Scene& scene, const DirectionalLightComponent& directionalLight, ViewInfo& viewInfo);
   SPtr<Framebuffer> renderShadowMap(const Scene& scene, const PointLightComponent& pointLight, float& nearPlane, float& farPlane);
   SPtr<Framebuffer> renderShadowMap(const Scene& scene, const SpotLightComponent& spotLight, ViewInfo& viewInfo);
   void renderShadowMaps(const Scene& scene, SceneRenderInfo& sceneRenderInfo);

   void renderTranslucencyPass(const SceneRenderInfo& sceneRenderInfo);
   void setTranslucencyPassAttachments(const SPtr<Texture>& depthAttachment, const SPtr<Texture>& colorAttachment);

   void renderBloomPass(const SceneRenderInfo& sceneRenderInfo, Framebuffer& lightingFramebuffer, int lightingBufferAttachmentIndex);

   void renderBlurPass(const SceneRenderInfo& sceneRenderInfo, const SPtr<Texture>& inputTexture, Framebuffer& resultFramebuffer, int iterations);

   void renderTonemapPass(const SceneRenderInfo& sceneRenderInfo);
   void setTonemapTextures(const SPtr<Texture>& hdrColorTexture, const SPtr<Texture>& bloomTexture);

   const SPtr<Texture>& getSSAOTexture() const
   {
      return ssaoTexture;
   }

   float getNearPlaneDistance() const
   {
      return nearPlaneDistance;
   }

   float getFarPlaneDistance() const
   {
      return farPlaneDistance;
   }

   const Mesh& getScreenMesh() const
   {
      return screenMesh;
   }

   const SPtr<UniformBufferObject>& getViewUniformBuffer() const
   {
      return viewUniformBuffer;
   }

   void loadForwardProgramPermutations();
   int selectForwardPermutation(const Material& material);
   void populateForwardUniforms(const SceneRenderInfo& sceneRenderInfo, std::array<DrawingContext, 8>& contexts);

   Material& getForwardMaterial()
   {
      return forwardMaterial;
   }

   std::array<SPtr<ShaderProgram>, 8>& getForwardProgramPermutations()
   {
      return forwardProgramPermutations;
   }

   Framebuffer& getBloomPassFramebuffer()
   {
      return bloomPassFramebuffer;
   }

   SPtr<Framebuffer> obtainShadowMap(int width, int height);
   SPtr<Framebuffer> obtainCubeShadowMap(int size);

private:
   float nearPlaneDistance;
   float farPlaneDistance;

   SPtr<ResourceManager> resourceManager;
   ResourcePool<Framebuffer> shadowMapPool;

   Mesh screenMesh;

   SPtr<UniformBufferObject> viewUniformBuffer;

   SPtr<Texture> dummyShadowMap;
   SPtr<Texture> dummyShadowCubeMap;

   Framebuffer prePassFramebuffer;
   SPtr<ShaderProgram> depthOnlyProgram;

   Framebuffer ssaoBuffer;
   Material ssaoMaterial;
   SPtr<ShaderProgram> ssaoProgram;
   SPtr<Texture> ssaoUnfilteredTexture;
   SPtr<Texture> ssaoNoiseTexture;

   Framebuffer ssaoBlurBuffer;
   Material ssaoBlurMaterial;
   SPtr<ShaderProgram> ssaoBlurProgram;
   SPtr<Texture> ssaoTexture;

   Framebuffer translucencyPassFramebuffer;
   Material forwardMaterial;
   std::array<SPtr<ShaderProgram>, 8> forwardProgramPermutations;

   Material thresholdMaterial;
   SPtr<ShaderProgram> thresholdProgram;

   Framebuffer blurFramebuffer;
   Material horizontalBlurMaterial;
   Material verticalBlurMaterial;
   SPtr<ShaderProgram> horizontalBlurProgram;
   SPtr<ShaderProgram> verticalBlurProgram;

   Framebuffer downsampledColorFramebuffer;
   Framebuffer bloomPassFramebuffer;

   Material tonemapMaterial;
   SPtr<ShaderProgram> tonemapProgram;
};
