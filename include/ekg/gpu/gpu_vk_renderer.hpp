#ifndef EKG_GPU_VK_H
#define EKG_GPU_VK_H

#include <iostream>
#include "gpu_vk.hpp"

namespace ekg::gpu {
    class vk_renderer {
    public:
        uint32_t vk_instance_ext_counts {};
        int32_t render_width {};
        int32_t render_height {};

        void create();
    };
};

#endif