#version 450 core
layout(location = 0) out vec4 fragColor;

layout (location = 0) in vec3 i_Normal;
layout (location = 1) in vec3 v_LocalPos;
layout (location = 2) in vec3 v_WorldPos;

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

FORR_LAYOUT(0, 0) readonly buffer SceneData {
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 model_matrices[];
} scene_data;

FORR_LAYOUT(0, 1) readonly buffer LightData {
uint lights_count;
GPULight lights[];
} light_data;

void main() {
	float time = scene_data.projection_matrix[0][0] * 5.0f + scene_data.view_matrix[3][0];
	
	float wave_x = sin(v_WorldPos.x * 10.0f + time);
	float wave_y = cos(v_WorldPos.y * 10.0f - time * 1.5f);
	float combined_wave = abs(wave_x + wave_y) * 0.5f;
	
	vec3 base_color = vec3(0.0f, 0.4f, 0.9f);
	vec3 glow_color = vec3(0.0f, 1.0f, 0.7f);
	
	float scanline = smoothstep(0.7f, 0.95f, combined_wave);
	vec3 final_rgb = mix(base_color, glow_color, scanline);
	
	final_rgb += vec3(0.3f, 0.0f, 0.5f) * (v_LocalPos.y + 0.5f);
	
	vec3 surface_color = final_rgb * (1.0f + scanline * 2.0f);

	vec3 accumulated_light = vec3(0.0f);
	vec3 normal = normalize(i_Normal);

	for (uint i = 0; i < light_data.lights_count; i++) {
		GPULight light = light_data.lights[i];
		
		vec3 light_dir = light.position.xyz - v_WorldPos;
		float dist = length(light_dir);
		light_dir = normalize(light_dir);
		
		float dot_nl = max(dot(normal, light_dir), 0.0f);
		
		float attenuation = 1.0f / (1.0f + 0.1f * dist + 0.05f * dist * dist);
		
		vec3 light_color = light.color_intensity.rgb * light.color_intensity.a;
		
		accumulated_light += light_color * dot_nl * attenuation;
	}
	
	vec3 ambient_light = vec3(0.05f, 0.07f, 0.1f);
	
	vec3 final_color = surface_color * (ambient_light + accumulated_light);
	
	fragColor = vec4(final_color, 1.0f);
}
