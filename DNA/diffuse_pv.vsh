#version 120

uniform mat4 viewProjMatrix;
uniform mat4 modelMatrix;
uniform mat4 normMatrix;

uniform vec3 lightPos0;
uniform vec3 lightIntensity0;
uniform vec3 lightAmbient;

attribute vec3 pos;
attribute vec3 normal;
attribute vec4 color;

varying vec4 out_color;

void main() {
	vec4 p = modelMatrix * vec4(pos, 1);
	vec3 n = normalize(normMatrix * vec4(normal, 0)).xyz;
	
	out_color.rgb = color.rgb * (lightIntensity0 * max(0,dot(normalize(lightPos0 - p.xyz), n)) + lightAmbient);
	out_color.a = color.a;
	gl_Position = viewProjMatrix * p;
}