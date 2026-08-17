public interface IMaterial {
    associatedtype MaterialData;

    public float4 GetSurfaceData(float2 uv, MaterialData data);
}

public interface IBuffer {
    T Load<T>(uint byte_offset);
}

public struct OpenGlBuffer : IBuffer {
    ByteAddressBuffer buffer;
    
    public T Load<T>(uint byte_offset) {
        return buffer.Load<T>(byte_offset);
    }
}

public struct VulkanBuffer : IBuffer {
    uint64_t gpu_address;
    
    public T Load<T>(uint byte_offset) {
        T* ptr = (T*)(gpu_address + byte_offset);
        return *ptr;
    }
}

public [[vk::binding(0, 0)]] ParameterBlock<IBuffer>    g_MaterialData;
public [[vk::binding(1, 0)]] StructuredBuffer<float4x4> g_ModelMatrices;
public [[vk::binding(0, 1)]] ByteAddressBuffer          g_MaterialsRawData;