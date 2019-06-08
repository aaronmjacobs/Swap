#pragma once

#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Scene/Rendering/SceneRenderer.h"

#include <glm/glm.hpp>

class Material;
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
   void renderPrePass(const SceneRenderInfo& sceneRenderInfo);
   void renderNormalPass(const SceneRenderInfo& sceneRenderInfo);
   void renderMainPass(const SceneRenderInfo& sceneRenderInfo);
   void renderPostProcessPasses(const SceneRenderInfo& sceneRenderInfo);

   void loadNormalProgramPermutations();
   SPtr<ShaderProgram>& selectNormalPermutation(const Material& material);

   void loadForwardProgramPermutations();
   SPtr<ShaderProgram>& selectForwardPermutation(const Material& material);

   Framebuffer mainPassFramebuffer;

   SPtr<ShaderProgram> depthOnlyProgram;

   std::array<SPtr<ShaderProgram>, 2> normalProgramPermutations;

   Material forwardMaterial;
   std::array<SPtr<ShaderProgram>, 8> forwardProgramPermutations;
};
