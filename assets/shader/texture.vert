#version 430 core
/*
* Copyright 2019 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out VertexData {
	vec3 position_world;
	vec3 normal_world;
	vec2 uv;
} vert;

uniform sampler2D heightMap;
uniform float scaleXZ;
uniform float scaleY;

uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;
uniform mat3 normalMatrix;
uniform bool isTerrain;

void main() {
	//vert.normal_world = normalMatrix * normal;
	//vert.uv = uv;
	//vec4 position_world_ = modelMatrix * vec4(position, 1);
	//vert.position_world = position_world_.xyz;
	
	// texture coordinates based on position and scale in XZ plane
	vec3 newPos = position;
	if(isTerrain){
		vec2 texCoord = position.xz / scaleXZ - 0.5;
		float height = texture(heightMap, texCoord).r * scaleY;
		newPos.y = height;
	}
	// new position
	//vec4 newPos = vec4(position.x, height, position.z, 1.0);

	gl_Position = viewProjMatrix * modelMatrix * vec4(newPos.x, newPos.y, newPos.z, 1);
}