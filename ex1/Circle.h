#pragma once

#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#define N 360
#define DEFAULT_RADIUS 0.1f

class Circle
{
    GLuint _vao, _vbo;

    std::vector<glm::vec4> _vertices;
    float _radius;
    glm::vec3 _direction;
    glm::vec3 _position;
    glm::vec4 _color;

    float distance_to_wall();

public:
    Circle(const Circle & other);
    Circle(Circle && other);
    Circle(float x, float y, float r);

    Circle& operator=(Circle other);
    Circle& operator=(Circle && other) noexcept;

    void draw(const Circle* closest_circle);

    ~Circle();

    const std::vector<glm::vec4>& vertices() const { return _vertices; }
    const glm::vec3& position() const { return _position; }
    const float radius() const { return _radius; }

    friend void swap(Circle& first, Circle& second)
    {
        using std::swap;

        // by swapping the members of two classes,
        // the two classes are effectively swapped
        swap(first._vao, second._vao);
        swap(first._vbo, second._vbo);

        swap(first._vertices, second._vertices);
        swap(first._radius, second._radius);
        swap(first._direction, second._direction);
        swap(first._position, second._position);
        swap(first._color, second._color);
    }
};

