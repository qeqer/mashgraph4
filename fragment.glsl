#version 330 core
in vec3 vFragPosition;
in vec2 vTexCoords;
in vec3 vNormal;
uniform sampler2D Texture1;
uniform int mode1;
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
	float dirl = sin(gorit / day_len);

	vec3 lightDir = vec3(dirl, 2.0f, 1.0f);
	vec3 col = vec3(0.0f, 0.9f, 0.75f);
	float kd = max(dot(vNormal, lightDir), 0.3) * 0.5;
	float foggy = exp(-pow((fog / ((gl_FragCoord.z / gl_FragCoord.w))), 2.0));

	if (mode1 == 1) {
		if (vFragPosition.y < 0) { //water
			col = vec3(col_to_fl(250), col_to_fl(250), col_to_fl(5));
			color = mix(texture(Texture1, vTexCoords), vec4(kd * col, 1.0), 0.5);

		} 
		if (vFragPosition.y >= 0 && vFragPosition.y < 3) { //sand
			col = vec3(col_to_fl(252), col_to_fl(221), col_to_fl(118));
			color = mix(texture(Texture1, vTexCoords), vec4(kd * col, 1.0), 0.9);
		}
		if (vFragPosition.y >= 3 && vFragPosition.y < 10) {
			col = vec3(col_to_fl(39), col_to_fl(140), col_to_fl(0));
			color = mix(texture(Texture1, vTexCoords), vec4(kd * col, 1.0), 0.9);

		}
		float h = vFragPosition.y;
		if (vFragPosition.y >= 10) { 
			col = vec3(col_to_fl(39 + (h - 10) * (255 - 39) / 20), col_to_fl(140 + (h - 10) * (255 - 140) / 11), col_to_fl(25 * (h - 10)));
			color = mix(texture(Texture1, vTexCoords), vec4(kd * col, 1.0), 0.9);

		}
	} else {
		color = vec4(vNormal * 0.5 + vec3(0.5, 0.5, 0.5), 1.0);
	}
	//fog
	if (fog_act) {
		color = mix(color, vec4(1.0, 1.0, 1.0, 1.0), foggy);
	}
	
	//changing days
	float real_time = 0.0; 
	if (sin(gorit / day_len) > 0.3) {
		real_time = 1.0;
	} else if (sin(gorit / day_len) < -0.7) {
			real_time = 0.0;
		} else {
				real_time = (sin(gorit / day_len) + 0.7); 
			}

	color = color * real_time;
	color.w = 1.0;
}