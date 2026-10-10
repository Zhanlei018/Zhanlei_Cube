#version 330 core

// ASCII ONLY, no BOM. (Both have broken this file before.)

in vec3 vNormal;
in float vU;
in float vV;

uniform vec3 uColor;

out vec4 fragColor;

void main() {
    vec3 N = normalize(vNormal);

    // Back-face culling is off, so we may be shading a back face here.
    // Flip the normal in that case or the lighting comes out inverted.
    if (!gl_FrontFacing) {
        N = -N;
    }

    // ---- lighting ----
    const vec3 L = normalize(vec3(0.45, 0.75, 0.55));
    const float ambient = 0.62;   // raised: the dark sides were too dark
    const float diffuse = 0.50;
    float ndl = max(dot(N, L), 0.0);
    float upShade = 1.0 + 0.08 * N.y;

    vec3 base = uColor * (ambient + diffuse * ndl) * upShade;

    // ---- black frame around each sticker ----
    // vU / vV are in [0,1] across the face, in the cubie's LOCAL space,
    // so this frame rotates with the cube instead of staying world-aligned.
    const float kFrameWidth = 0.035;
    float dEdge = min(min(vU, 1.0 - vU), min(vV, 1.0 - vV));

    // fwidth() gives the change of dEdge per screen pixel. Dividing by it makes
    // the transition always about one pixel wide, so the line is clean at any zoom.
    float aa = fwidth(dEdge);
    float frame = smoothstep(kFrameWidth - aa, kFrameWidth + aa, dEdge);

    vec3 col = base * mix(0.05, 1.0, frame);

    fragColor = vec4(col, 1.0);
}