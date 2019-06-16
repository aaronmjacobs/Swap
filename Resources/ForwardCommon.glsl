#include "Version.glsl"
#include "MaterialDefines.glsl"

#define VARYING_NORMAL !WITH_NORMAL_TEXTURE
#define VARYING_TEX_COORD (WITH_DIFFUSE_TEXTURE || WITH_SPECULAR_TEXTURE || WITH_NORMAL_TEXTURE)
#define VARYING_TBN WITH_NORMAL_TEXTURE
