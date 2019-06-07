#pragma once

#include "Graphics/Material.h"
#include "Graphics/Mesh.h"

#include <string>
#include <vector>

struct DrawingContext;

struct ModelSection
{
   ModelSection(Mesh&& inMesh, Material&& inMaterial)
      : mesh(std::move(inMesh))
      , material(std::move(inMaterial))
   {
   }

   void draw(DrawingContext& context, bool applyMaterial);

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

   void draw(DrawingContext& context, bool applyMaterials);

   template<typename T>
   bool setMaterialParameter(const std::string& name, const T& value)
   {
      bool success = true;

      for (ModelSection& section : sections)
      {
         success &= section.material.setParameter(name, value);
      }

      return success;
   }

   void setMaterialParameterEnabled(const std::string& name, bool enabled);

   void addSection(ModelSection&& section);

   std::vector<ModelSection>& getSections()
   {
      return sections;
   }

   const std::vector<ModelSection>& getSections() const
   {
      return sections;
   }

private:
   std::vector<ModelSection> sections;
};
