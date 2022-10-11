# vk-ekg

This project contains the `ekg::gpu::allocator` variant devlopment source, this uses the Vulkan API version 1_0_0.  
Note: The GPU allocator use the same logic system of OpenGL gpu allocator.

# Buffer and Draws

The VK ekg gpu allocator is incomplete, there is some phases before reach draws:  
Gen 2 buffers and re alloc the two buffers everytime the allocator is revoked.
After invoked, in revoke segment the allocator re alloc all vertex and uv data from CPU cache to the two buffers primary generated.
Clean the CPU data cache.

With all data sent to the two buffers into GPU, the allocator call draws using iterations to pass multiples uniforms.

---

The project is not a priority, I am learning Vulkan.
