#version 430

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;
uniform mat4 ProjectionMatrix;

out vec2 Vertex_UV;

void main() {
    const vec2 size = vec2(0.5, 0.5);
    vec4 P = gl_in[0].gl_Position;

     // a: left-bottom 
    vec2 va = P.xy + vec2(-0.5, -0.5) * size;
    gl_Position = ProjectionMatrix * vec4(va, P.zw);
    Vertex_UV = vec2(0.0f, 0.0f);
    EmitVertex();  
    
    // d: right-bottom
    vec2 vd = P.xy + vec2(0.5, -0.5) * size;
    gl_Position = ProjectionMatrix * vec4(vd, P.zw);
    Vertex_UV = vec2(1.0f, 0.0f);
    EmitVertex();  


    // b: left-top
    vec2 vb = P.xy + vec2(-0.5, 0.5) * size;
    gl_Position = ProjectionMatrix * vec4(vb, P.zw);
    Vertex_UV = vec2(0.0f, 1.0f);
    EmitVertex();  
    

    // c: right-top
    vec2 vc = P.xy + vec2(0.5, 0.5) * size;
    gl_Position = ProjectionMatrix * vec4(vc, P.zw);
    Vertex_UV = vec2(1.0f, 1.0f);
    EmitVertex();  
    
    EndPrimitive();
}  