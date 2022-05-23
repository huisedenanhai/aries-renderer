#include "RayTracing.h"
#include "../Context.h"
#include "../Mesh.h"
#include "../Profiler.h"
#include "../Scene.h"
#include "Drawer.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
AccelerationStructure::AccelerationStructure(
    Context *context,
    VkAccelerationStructureTypeKHR type,
    VkDeviceSize buffer_size)
    : _context(context) {
    _buffer = _context->create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
        VMA_MEMORY_USAGE_GPU_ONLY);

    VkAccelerationStructureCreateInfoKHR info{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    info.type = type;
    info.buffer = _buffer->buffer();
    info.offset = 0;
    info.size = buffer_size;

    auto device = _context->device();
    if (device->CreateAccelerationStructureKHR(
            &info, &_acceleration_structure) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create BLAS");
    }
}

AccelerationStructure::~AccelerationStructure() {
    if (_acceleration_structure != VK_NULL_HANDLE) {
        _context->device()->Destroy(_acceleration_structure);
    }
}

VkAccelerationStructureKHR
AccelerationStructure::acceleration_structure() const {
    return _acceleration_structure;
}

Handle<AccelerationStructure> AccelerationStructure::create(Mesh *mesh) {
    assert(mesh != nullptr);

    VkAccelerationStructureGeometryKHR geometry{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    auto &triangles = geometry.geometry.triangles;
    triangles.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    triangles.vertexData.deviceAddress =
        mesh->position_buffer()->device_address();
    triangles.vertexStride = sizeof(glm::vec3);
    triangles.maxVertex = mesh->vertex_capacity();
    triangles.indexType = VK_INDEX_TYPE_UINT32;
    triangles.indexData.deviceAddress = mesh->index_buffer()->device_address();

    return create(mesh->context(),
                  VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
                  geometry,
                  mesh->triangle_count());
}

Handle<AccelerationStructure> AccelerationStructure::create(
    Context *context,
    VkAccelerationStructureTypeKHR type,
    const VkAccelerationStructureGeometryKHR &geometry,
    uint32_t primitive_count) {
    VkAccelerationStructureBuildGeometryInfoKHR build_info{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
    build_info.type = type;
    build_info.flags =
        VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR |
        VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries = &geometry;
    VkAccelerationStructureBuildSizesInfoKHR size{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
    context->device()->GetAccelerationStructureBuildSizesKHR(
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &build_info,
        &primitive_count,
        &size);

    auto acceleration_structure = context->create_acceleration_structure(
        type, size.accelerationStructureSize);

    auto scratch_buffer =
        context->create_buffer(size.buildScratchSize,
                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                               VMA_MEMORY_USAGE_GPU_ONLY);

    // Fill in other infos for acceleration build
    build_info.scratchData.deviceAddress = scratch_buffer->device_address();
    build_info.dstAccelerationStructure =
        acceleration_structure->acceleration_structure();

    context->queue()->submit_once([&](CommandBuffer *cmd) {
        ARS_PROFILER_SAMPLE_VK_ONLY(
            cmd, "Build Acceleration Structure", 0xFF183757);
        VkAccelerationStructureBuildRangeInfoKHR range{};
        range.primitiveCount = primitive_count;
        auto ranges = &range;
        cmd->BuildAccelerationStructuresKHR(1, &build_info, &ranges);
    });

    return acceleration_structure;
}

Handle<AccelerationStructure> AccelerationStructure::create(Scene *scene) {
    assert(scene != nullptr);

    auto context = scene->context();

    auto draw_requests = scene->gather_draw_request(RenderPassID_RayTracing);
    auto inst_cnt = draw_requests.size();

    Handle<Buffer> inst_buffer{};
    if (inst_cnt > 0) {
        inst_buffer = context->create_buffer(
            sizeof(VkAccelerationStructureInstanceKHR) * inst_cnt,
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU);
        inst_buffer->map_once([&](void *ptr) {
            auto inst_arr =
                reinterpret_cast<VkAccelerationStructureInstanceKHR *>(ptr);
            for (int i = 0; i < inst_cnt; i++) {
                auto &inst = inst_arr[i];
                auto &req = draw_requests[i];
                auto blas = req.mesh->acceleration_structure();
                assert(blas != nullptr);

                inst.transform = to_vk_xform(req.M);
                inst.instanceCustomIndex = i;
                inst.accelerationStructureReference = blas->device_address();
                inst.flags =
                    VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
                inst.mask = 0xFF;
                inst.instanceShaderBindingTableRecordOffset = 0;
            }
        });
    }

    VkAccelerationStructureGeometryKHR geometry{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
    geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    auto &instances = geometry.geometry.instances;
    instances.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    if (inst_buffer != nullptr) {
        instances.data.deviceAddress = inst_buffer->device_address();
    }

    return create(scene->context(),
                  VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
                  geometry,
                  inst_cnt);
}

VkDeviceAddress AccelerationStructure::device_address() const {
    if (_acceleration_structure == VK_NULL_HANDLE) {
        return 0;
    }

    VkAccelerationStructureDeviceAddressInfoKHR info{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR};
    info.accelerationStructure = _acceleration_structure;

    return _context->device()->GetAccelerationStructureDeviceAddressKHR(&info);
}

RayTracing::RayTracing(View *view) : _view(view) {}

void RayTracing::render(RenderGraph &rg) {
    // TODO
    return;
    auto ctx = _view->context();
    auto device = ctx->device();
    VkRayTracingPipelineCreateInfoKHR info{
        VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
    //device->CreateRayTracingPipelinesKHR();
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.access(NamedRT_LinearColor,
                           VK_ACCESS_SHADER_WRITE_BIT,
                           VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
                           false);
        },
        [=](CommandBuffer *cmd) {
            auto tex = _view->render_target(NamedRT_LinearColor);
            auto extent = tex->info().extent;
            VkStridedDeviceAddressRegionKHR raygen_sbt{};
            VkStridedDeviceAddressRegionKHR miss_sbt{};
            VkStridedDeviceAddressRegionKHR hit_sbt{};
            VkStridedDeviceAddressRegionKHR callable_sbt{};
            cmd->TraceRaysKHR(&raygen_sbt,
                              &miss_sbt,
                              &hit_sbt,
                              &callable_sbt,
                              extent.width,
                              extent.height,
                              1);
        });
}

bool RayTracing::supported(Context *context) {
    if (context == nullptr) {
        return false;
    }

    auto &rt_features = context->info().ray_tracing_pipeline_features;
    auto &acc_features = context->info().acceleration_structure_features;

    return rt_features.rayTracingPipeline && acc_features.accelerationStructure;
}
} // namespace ars::render::vk