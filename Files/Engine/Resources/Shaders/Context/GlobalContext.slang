public struct SomeStruct {
    public float3 albedo;
}

public struct StandardSurfaceData {
    SomeStruct some_struct;
	public float  roughness;
}

public interface IMaterial {
    StandardSurfaceData GetStandardData(float2 uv);
}

public struct PBRMaterial : IMaterial {
    public uint2 base_color_texture_handle;

    public StandardSurfaceData GetStandardData(float2 uv) {
        StandardSurfaceData data;
        data.some_struct.albedo = float3(1.0, 0.0, 1.0);
        data.roughness = 0.5;
        return data;
    }
};

struct SceneData {
    float4x4 projection_matrix;
    float4x4 view_matrix;
    uint lights_count;
};

[[vk::binding(0, 0)]] ConstantBuffer<SceneData>     global_data;
[[vk::binding(1, 0)]] StructuredBuffer<float4x4>    model_matrices;
[[vk::binding(0, 1)]] StructuredBuffer<PBRMaterial> materials;