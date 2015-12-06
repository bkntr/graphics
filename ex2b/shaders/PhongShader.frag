#version 330

in vec3 normal;
in vec3 fragPosition;
in vec3 lightPosition1;
in vec3 lightPosition2;
in float fragSpecExp;

out vec3 outColor;

void main()
{
	vec3 l1 = normalize(lightPosition1 - fragPosition);
	vec3 l2 = normalize(lightPosition2 - fragPosition);

	vec3 i1 = vec3(1.0, 0.9, 0.7); // First light color
	vec3 i2 = vec3(0.6, 0.6, 1.0); // Second light color
	vec3 ambientColor = vec3(1.0, 1.0, 1.0); // Ambient light color

	vec3 ka = vec3(0.1, 0.1, 0.1); // Ambient coefficient
	vec3 kd = vec3(0.3, 0.3, 0.3); // Diffuse coefficient
	vec3 ks = vec3(0.3, 0.3, 0.3); // Specular coefficient

	vec3 n = normalize(normal);
	vec3 v = normalize(vec3(0, 0, 1) - fragPosition);
	vec3 r1 = normalize(2 * n * dot(n, l1) - l1);
	vec3 r2 = normalize(2 * n * dot(n, l2) - l2);
	outColor = kd * i1 * max(dot(l1, n), 0) + ks * i1 * pow(max(dot(v, r1), 0.001), fragSpecExp);
	outColor += kd * i2 * max(dot(l2, n), 0) + ks * i2 * pow(max(dot(v, r2), 0.001), fragSpecExp);
	outColor += ka * ambientColor;
}
