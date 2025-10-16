#include "Swapchain.h"
#include "VulkanContext.h"
#include "src/core/Window.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>

Swapchain::Swapchain(VulkanContext& context, const Window& window)
    : context_(context) {
    createSwapchain(window);
    createImageViews();
}

Swapchain::~Swapchain() {
    for (auto view : imageViews_) vkDestroyImageView(context_.device(), view, nullptr);
    if (swapchain_) vkDestroySwapchainKHR(context_.device(), swapchain_, nullptr);
}

void Swapchain::createSwapchain(const Window& window) {
    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context_.physicalDevice(), context_.surface(), &caps);

    extent_ = caps.currentExtent;
    if (extent_.width == UINT32_MAX) {
        int w, h;
        glfwGetFramebufferSize(window.handle(), &w, &h);
        extent_.width = static_cast<uint32_t>(w);
        extent_.height = static_cast<uint32_t>(h);
    }

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context_.physicalDevice(), context_.surface(), &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context_.physicalDevice(), context_.surface(), &formatCount, formats.data());
    VkSurfaceFormatKHR chosen = formats[0];
    for (const auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosen = f; break;
        }
    }
    imageFormat_ = chosen.format;

    VkSurfaceTransformFlagBitsKHR transform =
        (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
        : caps.currentTransform;

    VkCompositeAlphaFlagBitsKHR composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) composite = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) imageCount = caps.maxImageCount;

    VkSwapchainCreateInfoKHR sci{};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = context_.surface();
    sci.minImageCount = imageCount;
    sci.imageFormat = imageFormat_;
    sci.imageColorSpace = chosen.colorSpace;
    sci.imageExtent = extent_;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = transform;
    sci.compositeAlpha = composite;
    sci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    sci.clipped = VK_TRUE;
    sci.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(context_.device(), &sci, nullptr, &swapchain_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swapchain");
    }

    uint32_t actualCount = 0;
    vkGetSwapchainImagesKHR(context_.device(), swapchain_, &actualCount, nullptr);
    images_.resize(actualCount);
    vkGetSwapchainImagesKHR(context_.device(), swapchain_, &actualCount, images_.data());
}

void Swapchain::createImageViews() {
    imageViews_.resize(images_.size());
    for (size_t i = 0; i < images_.size(); ++i) {
        VkImageViewCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ci.image = images_[i];
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.format = imageFormat_;
        ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.subresourceRange.baseMipLevel = 0;
        ci.subresourceRange.levelCount = 1;
        ci.subresourceRange.baseArrayLayer = 0;
        ci.subresourceRange.layerCount = 1;
        if (vkCreateImageView(context_.device(), &ci, nullptr, &imageViews_[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view");
        }
    }
}


