#version 330

#define BLOCK_SIZE 20

uniform vec4 fillColor;

out vec4 outColor;

void main()
{
	if (mod(gl_FragCoord.x, BLOCK_SIZE) < BLOCK_SIZE/2 && mod(gl_FragCoord.y, BLOCK_SIZE) < BLOCK_SIZE/2 ||
		mod(gl_FragCoord.x, BLOCK_SIZE) >= BLOCK_SIZE/2 && mod(gl_FragCoord.y, BLOCK_SIZE) >= BLOCK_SIZE/2) {
		outColor = fillColor;
	}
	else {
		outColor = vec4(0,0,0,0);
	}
}
