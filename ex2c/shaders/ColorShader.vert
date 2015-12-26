#version 330

layout(location = 0) in vec4 position;

uniform mat4 transform;
uniform vec3 lowerLeft;
uniform vec3 upperRight;

out vec4 fragColor;

void main()
{
    gl_Position = transform * position;
	fragColor = vec4((position.xyz - lowerLeft) / (upperRight - lowerLeft), 1);
}
