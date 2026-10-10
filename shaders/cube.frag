#version 330 core

in vec3 vNormal;
uniform vec3 uColor;   
out vec4 fragColor;

void main() {
    vec3 N = normalize(vNormal);
    const vec3 L = normalize(vec3(0.45, 0.75, 0.55));
    const float ambient = 0.42;  
    const float diffuse = 0.58;
    float ndl = max(dot(N, L), 0.0);
    float upShade = 1.0 + 0.10 * N.y;
    vec3 lit = uColor * (ambient + diffuse * ndl) * upShade;
    fragColor = vec4(lit, 1.0);
}
