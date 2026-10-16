import "../../Context/GlobalContext.slang";
import "shader2.slang";

public ConstantBuffer<float4x4> g_LightProjection;

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
VertexStageOutput vertexMain<M : IMaterial>(VertexInput input) {
    VertexStageOutput output;

    float4x4 model_matrix = g_ModelMatrices[input.index];
    
    float4 world_position = mul(float4(input.position, 1.0f), model_matrix);
    float4 view_position  = mul(world_position, g_GlobalData.view);
    output.sv_position    = mul(view_position, g_GlobalData.projection);
    
    output.texture_coord  = input.texture_coord;
    output.world_position = world_position.xyz;

    output.light_position = mul(world_position, g_LightProjection);

    output.index = input.index;

    return output;
}

[shader("fragment")]
float4 fragmentMain<M : IMaterial>(VertexStageOutput input) : SV_Target { // temp lighting
    float4 base_color = float4(0.0f);
    if (input.index != 2) {
        M material = g_MaterialsRawData.Load<M>(input.index * 32);
        base_color = material.GetSurfaceData(input.texture_coord);
    }
    else {
        base_color = float4(float3(0.75f), 1.0f);
    }

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

    Sampler2D texture = reinterpret<Sampler2D>(4294969856);
    float shadow = 0.0f;
    float3 light_coords = input.light_position.xyz / input.light_position.w;
    if (light_coords.z <= 1.0f) {
        light_coords = (light_coords + 1.0f) / 2.0f;

        float closest_depth = texture.Sample(light_coords.xy).r;
        float current_depth = light_coords.z;

        float bias = 0.0002f;
        if (current_depth > closest_depth + bias) {
            shadow = 0.75f;
        }
    }

    float3 final_color = base_color.rgb * (1.0f - shadow);
    return float4(clamp(final_color, 0.0f, 1.0f), base_color.a);
}