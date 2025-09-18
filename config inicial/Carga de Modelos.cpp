// Std. Includes
#include <string>
#include <iostream>
#include <cstdlib>
#include <cmath>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include "SOIL2/SOIL2.h"
#include "stb_image.h"

// Properties
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main()
{
    // Init GLFW
    if (!glfwInit())
    {
        std::cout << "Failed to init GLFW\n";
        return EXIT_FAILURE;
    }

    // GLFW config
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Carga de modelos y camara sintetica", nullptr, nullptr);
    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // Viewport
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // OpenGL options
    glEnable(GL_DEPTH_TEST);

    // Shaders
    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // Load models
    Model dog((char*)"Models/RedDog.obj");
    Model cat((char*)"Models/miGato.obj");

    // Projection
    glm::mat4 projection = glm::perspective(camera.GetZoom(),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f, 100.0f);

    // --- Uniform locations (una sola vez) ---
    shader.Use();
    GLint uProj = glGetUniformLocation(shader.Program, "projection");
    GLint uView = glGetUniformLocation(shader.Program, "view");
    GLint uModel = glGetUniformLocation(shader.Program, "model");

    glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Time
        GLfloat t = glfwGetTime();
        deltaTime = t - lastFrame;
        lastFrame = t;

        // Input
        glfwPollEvents();
        DoMovement();

        // Clear
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        // View cada frame
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));

        // =========================
        //    PERROS (2 instancias)
        // =========================

        // Perro #1: T * R * S
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.2f, 0.0f, 0.0f));               // posición
            model = glm::rotate(model, glm::radians(fmod(t * 45.0f, 360.0f)),          // 45°/s
                glm::vec3(0.0f, 1.0f, 0.0f));                           // eje Y
            model = glm::scale(model, glm::vec3(0.6f));                                 // escala
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            dog.Draw(shader);
        }

        // Perro #2 (duplicado): otra T/R/S
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.2f, 0.0f, -2.0f));
            model = glm::rotate(model, glm::radians(fmod(t * 90.0f, 360.0f)),          // 90°/s
                glm::vec3(0.0f, 1.0f, 0.3f));                           // eje distinto
            model = glm::scale(model, glm::vec3(0.45f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            dog.Draw(shader);
        }

        // =========================
        //     GATOS (2 instancias)
        // =========================

        // Gato #1
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(1.2f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(fmod(-t * 60.0f, 360.0f)),         // -60°/s
                glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.6f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            cat.Draw(shader);
        }

        // Gato #2 (duplicado)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(1.8f, 0.3f, -1.5f));
            model = glm::rotate(model, glm::radians(fmod(t * 30.0f, 360.0f)),          // 30°/s
                glm::vec3(1.0f, 0.0f, 0.0f));                           // rota sobre X
            model = glm::scale(model, glm::vec3(0.5f, 0.7f, 0.5f));                     // escala no uniforme
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            cat.Draw(shader);
        }

        // Swap
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
    // Camera controls
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
        camera.ProcessKeyboard(FORWARD, deltaTime);

    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
        camera.ProcessKeyboard(BACKWARD, deltaTime);

    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
        camera.ProcessKeyboard(LEFT, deltaTime);

    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = (GLfloat)xPos;
        lastY = (GLfloat)yPos;
        firstMouse = false;
    }

    GLfloat xOffset = (GLfloat)xPos - lastX;
    GLfloat yOffset = lastY - (GLfloat)yPos; // Reversed since y-coordinates go from bottom to top

    lastX = (GLfloat)xPos;
    lastY = (GLfloat)yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}
