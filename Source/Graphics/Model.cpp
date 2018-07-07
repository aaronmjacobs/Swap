#include "Graphics/Model.h"

void Model::draw()
{
   for (ModelSection& section : sections)
   {
      section.material.commit();
      section.mesh.draw();
   }
}

void Model::setMaterialParameterEnabled(const std::string& name, bool enabled)
{
   for (ModelSection& section : sections)
   {
      section.material.setParameterEnabled(name, enabled);
   }
}

void Model::addSection(ModelSection&& section)
{
   sections.emplace_back(std::move(section));
}
