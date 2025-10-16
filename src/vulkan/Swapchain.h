#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class VulkanContext;
class Window;

class Swapchain {
public:
    Swapchain(VulkanContext& context, const Window& window);
    ~Swapchain();

    VkSwapchainKHR swapchain() const { return swapchain_; }
    VkFormat imageFormat() const { return imageFormat_; }
    VkExtent2D extent() const { return extent_; }
    const std::vector<VkImageView>& imageViews() const { return imageViews_; }

private:
    void createSwapchain(const Window& window);
    void createImageViews();

    VulkanContext& context_;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat imageFormat_ = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D extent_{};
    std::vector<VkImage> images_;
    std::vector<VkImageView> imageViews_;
};


