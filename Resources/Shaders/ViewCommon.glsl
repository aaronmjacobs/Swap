#include "Version.glsl"

layout(std140) uniform View
{
   mat4 uWorldToView;
   mat4 uViewToWorld;

   mat4 uViewToClip;
   mat4 uClipToView;

   mat4 uWorldToClip;
   mat4 uClipToWorld;

   vec3 uCameraPosition;
};
