#version 430 core
layout (location = 0) in vec3 aPos;

uniform sampler2D heightMap;
uniform float scaleXZ;
uniform float scaleY;
uniform mat4 lightSpaceMatrix;
uniform mat4 modelMatrix;
uniform bool isTerrain;

void main()
{
	vec3 newPos = aPos;
	if(isTerrain){
		vec2 texCoord = aPos.xz / scaleXZ - 0.5;
		float height = texture(heightMap, texCoord).r * scaleY;
		newPos.y = height;
	}
    gl_Position = lightSpaceMatrix * modelMatrix * vec4(newPos.x, newPos.y, newPos.z, 1.0);
} 