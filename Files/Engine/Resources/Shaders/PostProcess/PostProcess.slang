public struct VertexStageInput {
    public [[vk::location(0)]] float2 position : POSITION;
};

public struct VertexStageOutput {
    public float4 sv_position : SV_Position;
};

[shader("vertex")]
VertexStageOutput vertexMain(VertexStageInput input) {
    VertexStageOutput output;
    output.sv_position = float4(input.position, 0.0f, 1.0f);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexStageOutput input) : SV_Target {
    return float4(1.0f, 1.0f, 1.0f, 0.15f);
}
