#version 330

// Input vertex attributes (from vertex shader)
in vec4 fragPosition;
in vec2 fragTexCoord;

// Input uniform values
uniform sampler2D texture0;     // Depth texture

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    float zNear = 0.01; // camera z near
    float zFar = 10.0;  // camera z far
    float z = texture(texture0, fragTexCoord).x;

    // Linearize depth value
    float depth = (2.0*zNear)/(zFar + zNear - z*(zFar - zNear));

    // Calculate final fragment color
    finalColor = vec4(depth, depth, depth , 1.0f);
    //finalColor = vec4(fragTexCoord.xy, 0.0f, 1.0f);
}