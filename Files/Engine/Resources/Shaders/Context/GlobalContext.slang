public interface IMaterial {
    float4 GetSurfaceData(float2 uv);
}

public [[vk::binding(1, 0)]] StructuredBuffer<float4x4>  g_ModelMatrices;
public [[vk::binding(0, 1)]] StructuredBuffer<IMaterial> g_Materials;