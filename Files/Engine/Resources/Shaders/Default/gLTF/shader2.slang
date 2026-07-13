public struct StandardSurfaceData {
    public float3 albedo;
    public float  roughness;
}

public interface IMaterial {
    StandardSurfaceData GetStandardData(float2 uv);
}

public struct PBRMaterial : IMaterial {
    StandardSurfaceData GetStandardData(float2 uv)override { return {}; }
}