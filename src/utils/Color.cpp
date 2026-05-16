#include "sluagh/utils/Color.h"

Color::Color() : r {1.f}, g{1.f}, b{1.f} {}

Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	: r(r / 255.f), g(g / 255.f), b(b / 255.f), a(a / 255.f) {}

Color::Color(int r, int g, int b, int a)
	: r(static_cast<float>(r) / 255.f), g(static_cast<float>(g) / 255.f),
		b(static_cast<float>(b) / 255.f), a(static_cast<float>(a) / 255.f) {}

Color::Color(float r, float g, float b, float a)
	: r(r), g(g), b(b), a(a) {}

Vec4 Color::toVec() const {
	return Vec4(r, g, b, a);
}

Color Color::withAlpha(float alpha) const {
	return Color{r, g, b, alpha};
}

Color Color::darken(float by) const {
	if (by < 0.f) return lighten(-by);
	if (by >= 1.f) return Color(0.f, 0.f, 0.f);
	return Color{
		std::fmax(0.0f, r - by),
		std::fmax(0.0f, g - by),
		std::fmax(0.0f, b - by)
	};
}

Color Color::lighten(float by) const {
	if (by < 0.f) return darken(-by);
	if (by >= 1.f) return Color(1.f, 1.f, 1.f);
	return Color{
		std::fmin(1.0f, r + by),
		std::fmin(1.0f, g + by),
		std::fmin(1.0f, b + by)
	};
}