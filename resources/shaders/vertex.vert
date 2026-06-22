#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;

uniform mat4 vp;
uniform mat4 model;
uniform mat3 normal_matrix;

out vec3 post_vertex_position;
out vec3 post_vertex_norm;

void main() {
    vec4 worldPos = model * vec4(pos, 1.0);

    gl_Position = vp * worldPos;
    
    post_vertex_position = vec3(worldPos);
    
    post_vertex_norm = normalize(normal_matrix * norm);
}