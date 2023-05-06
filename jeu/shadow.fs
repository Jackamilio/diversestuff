#version 330

in vec3 fragPos;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec4 fragPosLightSpace;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform sampler2D shadowmap;
uniform vec3 lightpos;
uniform float bias;

float ShadowCalculation(vec4 fpls) {
    vec3 projCoords = fpls.xyz / fpls.w;
    projCoords = projCoords * 0.5f + 0.5f;
    if (projCoords.x < 0.0f || projCoords.x > 1.0f
     || projCoords.y < 0.0f || projCoords.y > 1.0f
     || projCoords.z < 0.0f || projCoords.z > 1.0f) {
        return 1.0f;
    }

    float currentDepth = projCoords.z;

    float shadow = 0.0f;
    vec2 texelSize = 1.0f / textureSize(shadowmap, 0);
    int pcfSampling = 2;
    for(int x=-pcfSampling; x<=pcfSampling; ++x) {
        for(int y=-pcfSampling; y<=pcfSampling; ++y) {
            float pcfDepth = texture(shadowmap, projCoords.xy + vec2(x,y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
        }
    }

    return 1.0f - shadow / 9.0f;
}

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);

    float shadow = ShadowCalculation(fragPosLightSpace);//1.0f - clamp(distance(fragPos, lightpos)/10.0f, 0.0f, 1.0f);
    finalColor = vec4( (texelColor*colDiffuse*fragColor*shadow).rgb, 1.0f);
}