#version 330

layout(location = 0) in vec4 position;

uniform mat4 transform;

out vec4 fragColor;

void main()
{
    gl_Position = transform * position;
	fragColor = vec4(0.4, 0.0, 0.2, 1.0);
}
