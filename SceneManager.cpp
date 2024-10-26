///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp
// ============
// Manages the loading and rendering of 3D scenes with enhanced lighting and textures
//
// AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// Load texture function
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "Texture loaded successfully from: " << path << std::endl;
    }
    else {
        std::cout << "Failed to load texture from: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// Shader uniform variable names
const char* g_ModelName = "model";
const char* g_ColorValueName = "objectColor";
const char* g_TextureValueName = "objectTexture";
const char* g_UseTextureName = "bUseTexture";
const char* g_ViewName = "viewMatrix";
const char* g_ProjectionName = "projectionMatrix";
const char* g_LightDirection = "lightDirection";
const char* g_LightColor = "lightColor";
const char* g_CameraPosition = "cameraPos";
const char* g_SpecularStrength = "specularStrength";

// Secondary point light (to avoid shadows)
const char* g_PointLightPosition = "pointLight.position";
const char* g_PointLightColor = "pointLight.color";
const char* g_PointLightIntensity = "pointLight.intensity";

unsigned int cupTexture, handleTexture, lampPostTexture, lampShadeTexture, lensTexture, notebookTexture, armTexture, pencilTexture, pencilHolderTexture, lampBaseTexture, bridgeTexture;

SceneManager::SceneManager(ShaderManager* pShaderManager) {
    m_pShaderManager = pShaderManager;  // Ensure this is properly initialized
    m_basicMeshes = new ShapeMeshes();
}

SceneManager::~SceneManager() {
    delete m_pShaderManager;
    delete m_basicMeshes;
}

/***********************************************************
 *  PrepareScene()
 *
 *  This function loads the 3D objects for the scene,
 *  including the coffee cup, notebook, and pencils.
 ***********************************************************/
void SceneManager::PrepareScene() {
    // Load the plane (ground) mesh
    m_basicMeshes->LoadPlaneMesh();

    // Load the coffee cup body (cylinder) and handle (torus)
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadTorusMesh();

    // Load the notebook (box)
    m_basicMeshes->LoadBoxMesh();

    // Load the pencils (cylinders)
    m_basicMeshes->LoadCylinderMesh();

    // Load the cone mesh for the lamp shade
    m_basicMeshes->LoadConeMesh();

    // Load textures for the cup and handle (these remain the same)
    cupTexture = loadTexture("Textures/TCom_RoughCeramic_header.jpg");
    handleTexture = loadTexture("Textures/TCom_Plastic_Scratched_header.jpg");

    // Load textures for the new elements
    lampPostTexture = loadTexture("Textures/TCom_BrushedStainlessSteel_header.jpg");
    lampShadeTexture = loadTexture("Textures/TCom_Various_ReflectiveTape_header4.jpg");
    lensTexture = loadTexture("Textures/TCom_RetroStainlessSheet_header.jpg");
    notebookTexture = loadTexture("Textures/TCom_Leather_Plain08_header.jpg");
    armTexture = loadTexture("Textures/TCom_BrushedStainlessSteel_header.jpg");
    pencilTexture = loadTexture("Textures/TCom_Leather_Italian_header.jpg");
    pencilHolderTexture = loadTexture("Textures/TCom_Leather_Italian_header.jpg");
    lampBaseTexture = loadTexture("Textures/TCom_BrushedStainlessSteel_header.jpg");
    bridgeTexture = loadTexture("Textures/TCom_RetroStainlessSheet_header.jpg");

    // Debug: Check if the textures were loaded successfully
    if (cupTexture == 0) {
        std::cout << "Error loading cup texture!" << std::endl;
    }
    if (handleTexture == 0) {
        std::cout << "Error loading handle texture!" << std::endl;
    }
}

/***********************************************************
 *  RenderScene()
 *
 *  This function renders the loaded 3D objects in the scene,
 *  including the coffee cup, notebook, pencils, pencil holder, and plane.
 ***********************************************************/
void SceneManager::RenderScene() {
    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);
    // Set the background color and clear buffers
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the camera view and projection matrices
    glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 10.0f);
    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    // Pass the view and projection matrices to the shader
    m_pShaderManager->setMat4Value(g_ViewName, view);
    m_pShaderManager->setMat4Value(g_ProjectionName, projection);

    // Set up the lighting directly here
    glm::vec3 lightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);  // Light direction
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);         // White light
    m_pShaderManager->setVec3Value(g_LightDirection, lightDirection);
    m_pShaderManager->setVec3Value(g_LightColor, lightColor);

    // Set up secondary point light
    glm::vec3 pointLightPosition = glm::vec3(2.0f, 2.0f, 2.0f);  // Point light position
    glm::vec3 pointLightColor = glm::vec3(0.8f, 0.8f, 0.8f);     // Slightly dimmer white light
    m_pShaderManager->setVec3Value(g_PointLightPosition, pointLightPosition);
    m_pShaderManager->setVec3Value(g_PointLightColor, pointLightColor);
    m_pShaderManager->setFloatValue(g_PointLightIntensity, 1.0f);

    // Draw the plane (ground) with reflection
    glm::vec3 planeScale = glm::vec3(10.0f, 1.0f, 10.0f);  // Scale the plane to cover a large area
    glm::vec3 planePosition = glm::vec3(0.0f, 0.0f, 0.0f); // Position it at the origin
    SetTransformations(planeScale, 0, 0, 0, planePosition);
    SetShaderColor(0.5f, 0.5f, 0.5f, 1.0f);  // Set the plane color to grey
    m_pShaderManager->setFloatValue(g_SpecularStrength, 0.6f);  // Add specular reflection for Phong lighting
    m_basicMeshes->DrawPlaneMesh();  // Render the plane

    // Draw the coffee cup body (cylinder) with texture (unchanged)
    glm::vec3 cupScale = glm::vec3(1.0f, 1.5f, 1.0f);
    glm::vec3 cupPosition = glm::vec3(0.0f, 0.0f, 0.0f);  // Lowered to rest directly on the platform
    SetTransformations(cupScale, 0, 0, 0, cupPosition);
    glBindTexture(GL_TEXTURE_2D, cupTexture);  // Bind the cup texture
    m_pShaderManager->setIntValue(g_UseTextureName, 1); // Enable texture for the cup
    m_basicMeshes->DrawCylinderMesh();

    // Draw the coffee cup handle (torus) with texture (unchanged)
    glm::vec3 handleScale = glm::vec3(0.3f, 0.3f, 0.3f);  // Proper scale for the handle
    glm::vec3 handlePosition = glm::vec3(1.0f, 0.375f, 0.0f);  // Closer to the cup
    SetTransformations(handleScale, 0, 0, 90, handlePosition);  // Rotated to ensure it sits vertically and arches out
    glBindTexture(GL_TEXTURE_2D, handleTexture);  // Bind the handle texture
    m_pShaderManager->setIntValue(g_UseTextureName, 1);  // Enable texture for the handle
    m_basicMeshes->DrawTorusMesh();  // Render the handle

    // Draw the notebook with leather texture
    glm::vec3 notebookScale = glm::vec3(2.0f, 0.1f, 3.0f);  // Thin box for the notebook
    glm::vec3 notebookPosition = glm::vec3(-2.0f, 0.05f, 1.5f);  // Position it near the cup
    SetTransformations(notebookScale, 0, 0, 0, notebookPosition);
    glBindTexture(GL_TEXTURE_2D, notebookTexture);  // Bind the notebook leather texture
    m_pShaderManager->setIntValue(g_UseTextureName, 1);  // Enable texture for the notebook
    m_basicMeshes->DrawBoxMesh();  // Render the notebook

    // Draw the lamp post with stainless steel texture
    glm::vec3 lampPostScale = glm::vec3(0.15f, 4.0f, 0.15f);  // Taller and thicker cylinder for the lamp post
    glm::vec3 lampPostPosition = glm::vec3(2.5f, 0.0f, -2.0f);  // Keep the same position on the table
    SetTransformations(lampPostScale, 0, 0, 0, lampPostPosition);
    glBindTexture(GL_TEXTURE_2D, lampPostTexture);  // Bind the lamp post texture
    m_pShaderManager->setIntValue(g_UseTextureName, 1);  // Enable texture for the lamp post
    m_basicMeshes->DrawCylinderMesh();  // Render the lamp post

    // Draw the lamp shade with reflective tape texture
    glm::vec3 lampShadeScale = glm::vec3(1.0f, 1.0f, 1.0f);  // Larger cone for the lamp shade
    glm::vec3 lampShadePosition = glm::vec3(2.5f, 4.0f, -2.0f);  // Adjust position to sit right on top of the taller post
    SetTransformations(lampShadeScale, 0, 0, 0, lampShadePosition);
    glBindTexture(GL_TEXTURE_2D, lampShadeTexture);  // Bind the lamp shade texture
    m_pShaderManager->setIntValue(g_UseTextureName, 1);  // Enable texture for the lamp shade
    m_basicMeshes->DrawConeMesh();  // Render the lamp shade

    // Draw the lamp base with stainless steel texture
    glm::vec3 lampBaseScale = glm::vec3(1.0f, 0.1f, 1.0f);  // Wider and flat cylinder for the base
    glm::vec3 lampBasePosition = glm::vec3(2.5f, -0.05f, -2.0f);  // Positioned slightly below the table surface
    SetTransformations(lampBaseScale, 0, 0, 0, lampBasePosition);
    glBindTexture(GL_TEXTURE_2D, lampBaseTexture);  // Bind the lamp base texture
    m_basicMeshes->DrawCylinderMesh();  // Render the lamp base

    // Draw the lenses with retro stainless steel texture
    glm::vec3 lensScale = glm::vec3(0.5f, 0.05f, 0.5f); // Scale for the lenses, keeping them thin and wide
    glm::vec3 lens1Position = glm::vec3(2.5f, 0.5f, 0.0f); // Increased y-value to fully lift it off the table
    SetTransformations(lensScale, 90, 0, 0, lens1Position); // Rotate 90 degrees to stand the lens vertically
    glBindTexture(GL_TEXTURE_2D, lensTexture);  // Bind the lens texture
    m_pShaderManager->setIntValue(g_UseTextureName, 1);  // Enable texture for the lenses
    m_basicMeshes->DrawCylinderMesh(); // Draw the first lens

    glm::vec3 lens2Position = glm::vec3(3.6f, 0.5f, 0.0f); // Same height adjustment for the second lens
    SetTransformations(lensScale, 90, 0, 0, lens2Position); // Same rotation and scale as the first lens
    glBindTexture(GL_TEXTURE_2D, lensTexture);  // Bind the lens texture again
    m_basicMeshes->DrawCylinderMesh(); // Draw the second lens

    // Draw the bridge between lenses with retro stainless steel texture
    glm::vec3 bridgeScale = glm::vec3(0.1f, 0.05f, 0.3f); // Make the bridge thinner and shorter
    glm::vec3 bridgePosition = glm::vec3(3.05f, 0.52f, 0.0f); // Slightly raise the position and align between lenses
    SetTransformations(bridgeScale, 0, 90, 0, bridgePosition); // Rotate to align horizontally
    glBindTexture(GL_TEXTURE_2D, bridgeTexture);  // Bind the bridge texture
    m_pShaderManager->setIntValue(g_UseTextureName, 1);  // Enable texture for the bridge
    m_basicMeshes->DrawCylinderMesh(); // Draw the bridge

    // Draw the arms for the lenses with stainless steel texture
    glm::vec3 arm1Scale = glm::vec3(0.05f, 0.05f, 0.7f); // Same size for the arm
    glm::vec3 arm1Position = glm::vec3(2.05f, 0.3f, -0.6f); // Moved backward slightly more
    SetTransformations(arm1Scale, 0, 0, 10, arm1Position); // Rotate slightly backward
    glBindTexture(GL_TEXTURE_2D, armTexture);  // Bind the arm texture
    m_basicMeshes->DrawCylinderMesh(); // Draw left arm

    glm::vec3 arm2Position = glm::vec3(4.1f, 0.3f, -0.6f); // Slightly further to the right
    SetTransformations(arm1Scale, 0, 0, -10, arm2Position); // Keep the same slight backward tilt
    m_basicMeshes->DrawCylinderMesh(); // Draw the right arm

    // Draw the pencil holder with leather texture
    glm::vec3 holderScale = glm::vec3(0.2f, 0.6f, 0.2f);  // Small cylinder for the pencil holder
    glm::vec3 holderPosition = glm::vec3(-2.5f, 0.0f, 2.0f);  // Adjusted to sit on the platform
    SetTransformations(holderScale, 0, 0, 0, holderPosition);
    glBindTexture(GL_TEXTURE_2D, pencilHolderTexture);  // Bind the pencil holder texture
    m_pShaderManager->setIntValue(g_UseTextureName, 1);  // Enable texture for the pencil holder
    m_basicMeshes->DrawCylinderMesh();  // Render the pencil holder

    // Draw the first pencil inside the holder
    glm::vec3 pencilScale = glm::vec3(0.05f, 0.8f, 0.05f);  // Shorter to fit inside the holder
    glm::vec3 pencilPosition = glm::vec3(-2.5f, 0.6f, 2.0f);  // Lowered position inside the holder
    SetTransformations(pencilScale, 0, 0, 0, pencilPosition);
    glBindTexture(GL_TEXTURE_2D, pencilTexture);  // Bind the pencil texture
    m_basicMeshes->DrawCylinderMesh();  // Render the first pencil

    // Draw the second pencil inside the holder
    glm::vec3 pencil2Position = glm::vec3(-2.45f, 0.6f, 2.05f);  // Slightly offset position, lowered
    SetTransformations(pencilScale, 0, 0, 0, pencil2Position);
    m_basicMeshes->DrawCylinderMesh();  // Render the second pencil
}

/***********************************************************
 *  SetTransformations()
 *
 *  This function applies transformations (translation, rotation,
 *  scaling) to the 3D objects in the scene.
 ***********************************************************/
void SceneManager::SetTransformations(glm::vec3 scale, float rotX, float rotY, float rotZ, glm::vec3 pos) {
    glm::mat4 model = glm::translate(pos) * glm::rotate(glm::radians(rotX), glm::vec3(1, 0, 0)) *
        glm::rotate(glm::radians(rotY), glm::vec3(0, 1, 0)) *
        glm::rotate(glm::radians(rotZ), glm::vec3(0, 0, 1)) * glm::scale(scale);
    m_pShaderManager->setMat4Value(g_ModelName, model);
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This function sets the color values into the shader.
 ***********************************************************/
void SceneManager::SetShaderColor(float r, float g, float b, float a) {
    m_pShaderManager->setVec4Value(g_ColorValueName, glm::vec4(r, g, b, a));
}
