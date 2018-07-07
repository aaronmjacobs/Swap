#pragma once

#include "Graphics/Material.h"
#include "Graphics/Mesh.h"

#include <string>
#include <vector>

struct ModelSection
{
   ModelSection(Mesh&& inMesh, Material&& inMaterial)
      : mesh(std::move(inMesh))
      , material(std::move(inMaterial))
   {
   }

   Mesh mesh;
   Material material;
};

class Model
{
public:
   Model() = default;
   Model(const Model& other) = delete;
   Model(Model&& other) = default;
   ~Model() = default;
   Model& operator=(const Model& other) = delete;
   Model& operator=(Model&& other) = default;

   void draw();

   template<typename T>
   void setMaterialParameter(const std::string& name, const T& value)
   {
      for (ModelSection& section : sections)
      {
         section.material.setParameter(name, value);
      }
   }

   void setMaterialParameterEnabled(const std::string& name, bool enabled);

   void addSection(ModelSection&& section);

   const std::vector<ModelSection>& getSections() const
   {
      return sections;
   }

private:
   std::vector<ModelSection> sections;
};
