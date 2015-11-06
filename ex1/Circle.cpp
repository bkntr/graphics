#include "ShaderIO.h"
#include "Circle.h"

#include <glm/gtc/matrix_transform.hpp>


float Circle::distance_to_wall()
{
    return glm::min(1 - abs(_position.x), 1 - abs(_position.y));
}

Circle::Circle(const Circle& other)
{
    _vao = other._vao;
    _vbo = other._vbo;
    _left = other._left;
    _right = other._right;
    _bottom = other._bottom;
    _top = other._top;
    _vertices = other._vertices;
    _radius = other._radius;
    _direction = other._direction;
    _position = other._position;
    _color = other._color;
}

Circle::Circle(Circle&& other) : _vao(0), _vbo(0)
{
    swap(*this, other);
}

Circle::Circle(float x, float y, float r, float left, float right, float bottom, float top) :
    _left(left), _right(right), _bottom(bottom), _top(top), 
    _position(x, y, 0), _vao(0), _vbo(0), _radius(r)
{
    // Set uniform variable with RGB values:
    _color.r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    _color.g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    _color.b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    _color.a = 1.0f;

    // Create circle vertices
    _vertices.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // position
    _vertices.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // color
    for (int i = 0; i < N + 1; i++) {
        float alpha = 2 * glm::pi<float>() / N * i;
        float x = _radius * glm::cos(alpha);
        float y = _radius * glm::sin(alpha);
        _vertices.push_back(glm::vec4(x, y, 0.0f, 1.0f));
        _vertices.push_back(_color);
    }

    // Choose a random initial direction
    float random_angle = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) *
        2 * glm::pi<float>();
    _direction.x = glm::sin(random_angle);
    _direction.y = glm::cos(random_angle);

    // Initialize vertices buffer and transfer it to OpenGL
    {
        GLuint program = programManager::sharedInstance().programWithID("default");

        // Create and bind the object's Vertex Array Object:
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        // Create and load vertex data into a Vertex Buffer Object:
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(glm::vec4) * _vertices.size(),
            &_vertices[0],
            GL_STATIC_DRAW);

        // Tells OpenGL that there is vertex data in this buffer object and what form that vertex
        // data takes:

        // Obtain attribute handles:
        GLint posAttrib = glGetAttribLocation(program, "position");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, // attribute handle
            4,          // number of scalars per vertex
            GL_FLOAT,   // scalar type
            GL_FALSE,
            2 * sizeof(glm::vec4),
            0);

        GLint colorAttrib = glGetAttribLocation(program, "color");
        glEnableVertexAttribArray(colorAttrib);
        glVertexAttribPointer(colorAttrib, // attribute handle
            4,          // number of scalars per vertex
            GL_FLOAT,   // scalar type
            GL_FALSE,
            2 * sizeof(glm::vec4),
            (GLvoid*)(sizeof(glm::vec4)));

        // Unbind vertex array:
        glBindVertexArray(0);
    }
}

/** Copy assignment operator */
Circle& Circle::operator= (Circle other)
{
    swap(*this, other);
    return *this;
}

/** Move assignment operator */
Circle& Circle::operator= (Circle&& other) noexcept
{
    swap(*this, other);
    return *this;
}

void Circle::draw(const Circle* closest_circle)
{
    // Set the program to be used in subsequent lines:
    GLuint program = programManager::sharedInstance().programWithID("default");
    glUseProgram(program);

    glUniform4f(glGetUniformLocation(program, "fillColor"),
        _color.r, _color.g, _color.b, _color.a);

    GLenum polygonMode = GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

    _position += _direction*(1.f / 50.f);
    glm::mat4 model_mat = glm::ortho(_left, _right, _bottom, _top);
    model_mat = glm::translate(model_mat, _position);
    float current_radius = _radius;
    // If colliding with other circle, shrink
    if (closest_circle) {
        float distance = glm::distance(_position, closest_circle->position());
        float collision_distance = _radius + closest_circle->radius();
        if (distance < collision_distance) {
            // Length of overlap between circles.
            float overlap = collision_distance - distance;
            // Relative length to be decreased from this circle.
            float weight = _radius / (_radius + closest_circle->radius());
            current_radius = _radius - overlap * weight;
            float scale = current_radius / _radius;
            model_mat = glm::scale(model_mat, glm::vec3(scale, scale, 1.f));
        }
    }

    glUniformMatrix4fv(glGetUniformLocation(program, "transform"), 1, GL_FALSE, glm::value_ptr(model_mat));
    glUniform4f(glGetUniformLocation(program, "light"), 1.0f, 1.0f, 0.0f, 1.0f);
    glUniform1f(glGetUniformLocation(program, "radius"), current_radius);

    // Change direction on wall collision
    if (_position.x + current_radius >= _right && _direction.x > 0 ||
            _position.x - current_radius <= _left && _direction.x < 0) {
        _direction.x = -_direction.x;
    }
    if (_position.y + current_radius >= _top && _direction.y > 0 ||
            _position.y - current_radius <= _bottom && _direction.y < 0) {
        _direction.y = -_direction.y;
    }

    // Draw using the state stored in the Vertex Array object:
    glBindVertexArray(_vao);

    size_t numberOfVertices = N + 2;
    glDrawArrays(GL_TRIANGLE_FAN, 0, numberOfVertices);

    // Unbind the Vertex Array object
    glBindVertexArray(0);

    // Cleanup, not strictly necessary
    glUseProgram(0);
}

void Circle::resize(float left, float right, float bottom, float top)
{
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;

    _position.x = glm::max(_position.x, _left);
    _position.x = glm::min(_position.x, _right);
    _position.y = glm::max(_position.y, _bottom);
    _position.y = glm::min(_position.y, _top);
}

Circle::~Circle()
{
    if (_vao != 0)
        glDeleteVertexArrays(1, &_vao);
    if (_vbo != 0)
        glDeleteBuffers(1, &_vbo);
}
