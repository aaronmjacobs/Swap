#include "Scene/Scene.h"

#include <algorithm>

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
