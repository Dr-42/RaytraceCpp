#pragma once

#include <cstdint>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Pix{
public:
    Pix(int width, int height, std::string title);
    ~Pix();

    void PixRun(void (*callback)(Pix* pix));

    void SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
    static double GetTime();

    int width, height;
private:
    GLFWwindow* window;
    GLuint VAO, VBO, EBO, shaderProgram, texture;
    uint8_t* pixels;

    static void _framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void _processInput(GLFWwindow* window);
    void _createTexture(GLuint *texture, uint8_t *pixels);
    GLFWwindow* _initializeWindow(int width, int height, std::string title);
    uint8_t* _setupScene(GLuint *VAO, GLuint *VBO, GLuint *EBO, GLuint *shaderProgram, GLuint *texture);
    void _updateTexture(void (*callback)(Pix* pix));
};