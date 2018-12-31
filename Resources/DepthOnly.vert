#include "Version.glsl"

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

layout(location = 0) in vec3 aPosition;

void main()
{
   vec4 worldPosition = uModelMatrix * vec4(aPosition, 1.0);
   gl_Position = uProjectionMatrix * uViewMatrix * worldPosition;
}
