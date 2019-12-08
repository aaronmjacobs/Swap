#pragma once

#include "Core/Assert.h"
#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
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

struct PerspectiveInfo
{
   glm::mat4 projectionMatrix;
   glm::mat4 viewMatrix;
   glm::vec3 cameraPosition;
};

struct ModelRenderInfo
{
   Transform localToWorld;
   std::vector<bool> visibilityMask;
   const Model* model = nullptr;
};

struct SceneRenderInfo
{
   PerspectiveInfo perspectiveInfo;
   std::vector<ModelRenderInfo> modelRenderInfo;
   std::vector<const DirectionalLightComponent*> directionalLights;
   std::vector<const PointLightComponent*> pointLights;
   std::vector<const SpotLightComponent*> spotLights;
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

   bool calcSceneRenderInfo(const Scene& scene, SceneRenderInfo& sceneRenderInfo) const;
   bool getPerspectiveInfo(const Scene& scene, PerspectiveInfo& perspectiveInfo) const;

   void populateViewUniforms(const PerspectiveInfo& perspectiveInfo);

   void renderPrePass(const SceneRenderInfo& sceneRenderInfo);
   void setPrePassDepthAttachment(const SPtr<Texture>& depthAttachment);

   void renderSSAOPass(const SceneRenderInfo& sceneRenderInfo);
   void setSSAOTextures(const SPtr<Texture>& depthTexture, const SPtr<Texture>& positionTexture, const SPtr<Texture>& normalTexture);

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
   SPtr<ShaderProgram>& selectForwardPermutation(const Material& material);
   void populateForwardUniforms(const SceneRenderInfo& sceneRenderInfo);

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

private:
   float nearPlaneDistance;
   float farPlaneDistance;

   SPtr<ResourceManager> resourceManager;

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
