#include "Window.h"
#include <stdexcept>

static void glfwErrorCallback(int error, const char* description) {
    (void)error;
    fprintf(stderr, "GLFW Error: %s\n", description);
}

Window::Window(int width, int height, const char* title) {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("Failed to create window");
    }
}

Window::~Window() {
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window_) != 0;
}

void Window::pollEvents() {
    glfwPollEvents();
}


