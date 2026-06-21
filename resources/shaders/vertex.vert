#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;

uniform mat4 vp;
uniform mat4 model;

out vec3 post_vertex_position;
out vec3 post_vertex_norm;

void main() {
    gl_Position = vp * vec4(pos, 1.f);

    vec4 worldPos = model * vec4(pos, 1.f);
    

    post_vertex_position = vec3(worldPos);
    post_vertex_norm = mat3(transpose(inverse(model))) * norm;
}