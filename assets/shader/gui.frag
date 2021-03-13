#version 430

in vec2 textureCoords;

out vec4 color;

uniform sampler2D guiTexture;
uniform float brightness;

void main(void){
	color = texture(guiTexture, textureCoords);
	color.a *= brightness;
}