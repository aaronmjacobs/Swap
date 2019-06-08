#pragma once

#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Scene/Rendering/SceneRenderer.h"

#include <glm/glm.hpp>

class Material;
class Model;

enum class GBufferTarget
{
   DepthStencil,

   Position,
   NormalShininess,
   Albedo,
   Specular,
   Emissive
};

class DeferredSceneRenderer : public SceneRenderer
{
public:
   DeferredSceneRenderer(int initialWidth, int initialHeight, const SPtr<ResourceManager>& inResourceManager);

   void renderScene(const Scene& scene) override;

   void onFramebufferSizeChanged(int newWidth, int newHeight) override;

private:
   void renderPrePass(const SceneRenderInfo& sceneRenderInfo);
   void renderBasePass(const SceneRenderInfo& sceneRenderInfo);
   void renderLightingPass(const SceneRenderInfo& sceneRenderInfo);
   void renderPostProcessPasses(const SceneRenderInfo& sceneRenderInfo);

   const SPtr<Texture>& getGBufferTexture(GBufferTarget target) const;

   void loadGBufferProgramPermutations();
   SPtr<ShaderProgram>& selectGBufferPermutation(const Material& material);

   Framebuffer gBuffer;

   SPtr<Mesh> sphereMesh;
   SPtr<Mesh> coneMesh;

   SPtr<ShaderProgram> depthOnlyProgram;

   std::array<SPtr<ShaderProgram>, 8> gBufferProgramPermutations;

   Material lightingMaterial;
   SPtr<ShaderProgram> directionalLightingProgram;
   SPtr<ShaderProgram> pointLightingProgram;
   SPtr<ShaderProgram> spotLightingProgram;
};
