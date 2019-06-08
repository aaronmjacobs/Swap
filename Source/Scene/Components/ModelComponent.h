#pragma once

#include "Scene/Components/SceneComponent.h"

#include "Core/Pointers.h"
#include "Graphics/Model.h"

class ModelComponent : public SceneComponent
{
protected:
   friend class ComponentRegistrar<ModelComponent>;

   ModelComponent(Entity& owningEntity);

public:
   ~ModelComponent();

   const Model& getModel() const
   {
      return model;
   }

   void setModel(Model newModel)
   {
      model = std::move(newModel);
   }

private:
   Model model;
};

SWAP_REFERENCE_COMPONENT(ModelComponent)
