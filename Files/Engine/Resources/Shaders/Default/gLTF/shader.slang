struct GPULight {
    float4 position;
    float4 direction;
    float4 color_intensity;
};

struct PBRMaterial {
    uint2 base_color_texture_handle;
};

struct SceneData {
    float4x4 projection_matrix;
    float4x4 view_matrix;
    uint lights_count;
};

struct SceneDataResource {
    SceneData global_data;
    StructuredBuffer<float4x4> model_matrices;
    StructuredBuffer<GPULight> lights;
};

struct MaterialDataResource {
    StructuredBuffer<PBRMaterial> materials;
};

[[vk::binding(0, 0)]] ParameterBlock<SceneDataResource> scene_set;
[[vk::binding(0, 1)]] ParameterBlock<MaterialDataResource> material_set;

struct VertexInput {
    [[vk::location(0)]] float3 position    : POSITION;
    [[vk::location(1)]] float3 normal      : NORMAL;
    [[vk::location(2)]] float2 texture_coord : TEXCOORD;
    [[vk::location(3)]] int instance_index : INSTANCE_INDEX;
};

struct VertexStageOutput {
    float4 sv_position : SV_Position;

    [[vk::location(0)]] float3 position      : POSITION;
    [[vk::location(1)]] float3 normal        : NORMAL;
    [[vk::location(2)]] float2 texture_coord : TEXCOORD;
    [[vk::location(3)]] int instance_index : INSTANCE_INDEX;
};

[shader("vertex")]
VertexStageOutput vertexMain(VertexInput input, [Layout(location = 0)] uniform int instance_index) {
    VertexStageOutput output;

    int index = instance_index;

    float4 world_position = mul(float4(input.position, 1.0f), scene_set.model_matrices[index]);
    
    output.sv_position = mul(world_position, scene_set.global_data.view_matrix);
    output.sv_position = mul(output.sv_position, scene_set.global_data.projection_matrix);
    
    output.position = world_position.xyz;
    output.normal = input.normal;
    output.texture_coord = input.texture_coord;
    output.instance_index = instance_index;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexStageOutput input) : SV_Target {
     int index = input.instance_index;
    
    uint2 handle = material_set.materials[index].base_color_texture_handle;
    
    Sampler2D tex = reinterpret<Sampler2D>(handle);
    
    float4 frag_color = tex.Sample(input.texture_coord);
    
    return float4(1.0f);
}
