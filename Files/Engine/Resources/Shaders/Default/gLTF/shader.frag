#version 450 core
layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec4 i_Position;

struct GPULight {
	//uint32_t type;
	
	//float range;
	//float inner_cone;
	//float outer_cone;
	
	vec4 position;
	vec4 direction;
	vec4 color_intensity;
};

layout (std430, set = 0, binding = 1) readonly buffer LightData {
uint lights_count;
GPULight lights[];
} light_data;

void main() {
	fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	
	for (uint i = 0; i < light_data.lights_count; i++) {
		GPULight light = light_data.lights[i];
		fragColor /= vec4(distance(i_Position.xyz, light.position.xyz) * 0.25f);
	}
}