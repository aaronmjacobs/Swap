#include "Version.glsl"

#include "ForwardCommon.glsl"
#include "ViewCommon.glsl"

uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

#if VARYING_TEX_COORD
layout(location = 2) in vec2 aTexCoord;
#endif

#if VARYING_TBN
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;
#endif

#if VARYING_NORMAL
out vec3 vNormal;
#endif

#if VARYING_TEX_COORD
out vec2 vTexCoord;
#endif

#if VARYING_TBN
out mat3 vTBN;
#endif

void main()
{
   vec4 worldPosition = uModelMatrix * vec4(aPosition, 1.0);

#if VARYING_NORMAL
   vNormal = (uNormalMatrix * vec4(aNormal, 1.0)).xyz;
#endif

#if VARYING_TEX_COORD
   vTexCoord = aTexCoord;
#endif

#if VARYING_TBN
   vec3 t = normalize(vec3(uModelMatrix * vec4(aTangent, 0.0)));
   vec3 b = normalize(vec3(uModelMatrix * vec4(aBitangent, 0.0)));
   vec3 n = normalize(vec3(uModelMatrix * vec4(aNormal, 0.0)));
   vTBN = mat3(t, b, n);
#endif

   gl_Position = uWorldToClip * worldPosition;
}
