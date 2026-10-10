#version 330 core

// ASCII ONLY, no BOM. (Both have broken this file before.)

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vNormal;
out float vU;   // two coordinates that vary across this face
out float vV;   // (used to draw the black frame around each sticker)

void main() {
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
    vNormal = mat3(uModel) * aNormal;

    // The mesh is a unit cube centred at the origin, so aPos is in [-0.5, 0.5].
    // Remap to [0, 1]. This is local space, so the frame rotates with the cubie
    // instead of staying fixed to the world.
    vec3 uv = aPos + vec3(0.5);

    // Pick the two components that actually vary on this face.
    // Hint: the face normal points along the axis that does NOT vary.
    // The mesh defines normals along X and Y, so we can use the Y component
    // of the normal to tell those two apart, and Z for the remaining case.
    vec3 an = abs(aNormal);
    vU = (an.x > 0.5) ? uv.z : ((an.y > 0.5) ? uv.z : uv.x);
    vV = (an.x > 0.5) ? uv.y : ((an.y > 0.5) ? uv.x : uv.y);
}