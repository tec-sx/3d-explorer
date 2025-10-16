#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanContext;
class Swapchain;

class Renderer {
public:
    Renderer(VulkanContext& context, Swapchain& swapchain);
    ~Renderer();

    void drawFrame();
    void waitIdle();

private:
    void createRenderPass();
    void createFramebuffers();
    void createCommandPoolAndBuffers();
    void createSyncObjects();

    VulkanContext& context_;
    Swapchain& swapchain_;

    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers_;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;
    VkSemaphore imageAvailable_ = VK_NULL_HANDLE;
    VkSemaphore renderFinished_ = VK_NULL_HANDLE;
    VkFence inFlight_ = VK_NULL_HANDLE;
};


