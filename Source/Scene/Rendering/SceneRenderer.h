#pragma once

#include "Core/Assert.h"
#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"

#include <glm/glm.hpp>

class ResourceManager;
class Scene;
class ShaderProgram;
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

   void renderSSAOPass(const PerspectiveInfo& perspectiveInfo);
   void setSSAOTextures(const SPtr<Texture>& depthTexture, const SPtr<Texture>& positionTexture, const SPtr<Texture>& normalTexture);
   const SPtr<Texture>& getSSAOBlurTexture() const;

   bool getPerspectiveInfo(const Scene& scene, PerspectiveInfo& perspectiveInfo) const;

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

   Framebuffer ssaoBuffer;
   Material ssaoMaterial;
   SPtr<ShaderProgram> ssaoProgram;
   SPtr<Texture> ssaoNoiseTexture;

   Framebuffer ssaoBlurBuffer;
   Material ssaoBlurMaterial;
   SPtr<ShaderProgram> ssaoBlurProgram;
};
