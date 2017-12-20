#version 330 core
in vec3 vFragPosition;
in vec2 vTexCoords;
in vec3 vNormal;
uniform sampler2D Texture2;
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

	vec3 lightDir = vec3(dirl, 1.0f, 1.0f);
	vec3 col = vec3(0.0f, 0.9f, 0.75f);
	float kd = max(dot(vNormal, lightDir), 0.0);
	float foggy = exp(-pow((fog / ((gl_FragCoord.z / gl_FragCoord.w))), 2.0));
	vec4 temp = texture(Texture2, vTexCoords);
	if (mode1 == 1) {
			col = vec3(col_to_fl(0), col_to_fl(82), col_to_fl(183));
			color = mix(temp, vec4(kd * col, 1.0), 0.8);
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
	color.w = 0.7;
}