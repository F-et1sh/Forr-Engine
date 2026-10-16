public struct VertexStageInput {
    public [[vk::location(0)]] float3 position      : POSITION;
    public [[vk::location(1)]] float3 normal        : NORMAL;
    public [[vk::location(2)]] float2 texture_coord : TEXCOORD;

    public int index : SV_StartInstanceLocation;
};

public struct VertexStageOutput {
    public float4 sv_position : SV_Position;
};

public [[vk::binding(1, 1)]] StructuredBuffer<float4x4> g_ModelMatrices;
public [[vk::binding(1, 2)]] ConstantBuffer<float4x4> g_LightProjection;

[shader("vertex")]
VertexStageOutput vertexMain(VertexStageInput input) {
    VertexStageOutput output;

    float4x4 model_matrix = g_ModelMatrices[input.index];
    float4 world_position = mul(float4(input.position, 1.0f), model_matrix);
    output.sv_position  = mul(world_position, g_LightProjection);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexStageOutput input) {
    return float4(1.0f);
}
