#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "gl_debug.h"
#include "utils.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void) {

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

  // Render loop
  int width, height;
  float ratio;
  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glUniform2f(res_loc, (float)width, (float)height);
    glUniform1f(time_loc, (float)glfwGetTime());

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
