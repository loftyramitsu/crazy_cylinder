#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uData;
uniform sampler2D uSolid;
uniform sampler2D uLinesP;   
uniform sampler2D uLinesV;  
uniform bool uShowLinesP;
uniform bool uShowLinesV; 
uniform float uGamma;    // gamma correction : < 1 = boost les faibles valeurs
uniform float uContrast; // contrast [0..4], 1 = neutre

vec3 viridis(float t) {
	t = clamp(t, 0.0, 1.0);
	vec3 c = vec3(0.0);
	c.r = 0.267 + t*(-0.003 + t*(1.785 + t*(-3.653 + t*(3.543 - t*1.344))));
	c.g = 0.005 + t*(1.015 + t*(-0.387 + t*(-1.540 + t*(2.101 - t*0.768))));
	c.b = 0.329 + t*(1.384 + t*(-2.994 + t*(2.994 + t*(-1.742 + t*0.389))));
	return clamp(c, 0.0, 1.0);
}

vec3 RdBu_r(float t) {
	t = clamp(t, 0.0, 1.0);
	vec3 blue  = vec3(0.4, 0.7, 1.0);
	vec3 turq  = vec3(0.1, 0.45, 0.45);
	vec3 black = vec3(0.0, 0.0, 0.0);
	vec3 red   = vec3(1.0, 0.45, 0.45);
	vec3 oran  = vec3(0.7, 0.4, 0.2);
	if (t < 0.5) {
		return mix(turq, black, t / 0.5);
	} else {
		return mix(black, oran, (t - 0.5) / 0.5);
	}
}

void main()
{
	float solid = texture(uSolid, TexCoord).r;

	if (solid > 0.5) {
		vec2 texel = 1.0 / vec2(textureSize(uSolid, 0));
		float n = texture(uSolid, TexCoord + vec2( 0.0,  texel.y)).r;
		float s = texture(uSolid, TexCoord + vec2( 0.0, -texel.y)).r;
		float e = texture(uSolid, TexCoord + vec2( texel.x,  0.0)).r;
		float w = texture(uSolid, TexCoord + vec2(-texel.x,  0.0)).r;
		float bord = abs(n - s) + abs(e - w);
		if (bord > 0.1)
			FragColor = vec4(1.0, 1.0, 1.0, 1.0);
		else
			FragColor = vec4(0.12, 0.12, 0.14, 1.0);
		return;
	}

	float val = texture(uData, TexCoord).r;

	// Contraste : amplifie les écarts autour de 0.5
	val = clamp((val - 0.5) * uContrast + 0.5, 0.0, 1.0);

	// Gamma : boost les faibles valeurs (gamma < 1 = plus lumineux)
	// On applique le gamma en conservant la symétrie autour de 0.5
	float centered = val - 0.5;
	float sign_c   = sign(centered);
	float abs_c    = abs(centered) * 2.0; // [0..1]
	abs_c = pow(abs_c, uGamma);
	val = 0.5 + sign_c * abs_c * 0.5;
	val = clamp(val, 0.0, 1.0);

	//affichage lignes de champ et de pression
	vec3 color = RdBu_r(val);

	if (uShowLinesP) {
    	float lp = texture(uLinesP, TexCoord).r;
    	if (lp > 0.5) {
    	    vec2 texel = 1.0 / vec2(textureSize(uLinesP, 0));
    	    float n = texture(uLinesP, TexCoord + vec2(0.0,  texel.y)).r;
    	    float s = texture(uLinesP, TexCoord + vec2(0.0, -texel.y)).r;
    	    float e = texture(uLinesP, TexCoord + vec2( texel.x, 0.0)).r;
    	    float w = texture(uLinesP, TexCoord + vec2(-texel.x, 0.0)).r;
    	    if (n < 0.5 || s < 0.5 || e < 0.5 || w < 0.5)
    	        color = vec3(1.0, 0.0, 0.0);
	    }
	}

	if (uShowLinesV) {
	    float lv = texture(uLinesV, TexCoord).r;
	    if (lv > 0.5) {
	        vec2 texel = 1.0 / vec2(textureSize(uLinesV, 0));
	        float n = texture(uLinesV, TexCoord + vec2(0.0,  texel.y)).r;
	        float s = texture(uLinesV, TexCoord + vec2(0.0, -texel.y)).r;
	        float e = texture(uLinesV, TexCoord + vec2( texel.x, 0.0)).r;
	        float w = texture(uLinesV, TexCoord + vec2(-texel.x, 0.0)).r;
	        if (n < 0.5 || s < 0.5 || e < 0.5 || w < 0.5)
	            color = vec3(0.0, 1.0, 0.0);
	    }
	}

	FragColor = vec4(color, 1.0);
}
