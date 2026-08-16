public interface IMaterial {
    associatedtype MaterialData;

    public float4 GetSurfaceData(float2 uv, MaterialData data);
}

public [[vk::binding(1, 0)]] StructuredBuffer<float4x4>  g_ModelMatrices;
public [[vk::binding(0, 1)]] ByteAddressBuffer           g_MaterialsRawData;