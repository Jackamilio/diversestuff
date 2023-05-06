#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec3 fragPos;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec4 fragPosLightSpace;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 lsm;

void main()
{
    fragPos = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragPosLightSpace = lsm * vec4(fragPos, 1.0);
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}