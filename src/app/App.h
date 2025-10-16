#pragma once

#include <memory>
#include <string>

class Window;
class VulkanContext;
class Swapchain;
class Renderer;

class App {
public:
    App(int width, int height, const std::string& title);
    ~App();

    void run();

private:
    std::unique_ptr<Window> window_;
    std::unique_ptr<VulkanContext> context_;
    std::unique_ptr<Swapchain> swapchain_;
    std::unique_ptr<Renderer> renderer_;
};


