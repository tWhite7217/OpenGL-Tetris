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

vec3 draw_with_lighting(vec3 base_color) {
	vec3 n = normalize(normal_cameraspace);
	vec3 l = normalize(lightDirection_cameraspace);
	float cosTheta = clamp( dot( n,l ), 0,1 );
   
	vec3 E = normalize(eyeDirection_cameraspace);
	vec3 R = reflect(l,n);
	float cosAlpha = clamp( dot( E,R ), 0,1 );

	vec3 ambient = base_color * 0.1f;
	vec3 diffuse = base_color * cosTheta * 0.8;
	// vec3 diffuse = vec3(0);
	vec3 specular = base_color * pow(cosAlpha, 50);
	// vec3 specular = vec3(0);

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