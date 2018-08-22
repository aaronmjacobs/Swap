#pragma once

#include "Core/Pointers.h"
#include "Scene/Entity.h"

#include <gsl/span>

#include <string>
#include <vector>

class Scene
{
public:
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

private:
   std::vector<UPtr<Entity>> entities;
};
