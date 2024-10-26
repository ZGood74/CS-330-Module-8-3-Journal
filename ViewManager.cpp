///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// Manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Namespace for declaring global variables
namespace {
    // Window width and height
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;

    // Camera object for interacting with the scene
    Camera* g_pCamera = nullptr;

    // Variables for mouse movement processing
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // Time between current frame and last frame
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    // Track if orthographic projection is active
    bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(ShaderManager* pShaderManager) {
    // Initialize the member variables
    m_pShaderManager = pShaderManager;
    m_pWindow = NULL;
    g_pCamera = new Camera();
    // Set default camera position and orientation
    g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
    g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
    g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    g_pCamera->Zoom = 80;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager() {
    // Free allocated memory
    if (g_pCamera != NULL) {
        delete g_pCamera;
        g_pCamera = NULL;
    }
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle) {
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle, NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

    // Capture all mouse events
    glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

    // Enable blending for transparent rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWindow = window;
    return window;
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue. Handles camera
 *  movement (WASD, QE) and projection toggling (P/O).
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents() {
    // Close window if ESC key is pressed
    if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_pWindow, true);
    }

    // Toggle between perspective and orthographic projection
    if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
        bOrthographicProjection = false;  // Perspective view
    if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
        bOrthographicProjection = true;   // Orthographic view

    // Camera movement with WASD keys
    float cameraSpeed = 2.5f * gDeltaTime; // Adjust speed based on deltaTime
    if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
        g_pCamera->Position += cameraSpeed * g_pCamera->Front;
    if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
        g_pCamera->Position -= cameraSpeed * g_pCamera->Front;
    if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
        g_pCamera->Position -= glm::normalize(glm::cross(g_pCamera->Front, g_pCamera->Up)) * cameraSpeed;
    if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
        g_pCamera->Position += glm::normalize(glm::cross(g_pCamera->Front, g_pCamera->Up)) * cameraSpeed;
    if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
        g_pCamera->Position.y += cameraSpeed; // Move up
    if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
        g_pCamera->Position.y -= cameraSpeed; // Move down
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering.
 ***********************************************************/
void ViewManager::PrepareSceneView() {
    glm::mat4 view;
    glm::mat4 projection;

    // Per-frame timing
    float currentFrame = glfwGetTime();
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;

    // Process any keyboard events that may be waiting in the event queue
    ProcessKeyboardEvents();

    // Get the current view matrix from the camera
    view = g_pCamera->GetViewMatrix();

    // Define the current projection matrix
    if (bOrthographicProjection) {
        projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f); // Orthographic
    }
    else {
        projection = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f); // Perspective
    }

    // Set the view and projection matrices into the shader
    if (NULL != m_pShaderManager) {
        m_pShaderManager->setMat4Value("view", view);
        m_pShaderManager->setMat4Value("projection", projection);
        m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
    }
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 *  It updates the camera direction based on mouse movement.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xpos, double ypos) {
    if (gFirstMouse) {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // Reversed since y-coordinates go from bottom to top
    gLastX = xpos;
    gLastY = ypos;

    float sensitivity = 0.1f; // Adjust the sensitivity if needed
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    g_pCamera->Yaw += xoffset;
    g_pCamera->Pitch += yoffset;

    // Constrain the pitch to avoid screen flip
    if (g_pCamera->Pitch > 89.0f)
        g_pCamera->Pitch = 89.0f;
    if (g_pCamera->Pitch < -89.0f)
        g_pCamera->Pitch = -89.0f;

    // Update camera front vector based on yaw and pitch
    glm::vec3 front;
    front.x = cos(glm::radians(g_pCamera->Yaw)) * cos(glm::radians(g_pCamera->Pitch));
    front.y = sin(glm::radians(g_pCamera->Pitch));
    front.z = sin(glm::radians(g_pCamera->Yaw)) * cos(glm::radians(g_pCamera->Pitch));
    g_pCamera->Front = glm::normalize(front);
}
