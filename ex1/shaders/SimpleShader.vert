#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

uniform mat4 transform;
uniform vec4 light;
uniform float radius;

out vec4 fragColor;

void main()
{
    fragColor = color;
    if (position.x == 0 && position.y == 0) {
        vec4 transformed_center = transform * position;
        float lightAngle = atan((light.y - transformed_center.y) / (light.x - transformed_center.x));
        vec4 reflectionPosition = position;
        reflectionPosition.x += radius * 0.7 * cos(lightAngle);
        reflectionPosition.y += radius * 0.7 * sin(lightAngle);
        gl_Position = transform * reflectionPosition;
    }
    else {
        gl_Position = transform * position;
    }
}
