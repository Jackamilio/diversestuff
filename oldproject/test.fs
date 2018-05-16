#version 330

uniform sampler2D texMain;
uniform vec3 lAmbient;
uniform mat2x4 lpointLights[8];

in vec3 io_pos;
in vec3 io_nor;
in vec2 io_uv;
out vec4 FragColor;

void main()
{
	vec3 accum = lAmbient;
	for(int i=0; i<8; ++i) {
		vec3 posToLight = lpointLights[i][0].xyz - io_pos;
		float len = length(posToLight);
		if (len < lpointLights[i][0].w) {
			posToLight /= len;
			float d = dot(posToLight, io_nor);
			if (d > 0.0) {
				float h = 1.0f;
				if (d < 0.5) {
					h = 0.5f;
				}
				accum += lpointLights[i][1].xyz * h;
			}
		}
	}
    FragColor = vec4(texture(texMain, io_uv).xyz * accum, 1.0);
}
