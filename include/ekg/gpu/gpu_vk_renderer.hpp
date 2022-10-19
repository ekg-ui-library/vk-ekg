#ifndef EKG_GPU_VK_RENDERER_H
#define EKG_GPU_VK_RENDERER_H

#include <iostream>
#include "gpu_vk.hpp"
#include <vector>
#include <SDL2/SDL.h>
#include <optional>

namespace ekg::gpu {
    struct queue_families {
        std::optional<uint32_t> graphics_family {};
        std::optional<uint32_t> present_family {};

        bool is_complete();
    };

    struct swap_chain_support_details {
        VkSurfaceCapabilitiesKHR capabilities {};
        std::vector<VkSurfaceFormatKHR> formats {};
        std::vector<VkPresentModeKHR> present_modes {};
    };

    class vk_renderer {
    protected:
        const std::vector<const char*> validation_layers {
                "VK_LAYER_KHRONOS_validation"
        };

        const std::vector<const char*> device_extensions {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* call_back_data, void* user_data);
        static VkResult CreateDebugUtilsMessengerEXT(VkInstance &instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debug_messenger);
    public:
        SDL_Window* sdl_window {};
        bool enable_validation_layers {};
        std::vector<VkImage> swap_chain_images {};
        std::vector<VkImageView> swap_chain_image_view {};
        std::vector<VkFramebuffer> swap_chain_framebuffer {};
        VkInstance vk_instance {};
        VkDebugUtilsMessengerEXT vk_debug_messenger {};
        VkSurfaceKHR vk_surface {};
        VkPhysicalDevice vk_physical_device {};
        VkDevice vk_device {};
        VkQueue vk_graphics_queue {};
        VkQueue vk_present_queue {};
        VkSwapchainKHR vk_swap_chain {};
        VkFormat vk_swap_chain_image_format {};
        VkExtent2D vk_swap_chain_extent {};
        VkRenderPass vk_render_pass {};
        VkPipelineLayout vk_pipeline_layout {};

        std::vector<const char*> &get_extensions();
        void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);
        void create_instance();
        void setup();
        void create_surface();
        void pick_physical_device();
        void setup_debug_messenger();
        bool is_device_suitable(VkPhysicalDevice device);
        void find_queue_families(ekg::gpu::queue_families &indices, VkPhysicalDevice &device);
        bool check_device_extension_support(VkPhysicalDevice &device);
        void query_swap_chain_support(ekg::gpu::swap_chain_support_details &details, VkPhysicalDevice &device);
        void create_logical_device();
        void create_swap_chain();
        void create_image_views();
        void create_render_pass();
        void create_graphics_pipeline();

        VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats);
        VkPresentModeKHR choose_swap_present_mode_format(const std::vector<VkPresentModeKHR> &available_present_modes);
        VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities);
        bool create_shader_module(VkShaderModule &shader_module, const std::string &code);
    };

    extern vk_renderer vulkan;
}

#endif