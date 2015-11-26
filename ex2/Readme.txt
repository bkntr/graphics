203788716 guyshanny
304865090 benk

implementation:
	To draw the mesh, we made two passes on the vertices. In the first pass we iterate on the 
	vertices using VertexIter and them to the array buffer. In the second pass we iterate on 
	the triangles using FaceIter and add all triangle vertices indices to the elements buffer. 
	We then draw the elements buffer. This approach enables us to reuse vertices that are 
	connected to several triangles (all vertices do).

	The modelview matrix consists of the following transformations, in this order:
	1. Move the object so that the center will be at (0,0,0).
	2. Scale the object so that it's bounding box will be [(-1,-1,-1),(1,1,1)].
	3. Active user translation.
	4. Active user rotation.
	5. Projection (includes user scaling).

	After user finished rotation/translation (mouse up), the base modelview matrix is 
	permanently updated, thus being effectively performed after stage 2 and before stage 3.

files:
	ex2.cpp:					project main, almost identical to exercise template.
	Model.cpp:					contains all model logic, including loading, drawing, rotation,
								translation, etc.
	ShaderIO.cpp:				nothing changed.
	shaders\SimpleShader.vert:	simple vertex shader
	shaders\SimpleShader.frag:	simple fragment shader
