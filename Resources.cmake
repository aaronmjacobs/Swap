set(SHADER_DIR "${RES_DIR}/Shaders")

set(RESOURCE_FILES
   "${SHADER_DIR}/Blur.frag"
   "${SHADER_DIR}/DeferredLighting.frag"
   "${SHADER_DIR}/DeferredLighting.vert"
   "${SHADER_DIR}/DepthOnly.frag"
   "${SHADER_DIR}/DepthOnly.vert"
   "${SHADER_DIR}/Forward.frag"
   "${SHADER_DIR}/Forward.vert"
   "${SHADER_DIR}/ForwardCommon.glsl"
   "${SHADER_DIR}/FramebufferCommon.glsl"
   "${SHADER_DIR}/GBuffer.frag"
   "${SHADER_DIR}/GBuffer.vert"
   "${SHADER_DIR}/GBufferCommon.glsl"
   "${SHADER_DIR}/LightingCommon.glsl"
   "${SHADER_DIR}/MaterialCommon.glsl"
   "${SHADER_DIR}/MaterialDefines.glsl"
   "${SHADER_DIR}/Normals.frag"
   "${SHADER_DIR}/Normals.vert"
   "${SHADER_DIR}/Screen.vert"
   "${SHADER_DIR}/SSAO.frag"
   "${SHADER_DIR}/SSAOBlur.frag"
   "${SHADER_DIR}/Threshold.frag"
   "${SHADER_DIR}/Tonemap.frag"
   "${SHADER_DIR}/Version.glsl"
   "${SHADER_DIR}/ViewCommon.glsl"
)

target_sources(${PROJECT_NAME} PRIVATE ${RESOURCE_FILES})

source_group(TREE "${RES_DIR}" PREFIX Resources FILES ${RESOURCE_FILES})
