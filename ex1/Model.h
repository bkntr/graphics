//
//  Model.h
//  cg-projects
//
//  Created by HUJI Computer Graphics course staff, 2013.
//

#ifndef __ex0__Model__
#define __ex0__Model__

#include <vector>
#include <iostream>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#include <glm/glm.hpp>

#include "Circle.h"

class Model {
    GLuint _vao;
    
    std::vector<Circle> _circles;

    // View port frame:
    float _width, _height, _offsetX, _offsetY;
    float _left, _right, _bottom, _top;

public:
    Model();
    void init();
    void add_circle(int x, int y);
    void draw();
    const Circle * closest_circle(const Circle & circle);
    const Circle * closest_circle(const glm::vec3 & position);
    void resize(int width, int height);
    float distance_to_wall(float x, float y);
};

#endif /* defined(__ex0__Model__) */
