#include "Scene/Scene.h"

#include "Core/Assert.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/ModelComponent.h"

#include <algorithm>

namespace
{
   template<typename T>
   void registerComponent(std::vector<T*>& components, T* component)
   {
      ASSERT(component);
      ASSERT(std::find(components.begin(), components.end(), component) == components.end());

      components.push_back(component);
   }

   template<typename T>
   void unregisterComponent(std::vector<T*>& components, T* component)
   {
      ASSERT(component);

      auto location = std::find(components.begin(), components.end(), component);
      ASSERT(location != components.end());

      components.erase(location);
   }
}

Scene::Scene()
   : time(0.0f)
   , activeCameraComponent(nullptr)
{
}

Scene::~Scene()
{
   entities.clear();

   cameraComponents.clear();
   activeCameraComponent = nullptr;

   modelComponents.clear();

   directionalLightComponents.clear();
   pointLightComponents.clear();
   spotLightComponents.clear();
}

void Scene::tick(float dt)
{
   time += dt;

   for (const UPtr<Entity>& entity : entities)
   {
      entity->tick(dt);
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

void Scene::setActiveCameraComponent(CameraComponent* newActiveCameraComponent)
{
   ASSERT(!newActiveCameraComponent || std::find(cameraComponents.begin(), cameraComponents.end(), newActiveCameraComponent) != cameraComponents.end());

   activeCameraComponent = newActiveCameraComponent;
}

void Scene::registerCameraComponent(CameraComponent* cameraComponent)
{
   registerComponent(cameraComponents, cameraComponent);
}

void Scene::unregisterCameraComponent(CameraComponent* cameraComponent)
{
   unregisterComponent(cameraComponents, cameraComponent);

   if (activeCameraComponent == cameraComponent)
   {
      activeCameraComponent = nullptr;
   }
}

void Scene::registerModelComponent(ModelComponent* modelComponent)
{
   registerComponent(modelComponents, modelComponent);
}

void Scene::unregisterModelComponent(ModelComponent* modelComponent)
{
   unregisterComponent(modelComponents, modelComponent);
}

void Scene::registerDirectionalLightComponent(DirectionalLightComponent* directionalLightComponent)
{
   registerComponent(directionalLightComponents, directionalLightComponent);
}

void Scene::unregisterDirectionalLightComponent(DirectionalLightComponent* directionalLightComponent)
{
   unregisterComponent(directionalLightComponents, directionalLightComponent);
}

void Scene::registerPointLightComponent(PointLightComponent* pointLightComponent)
{
   registerComponent(pointLightComponents, pointLightComponent);
}

void Scene::unregisterPointLightComponent(PointLightComponent* pointLightComponent)
{
   unregisterComponent(pointLightComponents, pointLightComponent);
}

void Scene::registerSpotLightComponent(SpotLightComponent* spotLightComponent)
{
   registerComponent(spotLightComponents, spotLightComponent);
}

void Scene::unregisterSpotLightComponent(SpotLightComponent* spotLightComponent)
{
   unregisterComponent(spotLightComponents, spotLightComponent);
}
