public struct StandardSurfaceData {
    public float3 albedo;
    public float  roughness;
}

public interface IMaterial {
    StandardSurfaceData GetStandardData(float2 uv);
}

public struct PBRMaterial : IMaterial {
    uint2 base_color_texture_handle;
};