public interface IMaterial {
    public float4 GetSurfaceData(float2 uv);
}

public interface IBuffer {
    public T Load<T>(uint byte_offset);
}

public struct OpenGLBuffer : IBuffer {
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

public type_param TBuffer : IBuffer;

public [[vk::binding(0, 0)]] ParameterBlock<TBuffer>    g_MaterialsRawData;
public [[vk::binding(1, 0)]] StructuredBuffer<float4x4> g_ModelMatrices;
