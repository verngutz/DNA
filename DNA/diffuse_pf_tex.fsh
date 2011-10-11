#version 120

uniform sampler2D normMap;

varying vec4 out_color;
varying vec3 out_worldPos;
varying vec2 out_normMapCoord;

uniform vec3 lightPos0;
uniform vec3 lightIntensity0;
uniform vec3 lightAmbient;

void main() {
	gl_FragColor.rgb = out_color.rgb
	
	* (lightIntensity0 * max(0,
		dot(
			normalize(lightPos0 - out_worldPos),
			normalize(texture2D(normMap, out_normMapCoord).rgb)
		)) + lightAmbient)
	
	;
	gl_FragColor.a = out_color.a;
}
