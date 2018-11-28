#pragma once

#include "Core/Delegate.h"
#include "Core/Pointers.h"
#include "Scene/Entity.h"

#include <gsl/span>

#include <string>
#include <unordered_map>
#include <vector>

class CameraComponent;
class ModelComponent;
class Tickable;

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

   void registerTickable(Tickable* tickable);
   void unregisterTickable(Tickable* tickable);

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

private:
   std::vector<UPtr<Entity>> entities;

   std::vector<Tickable*> tickables;
   std::vector<CameraComponent*> cameraComponents;
   std::vector<ModelComponent*> modelComponents;

   CameraComponent* activeCameraComponent;
};
