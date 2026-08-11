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

struct GPULight {
    float4 position;
    float4 direction;
    float4 color_intensity;
};

struct SceneData {
    float4x4 projection_matrix;
    float4x4 view_matrix;
    uint lights_count;
};

//[[vk::binding(0, 0)]] ConstantBuffer<SceneData>     global_data;
//[[vk::binding(1, 0)]] StructuredBuffer<float4x4>    model_matrices;
//[[vk::binding(2, 0)]] StructuredBuffer<GPULight>    lights;
//[[vk::binding(0, 1)]] StructuredBuffer<PBRMaterial> materials;
//
//[[vk::push_constant]] int instance_index;

struct VertexInput {
    [[vk::location(0)]] int vertex_id : VERTEX_ID_ATTR;
};

struct VertexStageOutput {
    float4 sv_position : SV_Position;

    [[vk::location(0)]] float3 position      : POSITION;
    //[[vk::location(1)]] float3 normal        : NORMAL;
    //[[vk::location(2)]] float2 texture_coord : TEXCOORD;
};

[shader("vertex")]
VertexStageOutput vertexMain<T>(VertexInput input) where T : IMaterial {
    VertexStageOutput output;

    //float4 world_position = mul(float4(input.position, 1.0f), model_matrices[instance_index]);
    //
    //output.sv_position = mul(world_position, global_data.view_matrix);
    //output.sv_position = mul(output.sv_position, global_data.projection_matrix);
    //
    //output.position = world_position.xyz;
    //output.normal = input.normal;
    //output.texture_coord = input.texture_coord;

    float2 positions[3] = {
        float2( 0.0f,  0.5f),
        float2(-0.5f, -0.5f),
        float2( 0.5f, -0.5f)
    };

    output.sv_position = float4(positions[input.vertex_id], 0.0f, 1.0f);
    output.position = float3(positions[input.vertex_id], 0.0f);
    
    return output;
}

[shader("fragment")]
float4 fragmentMain<T>(VertexStageOutput input) where T : IMaterial {
    //uint2 handle = materials[instance_index].base_color_texture_handle;
    //
    //Sampler2D texture = reinterpret<Sampler2D>(handle);
    //
    //float4 frag_color = texture.Sample(input.texture_coord);
    
    return float4(1.0f);
}
