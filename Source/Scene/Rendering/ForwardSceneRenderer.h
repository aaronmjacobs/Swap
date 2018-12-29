#pragma once

#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Scene/Rendering/SceneRenderer.h"

#include <glm/glm.hpp>

class ModelComponent;
class ResourceManager;
class ShaderProgram;

struct PerspectiveInfo
{
   glm::mat4 projectionMatrix;
   glm::mat4 viewMatrix;
   glm::vec3 cameraPosition;
};

class ForwardSceneRenderer : public SceneRenderer
{
public:
   ForwardSceneRenderer(int initialWidth, int initialHeight, int numSamples,
      const SPtr<ResourceManager>& inResourceManager);

   void renderScene(const Scene& scene) override;

   void onFramebufferSizeChanged(int newWidth, int newHeight) override;

private:
   bool getPerspectiveInfo(const Scene& scene, PerspectiveInfo& perspectiveInfo) const;

   void renderPrePass(const Scene& scene, const PerspectiveInfo& perspectiveInfo);
   void renderMainPass(const Scene& scene, const PerspectiveInfo& perspectiveInfo);
   void renderPostProcessPasses(const Scene& scene, const PerspectiveInfo& perspectiveInfo);

   Framebuffer mainPassFramebuffer;
   SPtr<ResourceManager> resourceManager;
   SPtr<ShaderProgram> depthOnlyProgram;
};
