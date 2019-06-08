#include "Graphics/Model.h"

#include "Graphics/DrawingContext.h"
#include "Graphics/Mesh.h"

void Model::draw(DrawingContext& context, bool applyMaterials) const
{
   ASSERT(mesh);
   ASSERT(mesh->getSections().size() == materials.size());

   for (std::size_t i = 0; i < mesh->getSections().size(); ++i)
   {
      DrawingContext localContext = context;
      if (applyMaterials)
      {
         materials[i].apply(localContext);
      }

      mesh->getSections()[i].draw(localContext);
   }
}

void Model::setMesh(SPtr<Mesh> newMesh, std::vector<Material> newMaterials)
{
   ASSERT((!newMesh && newMaterials.size() == 0) || (newMesh->getSections().size() == newMaterials.size()));

   mesh = std::move(newMesh);
   materials = std::move(newMaterials);
}

void Model::setMesh(SPtr<Mesh> newMesh)
{
   std::size_t numMaterials = newMesh ? newMesh->getSections().size() : 0;
   setMesh(std::move(newMesh), std::vector<Material>(numMaterials));
}

void Model::setMaterialParameterEnabled(const std::string& name, bool enabled)
{
   for (Material& material : materials)
   {
      material.setParameterEnabled(name, enabled);
   }
}
