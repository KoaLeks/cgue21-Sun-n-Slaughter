#version 430

layout (location = 0) in vec4 Position;

uniform mat4 ModelViewMatrix;

void main() {
    gl_Position = ModelViewMatrix * vec4(Position.xyz, 1.0f);
}