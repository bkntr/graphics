#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

uniform mat4 transform;

out vec4 fragColor;

void main()
{
    fragColor = color;
    gl_Position = transform * position;
}
