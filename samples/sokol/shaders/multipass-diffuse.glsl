@ctype mat4 math::Matrix4f
@ctype vec3 math::Vector3f
@ctype vec4 math::Vector4f

@vs vs_diffuse
uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec4 vertex_pos;
in vec4 vertex_color;
in vec3 vertex_normal;

out vec3 frag_pos;
out vec3 frag_normal;

void main() {
    frag_pos = vec3(model * vertex_pos);
    frag_normal = vertex_normal;
    frag_normal = mat3(transpose(inverse(model))) * vertex_normal; // normal in world space - transp(inv()) is needed to take into account the scaling
    gl_Position = projection * view * model * vertex_pos;
}
@end

@fs fs_diffuse
uniform fs_params {
    vec4 color;
};

// uniform uint object_id;

in vec3 frag_pos;
in vec3 frag_normal;

layout(location=0) out vec4 frag_color;

void main() {
    // ambient
    vec3 light_color = vec3(1,1,1);
    vec3 light_pos = vec3(100,100,100);
    float ambient_strenght = 0.1;
    vec3 ambient = ambient_strenght * light_color;
  	
    // diffuse 
    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    // float diff = max(dot(norm, light_dir), 0.0); // classical approach: no light for angles >90deg
    float diff = (dot(norm, light_dir) + 1.0)/2.0; // modified (non-physically correct) approach: consider all 180deg
    vec3 diffuse = diff * light_color;
            
    frag_color = vec4(ambient + diffuse,1.0) * color;
}
@end

@program offscreen_diffuse vs_diffuse fs_diffuse

