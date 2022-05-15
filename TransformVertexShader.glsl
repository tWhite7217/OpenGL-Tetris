#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 normal_modelspace;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 normal_cameraspace;
out vec3 eyeDirection_cameraspace;
out vec3 lightDirection_cameraspace;
out vec3 lightPosition_cameraspace;

// Values that stay constant for the whole mesh.
uniform vec3 lightPosition_worldspace;
uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;

	vec3 vertexPosition_cameraspace = (V * M * vec4(vertexPosition_modelspace,1)).xyz;
	normal_cameraspace = (V * M * vec4(normal_modelspace,1)).xyz;
	eyeDirection_cameraspace = vec3(0) - vertexPosition_cameraspace;
	lightPosition_cameraspace = (V * vec4(lightPosition_worldspace,1)).xyz;
	lightDirection_cameraspace = vertexPosition_cameraspace - lightPosition_cameraspace;
	// lightDirection_cameraspace = lightPosition_cameraspace - vertexPosition_cameraspace;
}