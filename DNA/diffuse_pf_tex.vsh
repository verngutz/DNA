#version 120

uniform mat4 viewProjMatrix;
uniform mat4 modelMatrix;
uniform mat4 normMatrix;

attribute vec3 pos;
attribute vec2 normMapCoord;
attribute vec4 color;

varying vec4 out_color;
varying vec3 out_worldPos;
varying vec2 out_normMapCoord;


void main() {
	vec4 p = modelMatrix * vec4(pos, 1);
	out_worldPos = p.xyz;
	out_color = color;
	out_normMapCoord = normMapCoord;
	
	gl_Position = viewProjMatrix * p;
}