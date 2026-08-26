import "../../Context/GlobalContext.slang";
import "shader2.slang";

public struct PBRMaterial : IMaterial {
    public uint2  base_color_texture_handle;
    public float4 base_color_factor;

    public float4 GetSurfaceData(float2 uv) {
        Sampler2D texture = reinterpret<Sampler2D>(base_color_texture_handle);
        float4 frag_color = texture.Sample(uv);
        return frag_color;
    }
};

public struct PushConstants {
    public int instance_index;
    public int material_buffer_offset;
};
public [[vk::push_constant]] PushConstants push_constants;

public struct SpotLight {
    public float3 position;
    public float3 direction;
    public float3 color;
    public float2 cone_angles;
};

[shader("vertex")]
VertexStageOutput vertexMain(VertexInput input) {
    VertexStageOutput output;

    float4x4 model_matrix = g_ModelMatrices[input.index];
    
    float4 world_position = mul(float4(input.position, 1.0f), model_matrix);
    float4 view_position  = mul(world_position, g_GlobalData.view);
    output.sv_position    = mul(view_position, g_GlobalData.projection);
    
    output.texture_coord  = input.texture_coord;
    output.world_position = world_position.xyz;

    output.index = input.index;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexStageOutput input) : SV_Target { // temp lighting
    PBRMaterial material = g_MaterialsRawData.Load<PBRMaterial>(input.index * 32);
    float4 base_color = material.GetSurfaceData(input.texture_coord);

    SpotLight lights[2];
    
    lights[0].position = float3(-3.0f, 3.0f, 6.0f);
    lights[0].direction = normalize(float3(0.0f, 0.0f, 0.0f) - lights[0].position);
    lights[0].color = float3(1.2f, 1.2f, 1.2f);
    lights[0].cone_angles = float2(cos(3.1415 / 12.0), cos(3.1415 / 8.0));

    lights[1].position = float3(4.0f, 2.0f, 5.0f);
    lights[1].direction = normalize(float3(0.0f, 0.0f, 0.0f) - lights[1].position);
    lights[1].color = float3(0.9f, 0.9f, 0.9f);
    lights[1].cone_angles = float2(cos(3.1415 / 10.0), cos(3.1415 / 6.0));

    float3 accumulated_light = float3(0.15f);

    for (int i = 0; i < 2; i++) {
        float3 light_dir = lights[i].position - input.world_position;
        light_dir = normalize(light_dir);

        float theta = dot(light_dir, -lights[i].direction);
        
        float epsilon = lights[i].cone_angles.x - lights[i].cone_angles.y;
        float intensity = clamp((theta - lights[i].cone_angles.y) / epsilon, 0.0f, 1.0f);

        accumulated_light += lights[i].color * intensity;
    }

    float3 final_color = base_color.rgb * accumulated_light;
    return float4(clamp(final_color, 0.0f, 1.0f), base_color.a);
}