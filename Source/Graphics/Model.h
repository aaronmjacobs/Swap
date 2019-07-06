#pragma once

#include "Graphics/Material.h"
#include "Graphics/Mesh.h"

#include <string>
#include <vector>

struct DrawingContext;

class Model
{
public:
   void draw(DrawingContext& context, bool applyMaterials) const;

   void setMesh(SPtr<Mesh> newMesh, std::vector<Material> newMaterials);
   void setMesh(SPtr<Mesh> newMesh);

   template<typename T>
   bool setMaterialParameter(const std::string& name, const T& value)
   {
      bool success = true;

      for (Material& material : materials)
      {
         success &= material.setParameter(name, value);
      }

      return success;
   }

   void setMaterialParameterEnabled(const std::string& name, bool enabled);

   const SPtr<Mesh>& getMesh() const
   {
      return mesh;
   }

   const std::vector<Material>& getMaterials() const
   {
      return materials;
   }

   std::size_t getNumMeshSections() const
   {
      return materials.size();
   }

   MeshSection& getMeshSection(std::size_t index)
   {
      ASSERT(mesh);
      ASSERT(index < mesh->getSections().size());

      return mesh->getSections()[index];
   }

   const MeshSection& getMeshSection(std::size_t index) const
   {
      ASSERT(mesh);
      ASSERT(index < mesh->getSections().size());

      return mesh->getSections()[index];
   }

   const Material& getMaterial(std::size_t index) const
   {
      ASSERT(index < materials.size());

      return materials[index];
   }

private:
   SPtr<Mesh> mesh;
   std::vector<Material> materials;
};
