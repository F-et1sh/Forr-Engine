import "../../Context/GlobalContext";

public struct PBRMaterialData {
    public uint2  base_color_texture_handle;
    public float4 base_color_factor;
}

public struct PBRMaterial : IMaterial {
    typealias MaterialData = PBRMaterialData;    

    public float4 GetSurfaceData(float2 uv, PBRMaterialData data) {
        Sampler2D texture = reinterpret<Sampler2D>(data.base_color_texture_handle);
        float4 frag_color = texture.Sample(uv);
        return frag_color * data.base_color_factor;
    }
};

[[vk::push_constant]] int instance_index;

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

    float4 world_position = mul(float4(input.position, 1.0f), g_ModelMatrices[instance_index]);
    
    output.position = world_position.xyz;
    output.sv_position = world_position;
    output.normal = input.normal;
    output.texture_coord = input.texture_coord;
    
    return output;
}

[shader("fragment")]
float4 fragmentMain<T>(VertexStageOutput input) : SV_Target where T : IMaterial {
    return g_Materials[instance_index].GetSurfaceData(input.texture_coord);
}