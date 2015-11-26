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
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

#include <vector>

#define SHADERS_DIR "shaders/"

#define ARCBALL_RADIUS 0.7f

#define PERSPECTIVE_ANGLE 30.f
#define PERSPECTIVE_NEAR 0.5f
#define PERSPECTIVE_FAR 100.f

Model::Model() :
    _zoom_active(false), _zoom_begin(0), _zoom_addition(1), _zoom_factor(1),
    _translation_active(false), _translation_begin(0),
    _rotation_active(false), _rotation_begin(0),
    _polygon_mode(GL_FILL)
{
    
}

void Model::init(char* mesh_path)
{
    programManager::sharedInstance().createProgram("default",
        SHADERS_DIR "SimpleShader.vert",
        SHADERS_DIR "SimpleShader.frag");

    load_mesh(mesh_path);

    compute_bounding_box();

    reset();

    std::vector<glm::vec4> vertices;
    std::vector<glm::vec4> circle_vertices;
    std::vector<GLuint> elements;

    // Add all vertices to vertices vector
    for (Mesh::VertexIter vertexIter = _mesh.vertices_begin(); vertexIter != _mesh.vertices_end(); ++vertexIter) {
        Mesh::Point& p = _mesh.point(*vertexIter);
        vertices.push_back(glm::vec4(p[0], p[1], p[2], 1.0f));
    }

    // Add all triangles to elements vector
    for (auto f_it = _mesh.faces_begin(); f_it != _mesh.faces_end(); ++f_it) {
        for (auto fv_it = _mesh.fv_iter(f_it.handle()); fv_it; ++fv_it) {
            elements.push_back(fv_it.handle().idx());
        }
    }

    // Create circle vertices
    for (int i = 0; i < 360; i++) {
        float alpha = 2 * glm::pi<float>() / 360 * i;
        float x = ARCBALL_RADIUS * glm::cos(alpha);
        float y = ARCBALL_RADIUS * glm::sin(alpha);
        circle_vertices.push_back(glm::vec4(x, y, 0.0f, 1.0f));
    }

    // Initialize vertices buffer and transfer it to OpenGL
    {
        GLuint program = programManager::sharedInstance().programWithID("default");
        GLuint vbo, ebo;

        _uniform_transform = glGetUniformLocation(program, "transform");
        _uniform_is_circle = glGetUniformLocation(program, "isCircle");
        _uniform_lower_left = glGetUniformLocation(program, "lowerLeft");
        _uniform_upper_right = glGetUniformLocation(program, "upperRight");

        // Create and bind the Mesh Vertex Array Object
        glGenVertexArrays(1, &_mesh_vao);
        glBindVertexArray(_mesh_vao);

        // Create and load vertex data into a Vertex Buffer Object:
        {
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER,
                sizeof(glm::vec4) * vertices.size(),
                &vertices[0],
                GL_STATIC_DRAW);

            glGenBuffers(1, &ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                sizeof(GLuint) * elements.size(),
                &elements[0],
                GL_STATIC_DRAW);

            GLint posAttrib = glGetAttribLocation(program, "position");
            glEnableVertexAttribArray(posAttrib);
            glVertexAttribPointer(posAttrib,
                4,
                GL_FLOAT,
                GL_FALSE,
                0,
                0);
        }

        // Create and bind the circle Vertex Array Object
        {
            glGenVertexArrays(1, &_circle_vao);
            glBindVertexArray(_circle_vao);

            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER,
                sizeof(glm::vec4) * circle_vertices.size(),
                &circle_vertices[0],
                GL_STATIC_DRAW);

            GLint posAttrib = glGetAttribLocation(program, "position");
            glEnableVertexAttribArray(posAttrib);
            glVertexAttribPointer(posAttrib,
                4,
                GL_FLOAT,
                GL_FALSE,
                0,
                0);
        }

        // Unbind vertex array:
        glBindVertexArray(0);
    }
}

// Load a mesh from the given input file
void Model::load_mesh(const char* filename)
{
    if (!OpenMesh::IO::read_mesh(_mesh, filename))
    {
        // if we didn't make it, exit...
        fprintf(stderr, "Error loading mesh, Aborting.\n");
        return;
    }
}

/** This function computes the geometrical center and the axis aligned bounding box of the
object. The bounding box is represented by the lower left and upper right corners. */
void Model::compute_bounding_box()
{
    _lower_left = Mesh::Point(std::numeric_limits<float>::max());
    _upper_right = Mesh::Point(-std::numeric_limits<float>::max());

    // This is how to go over all the vertices in the mesh:
    for (Mesh::VertexIter vertexIter = _mesh.vertices_begin(); vertexIter != _mesh.vertices_end(); ++vertexIter) {
        // This is how to get the point associated with the vertex:
        Mesh::Point& p = _mesh.point(*vertexIter);
        for (int i = 0; i < 3; i++) {
            _lower_left[i] = glm::min(_lower_left[i], p[i]);
            _upper_right[i] = glm::max(_upper_right[i], p[i]);
        }
    }

    _center = (_lower_left + _upper_right) / 2;
    _mesh_width = _upper_right[0] - _lower_left[0];
    _mesh_height = _upper_right[1] - _lower_left[1];
    _mesh_depth = _upper_right[2] - _lower_left[2];
}

// Convert screen coordinates to world coordinates (on z=0 plane)
glm::vec3 Model::to_world(int x, int y)
{
    float xnorm = (float)x / _width * 2.f - 1.f;
    float ynorm = -((float)y / _height * 2.f - 1.f);
    glm::vec3 world_coords(_inverse_arcball_projection * glm::vec4(xnorm, ynorm, 0, 1.f));
    world_coords.z = 0;
    return world_coords;
}

// Convert screen coordinates to arcball coordinates
glm::vec3 Model::to_arcball(int x, int y)
{
    glm::vec3 world_coords = to_world(x, y);
    float r = glm::length(glm::vec2(world_coords));
    if (r < ARCBALL_RADIUS) {
        world_coords.z = glm::sqrt(
            glm::pow2(ARCBALL_RADIUS) - glm::pow2(world_coords.x) - glm::pow2(world_coords.y));
    }
    return glm::normalize(world_coords);
}

void Model::draw()
{
    // Set the program to be used in subsequent lines:
    GLuint program = programManager::sharedInstance().programWithID("default");
    glUseProgram(program);

    glPolygonMode(GL_FRONT_AND_BACK, _polygon_mode);

    float aspect = _width / _height;
    float zoom = glm::clamp(_zoom_factor * _zoom_addition,
        PERSPECTIVE_ANGLE / 170,
        PERSPECTIVE_ANGLE);

    // calculate object depth so that object will fill half of the screen in each dimension when
    // using perspective view at 30 degrees angle.
    const float object_depth = 2.f / glm::tan(glm::radians(PERSPECTIVE_ANGLE / 2));

    glm::mat4 modelview;
    if (_perspective) {
        modelview = glm::perspective(
            glm::radians(PERSPECTIVE_ANGLE / zoom),
            aspect,
            PERSPECTIVE_NEAR,
            PERSPECTIVE_FAR);

        modelview *= glm::lookAt(
            glm::vec3(0.f, 0.f, object_depth),
            glm::vec3(0.f),
            glm::vec3(0.f, 1.f, 0.f));
    }
    else {
        // calculate the orthogonal scale factor to match the perspective factor (x * tan(alpha))
        float ortho_factor = object_depth * glm::tan(glm::radians(PERSPECTIVE_ANGLE / zoom / 2));
        modelview = glm::ortho(-ortho_factor*aspect, ortho_factor*aspect,
            -ortho_factor, ortho_factor,
            -100.f, 100.f);
    }

    // rotation
    modelview *= _rotation_addition;
    // translation
    modelview *= _translation_addition;

    modelview *= _modelview;

    // Draw mesh
    glBindVertexArray(_mesh_vao);
    glUniformMatrix4fv(_uniform_transform,
        1,
        GL_FALSE,
        glm::value_ptr(modelview));
    glUniform1i(_uniform_is_circle, 0);
    glUniform3f(_uniform_lower_left,  _lower_left[0], _lower_left[1], _lower_left[2]);
    glUniform3f(_uniform_upper_right, _upper_right[0], _upper_right[1], _upper_right[2]);

    glDrawElements(GL_TRIANGLES, _mesh.n_faces() * 3, GL_UNSIGNED_INT, 0);

    // Draw circle
    glBindVertexArray(_circle_vao);
    glUniformMatrix4fv(_uniform_transform, 1, GL_FALSE, glm::value_ptr(_arcball_projection));
    glUniform1i(_uniform_is_circle, 1);
    glDrawArrays(GL_LINE_LOOP, 0, 360);

    // Unbind the Vertex Array object
    glBindVertexArray(0);

    // Cleanup, not strictly necessary
    glUseProgram(0);
}

// Update model according to mouse movement
void Model::update(int x, int y)
{
    update_zoom(x, y);
    update_translation(x, y);
    update_rotation(x, y);
}

void Model::begin_zoom(int x, int y)
{
    _zoom_active = true;
    _zoom_begin = to_world(x, y).y;
}

void Model::update_zoom(int x, int y)
{
    if (_zoom_active) {
        _zoom_addition = 1 + _zoom_begin - to_world(x, y).y;
        float zoom = glm::clamp(_zoom_factor * _zoom_addition,
            PERSPECTIVE_ANGLE / 170,
            PERSPECTIVE_ANGLE);
        _zoom_addition = zoom / _zoom_factor;
    }
}

void Model::end_zoom()
{
    _zoom_active = false;
    _zoom_factor *= _zoom_addition;
    _zoom_addition = 1;
}

void Model::begin_translation(int x, int y)
{
    _translation_active = true;
    _translation_begin = to_world(x, y);
}

void Model::update_translation(int x, int y)
{
    if (_translation_active) {
        glm::vec3 translate_end = to_world(x, y);
        _translation_addition =
            glm::translate(glm::mat4(), (translate_end - _translation_begin) * 1.5f);
    }
}

void Model::end_translation()
{
    _translation_active = false;
    _modelview = _translation_addition * _modelview;
    _translation_addition = glm::mat4();
}

void Model::begin_rotation(int x, int y)
{
    if (to_arcball(x, y).z != 0) {
        _rotation_active = true;
        _rotation_begin = to_arcball(x, y);
    }
}

void Model::update_rotation(int x, int y)
{
    if (_rotation_active) {
        glm::vec3 rotation_end = to_arcball(x, y);
        glm::vec3 normal = glm::cross(_rotation_begin, rotation_end);
        _rotation_addition = glm::rotate(glm::mat4(),
            glm::acos(glm::dot(_rotation_begin, rotation_end)) * 2,
            normal);
    }
}

void Model::end_rotation()
{
    _rotation_active = false;
    _modelview = _rotation_addition * _modelview;
    _rotation_addition = glm::mat4();
}

void Model::resize(int width, int height)
{
    _width = (float)width;
    _height = (float)height;
    _offsetX = 0;
    _offsetY = 0;

    float aspect = _width / _height;
    _arcball_projection = glm::ortho(-aspect, aspect, -1.f, 1.f);
    // used to convert screen coordinates to world coordinates
    _inverse_arcball_projection = glm::inverse(_arcball_projection);
}

void Model::toggle_polygon_mode()
{
    if (_polygon_mode == GL_LINE) {
        _polygon_mode = GL_FILL;
    }
    else {
        _polygon_mode = GL_LINE;
    }
}

void Model::toggle_projection_mode() {
    _perspective = !_perspective;
}

void Model::reset() {
    _perspective = true;
    _zoom_factor = 1;

    float scale = 2.f / glm::max(glm::max(_mesh_width, _mesh_height), _mesh_depth);
    _modelview = glm::scale(glm::mat4(), glm::vec3(scale));
    _modelview = glm::translate(_modelview, glm::vec3(-_center[0], -_center[1], -_center[2]));
}
