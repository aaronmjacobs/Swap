#pragma once

#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Scene/Rendering/SceneRenderer.h"

#include <glm/glm.hpp>

class Material;
class Model;

class DeferredSceneRenderer : public SceneRenderer
{
public:
   DeferredSceneRenderer(const SPtr<ResourceManager>& inResourceManager);

   void renderScene(const Scene& scene) override;

   void onFramebufferSizeChanged(int newWidth, int newHeight) override;

private:
   void renderBasePass(const SceneRenderInfo& sceneRenderInfo);
   void renderLightingPass(const SceneRenderInfo& sceneRenderInfo);
   void renderPostProcessPasses(const SceneRenderInfo& sceneRenderInfo);

   void loadGBufferProgramPermutations();
   SPtr<ShaderProgram>& selectGBufferPermutation(const Material& material);

   SPtr<Texture> depthStencilTexture;
   SPtr<Texture> positionTexture;
   SPtr<Texture> normalShininessTexture;
   SPtr<Texture> albedoTexture;
   SPtr<Texture> specularTexture;
   SPtr<Texture> emissiveTexture;
   SPtr<Texture> colorTexture;

   Framebuffer basePassFramebuffer;
   std::array<SPtr<ShaderProgram>, 8> gBufferProgramPermutations;

   Framebuffer lightingPassFramebuffer;
   Material lightingMaterial;
   SPtr<ShaderProgram> directionalLightingProgram;
   SPtr<ShaderProgram> pointLightingProgram;
   SPtr<ShaderProgram> spotLightingProgram;
   SPtr<Mesh> sphereMesh;
   SPtr<Mesh> coneMesh;
};
