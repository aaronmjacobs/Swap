#include "Version.glsl"

#include "LightingCommon.glsl"

layout(location = 0) in vec3 aPosition;

#if LIGHT_TYPE == POINT_LIGHT || LIGHT_TYPE == SPOT_LIGHT
uniform mat4 uModelViewProjectionMatrix;
#endif

void main()
{
#if LIGHT_TYPE == DIRECTIONAL_LIGHT
   gl_Position = vec4(aPosition, 1.0);
#elif LIGHT_TYPE == POINT_LIGHT || LIGHT_TYPE == SPOT_LIGHT
   gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0);
#endif
}
