#version 450 core
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_bindless_texture : enable

layout (location = 0) out vec4 fragColor;

layout (location = 0) in vec3 i_Position;
layout (location = 1) in vec3 i_Normal;
layout (location = 2) in vec2 i_TextureCoord;

#define FORR_BINDING_COUNT_PER_SET 4

#ifdef FORR_USE_OPENGL
    #define FORR_LAYOUT(set_index, binding_index) layout(binding = (set_index * FORR_BINDING_COUNT_PER_SET) + binding_index)
#else
    #define FORR_LAYOUT(set_index, binding_index) layout(set = set_index, binding = binding_index)
#endif

struct GPULight {
	//uint32_t type;
	
	//float range;
	//float inner_cone;
	//float outer_cone;
	
	vec4 position;
	vec4 direction;
	vec4 color_intensity;
};

struct PBRMaterial {
    uvec2 base_color_texture_handle;
};

FORR_LAYOUT(0, 0) readonly buffer SceneData {
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 model_matrices[];
} scene_data;

FORR_LAYOUT(0, 1) readonly buffer LightData {
uint lights_count;
GPULight lights[];
} light_data;

FORR_LAYOUT(1, 0) readonly buffer MaterialData {
PBRMaterial materials[];
} material_data;

#ifdef FORR_USE_OPENGL
layout (location = 0) uniform int instance_index;
#else
layout(push_constant) uniform PushConstants {
	int instance_index;
} constants;
#endif

void main() {
	int index =
#ifdef FORR_USE_OPENGL
		instance_index;
#else
		constants.instance_index;
#endif

    uvec2 handle = material_data.materials[index].base_color_texture_handle;
    
    sampler2D tex = sampler2D(handle); 
    fragColor = texture(tex, i_TextureCoord);

    //vec3 base_color = vec3(1.0, 1.0, 1.0);
    //
    ////if (i_Position.y > 0.15)
    ////    base_color = vec3(0.0f, 1.0f, 0.1f);
    ////else if (i_Position.y < 0.15 && i_Position.y > 0)
    ////    base_color = vec3(1.0f, 1.0f, 1.0f);
    ////else if (i_Position.y < 0)
    ////    base_color = vec3(1.0f, 0.0f, 0.1f);
    //
    //vec3 accumulated_light = vec3(0.0);
    //
    //vec3 normal = normalize(i_Normal);
    //
    //for (uint i = 0; i < light_data.lights_count; i++) {
    //    GPULight light = light_data.lights[i];
    //    
    //    vec3 light_direction = light.position.xyz - i_Position;
    //    float dist = length(light_direction);
    //    light_direction = normalize(light_direction);
    //    
    //    float dot_nl = max(dot(normal, light_direction), 0.0);
    //    
    //    float attenuation = 1.0 / (1.0 + 0.1 * dist + 0.05 * dist * dist);
    //    
    //    vec3 light_color = light.color_intensity.rgb * light.color_intensity.a;
    //    
    //    accumulated_light += light_color * dot_nl * attenuation;
    //}
    //
    //vec3 ambient = vec3(0.05) * base_color;
    //
    ////fragColor = vec4(ambient + base_color * accumulated_light, 1.0);
    //fragColor = vec4(i_TextureCoord.x, i_TextureCoord.y, 1.0f, 1.0f);
}
