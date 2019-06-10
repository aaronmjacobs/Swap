#pragma once

#include "Core/Assert.h"
#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
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
   extern const char* kProjectionMatrix;
   extern const char* kViewMatrix;
   extern const char* kModelMatrix;
   extern const char* kNormalMatrix;

   extern const char* kCameraPos;
   extern const char* kViewport;
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

   void renderPrePass(const SceneRenderInfo& sceneRenderInfo);
   void setPrePassDepthAttachment(const SPtr<Texture>& depthAttachment);

   void renderSSAOPass(const SceneRenderInfo& sceneRenderInfo);
   void setSSAOTextures(const SPtr<Texture>& depthTexture, const SPtr<Texture>& positionTexture, const SPtr<Texture>& normalTexture);

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

private:
   int width;
   int height;
   float nearPlaneDistance;
   float farPlaneDistance;

   SPtr<ResourceManager> resourceManager;

   Mesh screenMesh;

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
};
