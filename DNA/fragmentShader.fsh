#version 120

varying vec4 out_color;
varying vec3 out_worldPos;
varying vec3 out_norm;

uniform vec3 lightPos0;
uniform vec3 lightIntensity0;
uniform vec3 lightAmbient;

void main()
{
	gl_FragColor.rgb = out_color.rgb * (lightIntensity0 * max(0,dot(normalize(lightPos0 - out_worldPos), normalize(out_norm))) + lightAmbient);
	gl_FragColor.a = out_color.a;
}
