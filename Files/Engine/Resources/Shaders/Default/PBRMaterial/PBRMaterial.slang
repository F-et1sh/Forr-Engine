import "../../Context/GlobalContext.slang";

public struct PBRMaterialData {
    public uint2  base_color_texture_handle;
    public float4 base_color_factor;
}

public struct PBRMaterial : IMaterial {
    typealias MaterialData = PBRMaterialData;    

    public float4 GetSurfaceData(float2 uv, PBRMaterialData data) {
        //Sampler2D texture = reinterpret<Sampler2D>(data.base_color_texture_handle);
        //float4 frag_color = texture.Sample(uv);
        return float4(uv, 0.0f, 1.0f);
    }
};

struct PushConstants {
    int instance_index;
    int material_buffer_offset;
};
[[vk::push_constant]] PushConstants push_constants;

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
VertexStageOutput vertexMain<M : IMaterial>(VertexInput input) {
    VertexStageOutput output;
    float4x4 model_matrix = g_ModelMatrices[push_constants.instance_index];
    output.sv_position    = mul(model_matrix, float4(input.position, 1.0f));
    output.texture_coord  = input.texture_coord;
    return output;
}

[shader("fragment")]
float4 fragmentMain<M : IMaterial>(VertexStageOutput input) : SV_Target {
    //M material;
    //M.MaterialData data = g_MaterialsRawData.Load<M.MaterialData>(push_constants.material_buffer_offset);
    //return material.GetSurfaceData(input.texture_coord, data);
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}