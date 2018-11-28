#include "Scene/Tickable.h"

#include "Scene/Scene.h"

Tickable::Tickable(Scene& scene)
   : tickableScene(scene)
{
   tickableScene.registerTickable(this);
}

Tickable::~Tickable()
{
   tickableScene.unregisterTickable(this);
}
