public struct VertexInput {
    public [[vk::location(0)]] float3 position      : POSITION;
    public [[vk::location(1)]] float3 normal        : NORMAL;
    public [[vk::location(2)]] float2 texture_coord : TEXCOORD;
};

public struct VertexStageOutput {
    public float4 sv_position : SV_Position;

    public [[vk::location(0)]] float3 position      : POSITION;
    public [[vk::location(1)]] float3 normal        : NORMAL;
    public [[vk::location(2)]] float2 texture_coord : TEXCOORD;
    public [[vk::location(3)]] float3 world_position : WorldPosition;
};