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
   extern const char* kModelMatrix;
   extern const char* kNormalMatrix;
}

class ViewInfo
{
public:
   void init(const glm::mat4& viewToClipMatrix, const glm::mat4& worldToViewMatrix)
   {
      viewToClip = viewToClipMatrix;
      worldToView = worldToViewMatrix;
      worldToClip = viewToClip * worldToView;
   }

   const glm::mat4& getViewToClip() const
   {
      return viewToClip;
   }

   const glm::mat4& getWorldToView() const
   {
      return worldToView;
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
   glm::mat4 viewToClip;
   glm::mat4 worldToView;
   glm::mat4 worldToClip;
};

struct ModelRenderInfo
{
   Transform localToWorld;
   std::vector<bool> visibilityMask;
   const Model* model = nullptr;
};

struct LightRenderInfo
{
   SPtr<Framebuffer> shadowMapFramebuffer;
};

struct DirectionalLightRenderInfo : public LightRenderInfo
{
   ViewInfo shadowViewInfo;
   const DirectionalLightComponent* component = nullptr;
};

struct PointLightRenderInfo : public LightRenderInfo
{
   float nearPlane = 0.1f;
   float farPlane = 1.0f;
   const PointLightComponent* component = nullptr;
};

struct SpotLightRenderInfo : public LightRenderInfo
{
   ViewInfo shadowViewInfo;
   const SpotLightComponent* component = nullptr;
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

   SceneRenderInfo calcSceneRenderInfo(const Scene& scene, const ViewInfo& viewInfo, bool includeLights) const;
   bool getViewInfo(const Scene& scene, ViewInfo& viewInfo) const;

   void setView(const ViewInfo& viewInfo);

   void renderDepthPass(const SceneRenderInfo& sceneRenderInfo, Framebuffer& framebuffer);

   void renderPrePass(const SceneRenderInfo& sceneRenderInfo);
   void setPrePassDepthAttachment(const SPtr<Texture>& depthAttachment);

   void renderSSAOPass(const SceneRenderInfo& sceneRenderInfo);
   void setSSAOTextures(const SPtr<Texture>& depthTexture, const SPtr<Texture>& positionTexture, const SPtr<Texture>& normalTexture);

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
