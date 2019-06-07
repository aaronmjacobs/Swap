#include "Graphics/Model.h"

#include "Graphics/DrawingContext.h"
#include "Graphics/ShaderProgram.h"

void ModelSection::draw(DrawingContext& context, bool applyMaterial)
{
   ASSERT(context.program);

   DrawingContext localContext = context;
   if (applyMaterial)
   {
      material.apply(localContext);
   }

   localContext.program->commit();

   mesh.draw();
}

void Model::draw(DrawingContext& context, bool applyMaterials)
{
   for (ModelSection& section : sections)
   {
      section.draw(context, applyMaterials);
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
