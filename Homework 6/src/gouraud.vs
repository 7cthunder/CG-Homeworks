#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 vLightColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform int nSpec;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
	gl_Position = proj * view * model * vec4(aPos, 1.0);

	vec3 Position = vec3(model * vec4(aPos, 1.0));
	vec3 Normal = mat3(transpose(inverse(model))) * aNormal;

	// ambient
	vec3 ambient = Ka * lightColor;

	// diffuse
	vec3  norm = normalize(Normal);
	vec3  lightDir = normalize(lightPos - Position);
	float arc_diff = max(dot(norm, lightDir), 0.0);
	vec3  diffuse = arc_diff * lightColor;

	// specular
	vec3 viewDir = normalize(viewPos - Position);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float arc_spec = pow(max(dot(viewDir, reflectDir), 0.0), nSpec);
	vec3 specular = Ks * arc_spec * lightColor;

	vLightColor = ambient + Kd * diffuse + specular;
}