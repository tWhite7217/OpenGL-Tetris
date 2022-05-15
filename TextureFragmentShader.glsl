#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform bool is_line;

void main(){
	if (is_line) {
		color = vec3(1.0f, 0.0f, 1.0f);
	} else {
		color = vec3(0.8f, 0.8f, 0.0f);
	}
}