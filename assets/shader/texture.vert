#version 430 core
/*
* Copyright 2019 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;


uniform sampler2D heightMap;
uniform float scaleXZ;
uniform float scaleY;

uniform mat4 normalMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;

out vec2 texCoord;

void main() {
	vec3 newPos = vec3(modelMatrix * vec4(position, 1));
	texCoord = newPos.xz / scaleXZ - 0.5;
	float height = texture(heightMap, texCoord).r * scaleY;

	gl_Position = viewProjMatrix * vec4(newPos.x, newPos.y + height, newPos.z, 1);
}