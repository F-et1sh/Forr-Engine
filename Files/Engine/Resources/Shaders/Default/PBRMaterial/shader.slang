import "shader2.slang";

struct GPULight {
    float4 position;
    float4 direction;
    float4 color_intensity;
};

struct SceneData {
    float4x4 projection_matrix;
    float4x4 view_matrix;
    uint lights_count;
};

//[[vk::binding(0, 0)]] ConstantBuffer<SceneData>     global_data;  // per-frame
[[vk::binding(1, 0)]] StructuredBuffer<float4x4>    model_matrices; // per-frame
//[[vk::binding(2, 0)]] StructuredBuffer<GPULight>    lights;       // per-pass ( maybe )
//[[vk::binding(0, 1)]] StructuredBuffer<PBRMaterial> materials;    // per-frame

[[vk::push_constant]] int instance_index;   // per-model

struct VertexInput {
    [[vk::location(0)]] float3 position      : POSITION;
    [[vk::location(1)]] float3 normal        : NORMAL;
    [[vk::location(2)]] float2 texture_coord : TEXCOORD;
};

struct VertexStageOutput {
    float4 sv_position : SV_Position;

    [[vk::location(0)]] float3 position      : POSITION;
    [[vk::location(1)]] float3 normal        : NORMAL;
    [[vk::location(2)]] float2 texture_coord : TEXCOORD;
};

[shader("vertex")]
VertexStageOutput vertexMain<T>(VertexInput input) where T : IMaterial {
    VertexStageOutput output;

    //float4 world_position = mul(float4(input.position, 1.0f), model_matrices[instance_index]);
    
    //output.sv_position = mul(world_position, global_data.view_matrix);
    //output.sv_position = mul(output.sv_position, global_data.projection_matrix);
    
    output.position = input.position;
    output.sv_position = float4(input.position, 1.0f);
    output.normal = input.normal;
    output.texture_coord = input.texture_coord;
    
    return output;
}

[shader("fragment")]
float4 fragmentMain<T>(VertexStageOutput input) : SV_Target where T : IMaterial {
    //uint2 handle = materials[instance_index].base_color_texture_handle;
    //
    //Sampler2D texture = reinterpret<Sampler2D>(handle);
    //
    //float4 frag_color = texture.Sample(input.texture_coord);
    
    return float4(1.0f);
}