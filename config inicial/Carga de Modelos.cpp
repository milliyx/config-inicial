// ===============================
// main.cpp - Gato más pequeño
// ===============================

// Std
#include <string>
#include <iostream>
#include <cmath>

// GLEW / GLFW
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Shaders, Cámara y Modelo (tus headers)
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Otras libs (si tu Shader/Model las usan)
#include "SOIL2/SOIL2.h"
#include "stb_image.h"

// -------------------- Propiedades ventana --------------------
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// -------------------- Prototipos --------------------
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// -------------------- Cámara / Input globals --------------------
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
bool keys[1024] = { false };
GLfloat lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;

glm::vec3 g_catPosition = glm::vec3(1.0f, 0.0f, 0.0f);

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main() {
    // -------------------- GLFW --------------------
    if (!glfwInit()) {
        std::cout << "Failed to init GLFW\n";
        return EXIT_FAILURE;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Control por Teclado", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // -------------------- GLEW --------------------
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialize GLEW\n";
        return EXIT_FAILURE;
    }

    // -------------------- Viewport / estado GL --------------------
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // -------------------- Shaders --------------------
    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // -------------------- Modelos --------------------
    Model dog((char*)"Models/RedDog.obj");
    Model cat((char*)"Models/miGato.obj");

    // -------------------- Matriz de proyección (fija) --------------------
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f, 100.0f
    );

    // Uniform locations
    shader.Use();
    GLint uProj = glGetUniformLocation(shader.Program, "projection");
    GLint uView = glGetUniformLocation(shader.Program, "view");
    GLint uModel = glGetUniformLocation(shader.Program, "model");
    glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));

    // -------------------- Loop principal --------------------
    while (!glfwWindowShouldClose(window)) {
        // Tiempo
        GLfloat t = (GLfloat)glfwGetTime();
        deltaTime = t - lastFrame;
        lastFrame = t;

        // Entrada
        glfwPollEvents();
        DoMovement();

        // Clear
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        // View (por frame)
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));

        // ======================================================
        //      PERRO #1
        // ======================================================
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.5f, 0.0f, -0.5f));
            model = glm::rotate(model, glm::radians(fmod(t * 45.0f, 360.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.60f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            dog.Draw(shader);
        }

        // ======================================================
        //      PERRO #2
        // ======================================================
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-3.0f, 0.0f, -2.0f));
            model = glm::rotate(model, glm::radians(fmod(-t * 70.0f, 360.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.60f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            dog.Draw(shader);
        }

        // ======================================================
        //      GATO (Controlado por Teclado y más pequeño)
        // ======================================================
        {
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, g_catPosition);
            model = glm::rotate(model, glm::radians(fmod(-t * 60.0f, 360.0f)), glm::vec3(0.0f, 1.0f, 0.0f));

            // --- ¡AQUÍ ESTÁ EL CAMBIO! ---
            model = glm::scale(model, glm::vec3(0.15f)); // Ahora es más pequeño

            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            cat.Draw(shader);
        }

        // Swap
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// -------------------- Movimiento con teclado --------------------
void DoMovement() {
    // --- Controles de la CÁMARA (W, A, S, D) ---
    if (keys[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime);

    // --- Controles del GATO (I, J, K, L) ---
    const float catSpeed = 2.5f;
    if (keys[GLFW_KEY_I]) g_catPosition.y += catSpeed * deltaTime;
    if (keys[GLFW_KEY_K]) g_catPosition.y -= catSpeed * deltaTime;
    if (keys[GLFW_KEY_J]) g_catPosition.x -= catSpeed * deltaTime;
    if (keys[GLFW_KEY_L]) g_catPosition.x += catSpeed * deltaTime;
}

// -------------------- Callback teclado --------------------
void KeyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
}

// -------------------- Callback mouse (solo para la cámara) --------------------
void MouseCallback(GLFWwindow*, double xPos, double yPos) {
    if (firstMouse) {
        lastX = (GLfloat)xPos;
        lastY = (GLfloat)yPos;
        firstMouse = false;
    }
    GLfloat xOffset = (GLfloat)xPos - lastX;
    GLfloat yOffset = lastY - (GLfloat)yPos;
    lastX = (GLfloat)xPos;
    lastY = (GLfloat)yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}