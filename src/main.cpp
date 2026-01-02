#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "gl_debug.h"
#include "utils.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct AppResources {
    GLFWwindow* window;
    GLuint program;
    GLuint vao;
    GLint res_loc;
    GLint time_loc;
};

static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void update_performance_metrics(double &last_time, float &frame_time, float &fps) {
  double current_time = glfwGetTime();
  frame_time = (float)(current_time - last_time);
  fps = 1.0f / frame_time;
  last_time = current_time;
}

void draw_performance_window(float fps, float frame_time) {
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(200, 80), ImGuiCond_FirstUseEver);
  ImGui::Begin("Performance", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs |
               ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar);

  // Right-aligned numbers with fixed width
  char fps_str[16];
  char frame_time_str[16];
  snprintf(fps_str, sizeof(fps_str), "%6.1f", fps);
  snprintf(frame_time_str, sizeof(frame_time_str), "%6.1f", frame_time * 1000.0f);

  ImGui::Text("FPS: %s", fps_str);
  ImGui::Text("Frame Time: %s ms", frame_time_str);
  ImGui::End();
}

AppResources initialize_application() {
  // GLFW Setup
  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);
  glfwMakeContextCurrent(window);

  gladLoadGL(glfwGetProcAddress);
  glfwSwapInterval(1);

  // ImGui setup
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;            // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // IF using Docking Branch

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 410");

  // OpenGL setup
  const float fullscreen_triangle[] = {-1.0f, -1.0f, 3.0f, -1.0f, -1.0f, 3.0f};

  GLuint vao, vbo;
  GL_CALL(glGenVertexArrays(1, &vao));
  GL_CALL(glBindVertexArray(vao));

  GL_CALL(glGenBuffers(1, &vbo));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreen_triangle),
                       fullscreen_triangle, GL_STATIC_DRAW));

  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                                (void *)0));

  // Shader setup
  std::string vert_shader_text = load_shader_source("./shaders/shader.vert");
  std::string frag_shader_text = load_shader_source("./shaders/shader.frag");
  const char *vert_src = vert_shader_text.c_str();
  const char *frag_src = frag_shader_text.c_str();

  const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vert_src, nullptr);
  glCompileShader(vertex_shader);
  check_shader(vertex_shader, "vertex");

  const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &frag_src, nullptr);
  glCompileShader(fragment_shader);
  check_shader(fragment_shader, "fragment");

  const GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  check_program(program);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  const GLint res_loc = glGetUniformLocation(program, "iResolution");
  const GLint time_loc = glGetUniformLocation(program, "iTime");

  return {window, program, vao, res_loc, time_loc};
}

int main(void) {
  // Initialize all application resources
  AppResources resources = initialize_application();

  // Frame time and FPS tracking
  double last_time = 0.0;
  float frame_time = 0.0f;
  float fps = 0.0f;

  // Render loop
  int width, height;
  float ratio;
  while (!glfwWindowShouldClose(resources.window)) {
    // Update performance metrics
    update_performance_metrics(last_time, frame_time, fps);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Draw performance window
    draw_performance_window(fps, frame_time);

    glfwGetFramebufferSize(resources.window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(resources.program);
    glUniform2f(resources.res_loc, (float)width, (float)height);
    glUniform1f(resources.time_loc, (float)glfwGetTime());

    glBindVertexArray(resources.vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(resources.window);
    glfwPollEvents();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(resources.window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
