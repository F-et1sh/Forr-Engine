#version 450 core
layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec4 i_Position;
layout(location = 1) in vec4 i_Normal;

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
    vec3 base_color = vec3(1.0, 1.0, 1.0);
    vec3 accumulated_light = vec3(0.0);
    
    vec3 normal = normalize(i_Normal.xyz);

    for (uint i = 0; i < light_data.lights_count; i++) {
        GPULight light = light_data.lights[i];
        
        vec3 light_direction = light.position.xyz - i_Position.xyz;
        float dist = length(light_direction);
        light_direction = normalize(light_direction);
        
        float dot_nl = max(dot(normal, light_direction), 0.0);
        
        float attenuation = 1.0 / (1.0 + 0.1 * dist + 0.05 * dist * dist);
        
        vec3 light_color = light.color_intensity.rgb * light.color_intensity.a;
        
        accumulated_light += light_color * dot_nl * attenuation;
    }
    
    vec3 ambient = vec3(0.05) * base_color;
    
    fragColor = vec4(ambient + base_color * accumulated_light, 1.0);
}