#version 330

uniform sampler2D my_specmap;
uniform sampler2D my_colormap;

// Interpolated values from the vertex shaders
in vec4 my_Color; // This replaces: uniform vec4 fillColor;
in vec2 fragTexCoord;

out vec4 outColor;

void main()
{
	vec4 diffuse = texture(my_colormap, fragTexCoord);
	
	outColor = my_Color * vec4(diffuse.xyz, 1);

	// Get mask:
	float mask = 1 - texture(my_specmap,fragTexCoord).x;
	outColor = outColor * mask;
}
