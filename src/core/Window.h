#pragma once

#include <GLFW/glfw3.h>

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    bool shouldClose() const;
    void pollEvents();
    GLFWwindow* handle() const { return window_; }

private:
    GLFWwindow* window_ = nullptr;
};


