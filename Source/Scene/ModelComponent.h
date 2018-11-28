#pragma once

#include "Scene/SceneComponent.h"

#include "Core/Pointers.h"
#include "Graphics/Model.h"

class ModelComponent : public SceneComponent
{
protected:
   friend class ComponentRegistrar<ModelComponent>;

   ModelComponent(Entity& owningEntity);

public:
   ~ModelComponent();

   const SPtr<Model>& getModel() const
   {
      return model;
   }

   void setModel(const SPtr<Model>& newModel)
   {
      model = newModel;
   }

private:
   SPtr<Model> model;
};

SWAP_REFERENCE_COMPONENT(ModelComponent)
