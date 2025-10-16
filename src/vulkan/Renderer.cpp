#include "Renderer.h"
#include "VulkanContext.h"
#include "Swapchain.h"
#include <stdexcept>

Renderer::Renderer(VulkanContext& context, Swapchain& swapchain)
    : context_(context), swapchain_(swapchain) {
    createRenderPass();
    createFramebuffers();
    createCommandPoolAndBuffers();
    createSyncObjects();
}

Renderer::~Renderer() {
    if (inFlight_) vkDestroyFence(context_.device(), inFlight_, nullptr);
    if (renderFinished_) vkDestroySemaphore(context_.device(), renderFinished_, nullptr);
    if (imageAvailable_) vkDestroySemaphore(context_.device(), imageAvailable_, nullptr);
    if (commandPool_) vkDestroyCommandPool(context_.device(), commandPool_, nullptr);
    for (auto fb : framebuffers_) vkDestroyFramebuffer(context_.device(), fb, nullptr);
    if (renderPass_) vkDestroyRenderPass(context_.device(), renderPass_, nullptr);
}

void Renderer::createRenderPass() {
    VkAttachmentDescription color{};
    color.format = swapchain_.imageFormat();
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    VkSubpassDependency dep{};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.srcAccessMask = 0;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rp{};
    rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp.attachmentCount = 1;
    rp.pAttachments = &color;
    rp.subpassCount = 1;
    rp.pSubpasses = &subpass;
    rp.dependencyCount = 1;
    rp.pDependencies = &dep;

    if (vkCreateRenderPass(context_.device(), &rp, nullptr, &renderPass_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }
}

void Renderer::createFramebuffers() {
    framebuffers_.resize(swapchain_.imageViews().size());
    for (size_t i = 0; i < swapchain_.imageViews().size(); ++i) {
        VkImageView attachments[] = { swapchain_.imageViews()[i] };
        VkFramebufferCreateInfo fb{};
        fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb.renderPass = renderPass_;
        fb.attachmentCount = 1;
        fb.pAttachments = attachments;
        fb.width = swapchain_.extent().width;
        fb.height = swapchain_.extent().height;
        fb.layers = 1;
        if (vkCreateFramebuffer(context_.device(), &fb, nullptr, &framebuffers_[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer");
        }
    }
}

void Renderer::createCommandPoolAndBuffers() {
    VkCommandPoolCreateInfo pci{};
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.queueFamilyIndex = context_.graphicsQueueFamily();
    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(context_.device(), &pci, nullptr, &commandPool_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }

    commandBuffers_.resize(framebuffers_.size());
    VkCommandBufferAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = commandPool_;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());
    if (vkAllocateCommandBuffers(context_.device(), &ai, commandBuffers_.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers");
    }
}

void Renderer::createSyncObjects() {
    VkSemaphoreCreateInfo si{}; si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fi{}; fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO; fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateSemaphore(context_.device(), &si, nullptr, &imageAvailable_) != VK_SUCCESS) throw std::runtime_error("Failed to create semaphore");
    if (vkCreateSemaphore(context_.device(), &si, nullptr, &renderFinished_) != VK_SUCCESS) throw std::runtime_error("Failed to create semaphore");
    if (vkCreateFence(context_.device(), &fi, nullptr, &inFlight_) != VK_SUCCESS) throw std::runtime_error("Failed to create fence");
}

void Renderer::drawFrame() {
    vkWaitForFences(context_.device(), 1, &inFlight_, VK_TRUE, UINT64_MAX);
    vkResetFences(context_.device(), 1, &inFlight_);

    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(context_.device(), swapchain_.swapchain(), UINT64_MAX, imageAvailable_, VK_NULL_HANDLE, &imageIndex);

    VkCommandBuffer cmd = commandBuffers_[imageIndex];
    VkCommandBufferBeginInfo bi{}; bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkResetCommandBuffer(cmd, 0);
    vkBeginCommandBuffer(cmd, &bi);

    VkClearValue clear{}; clear.color = { { 0.05f, 0.05f, 0.1f, 1.0f } };
    VkRenderPassBeginInfo rbi{};
    rbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rbi.renderPass = renderPass_;
    rbi.framebuffer = framebuffers_[imageIndex];
    rbi.renderArea.offset = {0, 0};
    rbi.renderArea.extent = swapchain_.extent();
    rbi.clearValueCount = 1;
    rbi.pClearValues = &clear;
    vkCmdBeginRenderPass(cmd, &rbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &imageAvailable_;
    submit.pWaitDstStageMask = &waitStage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &renderFinished_;
    vkQueueSubmit(context_.graphicsQueue(), 1, &submit, inFlight_);

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &renderFinished_;
    VkSwapchainKHR sc = swapchain_.swapchain();
    present.swapchainCount = 1;
    present.pSwapchains = &sc;
    present.pImageIndices = &imageIndex;
    vkQueuePresentKHR(context_.graphicsQueue(), &present);
}

void Renderer::waitIdle() {
    vkDeviceWaitIdle(context_.device());
}


