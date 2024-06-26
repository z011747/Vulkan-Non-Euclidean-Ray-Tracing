C:/VulkanSDK/1.3.261.1/Bin/glslc.exe base.vert -o vert.spv
C:/VulkanSDK/1.3.261.1/Bin/glslc.exe base.frag -o frag.spv
C:/VulkanSDK/1.3.261.1/Bin/glslc.exe raygen.rgen -o raygen.spv --target-env=vulkan1.3
C:/VulkanSDK/1.3.261.1/Bin/glslc.exe miss.rmiss -o miss.spv --target-env=vulkan1.3
C:/VulkanSDK/1.3.261.1/Bin/glslc.exe closesthit.rchit -o closesthit.spv --target-env=vulkan1.3
pause