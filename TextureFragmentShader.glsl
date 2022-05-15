#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform bool is_empty;

void main(){
	if (is_empty) {
		color = vec3(0.0f, 0.0f, 0.0f);
	} else {
		color = vec3(0.8f, 0.8f, 0.0f);
	}
}