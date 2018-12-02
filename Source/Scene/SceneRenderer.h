#pragma once

class Scene;

class SceneRenderer
{
public:
   SceneRenderer(int initialWidth, int initialHeight);

   void renderScene(const Scene& scene);

   void onFramebufferSizeChanged(int newWidth, int newHeight);
   void setNearPlaneDistance(float newNearPlaneDistance);
   void setFarPlaneDistance(float newFarPlaneDistance);

private:
   int width;
   int height;
   float nearPlaneDistance;
   float farPlaneDistance;
};
