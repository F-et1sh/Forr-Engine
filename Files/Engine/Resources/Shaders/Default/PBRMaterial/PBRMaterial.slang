import "../../Context/GlobalContext.slang";
import "shader2.slang";

public struct PBRMaterial : IMaterial {
    public uint2  base_color_texture_handle;
    public float4 base_color_factor;

    public float4 GetSurfaceData(float2 uv) {
        Sampler2D texture = reinterpret<Sampler2D>(base_color_texture_handle);
        float4 frag_color = texture.Sample(uv);
        return frag_color;
    }
};

struct PushConstants {
    int instance_index;
    int material_buffer_offset;
};
[[vk::push_constant]] PushConstants push_constants;

[shader("vertex")]
VertexStageOutput vertexMain(VertexInput input) {
    VertexStageOutput output;
    float4x4 model_matrix = g_ModelMatrices[push_constants.instance_index];
    output.sv_position    = mul(model_matrix, float4(input.position, 1.0f));
    output.texture_coord  = input.texture_coord;
    return output;
}

[shader("fragment")]
float4 fragmentMain<M : IMaterial>(VertexStageOutput input) : SV_Target {
    M material = g_MaterialsRawData.Load<M>(push_constants.material_buffer_offset);
    return material.GetSurfaceData(input.texture_coord);
}