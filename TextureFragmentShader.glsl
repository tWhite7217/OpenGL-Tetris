#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 normal_cameraspace;
in vec3 eyeDirection_cameraspace;
in vec3 lightDirection_cameraspace;
in vec3 lightPosition_cameraspace;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform int piece_type;
uniform float ambient_component;
uniform float diffuse_component;
uniform int specular_exponent;

vec3 draw_with_lighting(vec3 base_color) {
	vec3 n = normalize(normal_cameraspace);
	vec3 l = normalize(lightDirection_cameraspace);
	float cosTheta = clamp( dot( n,l ), 0,1 );
   
	vec3 E = normalize(eyeDirection_cameraspace);
	vec3 R = reflect(l,n);
	float cosAlpha = clamp( dot( E,R ), 0,1 );

	vec3 ambient = base_color * ambient_component;
	vec3 diffuse = base_color * cosTheta * diffuse_component;
	vec3 specular = base_color * pow(cosAlpha, specular_exponent);

	return ambient + diffuse + specular;
}

void main(){
	vec3 base_color;

	if (piece_type == 0) {
		base_color = vec3(0.1f, 1.0f, 1.0f);
	} else if (piece_type == 1) {
		base_color = vec3(0.0f, 0.12f, 0.7f);
	} else if (piece_type == 2) {
		base_color = vec3(1.0f, 0.5f, 0.0f);
	} else if (piece_type == 3) {
		base_color = vec3(1.0f, 0.9f, 0.0f);
	} else if (piece_type == 4) {
		base_color = vec3(0.3f, 0.9f, 0.0f);
	} else if (piece_type == 5) {
		base_color = vec3(1.0f, 0.0f, 0.0f);
	} else {
		base_color = vec3(0.6f, 0.0f, 0.6f);
	}
	
	// color = base_color;

	color = draw_with_lighting(base_color);
}