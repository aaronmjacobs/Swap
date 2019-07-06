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
   ForwardSceneRenderer(int numSamples, const SPtr<ResourceManager>& inResourceManager);

   void renderScene(const Scene& scene) override;

   void onFramebufferSizeChanged(int newWidth, int newHeight) override;

private:
   void renderNormalPass(const SceneRenderInfo& sceneRenderInfo);
   void renderMainPass(const SceneRenderInfo& sceneRenderInfo);
   void renderPostProcessPasses(const SceneRenderInfo& sceneRenderInfo);

   void loadNormalProgramPermutations();
   SPtr<ShaderProgram>& selectNormalPermutation(const Material& material);

   SPtr<Texture> depthStencilTexture;
   SPtr<Texture> hdrColorTexture;
   SPtr<Texture> normalTexture;

   Framebuffer normalPassFramebuffer;
   std::array<SPtr<ShaderProgram>, 2> normalProgramPermutations;

   Framebuffer mainPassFramebuffer;
};
