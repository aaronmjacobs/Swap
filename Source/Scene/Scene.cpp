#include "Scene/Scene.h"

#include "Core/Assert.h"
#include "Scene/CameraComponent.h"
#include "Scene/ModelComponent.h"
#include "Scene/Tickable.h"

#include <algorithm>

Scene::Scene()
   : activeCameraComponent(nullptr)
{
}

Scene::~Scene()
{
   entities.clear();
   activeCameraComponent = nullptr;
   cameraComponents.clear();
   modelComponents.clear();
}

void Scene::tick(float dt)
{
   for (Tickable* tickable : tickables)
   {
      tickable->tick(dt);
   }
}

bool Scene::destroyEntity(Entity* entityToDestroy)
{
   auto location = std::find_if(entities.begin(), entities.end(), [entityToDestroy](const UPtr<Entity>& entity)
   {
      return entity.get() == entityToDestroy;
   });

   if (location != entities.end())
   {
      entities.erase(location);
      return true;
   }

   return false;
}

void Scene::registerTickable(Tickable* tickable)
{
   ASSERT(tickable);
   ASSERT(std::find(tickables.begin(), tickables.end(), tickable) == tickables.end());

   tickables.push_back(tickable);
}

void Scene::unregisterTickable(Tickable* tickable)
{
   ASSERT(tickable);

   auto location = std::find(tickables.begin(), tickables.end(), tickable);
   ASSERT(location != tickables.end());

   tickables.erase(location);
}

void Scene::setActiveCameraComponent(CameraComponent* newActiveCameraComponent)
{
   ASSERT(!newActiveCameraComponent || std::find(cameraComponents.begin(), cameraComponents.end(), newActiveCameraComponent) != cameraComponents.end());

   activeCameraComponent = newActiveCameraComponent;
}

void Scene::registerCameraComponent(CameraComponent* cameraComponent)
{
   ASSERT(cameraComponent);
   ASSERT(std::find(cameraComponents.begin(), cameraComponents.end(), cameraComponent) == cameraComponents.end());

   cameraComponents.push_back(cameraComponent);
}

void Scene::unregisterCameraComponent(CameraComponent* cameraComponent)
{
   ASSERT(cameraComponent);

   if (activeCameraComponent == cameraComponent)
   {
      activeCameraComponent = nullptr;
   }

   auto location = std::find(cameraComponents.begin(), cameraComponents.end(), cameraComponent);
   ASSERT(location != cameraComponents.end());

   cameraComponents.erase(location);
}

void Scene::registerModelComponent(ModelComponent* modelComponent)
{
   ASSERT(modelComponent);
   ASSERT(std::find(modelComponents.begin(), modelComponents.end(), modelComponent) == modelComponents.end());

   modelComponents.push_back(modelComponent);
}

void Scene::unregisterModelComponent(ModelComponent* modelComponent)
{
   ASSERT(modelComponent);

   auto location = std::find(modelComponents.begin(), modelComponents.end(), modelComponent);
   ASSERT(location != modelComponents.end());

   modelComponents.erase(location);
}
