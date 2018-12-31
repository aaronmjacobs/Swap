#pragma once

#include "Graphics/Mesh.h"

#include <glm/glm.hpp>

class Scene;

struct PerspectiveInfo
{
   glm::mat4 projectionMatrix;
   glm::mat4 viewMatrix;
   glm::vec3 cameraPosition;
};

class SceneRenderer
{
public:
   SceneRenderer(int initialWidth, int initialHeight);
   virtual ~SceneRenderer() = default;

   virtual void renderScene(const Scene& scene) = 0;

   virtual void onFramebufferSizeChanged(int newWidth, int newHeight);

   void setNearPlaneDistance(float newNearPlaneDistance);
   void setFarPlaneDistance(float newFarPlaneDistance);

protected:
   bool getPerspectiveInfo(const Scene& scene, PerspectiveInfo& perspectiveInfo) const;

   int getWidth() const
   {
      return width;
   }

   int getHeight() const
   {
      return height;
   }

   float getNearPlaneDistance() const
   {
      return nearPlaneDistance;
   }

   float getFarPlaneDistance() const
   {
      return farPlaneDistance;
   }

   const Mesh& getScreenMesh() const
   {
      return screenMesh;
   }

private:
   int width;
   int height;
   float nearPlaneDistance;
   float farPlaneDistance;

   Mesh screenMesh;
};
