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
   void renderPrePass(const Scene& scene, const PerspectiveInfo& perspectiveInfo);
   void renderBasePass(const Scene& scene, const PerspectiveInfo& perspectiveInfo);
   void renderLightingPass(const Scene& scene, const PerspectiveInfo& perspectiveInfo);
   void renderPostProcessPasses(const Scene& scene, const PerspectiveInfo& perspectiveInfo);

   const SPtr<Texture>& getGBufferTexture(GBufferTarget target) const;

   void loadGBufferProgramPermutations();
   SPtr<ShaderProgram>& selectGBufferPermutation(const Material& material);

   Framebuffer gBuffer;

   SPtr<Model> sphereModel;
   SPtr<Model> coneModel;

   SPtr<ShaderProgram> depthOnlyProgram;

   std::array<SPtr<ShaderProgram>, 8> gBufferProgramPermutations;

   Material lightingMaterial;
   SPtr<ShaderProgram> directionalLightingProgram;
   SPtr<ShaderProgram> pointLightingProgram;
   SPtr<ShaderProgram> spotLightingProgram;
};
