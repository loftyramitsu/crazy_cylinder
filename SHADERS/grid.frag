#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uGrid;

void main()
{
	float value = texture(uGrid, TexCoord).r;
	
	if (value > 0.5)
		FragColor = vec4(0.8, 0.1, 0.1, 1.0);
	else
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}

