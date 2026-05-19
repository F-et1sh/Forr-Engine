#version 460
layout(location = 0) out vec4 fragColor;

layout (std430, binding = 0) readonly buffer SceneData {
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 model_matrices[32];
} scene_data;

layout (std430, set = 1, binding = 0) readonly buffer MaterialData {
mat4 some_data[];
} material_data;

layout (location = 0) in vec3 v_LocalPos;
layout (location = 1) in vec3 v_WorldPos;

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
	
	fragColor = vec4(final_rgb * (1.0f + scanline * 2.0f), 1.0f);
}
