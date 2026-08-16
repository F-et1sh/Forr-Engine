public interface IMaterial {
    associatedtype MaterialData;

    float4 GetSurfaceData(float2 uv, MaterialData data);
}

public [[vk::binding(1, 0)]] StructuredBuffer<float4x4>  g_ModelMatrices;
public [[vk::binding(0, 1)]] StructuredBuffer<IMaterial> g_Materials;