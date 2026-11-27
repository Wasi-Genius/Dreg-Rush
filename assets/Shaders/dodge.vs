#version 330

// Standard Raylib vertex attributes supplied by DrawTexturePro
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

// Model-View-Projection matrix (includes Camera2D transform)
uniform mat4 mvp;

// Outputs interpolated for every pixel in the fragment shader
out vec2 fragTexCoord;
out vec4 fragColor;
out vec2 fragPosition;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor    = vertexColor;
    // Pass world-space XY position so the fragment shader can compute
    // distances from the protagonist in world pixels
    fragPosition = vertexPosition.xy;

    gl_Position  = mvp * vec4(vertexPosition, 1.0);
}
