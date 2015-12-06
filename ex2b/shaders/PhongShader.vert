#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 faceNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 user;
uniform mat4 model;

uniform float specExp;
uniform bool normalPerVertex;

out vec3 normal;
out vec3 fragPosition;
out vec3 lightPosition1;
out vec3 lightPosition2;
out float fragSpecExp;

void main()
{
	mat4 mv = view * user * model;
	mat4 mvp = projection * mv;

    gl_Position = mvp * vec4(position, 1);
	fragPosition = (mv * vec4(position, 1)).xyz;

	vec3 n;
	if (normalPerVertex) {
		n = vertexNormal;
	}
	else {
		n = faceNormal;
	}
	normal = (transpose(inverse(mv)) * vec4(n, 0)).xyz;

	lightPosition1 = (view * model * vec4(3.0, 2.0, 1.0, 0.0)).xyz;
	lightPosition2 = (view * model * vec4(-3.0, 0.0, 1.0, 0.0)).xyz;
	fragSpecExp = specExp;
}
