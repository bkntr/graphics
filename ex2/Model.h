//
//  Model.h
//  cg-projects
//
//  Created by HUJI Computer Graphics course staff, 2013.
//

#ifndef __ex2__Model__
#define __ex2__Model__

#include <vector>
#include <iostream>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#include <glm/glm.hpp>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

typedef OpenMesh::PolyMesh_ArrayKernelT<> Mesh;

class Model {
    GLuint _mesh_vao, _circle_vao;

    Mesh _mesh;
    Mesh::Point _lower_left, _upper_right, _center;
    float _mesh_width, _mesh_height, _mesh_depth;

    glm::mat4 _arcball_projection;
    glm::mat4 _inverse_arcball_projection;

    glm::mat4 _modelview;

    bool _zoom_active;
    float _zoom_begin;
    float _zoom_addition;
    float _zoom_factor;

    bool _translation_active;
    glm::vec3 _translation_begin;
    glm::mat4 _translation_addition;

    bool _rotation_active;
    glm::vec3 _rotation_begin;
    glm::mat4 _rotation_addition;

    GLenum _polygon_mode;
    bool _perspective;

    // View port frame:
    float _width, _height, _offsetX, _offsetY;

    GLuint _uniform_transform;
    GLuint _uniform_is_circle;
    GLuint _uniform_upper_right;
    GLuint _uniform_lower_left;

    void load_mesh(const char * filename);
    void compute_bounding_box();
    glm::vec3 to_world(int x, int y);
    glm::vec3 to_arcball(int x, int y);

public:
    Model();
    void init(char* mesh_path);
    void draw();
    void update(int x, int y);
    void begin_zoom(int x, int y);
    void update_zoom(int x, int y);
    void end_zoom();
    void begin_translation(int x, int y);
    void update_translation(int x, int y);
    void end_translation();
    void begin_rotation(int x, int y);
    void update_rotation(int x, int y);
    void end_rotation();
    void resize(int width, int height);
    void toggle_polygon_mode();
    void toggle_projection_mode();
    void reset();
};

#endif /* defined(__ex2__Model__) */
