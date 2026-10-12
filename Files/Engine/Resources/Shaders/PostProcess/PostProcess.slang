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

public [[vk::binding(0, 0)]] StructuredBuffer<float> g_Time;

[shader("fragment")]
float4 fragmentMain(VertexStageOutput input) : SV_Target {
    float time = g_Time[0];
    
    float2 current_pos = input.sv_position.xy;
    
    float wave_frequency = 0.05f;
    float wave_speed = 0.01f;
    float wave_amplitude = 8.0f;
    
    float offsetX = sin(current_pos.y * wave_frequency + time * wave_speed) * wave_amplitude;
    float offsetY = cos(current_pos.x * wave_frequency + time * wave_speed) * wave_amplitude;
    
    int2 distorted_coords = int2(current_pos + float2(offsetX, offsetY));
    
    Sampler2D texture = reinterpret<Sampler2D>(4294969856);
    
    float4 frag_color = texture.Load(int3(distorted_coords, 0));
    
    return frag_color;
}
