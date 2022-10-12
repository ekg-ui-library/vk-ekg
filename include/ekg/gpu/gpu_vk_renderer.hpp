#ifndef EKG_GPU_VK_RENDERER_H
#define EKG_GPU_VK_RENDERER_H

#include <iostream>
#include "gpu_vk.hpp"
#include <vector>
#include <SDL2/SDL.h>

namespace ekg::gpu {
    class vk_renderer {
    public:
        int32_t render_width {};
        int32_t render_height {};

        uint32_t graphics_queue_family_index {};
        uint32_t present_queue_family_index {};

        VkSurfaceKHR vk_surface {};
        VkInstance vk_instance {};
        VkPhysicalDevice vk_physical_device {};
        VkDevice vk_device {};
        VkQueue graphic_queue {};
        VkQueue present_queue {};

        std::vector<const char*> validation_layers {};

        bool init(SDL_Window*);
        bool check_validation_layer_properties();
        void debug_message();
        void create_surface(SDL_Window*);
        void select_physical_device();
        void select_queue_family();
        void create_device();
        void quit();
    };
}

#endif