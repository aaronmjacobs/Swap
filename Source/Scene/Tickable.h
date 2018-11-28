#pragma once

class Scene;

class Tickable
{
public:
   Tickable(Scene& scene);
   virtual ~Tickable();

   virtual void tick(float dt) = 0;

private:
   Scene& tickableScene;
};
