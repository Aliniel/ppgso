// Example of:
// - More than one object (loaded from OBJ file)
// - Keyboard events (press A to start/stop animation)
// - Mouse events
// - Orthographic camera projection
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh.h"

#define WIDTH 1920
#define HEIGHT 1080
#define MOVE_SENSITIVITY 10
#define ZOOM_SENZITIVITY 10

using namespace std;
using namespace glm;

struct GameObject{
    Mesh object;

    float x, y, z;
    mat4 matrix;
};

double mousePosX = 0.0;
double mousePosY = 0.0;
float cameraZoom = 0.0f;

GameObject *alduin;

GLuint ShaderProgram(const std::string &vertex_shader_file, const std::string &fragment_shader_file) {
  // Create shaders
  auto vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  auto fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  auto result = GL_FALSE;
  auto info_length = 0;

  // Load shader code
  // Reading the Vertex Shader Code.
  string VertexShaderCode;
  ifstream VertexShaderStream(vertex_shader_file, ios::in);
  string line = "";
  while(getline(VertexShaderStream, line)){
    VertexShaderCode += string("\n");
    VertexShaderCode += line;
  }
  VertexShaderStream.close();

  // Reading the Fragment SHader Code.
  string FragmentShaderCode;
  ifstream FragmentShaderStream(fragment_shader_file, ios::in);
  line = "";
  while(getline(FragmentShaderStream, line)){
    FragmentShaderCode += "\n";
    FragmentShaderCode += line;
  }
  FragmentShaderStream.close();

//  std::ifstream vertex_shader_stream(vertex_shader_file);
//  std::string VertexShaderCode((std::istreambuf_iterator<char>(vertex_shader_stream)), std::istreambuf_iterator<char>());
//
//  std::ifstream fragment_shader_stream(fragment_shader_file);
//  std::string FragmentShaderCode((std::istreambuf_iterator<char>(fragment_shader_stream)), std::istreambuf_iterator<char>());

  // Compile vertex shader
  std::cout << "Compiling Vertex Shader ..." << std::endl;
  auto vertex_shader_code_ptr = VertexShaderCode.c_str();
  glShaderSource(vertex_shader_id, 1, &vertex_shader_code_ptr, NULL);
  glCompileShader(vertex_shader_id);

  // Check vertex shader log
  glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_length);
    std::string vertex_shader_log((unsigned int)info_length, ' ');
    glGetShaderInfoLog(vertex_shader_id, info_length, NULL, &vertex_shader_log[0]);
    std::cout << vertex_shader_log << std::endl;
  }

  // Compile fragment shader
  std::cout << "Compiling Fragment Shader ..." << std::endl;
  auto fragment_shader_code_ptr = FragmentShaderCode.c_str();
  glShaderSource(fragment_shader_id, 1, &fragment_shader_code_ptr, NULL);
  glCompileShader(fragment_shader_id);

  // Check fragment shader log
  glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &info_length);
    std::string fragment_shader_log((unsigned long)info_length, ' ');
    glGetShaderInfoLog(fragment_shader_id, info_length, NULL, &fragment_shader_log[0]);
    std::cout << fragment_shader_log << std::endl;
  }

  // Create and link the program
  std::cout << "Linking Shader Program ..." << std::endl;
  auto program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glBindFragDataLocation(program_id, 0, "FragmentColor");
  glLinkProgram(program_id);

  // Check program log
  glGetProgramiv(program_id, GL_LINK_STATUS, &result);
  if (result == GL_FALSE) {
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_length);
    std::string program_log((unsigned long)info_length, ' ');
    glGetProgramInfoLog(program_id, info_length, NULL, &program_log[0]);
    std::cout << program_log << std::endl;
  }
  glDeleteShader(vertex_shader_id);
  glDeleteShader(fragment_shader_id);

  return program_id;
}

void UpdateProjection(GLuint program_id, bool is_perspective, glm::mat4 camera) {
  glUseProgram(program_id);

  // Create projection matrix
  glm::mat4 Projection;
  if (is_perspective) {
    // Perspective projection matrix (field of view, aspect ratio, near plane distance, far plane distance)
    Projection = glm::perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
  } else {
    // Orthographic projection matrix (left, right, bottom, top, near plane distance, far plane distance)
    Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1000.0f, 1000.0f);
  }

  // Send projection matrix value to program
  auto projection_uniform = glGetUniformLocation(program_id, "ProjectionMatrix");
  glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(Projection));

  // Send view matrix value to program
  auto view_uniform = glGetUniformLocation(program_id, "ViewMatrix");
  glm::mat4 View = glm::inverse(camera);
  glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(View));
}

void InitializeGLState() {
  // Enable Z-buffer
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // Enable polygon culling
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
}

// Keyboard press event handler
void OnKeyPress(GLFWwindow* /* window */, int key, int /* scancode */, int /* action */, int /* mods */) {
  switch(key){
    case GLFW_KEY_A:{
      alduin->x -= MOVE_SENSITIVITY;
      break;
    }
    case GLFW_KEY_D:{
      alduin->x += MOVE_SENSITIVITY;
      break;
    }
    case GLFW_KEY_W:{
      alduin->z -= MOVE_SENSITIVITY;
      break;
    }
    case GLFW_KEY_S:{
      alduin->z += MOVE_SENSITIVITY;
      break;
    }
    case GLFW_KEY_UP:{
      alduin->y += MOVE_SENSITIVITY;
      break;
    }
    case GLFW_KEY_DOWN: {
      alduin->y -= MOVE_SENSITIVITY;
      break;
    }
    default:break;
  }
}

// Mouse move event handler
void OnMouseMove(GLFWwindow* /* window */, double xpos, double ypos) {
  mousePosX = (xpos / ((double) WIDTH) * 2) - 1;
  mousePosY = -((ypos / ((double) HEIGHT) * 2) - 1);
}

void OnMouseScroll(GLFWwindow* /* window */, double xoffset, double yoffset){
  cameraZoom -= (float)yoffset * 10;
}

int main() {
  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW!" << std::endl;
    return EXIT_FAILURE;
  }

  // Setup OpenGL context
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Try to create a window
  auto window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", glfwGetPrimaryMonitor(), NULL);
  if (window == NULL) {
    std::cerr << "Failed to open GLFW window, your graphics card is probably only capable of OpenGL 2.1" << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  // Finalize window setup
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = GL_TRUE;
  glewInit();
  if (!glewIsSupported("GL_VERSION_3_3")) {
    std::cerr << "Failed to initialize GLEW with OpenGL 3.3!" << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  // Add keyboard and mouse handlers
  glfwSetKeyCallback(window, OnKeyPress);
  glfwSetCursorPosCallback(window, OnMouseMove);
  glfwSetScrollCallback(window, OnMouseScroll);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // Hide mouse cursor

  // Load shaders
  GLuint program_id = ShaderProgram("gl_objects.vert", "gl_objects.frag");

  // Initialize OpenGL state
  InitializeGLState();

  // Create objects from OBJ files with the specified textures
//  Object3D object0 = Object3D(
//          program_id, // Render object with this program (ID)
//          "sphere.obj", // OBJ file
//          "sphere.rgb", 256, 256 // Texture file (and its width, height)
//  );
  alduin = (GameObject *)malloc(sizeof(GameObject));
  alduin->matrix = mat4(1.0f);
  alduin->x = 0.0f;
  alduin->y = 0.0f;
  alduin->z = -500.0f;
  alduin->object = MeshPtr( new Mesh(program_id, "models/alduin/alduin.obj", "models/alduin/alduin.rgb"));
//  Object3D cursor = Object3D(program_id, "cursor.obj", "lena.rgb", 512, 512);

  float time = 0;
  float prevTime = 0;

  // Main execution loop
  while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
    // Set gray background
    glClearColor(.5f,.5f,.5f,0);
    // Clear depth and color buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    if (animationEnabled) time += (float) glfwGetTime() - prevTime;
//    prevTime = (float) glfwGetTime();

    // Camera position/rotation - for example, translate camera a bit backwards (positive value in Z axis), so we can see the objects
    glm::mat4 cameraMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, cameraZoom));

    // --- Draw objects using perspective projection (spheres) ---
    // Create object matrices
//    glm::mat4 object0ModelMat = glm::rotate(glm::mat4(1.0f), time, glm::vec3(0.5f, 1.0f, 0.0f));
    mat4 alduinModelMatrix = translate(mat4(1.0f), vec3(alduin->x, alduin->y - 300.0f, alduin->z));
    alduinModelMatrix = scale(alduinModelMatrix, vec3(0.8f, 0.8f, 0.8f));
    alduinModelMatrix = rotate(alduinModelMatrix, (float)radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
//    glm::mat4 object1ModelMat = translate(glm::mat4(1.0f), vec3(5, 5, 50000));
//    object1ModelMat = glm::scale(object1ModelMat, glm::vec3(0.5f, 0.5f, 0.5f));

    // Update camera with perspective projection
    UpdateProjection(program_id, true, cameraMat);

    // Render objects
    alduin->object.render(alduinModelMatrix);
//    object1.render(object1ModelMat);

    // --- Draw objects using orthographic projection (mouse "cursor") ---
    // Create object matrix
//    glm::mat4 cursorModelMat = glm::translate(glm::mat4(1.0f), glm::vec3(mousePosX, mousePosY, 0.0f));

    // Update camera with orthographic projection
    UpdateProjection(program_id, false, cameraMat);

    // Render objects
//    cursor.render(cursorModelMat);

    // Display result
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Clean up
  glfwTerminate();

  return EXIT_SUCCESS;
}
