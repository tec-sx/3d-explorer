#include "App.h"
#include "core/Window.h"
#include "vulkan/VulkanContext.h"
#include "vulkan/Swapchain.h"
#include "vulkan/Renderer.h"

App::App(int width, int height, const std::string& title) {
    window_ = std::make_unique<Window>(width, height, title.c_str());
    context_ = std::make_unique<VulkanContext>(*window_);
    swapchain_ = std::make_unique<Swapchain>(*context_, *window_);
    renderer_ = std::make_unique<Renderer>(*context_, *swapchain_);
}

App::~App() = default;

void App::run() {
    while (!window_->shouldClose()) {
        window_->pollEvents();
        renderer_->drawFrame();
    }
    // Ensure GPU idle before destruction order occurs
    renderer_->waitIdle();
}


