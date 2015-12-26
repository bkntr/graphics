#version 330

#define MY_PI 3.1415926

layout(location = 0) in vec4 position;

uniform mat4x4 transform;

out vec4 my_Color;
out vec2 fragTexCoord;

void main()
{
	// Out position
	gl_Position = transform * position;
	
	// Light position (predefined):
	vec4 light_position = vec4(0.0, 0.0, 5, 1.0);
	
	// Light color:
	vec4 light_color = vec4(1.0, 1.0, 1.0, 1.0);
	
	// Normalize light vector
	{
		light_position.xyz = -light_position.xyz;
		float length = sqrt(dot(light_position.xyz, light_position.xyz));
		light_position.xyz /= length;
	}
	
	// Compute normal (assuming that the vertex belong to a unit sphere)
	vec4 normal;
	{
		vec4 sphere_center = transform * vec4(0.0, 0.0, 0.0, 1.0);
		vec4 position_centered = gl_Position - sphere_center;
		
		float length = sqrt(dot(position_centered.xyz, position_centered.xyz));
		normal = position_centered * (1.0 / length);
	}
	
	// Out color (for vertex, to be interpolated per pixel):
	float reflected_light = dot(light_position.xyz, normal.xyz); // i.e. cos(theta)
	reflected_light = max(0, reflected_light);
	
	//vec4 diffuse = vec4(0.8, 0.3, 0.8, 1.0);
	my_Color = reflected_light * light_color;
	
	// Set texture coordinates using spherical mapping:
	{
		float theta = atan(position.x/position.z);
		float phi   = atan(position.y/length(vec2(position.x,position.z)));
		float r     = length(position.xyz);
		fragTexCoord.x = 1.0 - (theta + MY_PI) / (2*MY_PI);
		fragTexCoord.y = 1.0 - (phi + MY_PI/2)  / MY_PI;
	}
}
