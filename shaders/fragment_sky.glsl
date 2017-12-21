#version 330 core
in vec3 vFragPosition;
in vec3 vTexCoords;
uniform samplerCube Texture3;
uniform samplerCube Texture4;
uniform float fog;
uniform float gorit;
uniform bool fog_act;
out vec4 color;

float col_to_fl(float color) {
	if (color > 255) {
		color = 255;
	}
	if (color < 0) {
		color = 0;
	}

	return (color) / 255;
}


void main()
{
	float day_len = 4;
	float foggy = exp(-pow((fog / ((gl_FragCoord.z / gl_FragCoord.w))), 2.0));
	//changing days
	vec4 texttemp; 
	if (sin(gorit / day_len) > 0.3) {
		texttemp = texture(Texture3, vTexCoords);
	} else if (sin(gorit / day_len) < -0.7) {
			texttemp = texture(Texture4, vTexCoords);
		} else {
				texttemp = mix(texture(Texture4, vTexCoords), 
					texture(Texture3, vTexCoords), (sin(gorit / day_len) + 0.7));
			}
	color = texttemp;
	if (fog_act) {
		color = mix(color, vec4(1.0, 1.0, 1.0, 1.0), foggy);
	}

	color.w = 1.0;
}