#version 410 core

in vec3 post_vertex_position;
in vec3 post_vertex_norm;

out vec4 fragment_color;

uniform vec3 cam_pos;

// Proprietà del materiale
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;

// Proprietà della luce
struct Light {
    vec3 direct_pos;
    vec3 direct_val;
    vec3 ambient_val;
};
uniform Light light;

void main() {
    vec3 L = normalize(light.direct_pos - post_vertex_position);
    vec3 V = normalize(cam_pos - post_vertex_position);
    vec3 H = normalize(L + V);
    vec3 N = normalize(post_vertex_norm);

    // Calcolo della componente ambientale della luce
    vec3 ambient_light = material.ambient * light.ambient_val;

    // Calcolo della componente diffusa della luce
    float diff_factor = max(0.0, dot(N, L));
    vec3 diffuse_light = material.diffuse * light.direct_val * diff_factor;

    // Calcolo della componente speculare della luce
    float spec_factor = pow(max(0.0, dot(N, H)), material.shininess);
    if (diff_factor <= 0.0) {
        spec_factor = 0.0;
    }
    vec3 specular_light = material.specular * light.direct_val * spec_factor;

    vec3 tot_light = ambient_light + diffuse_light + specular_light;

    // Calcolo e restituzione del colore interpolato per il frammento
    fragment_color = vec4(clamp(tot_light, 0.f, 1.f), 1.0);
}