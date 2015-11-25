#version 330

layout(location = 0) in vec4 position;

uniform mat4 transform;
uniform vec3 lowerLeft;
uniform vec3 upperRight;
uniform bool isCircle;

out vec4 fragColor;

void main()
{
    gl_Position = transform * position;

	if (isCircle) {
		fragColor = vec4(0.4, 0.0, 0.2, 1.0);
	}
	else {
		fragColor = vec4((position.xyz - lowerLeft) / (upperRight - lowerLeft), 1);
	}
}
