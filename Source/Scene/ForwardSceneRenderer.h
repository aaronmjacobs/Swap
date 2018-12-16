#pragma once

#include "Scene/SceneRenderer.h"

class ForwardSceneRenderer : public SceneRenderer
{
public:
   ForwardSceneRenderer(int initialWidth, int initialHeight);

   void renderScene(const Scene& scene) override;
};
