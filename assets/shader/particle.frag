#version 430

in vec2 Vertex_UV;
out vec4 FragColor;
uniform sampler2D tex0;

void main() {
    vec4 tex = texture(tex0, Vertex_UV);
    FragColor = tex;
}