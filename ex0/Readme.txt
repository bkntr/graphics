203788716 guyshanny
304865090 benk

description:
	ex0.cpp:					nothing changed.
	Model.cpp:					changed vertices to draw a circle, using GL_TRIANGLE_FAN;
								changed polygonMode to GL_FILL for color interpolation.
	ShaderIO.cpp:				nothing changed.
	shaders\SimpleShader.frag:	we split the space to a grid of 20x20 and applied color accordingly.
								we decided which fragment to color by the doing:

								((fragment.x % BLOCK_SIZE < BLOCK_SIZE/2 && 
								fragment.y % BLOCK_SIZE < BLOCK_SIZE/2) ||
								(fragment.x % BLOCK_SIZE >= BLOCK_SIZE/2 && 
								fragment.y % BLOCK_SIZE >= BLOCK_SIZE/2))   ->  color.
								Otherwise color black.
