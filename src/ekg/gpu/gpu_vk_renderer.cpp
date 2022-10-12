#include "ekg/gpu/gpu_vk_renderer.hpp"
#include "ekg/util/util.hpp"
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <set>

bool ekg::gpu::vk_renderer::init(SDL_Window* root) {
    uint32_t extensions_count {};
    const char** extensions {};

    if (!this->check_validation_layer_properties()) {
        ekg::log("validation layers requested but not found");
        return true;
    }

    SDL_Vulkan_GetInstanceExtensions(root, &extensions_count, nullptr);
    std::vector<const char*> extensions_list {extensions_count};
    SDL_Vulkan_GetInstanceExtensions(root, &extensions_count, extensions_list.data());

    VkApplicationInfo vk_app_info {};
    vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_app_info.pApplicationName = "vk ekg test";
    vk_app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vk_app_info.pEngineName = "No Engine";
    vk_app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    vk_app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo vk_instance_create_info {};
    vk_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_instance_create_info.pApplicationInfo = &vk_app_info;
    vk_instance_create_info.enabledLayerCount = this->validation_layers.size();
    vk_instance_create_info.ppEnabledLayerNames = this->validation_layers.data();
    vk_instance_create_info.enabledExtensionCount = extensions_list.size();
    vk_instance_create_info.ppEnabledExtensionNames = extensions_list.data();

    VkResult state = vkCreateInstance(&vk_instance_create_info, nullptr, &this->vk_instance);
    return state != VK_SUCCESS;
}

void ekg::gpu::vk_renderer::quit() {
    vkDestroyInstance(this->vk_instance, nullptr);
}

bool ekg::gpu::vk_renderer::check_validation_layer_properties() {
    uint32_t layer_count {};
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers {layer_count};
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer_name : this->validation_layers) {
        bool layer_found {};

        for (const auto &layer_properties : available_layers) {
            if (strcmp(layer_name, layer_properties.layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) {
            return false;
        }
    }

    return true;
}

void ekg::gpu::vk_renderer::debug_message() {
    // todo uhhh the debug message stuff
}

void ekg::gpu::vk_renderer::create_surface(SDL_Window* root) {
    SDL_Vulkan_CreateSurface(root, this->vk_instance, &this->vk_surface);
}

void ekg::gpu::vk_renderer::select_physical_device() {
    uint32_t physical_device_count {};
    vkEnumeratePhysicalDevices(this->vk_instance, &physical_device_count, nullptr);

    std::vector<VkPhysicalDevice> physic_devices {physical_device_count};
    vkEnumeratePhysicalDevices(this->vk_instance, &physical_device_count, physic_devices.data());
    this->vk_physical_device = physic_devices[0];
}

void ekg::gpu::vk_renderer::select_queue_family() {
    uint32_t queue_family_count {};
    vkGetPhysicalDeviceQueueFamilyProperties(this->vk_physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> vk_queue_family_properties {queue_family_count};
    vkGetPhysicalDeviceQueueFamilyProperties(this->vk_physical_device, &queue_family_count, vk_queue_family_properties.data());

    int32_t graphic_index {-1};
    int32_t present_index {-1};
    int32_t i {};

    for (const auto &queue_family : vk_queue_family_properties) {
        if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphic_index = i;
        }

        VkBool32 present_support {};
        vkGetPhysicalDeviceSurfaceSupportKHR(this->vk_physical_device, i, this->vk_surface, &present_support);

        if (queue_family.queueCount > 0 && present_support) {
            present_index = i;
        }

        if (graphic_index != -1 && present_index != -1) {
            break;
        }

        i++;
    }

    this->graphics_queue_family_index = graphic_index;
    this->present_queue_family_index = present_index;
}

void ekg::gpu::vk_renderer::create_device() {
    const std::vector<const char*> device_extensions {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const float queue_priority_arr[] = {1.0f};

    std::vector<VkDeviceQueueCreateInfo> vk_create_info_list {};
    std::set<uint32_t> unique_queue_families = {this->graphics_queue_family_index, this->present_queue_family_index};

    float queue_priority {queue_priority_arr[0]};
    for (int32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo vk_queue_create_info {};
        vk_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        vk_queue_create_info.queueFamilyIndex = queue_family;
        vk_queue_create_info.queueCount = 1;
        vk_queue_create_info.pQueuePriorities = &queue_priority;
        vk_create_info_list.push_back(vk_queue_create_info);
    }

    VkDeviceQueueCreateInfo vk_queue_create_info {};
    vk_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vk_queue_create_info.queueFamilyIndex = this->graphics_queue_family_index;
    vk_queue_create_info.queueCount = 1;
    vk_queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures vk_device_features {};
    vk_device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo vk_create_info {};
    vk_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vk_create_info.pQueueCreateInfos = &vk_queue_create_info;
    vk_create_info.queueCreateInfoCount = vk_create_info_list.size();
    vk_create_info.pQueueCreateInfos = vk_create_info_list.data();
    vk_create_info.pEnabledFeatures = &vk_device_features;
    vk_create_info.enabledExtensionCount = device_extensions.size();
    vk_create_info.ppEnabledExtensionNames = device_extensions.data();

    vk_create_info.enabledLayerCount = this->validation_layers.size();
    vk_create_info.ppEnabledLayerNames = this->validation_layers.data();

    vkCreateDevice(vk_physical_device, &vk_create_info, nullptr, &this->vk_device);
    vkGetDeviceQueue(this->vk_device, this->graphics_queue_family_index, 0, &this->graphic_queue);
    vkGetDeviceQueue(this->vk_device, this->present_queue_family_index, 0, &this->present_queue);
}
