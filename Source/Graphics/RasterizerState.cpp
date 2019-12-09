#include "Graphics/GraphicsContext.h"
#include "Graphics/RasterizerState.h"

RasterizerStateScope::RasterizerStateScope(const RasterizerState& state)
{
   GraphicsContext::current().pushRasterizerState(state);
}

RasterizerStateScope::~RasterizerStateScope()
{
   GraphicsContext::current().popRasterizerState();
}
