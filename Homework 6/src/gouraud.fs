#version 330 core
out vec4 FragColor;

in vec3 vLightColor;

uniform vec3 objectColor;

void main() {
	FragColor = vec4(vLightColor * objectColor, 1.0);
}