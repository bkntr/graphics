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

#define PHONG_ID "phong"
#define GOURAUD_ID "gouraud"
#define COLOR_ID "color"
#define SIMPLE_ID "simple"

#define SHADERS_DIR "shaders/"

#define ARCBALL_RADIUS 0.7f

#define PERSPECTIVE_ANGLE 30.f
#define PERSPECTIVE_NEAR 0.5f
#define PERSPECTIVE_FAR 100.f

Model::Model() :
    _normal_per_vertex(true), _spec_exp(200), _shading_mode(SHADING_MODE_PHONG),
    _zoom_active(false), _zoom_begin(0), _zoom_addition(1), _zoom_factor(1),
    _translation_active(false), _translation_begin(0),
    _rotation_active(false), _rotation_begin(0),
    _polygon_mode(GL_FILL)
{
    
}

void Model::init(char* mesh_path)
{
    programManager::sharedInstance().createProgram(PHONG_ID,
        SHADERS_DIR "PhongShader.vert",
        SHADERS_DIR "PhongShader.frag");
    programManager::sharedInstance().createProgram(GOURAUD_ID,
        SHADERS_DIR "GouraudShader.vert",
        SHADERS_DIR "GouraudShader.frag");
    programManager::sharedInstance().createProgram(COLOR_ID,
        SHADERS_DIR "ColorShader.vert",
        SHADERS_DIR "ColorShader.frag");
    programManager::sharedInstance().createProgram(SIMPLE_ID,
        SHADERS_DIR "SimpleShader.vert",
        SHADERS_DIR "SimpleShader.frag");

    load_mesh(mesh_path);

    compute_bounding_box();

    reset();

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> circle_vertices;

    _mesh.request_vertex_normals();
    _mesh.request_face_normals();
    _mesh.update_normals();

    // Add all vertices to vector
    for (auto f_it = _mesh.faces_begin(); f_it != _mesh.faces_end(); ++f_it) {
        for (auto fv_it = _mesh.fv_ccwiter(f_it.handle()); fv_it.is_valid(); ++fv_it) {
            const Mesh::Point& p = _mesh.point(*fv_it);
            const Mesh::Point& vertex_normal = _mesh.normal(*fv_it);
            const Mesh::Point& face_normal = _mesh.normal(*f_it);

            vertices.push_back(glm::vec3(p[0], p[1], p[2]));
            vertices.push_back(glm::vec3(vertex_normal[0], vertex_normal[1], vertex_normal[2]));
            vertices.push_back(glm::vec3(face_normal[0], face_normal[1], face_normal[2]));
        }
    }

    // Create circle vertices
    for (int i = 0; i < 360; i++) {
        float alpha = 2 * glm::pi<float>() / 360 * i;
        float x = ARCBALL_RADIUS * glm::cos(alpha);
        float y = ARCBALL_RADIUS * glm::sin(alpha);
        circle_vertices.push_back(glm::vec3(x, y, 0.0f));
    }

    // Initialize vertices buffer and transfer it to OpenGL
    {
        GLuint program = programManager::sharedInstance().programWithID(PHONG_ID);
        GLuint vbo, ebo;

        // Create and bind the Mesh Vertex Array Object
        glGenVertexArrays(1, &_mesh_vao);
        glBindVertexArray(_mesh_vao);

        // Create and load vertex data into a Vertex Buffer Object:
        {
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER,
                sizeof(glm::vec3) * vertices.size(),
                &vertices[0],
                GL_STATIC_DRAW);

            GLint posAttrib = glGetAttribLocation(program, "position");
            glEnableVertexAttribArray(posAttrib);
            glVertexAttribPointer(posAttrib,
                3,
                GL_FLOAT,
                GL_FALSE,
                3 * sizeof(glm::vec3),
                0);

            GLint vertexNormalAttrib = glGetAttribLocation(program, "vertexNormal");
            glEnableVertexAttribArray(vertexNormalAttrib);
            glVertexAttribPointer(vertexNormalAttrib,
                3,
                GL_FLOAT,
                GL_FALSE,
                3 * sizeof(glm::vec3),
                (GLvoid*)sizeof(glm::vec3));

            GLint faceNormalAttrib = glGetAttribLocation(program, "faceNormal");
            glEnableVertexAttribArray(faceNormalAttrib);
            glVertexAttribPointer(faceNormalAttrib,
                3,
                GL_FLOAT,
                GL_FALSE,
                3 * sizeof(glm::vec3),
                (GLvoid*)(2 * sizeof(glm::vec3)));
        }

        program = programManager::sharedInstance().programWithID(SIMPLE_ID);

        // Create and bind the circle Vertex Array Object
        {
            glGenVertexArrays(1, &_circle_vao);
            glBindVertexArray(_circle_vao);

            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER,
                sizeof(glm::vec3) * circle_vertices.size(),
                &circle_vertices[0],
                GL_STATIC_DRAW);

            GLint posAttrib = glGetAttribLocation(program, "position");
            glEnableVertexAttribArray(posAttrib);
            glVertexAttribPointer(posAttrib,
                3,
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

void Model::draw_mesh(glm::mat4 model, glm::mat4 user, glm::mat4 view, glm::mat4 projection)
{
    std::string program_id;
    switch (_shading_mode) {
    case SHADING_MODE_PHONG:
        program_id = PHONG_ID;
        break;
    case SHADING_MODE_GOURAUD:
        program_id = GOURAUD_ID;
        break;
    case SHADING_MODE_COLOR:
        program_id = COLOR_ID;
        break;
    }

    GLuint program = programManager::sharedInstance().programWithID(program_id);
    glUseProgram(program);

    // Draw mesh
    glBindVertexArray(_mesh_vao);
    if (_shading_mode == SHADING_MODE_COLOR) {
        glUniformMatrix4fv(glGetUniformLocation(program, "transform"),
            1,
            GL_FALSE,
            glm::value_ptr(projection * view * user * model));
        glUniform3f(glGetUniformLocation(program, "lowerLeft"),
            _lower_left[0], _lower_left[1], _lower_left[2]);
        glUniform3f(glGetUniformLocation(program, "upperRight"),
            _upper_right[0], _upper_right[1], _upper_right[2]);
    }
    else {
        glUniformMatrix4fv(glGetUniformLocation(program, "model"),
            1,
            GL_FALSE,
            glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(program, "user"),
            1,
            GL_FALSE,
            glm::value_ptr(user));
        glUniformMatrix4fv(glGetUniformLocation(program, "view"),
            1,
            GL_FALSE,
            glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"),
            1,
            GL_FALSE,
            glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(program, "specExp"), _spec_exp);
        glUniform1d(glGetUniformLocation(program, "normalPerVertex"), _normal_per_vertex);
    }

    glDrawArrays(GL_TRIANGLES, 0, _mesh.n_faces() * 3);

    glBindVertexArray(0);
    glUseProgram(0);
}

void Model::draw_arcball()
{
    GLuint program = programManager::sharedInstance().programWithID(SIMPLE_ID);
    glUseProgram(program);
    glBindVertexArray(_circle_vao);
    glUniformMatrix4fv(glGetUniformLocation(program, "transform"),
        1,
        GL_FALSE,
        glm::value_ptr(_arcball_projection));

    glDrawArrays(GL_LINE_LOOP, 0, 360);

    glBindVertexArray(0);
    glUseProgram(0);
}

void Model::draw()
{
    glPolygonMode(GL_FRONT_AND_BACK, _polygon_mode);

    float aspect = _width / _height;
    float zoom = glm::clamp(_zoom_factor * _zoom_addition,
        PERSPECTIVE_ANGLE / 170,
        PERSPECTIVE_ANGLE);

    // calculate object depth so that object will fill half of the screen in each dimension when
    // using perspective view at 30 degrees angle.
    const float object_depth = 2.f / glm::tan(glm::radians(PERSPECTIVE_ANGLE / 2));

    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 user;
    glm::mat4 model;
    if (_perspective) {
        projection = glm::perspective(
            glm::radians(PERSPECTIVE_ANGLE / zoom),
            aspect,
            PERSPECTIVE_NEAR,
            PERSPECTIVE_FAR);
    }
    else {
        // calculate the orthogonal scale factor to match the perspective factor (x * tan(alpha))
        float ortho_factor = object_depth * glm::tan(glm::radians(PERSPECTIVE_ANGLE / zoom / 2));
        projection = glm::ortho(-ortho_factor*aspect, ortho_factor*aspect,
            -ortho_factor, ortho_factor,
            -100.f, 100.f);
    }

    view = glm::lookAt(
        glm::vec3(0.f, 0.f, object_depth),
        glm::vec3(0.f),
        glm::vec3(0.f, 1.f, 0.f));

    // user actions matrix
    user = _rotation_addition * _translation_addition * _user;

    // model matrix
    float scale = 2.f / glm::max(glm::max(_mesh_width, _mesh_height), _mesh_depth);
    model = glm::scale(glm::mat4(), glm::vec3(scale));
    model = glm::translate(model, glm::vec3(-_center[0], -_center[1], -_center[2]));

    draw_mesh(model, user, view, projection);
    draw_arcball();
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
    _user = _translation_addition * _user;
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
    _user = _rotation_addition * _user;
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

void Model::toggle_normal_mode()
{
    _normal_per_vertex = !_normal_per_vertex;
}

void Model::increase_spec()
{
    _spec_exp = std::min(_spec_exp + 10, 2000.f);
}

void Model::decrease_spec()
{
    _spec_exp = std::max(_spec_exp - 10, 0.f);
}

void Model::set_shading_mode(ShadingMode mode)
{
    _shading_mode = mode;
}

void Model::reset() {
    _perspective = true;
    _zoom_factor = 1;
    _user = glm::mat4();
}
