#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uData;  // texture unit 0 — champ physique normalisé [0,1]
uniform sampler2D uSolid; // texture unit 1 — masque solide

// Colormap "Viridis" approximée par polynômes
vec3 viridis(float t) {
	t = clamp(t, 0.0, 1.0);
	vec3 c = vec3(0.0);
	c.r = 0.267 + t*(-0.003 + t*(1.785 + t*(-3.653 + t*(3.543 - t*1.344))));
	c.g = 0.005 + t*(1.015 + t*(-0.387 + t*(-1.540 + t*(2.101 - t*0.768))));
	c.b = 0.329 + t*(1.384 + t*(-2.994 + t*(2.994 + t*(-1.742 + t*0.389))));
	return clamp(c, 0.0, 1.0);
}

void main()
{
	float solid = texture(uSolid, TexCoord).r;

	// Cylindre : intérieur sombre + contour blanc détecté par gradient
	if (solid > 0.5) {
		vec2 texel = 1.0 / vec2(textureSize(uSolid, 0));
		float n = texture(uSolid, TexCoord + vec2( 0.0,  texel.y)).r;
		float s = texture(uSolid, TexCoord + vec2( 0.0, -texel.y)).r;
		float e = texture(uSolid, TexCoord + vec2( texel.x,  0.0)).r;
		float w = texture(uSolid, TexCoord + vec2(-texel.x,  0.0)).r;
		float bord = abs(n - s) + abs(e - w);

		if (bord > 0.1)
			FragColor = vec4(1.0, 1.0, 1.0, 1.0);   // contour blanc
		else
			FragColor = vec4(0.12, 0.12, 0.14, 1.0); // intérieur sombre
		return;
	}

	// Champ de vitesse : rehaussement sigmoïde
	// centre : décale le contraste (0.3 → accentue les basses vitesses)
	// pente  : contrôle la transition (8.0 = assez franche)
	float val = texture(uData, TexCoord).r;
	float centre = 0.3;
	float pente  = 8.0;
	val = 1.0 / (1.0 + exp(-pente * (val - centre)));
	FragColor = vec4(viridis(val), 1.0);
}
