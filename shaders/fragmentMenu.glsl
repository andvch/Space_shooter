#version 330 core

in vec2 coords;

out vec4 color;

uniform sampler2D font;

uniform vec2 aimPos;
uniform vec3 menuColor;

uniform int playerHP;
uniform int playerScore;

vec2 screenCoord(vec2 old)
{
	return vec2((0.5 * old.x + 0.5), (-0.5 * old.y + 0.5));
}

float draw(vec2 p, vec2 min, vec2 max, vec2 uv_min, vec2 uv_max)
{
	if (p.x < min.x || p.y < min.y || p.x > max.x || p.y > max.y)
		return 0.0;
	vec2 tmp = uv_min +
		vec2((uv_max.x - uv_min.x) * (p.x - min.x) / (max.x - min.x),
		(uv_max.y - uv_min.y) * (p.y - min.y) / (max.y - min.y));
	return texture(font, tmp).a;
}

float drawNum(vec2 p, vec2 min, vec2 max, int num, int n)
{
	int i, d;
	float f, width = (max.x - min.x) / n;
	for (i = n - 1; i >= 0; --i) {
		d = num % 10;
		f = draw(p, vec2(min.x + i * width, min.y),
			vec2(min.x + (i + 1) * width, max.y),
			vec2(0.1 * d, 0.866), vec2(0.1 * (d + 1), 1.0));
		if (f > 0.1)
			return f;
		num /= 10;
	}
	return 0.0;
}

float getAlf(vec2 p)
{
	float alf = draw(p, vec2(0, 0), vec2(1, 1), vec2(0, 0), vec2(1, 0.6));
	if (alf > 0.1)
		return alf;;
	if (playerHP <= 0) {
		alf = draw(p, vec2(0.3, 0.35), vec2(0.7, 0.5),
			vec2(0.133, 0.6), vec2(1, 0.733));
		if (alf > 0.1)
			return alf;
		alf = draw(p, vec2(0.3, 0.55), vec2(0.5, 0.65),
			vec2(0.6, 0.733), vec2(1.0, 0.866));
		if (alf > 0.1)
			return alf;
		alf = drawNum(p, vec2(0.5, 0.55), vec2(0.65, 0.64), playerScore, 6);
		if (alf > 0.1)
			return alf;
	} else {
		alf = draw(p, vec2(0.1, 0.1), vec2(0.3, 0.2),
			vec2(0.0, 0.733), vec2(0.6, 0.866));
		if (alf > 0.1)
			return alf;
		alf = draw(p, vec2(0.77, 0.1), vec2(0.9, 0.2),
			vec2(0.6, 0.733), vec2(1.0, 0.866));
		if (alf > 0.1)
			return alf;
		alf = drawNum(p, vec2(0.1, 0.2), vec2(0.2, 0.25), playerHP, 3);
		if (alf > 0.1)
			return alf;
		alf = drawNum(p, vec2(0.7, 0.2), vec2(0.9, 0.25), playerScore, 6);
		if (alf > 0.1)
			return alf;
	}
	return 0.0;
}

void main()
{
	float d = 0.5;
	vec2 screen = screenCoord(coords);
	vec2 mouse = screenCoord(aimPos);
	float alf = getAlf(screen);
	alf += draw(screen, mouse - vec2(0.03, 0.05), mouse + vec2(0.03, 0.05),
		vec2(0.0, 0.6), vec2(0.133, 0.733));
	color = vec4(menuColor, d * alf);
}
