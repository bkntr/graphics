203788716 guyshanny
304865090 benk

implementation:
	We changed the array buffer passed to GPU to contain position, vertex normal (average of face
	normals connected to vertex) and face normal (each vertex is multiplied by the number of faces
	its connected to).

	We created 4 shaders:
		- ColorShader implements the coloring from previous exercise.
		- GouraudShader implements gouraud shading
		- PhongShader implements phong shading
		- SimpleShader implements a naive shader for the arcball circle.

	Gouraud and Phong implementation details were taken from the slides.

files:
	ex2.cpp:							project main, almost identical to exercise template.
	Model.cpp:							contains all model logic, including loading, drawing,
										rotation, translation, etc.
	ShaderIO.cpp:						nothing changed.
	shaders/SimpleShader.{vert,frag}:	naive shader for the arcball circle.
	shaders/ColorShader.{vert,frag}:	color by pixel position, from previous exercise.
	shaders/GouraudShader.{vert,frag}:	gouraud shader.
	shaders/PhongShader.{vert,frag}:	phong shader.
