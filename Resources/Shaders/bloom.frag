#version 450

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 0) uniform sampler2D hdr;

const float glow_intensity  = 2.0;
const float glow_threshold  = 0.8;

const float glow_size = 40.0;
const float invSigma =  1.0 / (2.0 * glow_size * 0.5 * glow_size * 0.5);

float gaussian2D(float distSquared) {
	return exp(-(distSquared) * invSigma);
}

vec4 blur_2d(vec2 uv, ivec2 size, float radius) {
	vec4 sum    	= vec4(0.0);
	float weight 	= 0.0;
	float step 		= max(1.0, radius / 8.0);

	for (float y = -radius; y <= radius; y += step) {
		for (float x = -radius; x <= radius; x += step) {
			vec2 v = vec2(x, y);
			float dist = dot(v, v);

			if (dist > radius * radius) continue;

			ivec2 coord = clamp(
			ivec2(uv.x * size.x + x, uv.y * size.y + y),
			ivec2(0), size - 1
			);

			float w  = gaussian2D(dist);
			sum     += texelFetch(hdr, coord, 0) * w;
			weight  += w;
		}
	}

	return weight > 0.0 ? sum / weight : vec4(0.0);
}

void main() {
	vec4  base = texture(hdr, TexCoords);
	ivec2 size = textureSize(hdr, 0);

	vec4  glow = blur_2d(TexCoords, size, glow_size) * glow_intensity;

	if (base.a > glow_threshold) {
		outColor = base + glow * (1.0 - base.a);
	} else {
		outColor = vec4(glow.rgb, glow.a * glow_intensity);
	}
}