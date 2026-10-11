public struct VertexStageOutput {
    public float4 sv_position : SV_Position;
};

[shader("vertex")]
VertexStageOutput vertexMain(uint vertex_index : SV_VertexID) {
    VertexStageOutput output;

    // create a procedural triangle
    float2 output_uv = float2((vertex_index << 1) & 2, vertex_index & 2);
    output.sv_position = float4(output_uv * 2.0f - 1.0f, 0.0f, 1.0f);
    
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexStageOutput input) : SV_Target {
    return float4(1.0f);
}