//
//  Model.cpp
//  cg-projects
//
//  Created by HUJI Computer Graphics course staff, 2013.
//

#include "ShaderIO.h"
#include "Model.h"

#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtc/matrix_transform.hpp"

#include <vector>

#define SHADERS_DIR "shaders/"

Model::Model()
{
    
}

void Model::init()
{
    programManager::sharedInstance().createProgram("default",
        SHADERS_DIR "SimpleShader.vert",
        SHADERS_DIR "SimpleShader.frag");

    _circles.emplace_back(0.f, 0.f, DEFAULT_RADIUS, _left, _right, _bottom, _top);
}

void Model::add_circle(int x, int y)
{
    float screen_x = (float)x / _width * (_right-_left) - _right;
    float screen_y = ((float)y / _height * (_top-_bottom) - _top) * -1;
    glm::vec3 new_position(screen_x, screen_y, 0.0f);

    const Circle& closest = *closest_circle(new_position);
    float new_radius = glm::min(glm::distance(new_position, closest.position()) - closest.radius(), DEFAULT_RADIUS);
    // Don't add circles inside other circles
    if (new_radius > 0.f) {
        // We don't let the new circle go beyond the wall
        new_radius = glm::min(new_radius, distance_to_wall(screen_x, screen_y));
        _circles.emplace_back(screen_x, screen_y, new_radius, _left, _right, _bottom, _top);
    }
}

void Model::draw()
{
    for (size_t i = 0; i < _circles.size(); i++) {
        _circles[i].draw(closest_circle(_circles[i]));
    }
}

const Circle* Model::closest_circle(const Circle& circle)
{
    Circle* closest = nullptr;
    float closest_dist = _width*_height;
    for (size_t i = 0; i < _circles.size(); i++) {
        // Ignore this circle
        if (&circle == &_circles[i]) continue;
        float dist = glm::distance(circle.position(), _circles[i].position());
        dist -= circle.radius() + _circles[i].radius();
        if (dist < closest_dist) {
            closest_dist = dist;
            closest = &_circles[i];
        }
    }

    return closest;
}

const Circle* Model::closest_circle(const glm::vec3& position)
{
    Circle tmp(position.x, position.y, DEFAULT_RADIUS, _left, _right, _bottom, _top);
    return closest_circle(tmp);
}

void Model::resize(int width, int height)
{
    _width = (float)width;
    _height = (float)height;
    _offsetX = 0;
    _offsetY = 0;

    float aspect_ratio = _width / _height;

    _left = -1.f * aspect_ratio;
    _right = 1.f * aspect_ratio;
    _bottom = -1.f;
    _top = 1.f;

    for (Circle& circle : _circles) {
        circle.resize(_left, _right, _bottom, _top);
    }
}

float Model::distance_to_wall(float x, float y)
{
    return glm::min(_right - fabs(x), _top - fabs(y));
}
