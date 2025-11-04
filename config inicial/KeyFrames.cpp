#include <iostream>
#include <cmath>

// Librerías de manejo gráfico
#include <GL/glew.h>      // Gestiona extensiones de OpenGL
#include <GLFW/glfw3.h>   // Permite crear ventanas y capturar eventos del sistema

// Librerías auxiliares
#include "stb_image.h"    // Carga de imágenes (texturas)
#include "SOIL2/SOIL2.h"  // Librería adicional para manejo de imágenes

// Librerías matemáticas de OpenGL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Módulos personalizados del proyecto
#include "Shader.h"       // Clase para manejar programas de sombreado (VS y FS)
#include "Camera.h"       // Clase que gestiona el movimiento de la cámara
#include "Model.h"        // Clase para cargar y dibujar modelos 3D

// Declaración de funciones utilizadas en el flujo del programa
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void Animation();

// Parámetros de ventana
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Configuración inicial de cámara
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool keys[1024];      // Mapa de teclas presionadas
bool firstMouse = true;

// Parámetros de iluminación
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;          // Control del estado de la luz (encendida/apagada)

// Posiciones de luces puntuales
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.0f, 2.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f)
};

// Definición de vértices para un cubo con normales
float vertices[] = {
    // Posiciones             // Normales
    -0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,
    // (... resto de las caras del cubo ...)
};

// Variables para animación
glm::vec3 Light1 = glm::vec3(0.0f);
float rotBall = 0.0f, rotDog = 0.0f;
int dogAnim = 0;
float FLegs = 0.0f, RLegs = 0.0f, head = 0.0f, tail = 0.0f;

// Variables base del modelo principal
float dogPosX, dogPosY, dogPosZ;

// Estructura de keyframes y control de interpolación
#define MAX_FRAMES 9
int i_max_steps = 190;
int i_curr_steps = 0;

typedef struct _frame {
    float rotDog, rotDogInc;
    float dogPosX, dogPosY, dogPosZ;
    float incX, incY, incZ;
    float head, headInc;
} FRAME;

FRAME KeyFrame[MAX_FRAMES];  // Arreglo con los cuadros clave
int FrameIndex = 0;
bool play = false;
int playIndex = 0;

// Guarda un nuevo keyframe con la pose actual
void saveFrame() {
    printf("frameindex %d\n", FrameIndex);
    KeyFrame[FrameIndex].dogPosX = dogPosX;
    KeyFrame[FrameIndex].dogPosY = dogPosY;
    KeyFrame[FrameIndex].dogPosZ = dogPosZ;
    KeyFrame[FrameIndex].rotDog  = rotDog;
    KeyFrame[FrameIndex].head    = head;
    FrameIndex++;
}

// Restaura el modelo a la primera pose grabada
void resetElements() {
    dogPosX = KeyFrame[0].dogPosX;
    dogPosY = KeyFrame[0].dogPosY;
    dogPosZ = KeyFrame[0].dogPosZ;
    rotDog  = KeyFrame[0].rotDog;
    head    = KeyFrame[0].head;
}

// Calcula los incrementos para interpolar entre dos keyframes
void interpolation() {
    KeyFrame[playIndex].incX = (KeyFrame[playIndex + 1].dogPosX - KeyFrame[playIndex].dogPosX) / i_max_steps;
    KeyFrame[playIndex].incY = (KeyFrame[playIndex + 1].dogPosY - KeyFrame[playIndex].dogPosY) / i_max_steps;
    KeyFrame[playIndex].incZ = (KeyFrame[playIndex + 1].dogPosZ - KeyFrame[playIndex].dogPosZ) / i_max_steps;

    KeyFrame[playIndex].rotDogInc = (KeyFrame[playIndex + 1].rotDog - KeyFrame[playIndex].rotDog) / i_max_steps;
    KeyFrame[playIndex].headInc   = (KeyFrame[playIndex + 1].head   - KeyFrame[playIndex].head)   / i_max_steps;
}

// Control de tiempo entre cuadros
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main() {
    // Inicialización de GLFW y creación de la ventana
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Animacion maquina de estados", nullptr, nullptr);

    if (!window) {
        std::cout << "Error al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Asignación de funciones de callback para eventos
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    // Inicialización de GLEW (manejador de extensiones)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Error al inicializar GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Compilación y vinculación de shaders
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

    // Carga de modelos 3D (componentes del perro y entorno)
    Model DogBody("Models/DogBody.obj");
    Model HeadDog("Models/HeadDog.obj");
    Model DogTail("Models/TailDog.obj");
    Model F_RightLeg("Models/F_RightLegDog.obj");
    Model F_LeftLeg("Models/F_LeftLegDog.obj");
    Model B_RightLeg("Models/B_RightLegDog.obj");
    Model B_LeftLeg("Models/B_LeftLegDog.obj");
    Model Piso("Models/piso.obj");
    Model Ball("Models/ball.obj");

    // Inicialización de todos los keyframes en cero
    for (int i = 0; i < MAX_FRAMES; i++) {
        KeyFrame[i] = {0,0,0,0,0,0,0,0,0,0};
    }

    // Configuración de buffers de vértices
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Atributos: posición y normales
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Configuración inicial del shader de iluminación
    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

    glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);

    // Bucle principal de renderizado
    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();
        Animation();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        lightingShader.Use();
        // (Aquí sigue toda la configuración de luces, materiales y dibujo de modelos)
        // ...
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
