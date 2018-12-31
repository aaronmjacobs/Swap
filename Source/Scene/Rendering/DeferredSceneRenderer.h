#pragma once

#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Scene/Rendering/SceneRenderer.h"

#include <glm/glm.hpp>

class ResourceManager;
class ShaderProgram;

class DeferredSceneRenderer : public SceneRenderer
{
public:
   DeferredSceneRenderer(int initialWidth, int initialHeight, const SPtr<ResourceManager>& inResourceManager);

   void renderScene(const Scene& scene) override;

   void onFramebufferSizeChanged(int newWidth, int newHeight) override;

private:
   void renderPrePass(const Scene& scene, const PerspectiveInfo& perspectiveInfo);
   void renderBasePass(const Scene& scene, const PerspectiveInfo& perspectiveInfo);
   void renderLightingPass(const Scene& scene, const PerspectiveInfo& perspectiveInfo);
   void renderPostProcessPasses(const Scene& scene, const PerspectiveInfo& perspectiveInfo);

   SPtr<ResourceManager> resourceManager;

   Framebuffer gBuffer;

   Mesh sphereMesh;
   Mesh coneMesh;

   SPtr<ShaderProgram> depthOnlyProgram;
   SPtr<ShaderProgram> directionalLightingProgram;
   SPtr<ShaderProgram> pointLightingProgram;
   SPtr<ShaderProgram> spotLightingProgram;
};
