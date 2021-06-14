#version 430 core

// gets position
layout(location = 0) in vec3 position;

// sends vertex position to tessellation controll shader
out vec4 vPosition;
out vec4 vFragPosLightSpace;

uniform sampler2D heightMap;
uniform float scaleXZ;
uniform float scaleY;
uniform mat4 lightSpaceMatrix;

void main(){

	// texture coordinates based on position and scale in XZ plane
	vec2 texCoord = position.xz / scaleXZ;

	float height = texture(heightMap, texCoord).r * scaleY;
	
	// new position
	vec4 newPos = vec4(position.x, height, position.z, 1.0);
	vPosition = vec4(position, 1);
	vFragPosLightSpace = lightSpaceMatrix * newPos;
}