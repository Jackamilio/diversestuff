#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nor;
layout (location = 2) in vec2 uv;
layout (location = 3) in ivec4 boneids;
layout (location = 4) in vec4 weights;

uniform mat4 trFinal;
uniform mat4 trWorld;
uniform mat4 bones[64];
uniform int animate;

out vec3 io_pos;
out vec3 io_nor;
out vec2 io_uv;

void main()
{
	mat4 trBone = mat4(1.0);
	
	if (animate != 0) {
		trBone   = bones[boneids[0]] * weights[0];
		trBone  += bones[boneids[1]] * weights[1];
		trBone  += bones[boneids[2]] * weights[2];
		trBone  += bones[boneids[3]] * weights[3];
	}

	mat4 boneWorld = trBone * trWorld;
	vec4 skinnedPos = boneWorld * vec4(pos, 1.0);

	io_pos = skinnedPos.xyz;
	io_nor = (boneWorld * vec4(nor, 0.0)).xyz;
	io_uv = uv;

    gl_Position = trFinal * vec4(io_pos, 1.0);
}
