#include "pix/pix.hpp"

#include <math/utils.hpp>

static const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec2 aTexCoord;\n"
                                 "out vec2 TexCoord;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos, 1.0);\n"
                                 "   TexCoord = aTexCoord;\n"
                                 "}\n";

static const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec2 TexCoord;\n"
                                   "uniform sampler2D texture1;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = texture(texture1, TexCoord);\n"
                                   "}\n";

Pix::Pix(int width, int height, std::string title){
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    GLFWwindow *window = this->_initializeWindow(width, height, "Pixren");
    if (!window)
    {
        std::exit(EXIT_FAILURE);
    }

    // Setup the scene
    GLuint VAO, VBO, EBO, shaderProgram, texture;
    uint8_t* pixels = this->_setupScene(&VAO, &VBO, &EBO, &shaderProgram, &texture);

    this->window = window;
    this->VAO = VAO;
    this->VBO = VBO;
    this->EBO = EBO;
    this->shaderProgram = shaderProgram;
    this->texture = texture;
    this->pixels = pixels;
}

Pix::~Pix(){
    delete[] this->pixels;
    glfwTerminate();
}
void Pix::PixRun(void (*callback)(Pix* pix)){
    while (!glfwWindowShouldClose(this->window))
    {
        // Input
        _processInput(this->window);

        // Update
        _updateTexture(callback);

        // Render
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(this->shaderProgram);
        glBindTexture(GL_TEXTURE_2D, this->texture);
        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(this->window);
        glfwPollEvents();
    }
}

void Pix::SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b){
    this->pixels[(y * this->width + x) * 3 + 0] = r;
    this->pixels[(y * this->width + x) * 3 + 1] = g;
    this->pixels[(y * this->width + x) * 3 + 2] = b;
}

void Pix::SetPixel(int x, int y, color col, int samples_per_pixel){
    auto r = col.x();
    auto g = col.y();
    auto b = col.z();

    auto scale = 1.0 / samples_per_pixel;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);
    this->pixels[(y * this->width + x) * 3 + 0] = static_cast<uint8_t>(256 * clamp(r, 0.0, 0.999));
    this->pixels[(y * this->width + x) * 3 + 1] = static_cast<uint8_t>(256 * clamp(g, 0.0, 0.999));
    this->pixels[(y * this->width + x) * 3 + 2] = static_cast<uint8_t>(256 * clamp(b, 0.0, 0.999));
}

double Pix::GetTime(){
    return glfwGetTime();
}

void Pix::_framebuffer_size_callback(GLFWwindow* window, int width, int height){
    (void)window;
    glViewport(0, 0, width, height);
}

void Pix::_processInput(GLFWwindow* window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void Pix::_createTexture(GLuint *texture, uint8_t *pixels){
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
}

GLFWwindow* Pix::_initializeWindow(int width, int height, std::string title){
    this->width = width;
    this->height = height;
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, this->_framebuffer_size_callback);

    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return NULL;
    }

    return window;
}

uint8_t* Pix::_setupScene(GLuint *VAO, GLuint *VBO, GLuint *EBO, GLuint *shaderProgram, GLuint *texture){
    float vertices[] = {
        // positions         // texture coords
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3};

    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    glGenBuffers(1, EBO);

    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    *shaderProgram = glCreateProgram();
    glAttachShader(*shaderProgram, vertexShader);
    glAttachShader(*shaderProgram, fragmentShader);
    glLinkProgram(*shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    uint8_t* pixels = new uint8_t[this->width * this->height * 3];
    this->_createTexture(texture, pixels);

    return pixels;
}

void Pix::_updateTexture(void (*callback)(Pix* pix)){
    callback(this);
    // Update the texture with the new pixel data
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->width, this->height, GL_RGB, GL_UNSIGNED_BYTE, this->pixels);
}

