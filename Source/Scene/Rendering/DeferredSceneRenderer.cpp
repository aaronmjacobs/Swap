#include "Scene/Rendering/DeferredSceneRenderer.h"

#include "Core/Assert.h"
#include "Graphics/DrawingContext.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Material.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Texture.h"
#include "Platform/IOUtils.h"
#include "Resources/ResourceManager.h"
#include "Scene/Components/Lights/DirectionalLightComponent.h"
#include "Scene/Components/Lights/PointLightComponent.h"
#include "Scene/Components/Lights/SpotLightComponent.h"
#include "Scene/Components/ModelComponent.h"

#include <vector>

DeferredSceneRenderer::DeferredSceneRenderer(const SPtr<ResourceManager>& inResourceManager)
   : SceneRenderer(inResourceManager, true)
{
   Viewport viewport = GraphicsContext::current().getDefaultViewport();

   {
      static const std::array<Tex::InternalFormat, 6> kColorAttachmentFormats =
      {
         // Position
         Tex::InternalFormat::RGB32F,

         // Normal + shininess
         Tex::InternalFormat::RGBA32F,

         // Albedo
         Tex::InternalFormat::RGBA8,

         // Specular
         Tex::InternalFormat::RGBA8,

         // Emissive
         Tex::InternalFormat::RGB16F,

         // Color
         Tex::InternalFormat::RGB16F
      };

      Fb::Specification specification;
      specification.width = viewport.width;
      specification.height = viewport.height;
      specification.depthStencilType = Fb::DepthStencilType::Depth24Stencil8;
      specification.colorAttachmentFormats = kColorAttachmentFormats;

      Fb::Attachments attachments = Fb::generateAttachments(specification);
      ASSERT(attachments.colorAttachments.size() == kColorAttachmentFormats.size());

      depthStencilTexture = attachments.depthStencilAttachment;
      depthStencilTexture->setLabel("Depth / Stencil");
      positionTexture = attachments.colorAttachments[0];
      positionTexture->setLabel("Position");
      normalShininessTexture = attachments.colorAttachments[1];
      normalShininessTexture->setLabel("Normal / Shininess");
      albedoTexture = attachments.colorAttachments[2];
      albedoTexture->setLabel("Albedo");
      specularTexture = attachments.colorAttachments[3];
      specularTexture->setLabel("Specular");
      emissiveTexture = attachments.colorAttachments[4];
      emissiveTexture->setLabel("Emissive");
      hdrColorTexture = attachments.colorAttachments[5];
      hdrColorTexture->setLabel("HDR Color");

      Fb::Attachments basePassAttachments;
      basePassAttachments.depthStencilAttachment = depthStencilTexture;
      basePassAttachments.colorAttachments.push_back(positionTexture);
      basePassAttachments.colorAttachments.push_back(normalShininessTexture);
      basePassAttachments.colorAttachments.push_back(albedoTexture);
      basePassAttachments.colorAttachments.push_back(specularTexture);
      basePassAttachments.colorAttachments.push_back(emissiveTexture);
      basePassFramebuffer.setAttachments(std::move(basePassAttachments));
      basePassFramebuffer.setLabel("Base Pass Framebuffer");

      Fb::Attachments lightingPassAttachments;
      lightingPassAttachments.colorAttachments.push_back(hdrColorTexture);
      lightingPassFramebuffer.setAttachments(std::move(lightingPassAttachments));
      lightingPassFramebuffer.setLabel("Lighting Pass Framebuffer");
   }

   loadGBufferProgramPermutations();

   {
      std::vector<ShaderSpecification> shaderSpecifications;
      shaderSpecifications.resize(2);
      shaderSpecifications[0].type = ShaderType::Vertex;
      shaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("Shaders/DeferredLighting.vert", shaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("Shaders/DeferredLighting.frag", shaderSpecifications[1].path);

      shaderSpecifications[0].definitions["LIGHT_TYPE"] = "DIRECTIONAL_LIGHT";
      shaderSpecifications[1].definitions["LIGHT_TYPE"] = "DIRECTIONAL_LIGHT";
      directionalLightingProgram = getResourceManager().loadShaderProgram(shaderSpecifications);
      directionalLightingProgram->bindUniformBuffer(GraphicsContext::current().getFramebufferUniformBuffer());
      directionalLightingProgram->bindUniformBuffer(getViewUniformBuffer());

      shaderSpecifications[0].definitions["LIGHT_TYPE"] = "POINT_LIGHT";
      shaderSpecifications[1].definitions["LIGHT_TYPE"] = "POINT_LIGHT";
      pointLightingProgram = getResourceManager().loadShaderProgram(shaderSpecifications);
      pointLightingProgram->bindUniformBuffer(GraphicsContext::current().getFramebufferUniformBuffer());
      pointLightingProgram->bindUniformBuffer(getViewUniformBuffer());

      shaderSpecifications[0].definitions["LIGHT_TYPE"] = "SPOT_LIGHT";
      shaderSpecifications[1].definitions["LIGHT_TYPE"] = "SPOT_LIGHT";
      spotLightingProgram = getResourceManager().loadShaderProgram(shaderSpecifications);
      spotLightingProgram->bindUniformBuffer(GraphicsContext::current().getFramebufferUniformBuffer());
      spotLightingProgram->bindUniformBuffer(getViewUniformBuffer());
   }

   {
      lightingMaterial.setParameter("uPosition", positionTexture);
      lightingMaterial.setParameter("uNormalShininess", normalShininessTexture);
      lightingMaterial.setParameter("uAlbedo", albedoTexture);
      lightingMaterial.setParameter("uSpecular", specularTexture);

      lightingMaterial.setParameter("uAmbientOcclusion", getSSAOTexture());
   }

   {
      ModelSpecification sphereSpecification;
      IOUtils::getAbsoluteResourcePath("Meshes/Sphere.obj", sphereSpecification.path);
      sphereSpecification.normalGenerationMode = NormalGenerationMode::None;
      sphereSpecification.cache = false;
      sphereSpecification.cacheTextures = false;

      sphereMesh = getResourceManager().loadModel(sphereSpecification).getMesh();
   }

   {
      ModelSpecification coneSpecification;
      IOUtils::getAbsoluteResourcePath("Meshes/Cone.obj", coneSpecification.path);
      coneSpecification.normalGenerationMode = NormalGenerationMode::None;
      coneSpecification.cache = false;
      coneSpecification.cacheTextures = false;

      coneMesh = getResourceManager().loadModel(coneSpecification).getMesh();
   }

   setPrePassDepthAttachment(depthStencilTexture);
   setSSAOTextures(nullptr, positionTexture, normalShininessTexture);

   setTranslucencyPassAttachments(depthStencilTexture, hdrColorTexture);

   setTonemapTextures(hdrColorTexture, getBloomPassFramebuffer().getColorAttachment(0));
}

void DeferredSceneRenderer::renderScene(const Scene& scene)
{
   ViewInfo viewInfo;
   if (!getViewInfo(scene, viewInfo))
   {
      return;
   }
   setView(viewInfo);

   SceneRenderInfo sceneRenderInfo = calcSceneRenderInfo(scene, viewInfo, true);
   renderPrePass(sceneRenderInfo);
   renderBasePass(sceneRenderInfo);
   renderSSAOPass(sceneRenderInfo);
   renderShadowMaps(scene, sceneRenderInfo);
   renderLightingPass(sceneRenderInfo);
   renderTranslucencyPass(sceneRenderInfo);
   renderPostProcessPasses(sceneRenderInfo);
}

void DeferredSceneRenderer::onFramebufferSizeChanged(int newWidth, int newHeight)
{
   SceneRenderer::onFramebufferSizeChanged(newWidth, newHeight);

   Viewport viewport = GraphicsContext::current().getDefaultViewport();

   depthStencilTexture->updateResolution(viewport.width, viewport.height);
   positionTexture->updateResolution(viewport.width, viewport.height);
   normalShininessTexture->updateResolution(viewport.width, viewport.height);
   albedoTexture->updateResolution(viewport.width, viewport.height);
   specularTexture->updateResolution(viewport.width, viewport.height);
   emissiveTexture->updateResolution(viewport.width, viewport.height);
   hdrColorTexture->updateResolution(viewport.width, viewport.height);
}

void DeferredSceneRenderer::renderBasePass(const SceneRenderInfo& sceneRenderInfo)
{
   basePassFramebuffer.bind();

   RasterizerState rasterizerState;
   rasterizerState.depthFunc = DepthFunc::LessEqual;
   RasterizerStateScope rasterizerStateScope(rasterizerState);

   glClear(GL_COLOR_BUFFER_BIT);

   for (const ModelRenderInfo& modelRenderInfo : sceneRenderInfo.modelRenderInfo)
   {
      ASSERT(modelRenderInfo.model);

      glm::mat4 localToWorld = modelRenderInfo.localToWorld.toMatrix();
      glm::mat4 localToNormal = glm::transpose(glm::inverse(localToWorld));

      for (std::size_t i = 0; i < modelRenderInfo.model->getNumMeshSections(); ++i)
      {
         const MeshSection& section = modelRenderInfo.model->getMeshSection(i);
         const Material& material = modelRenderInfo.model->getMaterial(i);

         bool visible = i >= modelRenderInfo.visibilityMask.size() || modelRenderInfo.visibilityMask[i];
         if (visible && material.getBlendMode() == BlendMode::Opaque)
         {
            SPtr<ShaderProgram>& gBufferProgramPermutation = selectGBufferPermutation(material);

            gBufferProgramPermutation->setUniformValue(UniformNames::kLocalToWorld, localToWorld);
            gBufferProgramPermutation->setUniformValue(UniformNames::kLocalToNormal, localToNormal, false);

            DrawingContext context(gBufferProgramPermutation.get());
            material.apply(context);
            section.draw(context);
         }
      }
   }
}

void DeferredSceneRenderer::renderLightingPass(const SceneRenderInfo& sceneRenderInfo)
{
   lightingPassFramebuffer.bind();

   // Blit the emissive color
   Framebuffer::blit(basePassFramebuffer, lightingPassFramebuffer, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT0, GL_COLOR_BUFFER_BIT, GL_NEAREST);

   RasterizerState baseRasterizerState;
   baseRasterizerState.enableDepthTest = false;
   baseRasterizerState.enableBlending = true;
   baseRasterizerState.sourceBlendFactor = BlendFactor::One;
   baseRasterizerState.destinationBlendFactor = BlendFactor::One;
   RasterizerStateScope baseRasterierStateScope(baseRasterizerState);

   for (const DirectionalLightRenderInfo& directionalLightRenderInfo : sceneRenderInfo.directionalLights)
   {
      DrawingContext context(directionalLightingProgram.get());

      DirectionalLightUniformData uniformData = directionalLightRenderInfo.getUniformData();

      directionalLightingProgram->setUniformValue("uDirectionalLight.color", uniformData.color);
      directionalLightingProgram->setUniformValue("uDirectionalLight.direction", uniformData.direction);

      directionalLightingProgram->setUniformValue("uDirectionalLight.castShadows", uniformData.castShadows);
      directionalLightingProgram->setUniformValue("uDirectionalLight.worldToShadow", uniformData.worldToShadow);
      directionalLightingProgram->setUniformValue("uDirectionalLight.shadowBias", uniformData.shadowBias);

      const SPtr<Texture>& shadowMap = uniformData.shadowMap ? uniformData.shadowMap : getDummyShadowMap();
      GLint shadowMapTextureUnit = shadowMap->activateAndBind(context);
      directionalLightingProgram->setUniformValue("uDirectionalLight.shadowMap", shadowMapTextureUnit);

      lightingMaterial.apply(context);
      getScreenMesh().draw(context);
   }

   {
      RasterizerState pointAndSpotRasterizerState = baseRasterizerState;
      pointAndSpotRasterizerState.faceCullMode = FaceCullMode::Front;
      RasterizerStateScope pointAndSpotRasterizerStateScope(pointAndSpotRasterizerState);

      for (const PointLightRenderInfo& pointLightRenderInfo : sceneRenderInfo.pointLights)
      {
         DrawingContext context(pointLightingProgram.get());

         PointLightUniformData uniformData = pointLightRenderInfo.getUniformData();

         const PointLightComponent* component = pointLightRenderInfo.component;
         ASSERT(component);
         Transform transform = component->getAbsoluteTransform();
         transform.scale = glm::vec3(uniformData.radius);
         pointLightingProgram->setUniformValue("uLocalToClip", sceneRenderInfo.viewInfo.getWorldToClip() * transform.toMatrix());

         pointLightingProgram->setUniformValue("uPointLight.color", uniformData.color);
         pointLightingProgram->setUniformValue("uPointLight.position", uniformData.position);
         pointLightingProgram->setUniformValue("uPointLight.radius", uniformData.radius);

         pointLightingProgram->setUniformValue("uPointLight.castShadows", uniformData.castShadows);
         pointLightingProgram->setUniformValue("uPointLight.nearFar", uniformData.nearFar);
         pointLightingProgram->setUniformValue("uPointLight.shadowBias", uniformData.shadowBias);

         const SPtr<Texture>& shadowMap = uniformData.shadowMap ? uniformData.shadowMap : getDummyShadowCubeMap();
         GLint shadowMapTextureUnit = shadowMap->activateAndBind(context);
         pointLightingProgram->setUniformValue("uPointLight.shadowMap", shadowMapTextureUnit);

         lightingMaterial.apply(context);
         sphereMesh->draw(context);
      }

      for (const SpotLightRenderInfo& spotLightRenderInfo : sceneRenderInfo.spotLights)
      {
         DrawingContext context(spotLightingProgram.get());

         SpotLightUniformData uniformData = spotLightRenderInfo.getUniformData();

         const SpotLightComponent* component = spotLightRenderInfo.component;
         ASSERT(component);
         Transform transform = component->getAbsoluteTransform();
         float widthScale = glm::tan(uniformData.cutoffAngle) * uniformData.radius * 2.0f;
         transform.scale = glm::vec3(widthScale, widthScale, uniformData.radius);
         spotLightingProgram->setUniformValue("uLocalToClip", sceneRenderInfo.viewInfo.getWorldToClip() * transform.toMatrix());

         spotLightingProgram->setUniformValue("uSpotLight.color", uniformData.color);
         spotLightingProgram->setUniformValue("uSpotLight.direction", uniformData.direction);
         spotLightingProgram->setUniformValue("uSpotLight.position", uniformData.position);
         spotLightingProgram->setUniformValue("uSpotLight.radius", uniformData.radius);
         spotLightingProgram->setUniformValue("uSpotLight.beamAngle", uniformData.beamAngle);
         spotLightingProgram->setUniformValue("uSpotLight.cutoffAngle", uniformData.cutoffAngle);
         spotLightingProgram->setUniformValue("uSpotLight.castShadows", uniformData.castShadows);
         spotLightingProgram->setUniformValue("uSpotLight.worldToShadow", uniformData.worldToShadow);
         spotLightingProgram->setUniformValue("uSpotLight.shadowBias", uniformData.shadowBias);

         const SPtr<Texture>& shadowMap = uniformData.shadowMap ? uniformData.shadowMap : getDummyShadowMap();
         GLint shadowMapTextureUnit = shadowMap->activateAndBind(context);
         spotLightingProgram->setUniformValue("uSpotLight.shadowMap", shadowMapTextureUnit);

         lightingMaterial.apply(context);
         coneMesh->draw(context);
      }
   }
}

void DeferredSceneRenderer::renderPostProcessPasses(const SceneRenderInfo& sceneRenderInfo)
{
   renderBloomPass(sceneRenderInfo, lightingPassFramebuffer, 0);

   Framebuffer::bindDefault();
   renderTonemapPass(sceneRenderInfo);
}

void DeferredSceneRenderer::loadGBufferProgramPermutations()
{
   std::vector<ShaderSpecification> shaderSpecifications;
   shaderSpecifications.resize(2);
   shaderSpecifications[0].type = ShaderType::Vertex;
   shaderSpecifications[1].type = ShaderType::Fragment;
   IOUtils::getAbsoluteResourcePath("Shaders/GBuffer.vert", shaderSpecifications[0].path);
   IOUtils::getAbsoluteResourcePath("Shaders/GBuffer.frag", shaderSpecifications[1].path);

   for (std::size_t i = 0; i < gBufferProgramPermutations.size(); ++i)
   {
      for (ShaderSpecification& shaderSpecification : shaderSpecifications)
      {
         shaderSpecification.definitions["WITH_DIFFUSE_TEXTURE"] = i & 0b001 ? "1" : "0";
         shaderSpecification.definitions["WITH_SPECULAR_TEXTURE"] = i & 0b010 ? "1" : "0";
         shaderSpecification.definitions["WITH_NORMAL_TEXTURE"] = i & 0b100 ? "1" : "0";
      }

      gBufferProgramPermutations[i] = getResourceManager().loadShaderProgram(shaderSpecifications);
      gBufferProgramPermutations[i]->bindUniformBuffer(getViewUniformBuffer());
   }
}

SPtr<ShaderProgram>& DeferredSceneRenderer::selectGBufferPermutation(const Material& material)
{
   int index = material.hasCommonParameter(CommonMaterialParameter::DiffuseTexture) * 0b001
      + material.hasCommonParameter(CommonMaterialParameter::SpecularTexture) * 0b010
      + material.hasCommonParameter(CommonMaterialParameter::NormalTexture) * 0b100;

   return gBufferProgramPermutations[index];
}
