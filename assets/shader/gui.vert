#version 430

in vec2 position;

out vec2 textureCoords;

uniform mat4 transformationMatrix;

void main(void){

	textureCoords = vec2((position.x + 1.0) / 2.0, 1 - (position.y + 1.0) / 2.0);
	gl_Position = transformationMatrix * vec4(position, 0.999999, 1.0);
}