// ======================================================================
// LIBRERÍAS Y CONFIGURACIÓN INICIAL
// ======================================================================

#include <iostream>
#include <cmath>
#include <GL/glew.h>           // Extensiones de OpenGL
#include <GLFW/glfw3.h>        // Creación de ventanas y manejo de eventos
#include "stb_image.h"         // Carga de imágenes (texturas)
#include <glm/glm.hpp>         // Librería para operaciones con vectores y matrices
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"       // Alternativa para carga de imágenes
#include "Shader.h"            // Clase para manejar los shaders
#include "Camera.h"            // Clase de cámara para navegación 3D
#include "Model.h"             // Clase que carga y renderiza modelos OBJ

// ======================================================================
// DECLARACIÓN DE FUNCIONES
// ======================================================================
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void Animation();

// ======================================================================
// VARIABLES GLOBALES
// ======================================================================
const GLuint WIDTH = 800, HEIGHT = 600; // Tamaño de la ventana
int SCREEN_WIDTH, SCREEN_HEIGHT;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f)); // Posición inicial de la cámara
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];       // Arreglo para registrar teclas presionadas
bool firstMouse = true;

// Iluminación
glm::vec3 lightPos(0.0f, 0.0f, 0.0f); // Posición de la luz
bool active;                           // Bandera para activar/desactivar luz

// Posiciones de las luces puntuales
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.0f,2.0f, 0.0f),
    glm::vec3(0.0f,0.0f, 0.0f),
    glm::vec3(0.0f,0.0f, 0.0f),
    glm::vec3(0.0f,0.0f, 0.0f)
};

// ======================================================================
// VÉRTICES DE UN CUBO CON NORMALES
// ======================================================================
// Cada grupo de 6 valores: 3 de posición (x, y, z) y 3 de normales (nx, ny, nz)
float vertices[] = {
    // Cara trasera
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    // ... (resto de las caras omitidas por brevedad)
};

// ======================================================================
// VARIABLES DE ANIMACIÓN
// ======================================================================
glm::vec3 Light1 = glm::vec3(0);
float rotBall = 0.0f;     // Rotación de la pelota
bool AnimBall = false;    // Control de animación de la pelota
bool AnimDog = false;     // Control de animación del perro
float rotDog = 0.0f;      // Rotación general del perro
int dogAnim = 0;          // Dirección del movimiento
float FLegs = 0.0f;       // Patas delanteras
float RLegs = 0.0f;       // Patas traseras
float head = 0.0f;        // Cabeza
float tail = 0.0f;        // Cola
glm::vec3 dogPos(0.0f);   // Posición del perro
bool step = false;

// Control de tiempo entre frames
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// ======================================================================
// FUNCIÓN PRINCIPAL
// ======================================================================
int main()
{
    // Inicialización de GLFW (ventana y contexto OpenGL)
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Animacion maquina de estados", nullptr, nullptr);
    if (!window) {
        std::cout << "Error al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Callbacks para teclado y ratón
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    // Inicializa GLEW (funciones modernas de OpenGL)
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        std::cout << "Error al inicializar GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // Configuración del área de renderizado
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // ==================================================================
    // CARGA DE SHADERS Y MODELOS
    // ==================================================================
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

    // Modelos del perro y escenario
    Model DogBody((char*)"Models/DogBody.obj");
    Model HeadDog((char*)"Models/HeadDog.obj");
    Model DogTail((char*)"Models/TailDog.obj");
    Model F_RightLeg((char*)"Models/F_RightLegDog.obj");
    Model F_LeftLeg((char*)"Models/F_LeftLegDog.obj");
    Model B_RightLeg((char*)"Models/B_RightLegDog.obj");
    Model B_LeftLeg((char*)"Models/B_LeftLegDog.obj");
    Model Piso((char*)"Models/piso.obj");
    Model Ball((char*)"Models/ball.obj");

    // ==================================================================
    // CONFIGURACIÓN DE LOS BUFFERS PARA DIBUJAR
    // ==================================================================
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Atributos de los vértices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); // posición
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // normales
    glEnableVertexAttribArray(1);

    // ==================================================================
    // CICLO PRINCIPAL DE RENDERIZADO
    // ==================================================================
    glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(window))
    {
        // Calcular tiempo entre frames
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();  // Procesar entradas
        DoMovement();      // Movimiento de cámara
        Animation();       // Actualización de animaciones

        // Limpieza del frame anterior
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // ==================================================================
        // CONFIGURACIÓN DE ILUMINACIÓN Y CÁMARA
        // ==================================================================
        lightingShader.Use();

        // Luz direccional
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.6f, 0.6f, 0.6f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.6f, 0.6f, 0.6f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.3f, 0.3f, 0.3f);

        // Luz puntual animada
        glm::vec3 lightColor;
        lightColor.x = abs(sin(glfwGetTime() * Light1.x));
        lightColor.y = abs(sin(glfwGetTime() * Light1.y));
        lightColor.z = sin(glfwGetTime() * Light1.z);

        // Posición de la cámara para el shader
        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // ==================================================================
        // RENDERIZADO DE MODELOS (SUELO, PERRO, PELOTA)
        // ==================================================================
        glm::mat4 model(1.0f);
        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");

        // Suelo
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);

        // Perro: cuerpo principal
        model = glm::translate(model, dogPos);
        model = glm::rotate(model, glm::radians(dogRot), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        DogBody.Draw(lightingShader);

        // (Cabeza, cola, patas... cada parte se transforma individualmente)
        // ...

        // Pelota (con transparencia activada)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        model = glm::rotate(glm::mat4(1.0f), glm::radians(rotBall), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Ball.Draw(lightingShader);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window); // Intercambia buffers para mostrar el frame actual
    }

    glfwTerminate(); // Libera recursos al cerrar la ventana
    return 0;
}

// ======================================================================
// FUNCIONES DE MOVIMIENTO Y ANIMACIÓN
// ======================================================================

// Movimiento de cámara con teclado
void DoMovement() {
    if (keys[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Teclado: activa animaciones o luces
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
        keys[key] = (action == GLFW_PRESS);

    // Alterna luz
    if (keys[GLFW_KEY_SPACE]) {
        active = !active;
        Light1 = active ? glm::vec3(0.2f, 0.8f, 1.0f) : glm::vec3(0.0f);
    }

    // Alterna animaciones
    if (key == GLFW_KEY_N && action == GLFW_PRESS) AnimBall = !AnimBall;
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) AnimDog = !AnimDog;
}

// Animación del perro y pelota
void Animation() {
    if (AnimBall) rotBall += 0.4f;

    if (AnimDog) {
        const float headStep = 0.8f, tailStep = 1.2f, legStep = 1.2f;
        const float headMax = 15.0f, headMin = -15.0f;
        head += headStep * dogAnim;
        if (head >= headMax || head <= headMin) dogAnim *= -1;
    }
}

// Movimiento del ratón para controlar la cámara
void MouseCallback(GLFWwindow* window, double xPos, double yPos) {
    if (firstMouse) {
        lastX = xPos; lastY = yPos;
        firstMouse = false;
    }

    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos; // Eje Y invertido
    lastX = xPos; lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}
