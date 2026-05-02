#version 450

const vec2 VERTICES[3] = {
    vec2(-1.0,  1.0),
    vec2( 3.0,  1.0),
    vec2(-1.0, -3.0)
};

layout(location = 0) out vec2 TexCoord;

void main() {

    const vec2 vertex = VERTICES[gl_VertexIndex];

    gl_Position = vec4(vertex, 0.0, 1.0);

    TexCoord = vertex * 0.5 + 0.5;

}