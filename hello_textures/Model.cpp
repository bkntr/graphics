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

#include "OpenMesh/Core/IO/MeshIO.hh"
#include "OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtc/matrix_transform.hpp"

#include "bimage.h"

#define SHADERS_DIR "shaders/"

typedef OpenMesh::PolyMesh_ArrayKernelT<> Mesh;

Model::Model() :
_vao(0), _vbo(0)
{

}

Model::~Model()
{
	if (_vao != 0)
		glDeleteVertexArrays(1, &_vao);
	if (_vbo != 0)
		glDeleteBuffers(1, &_vbo);
}

void Model::init()
{
	programManager::sharedInstance()
	.createProgram("default",
				   SHADERS_DIR "SimpleShader.vert",
				   SHADERS_DIR "SimpleShader.frag");

	GLuint program = programManager::sharedInstance().programWithID("default");
		
	// Obtain uniform variable handles:
	_fillColorUV  = glGetUniformLocation(program, "fillColor");

	// Initialize vertices buffer and transfer it to OpenGL
	{
		const char* filename = "meshes/sphere_960.off";

		Mesh mesh;
		
		// Read the mesh from a file:
		if (!OpenMesh::IO::read_mesh(mesh, filename))
		{
			// if we didn't make it, exit...
			fprintf(stderr, "Error loading mesh, Aborting.\n");
			exit(-1);
		}
		else
		{
			std::cout << "Loaded a mesh with " << mesh.n_vertices() <<
			" vertices." << std::endl;
		}
			

		// The vector will be use as a buffer, before transfer the vertices to
		// the VBO:
		std::vector<glm::vec4> vertices(mesh.n_vertices());
		
		size_t i = 0;
		for (Mesh::VertexIter vertexIter = mesh.vertices_begin();
			 vertexIter != mesh.vertices_end();
			 ++vertexIter)
		{
			// this is how to get the point associated with the vertex:
			Mesh::Point p = mesh.point(vertexIter);
			glm::vec4 position(p[0], p[1], p[2], 1.0f);
			vertices[i++] = position;
		}

		// Iterate over faces and create a traingle for each face by referencing
		// to its vertices:
		struct face_indices_t
		{
			GLuint a,b,c;
		};
		i = 0;
		std::vector<face_indices_t> faces(mesh.n_faces());
		for (Mesh::FaceIter faceIter = mesh.faces_begin();
			 faceIter != mesh.faces_end();
			 ++faceIter)
		{
			Mesh::ConstFaceVertexIter cfvlt = mesh.cfv_iter(faceIter.handle());
			face_indices_t face;
			face.a = cfvlt.handle().idx();
			++cfvlt;
			face.b = cfvlt.handle().idx();
			++cfvlt;
			face.c = cfvlt.handle().idx();
			faces[i++] = face;
		}
		
		_numTriangles = faces.size();
		
		// Create and bind the object's Vertex Array Object:
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);
		
		// Create and load vertex data into a Vertex Buffer Object:
		glGenBuffers(1, &_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * vertices.size(), &(vertices[0]), GL_STATIC_DRAW);
		
		// Create and load face (elements) data into an Element Buffer Object:
		glGenBuffers(1, &_ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face_indices_t)*faces.size(), &(faces[0]), GL_STATIC_DRAW);
		
		// Tells OpenGL that there is vertex data in this buffer object and what form that vertex data takes:

		// Obtain attribute handles:
		_posAttrib = glGetAttribLocation(program, "position");
		glEnableVertexAttribArray(_posAttrib);
		glVertexAttribPointer(_posAttrib, // attribute handle
							  4,          // number of scalars per vertex
							  GL_FLOAT,   // scalar type
							  GL_FALSE,
							  0,
							  0);
		
		
		// Unbind vertex array:
		glBindVertexArray(0);

		// Load and bind textures:
		{
			// Load a bitmap file:
			{
				const char* texture_filename = "earthmap.bmp";
				BImage image( texture_filename );
				
				glGenTextures(1, &_texture); // Generate a texture in OpenGL
				glBindTexture(GL_TEXTURE_2D, _texture); // Bind texture before setting its properties
				
				// Setting texture properties
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				
				// Transfer data to texture:
				glTexImage2D(GL_TEXTURE_2D,
							 0, // level
							 GL_RGBA, // Internal representation
							 image.width(), image.height(), // Texture size
							 0, // must be 0
							 GL_RGB, GL_UNSIGNED_BYTE, // Pixel data format
							 image.getImageData()); // Pixel data
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			
			// Load bump map:
			{
				const char* specmap_filename = "earthspec.bmp";
				BImage image( specmap_filename );
				
				glGenTextures(1, &_specMap);
				glBindTexture(GL_TEXTURE_2D, _specMap);
				// Setting texture properties
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				
				// Transfer data to texture:
				glTexImage2D(GL_TEXTURE_2D,
							 0, // level
							 GL_RGBA, // Internal representation
							 image.width(), image.height(), // Texture size
							 0, // must be 0
							 GL_RGB, GL_UNSIGNED_BYTE, // Pixel data format
							 image.getImageData()); // Pixel data
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
	}
}

void Model::draw()
{
	// Set the program to be used in subsequent lines:
	GLuint program = programManager::sharedInstance().programWithID("default");
	glUseProgram(program);

	GLenum polygonMode = GL_FILL;   // Also try using GL_FILL and GL_POINT
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

	// Draw using the state stored in the Vertex Array object:
	glBindVertexArray(_vao);
	
	glm::mat4 model_mat = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
//	model_mat = glm::rotate(model_mat, 45.f, glm::vec3(0.f, 0.f, 1.f));
	
	
	glm::mat4 view_mat;
	{
		glm::vec3 camera_location(2.f,2.f,-5);
		glm::vec3 camera_target(0.0f, 0.0f, 1.0f);
		view_mat
		= glm::lookAt(
					  camera_location,   // Camera location in World Space
					  camera_target,     // and looks at its target
					  glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
					  );
		
		std::cout << "view_mat: \n";
		for (size_t row = 0; row < 4; row++)
		{
			std::cout << view_mat[row].x << "\t" << view_mat[row].y << "\t" << view_mat[row].z << "\t" << view_mat[row].w << std::endl;
		}
	}
	glm::mat4 projection_mat;
	{
//		projection_mat = glm::ortho(-1.f, 1.f, -1.0f, 1.f, 1.f, -1.f);
		
		float FoV = 35.f;
		// Generates a really hard-to-read matrix, but a normal, standard 4x4 matrix nonetheless
		projection_mat = glm::perspective(
										  FoV,         // The horizontal Field of View, in degrees : the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
										  float(_width)/float(_height), // Aspect Ratio. Depends on the size of your window.
										  0.1f,        // Near clipping plane. Keep as big as possible, or you'll get precision issues.
										  100.0f       // Far clipping plane. Keep as little as possible.
										  );
	}
	
	glm::mat4 transform = /*projection_mat * view_mat * */model_mat;
	
	GLint transform_loc = glGetUniformLocation(program, "transform");
	glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(transform));
	
	// Set texture
	{
		int loc0 = glGetUniformLocation(program, "my_colormap");
		glUniform1i(loc0, 0);
		int loc1 = glGetUniformLocation(program, "my_specmap");
		glUniform1i(loc1, 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _texture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _specMap);
	}
	
	// Draw using the state stored in the Vertex Array object:
	glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, 3*_numTriangles, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	
	// Unbind the Vertex Array object
	glBindVertexArray(0);
	
	// Cleanup, not strictly necessary
	glUseProgram(0);
}

void Model::resize(int width, int height)
{
    _width	= width;
    _height = height;
    _offsetX = 0;
    _offsetY = 0;
}
