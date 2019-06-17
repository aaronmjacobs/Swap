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
   extern const char* kModelMatrix;
   extern const char* kNormalMatrix;
}

struct PerspectiveInfo
{
   glm::mat4 projectionMatrix;
   glm::mat4 viewMatrix;
   glm::vec3 cameraPosition;
   glm::vec4 viewport;
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
   SceneRenderer(int initialWidth, int initialHeight, const SPtr<ResourceManager>& inResourceManager, bool hasPositionBuffer);
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

   void renderTonemapPass(const SceneRenderInfo& sceneRenderInfo);
   void setTonemapTexture(const SPtr<Texture>& hdrColorTexture);

   const SPtr<Texture>& getSSAOBlurTexture() const
   {
      return ssaoBlurTexture;
   }

   int getWidth() const
   {
      return width;
   }

   int getHeight() const
   {
      return height;
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

   const UniformBufferObject& getViewUniformBuffer() const
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

private:
   int width;
   int height;
   float nearPlaneDistance;
   float farPlaneDistance;

   SPtr<ResourceManager> resourceManager;

   Mesh screenMesh;

   UniformBufferObject viewUniformBuffer;

   Framebuffer prePassFramebuffer;
   SPtr<ShaderProgram> depthOnlyProgram;

   Framebuffer ssaoBuffer;
   Material ssaoMaterial;
   SPtr<ShaderProgram> ssaoProgram;
   SPtr<Texture> ssaoTexture;
   SPtr<Texture> ssaoNoiseTexture;

   Framebuffer ssaoBlurBuffer;
   Material ssaoBlurMaterial;
   SPtr<ShaderProgram> ssaoBlurProgram;
   SPtr<Texture> ssaoBlurTexture;

   Framebuffer translucencyPassFramebuffer;
   Material forwardMaterial;
   std::array<SPtr<ShaderProgram>, 8> forwardProgramPermutations;

   Material tonemapMaterial;
   SPtr<ShaderProgram> tonemapProgram;
};
