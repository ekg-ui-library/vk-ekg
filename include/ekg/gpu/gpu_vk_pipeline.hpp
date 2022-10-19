#ifndef EKG_GPU_VK_SHADER_H
#define EKG_GPU_VK_SHADER_H

#include "ekg/gpu/gpu_vk.hpp"
#include <iostream>

namespace ekg {
    namespace gpu {
        struct pipeline {
            VkPipeline pipeline_info {};
        };
    }

    bool create_pipeline(ekg::gpu::pipeline&, std::string_view, std::string_view);
}

#endif