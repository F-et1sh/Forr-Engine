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

struct MaterialDataResource {
    StructuredBuffer<PBRMaterial> materials;
};

[[vk::binding(0, 0)]] ConstantBuffer<SceneData>   global_data;
[[vk::binding(1, 0)]] StructuredBuffer<float4x4>  model_matrices;
[[vk::binding(2, 0)]] StructuredBuffer<GPULight>  lights;
[[vk::binding(0, 1)]] StructuredBuffer<PBRMaterial> materials;

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

    float4 world_position = mul(float4(input.position, 1.0f), model_matrices[index]);
    
    output.sv_position = mul(world_position, global_data.view_matrix);
    output.sv_position = mul(output.sv_position, global_data.projection_matrix);
    
    output.position = world_position.xyz;
    output.normal = input.normal;
    output.texture_coord = input.texture_coord;
    output.instance_index = instance_index;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexStageOutput input) : SV_Target {
     int index = input.instance_index;
    
    uint2 handle = materials[index].base_color_texture_handle;
    
    Sampler2D texture = reinterpret<Sampler2D>(handle);
    
    float4 frag_color = texture.Sample(input.texture_coord);
    
    return float4(1.0f);
}
