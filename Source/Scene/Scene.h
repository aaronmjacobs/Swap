#pragma once

#include "Core/Delegate.h"
#include "Core/Pointers.h"
#include "Scene/Entity.h"

#include <gsl/span>

#include <string>
#include <unordered_map>
#include <vector>

class CameraComponent;
class DirectionalLightComponent;
class ModelComponent;
class PointLightComponent;
class SpotLightComponent;

class Scene
{
public:
   Scene();
   ~Scene();

   void tick(float dt);

   template<typename... ComponentTypes>
   Entity* createEntity()
   {
      entities.push_back(Entity::create<ComponentTypes...>(*this));
      return entities.back().get();
   }

   Entity* createEntity(gsl::span<std::string> componentClassNames)
   {
      entities.push_back(Entity::create(componentClassNames, *this));
      return entities.back().get();
   }

   bool destroyEntity(Entity* entityToDestroy);

   const std::vector<UPtr<Entity>>& getEntities() const
   {
      return entities;
   }

   const CameraComponent* getActiveCameraComponent() const
   {
      return activeCameraComponent;
   }

   void setActiveCameraComponent(CameraComponent* newActiveCameraComponent);

   const std::vector<CameraComponent*>& getCameraComponents() const
   {
      return cameraComponents;
   }

   void registerCameraComponent(CameraComponent* cameraComponent);
   void unregisterCameraComponent(CameraComponent* cameraComponent);

   const std::vector<ModelComponent*>& getModelComponents() const
   {
      return modelComponents;
   }

   void registerModelComponent(ModelComponent* modelComponent);
   void unregisterModelComponent(ModelComponent* modelComponent);

   const std::vector<DirectionalLightComponent*>& getDirectionalLightComponents() const
   {
      return directionalLightComponents;
   }

   void registerDirectionalLightComponent(DirectionalLightComponent* directionalLightComponent);
   void unregisterDirectionalLightComponent(DirectionalLightComponent* directionalLightComponent);

   const std::vector<PointLightComponent*>& getPointLightComponents() const
   {
      return pointLightComponents;
   }

   void registerPointLightComponent(PointLightComponent* pointLightComponent);
   void unregisterPointLightComponent(PointLightComponent* pointLightComponent);

   const std::vector<SpotLightComponent*>& getSpotLightComponents() const
   {
      return spotLightComponents;
   }

   void registerSpotLightComponent(SpotLightComponent* spotLightComponent);
   void unregisterSpotLightComponent(SpotLightComponent* spotLightComponent);

private:
   std::vector<UPtr<Entity>> entities;

   std::vector<CameraComponent*> cameraComponents;
   CameraComponent* activeCameraComponent;

   std::vector<ModelComponent*> modelComponents;

   std::vector<DirectionalLightComponent*> directionalLightComponents;
   std::vector<PointLightComponent*> pointLightComponents;
   std::vector<SpotLightComponent*> spotLightComponents;
};
