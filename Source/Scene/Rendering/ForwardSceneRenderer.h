#pragma once

#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Scene/Rendering/SceneRenderer.h"

#include <glm/glm.hpp>

class ResourceManager;
class ShaderProgram;

class ForwardSceneRenderer : public SceneRenderer
{
public:
   ForwardSceneRenderer(int initialWidth, int initialHeight, int numSamples,
      const SPtr<ResourceManager>& inResourceManager);

   void renderScene(const Scene& scene) override;

   void onFramebufferSizeChanged(int newWidth, int newHeight) override;

private:
   void renderPrePass(const Scene& scene, const PerspectiveInfo& perspectiveInfo);
   void renderMainPass(const Scene& scene, const PerspectiveInfo& perspectiveInfo);
   void renderPostProcessPasses(const Scene& scene, const PerspectiveInfo& perspectiveInfo);

   SPtr<ResourceManager> resourceManager;

   Framebuffer mainPassFramebuffer;

   SPtr<ShaderProgram> depthOnlyProgram;
};
