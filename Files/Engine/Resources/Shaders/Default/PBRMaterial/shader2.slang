module M_Material;

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