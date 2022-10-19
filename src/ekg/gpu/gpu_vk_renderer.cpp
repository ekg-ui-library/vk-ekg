#include "ekg/gpu/gpu_vk_renderer.hpp"
#include "ekg/util/env.hpp"
#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>
#include <set>
#include <limits>
#include <functional>

ekg::gpu::vk_renderer ekg::gpu::vulkan {};

bool ekg::gpu::queue_families::is_complete() {
    return graphics_family.has_value() && present_family.has_value();
}

std::vector<const char*> &ekg::gpu::vk_renderer::get_extensions() {
    uint32_t extension_counts {};
    SDL_Vulkan_GetInstanceExtensions(this->sdl_window, &extension_counts, nullptr);
    std::vector<const char*> extensions {extension_counts};
    SDL_Vulkan_GetInstanceExtensions(this->sdl_window, &extension_counts, extensions.data());
    return extensions;
}

void ekg::gpu::vk_renderer::create_instance() {
    VkApplicationInfo vk_app_info {};
    vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_app_info.pApplicationName = "vk ekg test";
    vk_app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vk_app_info.pEngineName = "No Engine";
    vk_app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    vk_app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &vk_app_info;

    auto extensions = this->get_extensions();
    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_info {};
    if (this->enable_validation_layers) {
        create_info.enabledLayerCount = static_cast<uint32_t>(this->validation_layers.size());
        create_info.ppEnabledLayerNames = this->validation_layers.data();
        this->populate_debug_messenger_create_info(debug_info);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &create_info;
    } else {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }

    if (vkCreateInstance(&create_info, nullptr, &this->vk_instance) != VK_SUCCESS) {
        ekg::log("failed to create vulkan instance.");
    }
}

void ekg::gpu::vk_renderer::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info) {
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = ekg::gpu::vk_renderer::debug_callback;
}

VkBool32 ekg::gpu::vk_renderer::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT *call_back_data, void *user_data) {
    std::cerr << "validation layer: " << call_back_data->pMessage << std::endl;
    return 0;
}

void ekg::gpu::vk_renderer::setup() {
    this->create_instance();
    this->setup_debug_messenger();
    this->pick_physical_device();
    this->create_logical_device();
    this->create_swap_chain();
    this->create_image_views();
    this->create_render_pass();
    this->create_graphics_pipeline();
}

void ekg::gpu::vk_renderer::setup_debug_messenger() {
    if (!this->enable_validation_layers) return;

    VkDebugUtilsMessengerCreateInfoEXT create_info {};
    this->populate_debug_messenger_create_info(create_info);

    if (CreateDebugUtilsMessengerEXT(this->vk_instance, &create_info, nullptr, &this->vk_debug_messenger) != VK_SUCCESS) {
        ekg::log("failed to set up debug messenger!");
    }
}

VkResult ekg::gpu::vk_renderer::CreateDebugUtilsMessengerEXT(VkInstance &instance, const VkDebugUtilsMessengerCreateInfoEXT *create_info, const VkAllocationCallbacks *allocator, VkDebugUtilsMessengerEXT *debug_messenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, create_info, allocator, debug_messenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void ekg::gpu::vk_renderer::create_surface() {
    if (SDL_Vulkan_CreateSurface(this->sdl_window, this->vk_instance, &this->vk_surface)) {
        ekg::log("could not create vulkan surface!!");
    }
}

void ekg::gpu::vk_renderer::pick_physical_device() {
    uint32_t device_count {};
    vkEnumeratePhysicalDevices(this->vk_instance, &device_count, nullptr);

    if (device_count != 0) {
        ekg::log("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(this->vk_instance, &device_count, devices.data());

    for (const auto& device : devices) {
        if (this->is_device_suitable(device)) {
            this->vk_physical_device = device;
            break;
        }
    }

    if (this->vk_physical_device == VK_NULL_HANDLE) {
        ekg::log("failed to find a suitable GPU!");
    }
}

bool ekg::gpu::vk_renderer::is_device_suitable(VkPhysicalDevice device) {
    queue_families indices {};
    this->find_queue_families(indices, device);

    bool extensions_supported = this->check_device_extension_support(device);
    bool swap_chain_adequate {};

    if (extensions_supported) {
        ekg::gpu::swap_chain_support_details details {};
        this->query_swap_chain_support(details, device);
        swap_chain_adequate = !details.formats.empty() && !details.present_modes.empty();
    }

    return indices.is_complete() && extensions_supported && swap_chain_adequate;
}

void ekg::gpu::vk_renderer::find_queue_families(ekg::gpu::queue_families &indices, VkPhysicalDevice &device) {
    uint32_t queue_family_count {};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    int32_t i {};
    for (const auto &queue_family : queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        VkBool32 present_support {};
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->vk_surface, &present_support);

        if (present_support) {
            indices.present_family = i;
        }

        if (indices.is_complete()) {
            break;
        }

        i++;
    }
}

bool ekg::gpu::vk_renderer::check_device_extension_support(VkPhysicalDevice &device) {
    uint32_t extension_count {};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(this->device_extensions.begin(), this->device_extensions.end());

    for (const auto &extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

void ekg::gpu::vk_renderer::query_swap_chain_support(ekg::gpu::swap_chain_support_details &details, VkPhysicalDevice &device) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->vk_surface, &details.capabilities);

    uint32_t format_count {};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vk_surface, &format_count, nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vk_surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count {};
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vk_surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vk_surface, &present_mode_count, details.present_modes.data());
    }
}

void ekg::gpu::vk_renderer::create_logical_device() {
    ekg::gpu::queue_families indices {};
    this->find_queue_families(indices, this->vk_physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos {};
    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value()};

    float queue_priority {1.0f};
    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features {};
    VkDeviceCreateInfo create_info {};

    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();

    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    create_info.ppEnabledExtensionNames = device_extensions.data();

    if (this->enable_validation_layers) {
        create_info.enabledLayerCount = static_cast<uint32_t>(this->validation_layers.size());
        create_info.ppEnabledLayerNames = this->validation_layers.data();
    } else {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(this->vk_physical_device, &create_info, nullptr, &this->vk_device) != VK_SUCCESS) {
        ekg::log("failed to create logical device!");
    }

    vkGetDeviceQueue(this->vk_device, indices.graphics_family.value(), 0, &vk_graphics_queue);
    vkGetDeviceQueue(this->vk_device, indices.present_family.value(), 0, &vk_present_queue);
}

void ekg::gpu::vk_renderer::create_swap_chain() {
    ekg::gpu::swap_chain_support_details support {};
    this->query_swap_chain_support(support, this->vk_physical_device);

    VkSurfaceFormatKHR surface_format {this->choose_swap_surface_format(support.formats)};
    VkPresentModeKHR present_mode_format {this->choose_swap_present_mode_format(support.present_modes)};
    VkExtent2D extent {choose_swap_extent(support.capabilities)};

    uint32_t image_count {};
    if (support.capabilities.maxImageCount < 0 && image_count > support.capabilities.maxImageCount) {
        image_count = support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = this->vk_surface;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    ekg::gpu::queue_families indicies {};
    this->find_queue_families(indicies, this->vk_physical_device);
    uint32_t queue_family_indices[] = {indicies.graphics_family.value(), indicies.present_family.value()};

    if (indicies.graphics_family != indicies.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode_format;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(this->vk_device, &create_info, nullptr, &this->vk_swap_chain) != VK_SUCCESS) {
        ekg::log("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(this->vk_device, this->vk_swap_chain, &image_count, nullptr);
    this->swap_chain_images.resize(image_count);
    vkGetSwapchainImagesKHR(this->vk_device, this->vk_swap_chain, &image_count, this->swap_chain_images.data());

    this->vk_swap_chain_image_format = surface_format.format;
    this->vk_swap_chain_extent = extent;
}

VkSurfaceFormatKHR
ekg::gpu::vk_renderer::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats) {
    for (const auto &available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR
ekg::gpu::vk_renderer::choose_swap_present_mode_format(const std::vector<VkPresentModeKHR> &available_present_modes) {
    for (const auto &available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return available_present_modes[0];
}

VkExtent2D ekg::gpu::vk_renderer::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int32_t framebuffer_w {}, framebuffer_h {};
        SDL_Vulkan_GetDrawableSize(this->sdl_window, &framebuffer_w, &framebuffer_h);

        VkExtent2D actual_extent {
            static_cast<uint32_t>(framebuffer_w),
            static_cast<uint32_t>(framebuffer_h)
        };

        actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.width, capabilities.maxImageExtent.height);

        return actual_extent;
    }
}

void ekg::gpu::vk_renderer::create_image_views() {
    this->swap_chain_image_view.resize(this->swap_chain_image_view.size());

    for (size_t i = 0; i < this->swap_chain_images.size(); i++) {
        VkImageViewCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = this->swap_chain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = this->vk_swap_chain_image_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_R;
        create_info.components.g = VK_COMPONENT_SWIZZLE_G;
        create_info.components.b = VK_COMPONENT_SWIZZLE_B;
        create_info.components.a = VK_COMPONENT_SWIZZLE_A;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(this->vk_device, &create_info, nullptr, &this->swap_chain_image_view[i]) != VK_SUCCESS) {
            ekg::log("failed to create image views!");
        }
    }
}

void ekg::gpu::vk_renderer::create_render_pass() {
    VkAttachmentDescription color_attachment {};
    color_attachment.format = this->vk_swap_chain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkRenderPassCreateInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    if (vkCreateRenderPass(this->vk_device, &render_pass_info, nullptr, &this->vk_render_pass)) {
        ekg::log("failed to create render pass!");
    }
}

void ekg::gpu::vk_renderer::create_graphics_pipeline() {
    VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(this->vk_device, &pipeline_layout_create_info, nullptr, &this->vk_pipeline_layout) != VK_SUCCESS) {
        ekg::log("failed to create pipeline layout!");
    }
}

bool ekg::gpu::vk_renderer::create_shader_module(VkShaderModule &shader_module, const std::string &code) {
    VkShaderModuleCreateInfo create_info {};

    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(this->vk_device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
        return false;
    }

    return true;
}
