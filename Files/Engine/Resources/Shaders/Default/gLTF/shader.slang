struct GPULight {
    float4 position;
    float4 direction;
    float4 color_intensity;
};

struct PBRMaterial {
    uint2 base_color_texture_handle;
};

ParameterBlock<SceneDataResource> scene_set;
ParameterBlock<MaterialDataResource> material_set;

struct SceneDataResource {
    float4x4 projection_matrix;
    float4x4 view_matrix;
    StructuredBuffer<float4x4> model_matrices;
    StructuredBuffer<GPULight> lights;
    uint lights_count;
};

struct MaterialDataResource {
    StructuredBuffer<PBRMaterial> materials;
};

struct PushConstants {
    int instance_index;
};
ParameterBlock<PushConstants> constants;

struct VertexInput {
    [[vk::location(0)]] float3 position    : POSITION;
    [[vk::location(1)]] float3 normal      : NORMAL;
    [[vk::location(2)]] float2 texture_coord : TEXCOORD;
};

struct VertexStageOutput {
    float4 sv_position : SV_Position;

    [[vk::location(0)]] float3 position      : POSITION;
    [[vk::location(1)]] float3 normal        : NORMAL;
    [[vk::location(2)]] float2 texture_coord : TEXCOORD;
};

[shader("vertex")]
VertexStageOutput vertexMain(VertexInput input) {
    VertexStageOutput output;

    int index = constants.instance_index;

    float4 worldPosition = mul(float4(input.position, 1.0f), scene_set.model_matrices[index]);
    
    output.sv_position = mul(worldPosition, scene_set.view_matrix);
    output.sv_position = mul(output.sv_position, scene_set.projection_matrix);
    
    output.position = worldPosition.xyz;
    output.normal = input.normal;
    output.texture_coord = input.texture_coord;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexStageOutput input) : SV_Target {
     int index = constants.instance_index;
    
    uint2 handle = material_set.materials[index].base_color_texture_handle;
    
    Sampler2D tex = reinterpret<Sampler2D>(handle);
    
    float4 fragColor = tex.Sample(input.texture_coord);
    
    return fragColor;
}
