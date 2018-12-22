#pragma once

#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Scene/SceneRenderer.h"

class ShaderProgram;

class ForwardSceneRenderer : public SceneRenderer
{
public:
   ForwardSceneRenderer(int initialWidth, int initialHeight, int numSamples);

   void renderScene(const Scene& scene) override;

   void onFramebufferSizeChanged(int newWidth, int newHeight) override;

private:
   Framebuffer mainPassFramebuffer;
   SPtr<ShaderProgram> ssaoProgram;
};
