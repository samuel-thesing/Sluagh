#ifndef SLUAGH_VECTOR_H
#define SLUAGH_VECTOR_H
#include <cmath>
#include <cstdint>
#include <cstring>

template <typename T, typename U>
concept supportsArithmetics = requires(T t, U u) {
	std::is_integral_v<U> || std::is_floating_point_v<U>;
	{ t + u } -> std::convertible_to<T>;
	{ t - u } -> std::convertible_to<T>;
	{ t * u } -> std::convertible_to<T>;
	{ t / u } -> std::convertible_to<T>;
};

#define VECTOR_OP_SCALAR(op) \
	template <typename U> requires supportsArithmetics<T, U>			\
	Type operator op(U const scalar) const {							\
		auto result = Type();											\
		for (int i = 0; i < Rows; ++i) {								\
			result.data[i] = static_cast<T>(data[i] op scalar);			\
		}																\
		return result;													\
	}																	\
	template <typename U> requires supportsArithmetics<T, U>			\
	friend Type operator op(U const scalar, const Vector<Rows, U>& v) {	\
		auto result = Type();											\
		for (int i = 0; i < Rows; ++i) {								\
			result.data[i] = static_cast<T>(scalar op v.data[i]);		\
		}																\
		return result;													\
	}

#define VECTOR_OP_EQUAL_SCALAR(op) \
	template <typename U> requires supportsArithmetics<T, U>	\
	Type& operator op##=(U const scalar) {						\
		for (int i = 0; i < Rows; ++i) {						\
			data[i] = static_cast<T>(data[i] op scalar);		\
		}														\
		return *this;											\
	}

#define VECTOR_OP(op) \
	Type operator op(Type const& o) const {							\
		auto result = Type();										\
		for (int i = 0; i < Rows; ++i) {							\
			result.data[i] = static_cast<T>(data[i] op o.data[i]);	\
		}															\
		return result;												\
	}

#define VECTOR_OP_EQUAL(op) \
	Type& operator op##=(Type const& o) {					\
		for (int i = 0; i < Rows; ++i) {					\
			data[i] = static_cast<T>(data[i] op o.data[i]);	\
		}													\
		return *this;										\
	}

template<int Rows, typename T>
class Vector {
public:
	typedef Vector<Rows, T> Type;

private:
	T data[Rows];

public:
	Vector() : data{static_cast<T>(0)} {}

	template <typename... Ts> requires ((std::is_convertible_v<Ts, T> && ...) && (sizeof...(Ts) == Rows))
	Vector(Ts... elements)
		: data{static_cast<T>(elements)...} {}

	Vector(Type const& o) : data{} {
		std::memcpy(data, o.data, sizeof(T) * Rows);
	}

	template <typename U> requires (std::is_convertible_v<U, T> && !std::is_same_v<U, T>)
	Vector(const Vector<Rows, U>& o) : data{} {
		for (int i = 0; i < Rows; ++i) {
			data[i] = static_cast<T>(o.data[i]);
		}
	}

public:
	T x() const { return data[0]; }
	T y() const requires (Rows >= 2) { return data[1]; }
	T z() const requires (Rows >= 3) { return data[2]; }
	T w() const requires (Rows >= 4) { return data[3]; }

	T& x() { return data[0];}
	T& y() requires (Rows >= 2) { return data[1];}
	T& z() requires (Rows >= 3) { return data[2];}
	T& w() requires (Rows >= 4) { return data[3];}

	T operator[](int row) const { return data[row]; }
	T& operator[](int row) { return data[row]; }

	bool operator==(Type const& o) const {
		for (int i = 0; i < Rows; ++i) {
			if (data[i] != o.data[i]) {
				return false;
			}
		}
		return true;
	}

	Type& operator=(Type const& o) {
		std::memcpy(data, o.data, sizeof(T) * Rows);
		return *this;
	}

	VECTOR_OP_SCALAR(+)
	VECTOR_OP_SCALAR(-)
	VECTOR_OP_SCALAR(*)
	VECTOR_OP_SCALAR(/)

	VECTOR_OP_EQUAL_SCALAR(+)
	VECTOR_OP_EQUAL_SCALAR(-)
	VECTOR_OP_EQUAL_SCALAR(*)
	VECTOR_OP_EQUAL_SCALAR(/)

	VECTOR_OP(+)
	VECTOR_OP(-)
	VECTOR_OP_EQUAL(+)
	VECTOR_OP_EQUAL(-)

	template <typename U> requires supportsArithmetics<T, U>
	T dot(const Vector<Rows, U>& o) const {
		T sum = static_cast<T>(0);
		for (int i = 0; i < Rows; ++i) {
			sum += static_cast<T>(data[i] * o.data[i]);
		}
		return sum;
	}

	template <typename U> requires (supportsArithmetics<T, U> && Rows == 3)
	Type cross(const Vector<Rows, U>& o) const {
		auto result = Type();
		result.data[0] = static_cast<T>(data[1] * o.data[2] - data[2] * o.data[1]);
		result.data[1] = static_cast<T>(data[2] * o.data[0] - data[0] * o.data[2]);
		result.data[2] = static_cast<T>(data[0] * o.data[1] - data[1] * o.data[0]);
		return result;
	}

	Type cross() const requires (Rows == 2) {
		auto result = Type();
		result.x() = data[1];
		result.y() = -data[0];
		return result;
	}

	double norm() const {
		double sum = 0;
		for (int i = 0; i < Rows; ++i) {
			sum += data[i] * data[i];
		}
		return std::sqrt(sum);
	}

	Type abs() const {
		auto result = Type();
		for (int i = 0; i < Rows; ++i) {
			result.data[i] = std::abs(data[i]);
		}
		return result;
	}

	Type normalized() const {
		auto len = norm();
		auto result = Type();
		for (int i = 0; i < Rows; ++i) {
			result.data[i] = static_cast<T>(data[i] / len);
		}
		return result;
	}
};

typedef Vector<2, int32_t> Vec2i;
typedef Vector<3, int32_t> Vec3i;
typedef Vector<4, int32_t> Vec4i;
typedef Vector<2, uint32_t> Vec2u;
typedef Vector<3, uint32_t> Vec3u;
typedef Vector<4, uint32_t> Vec4u;
typedef Vector<2, int64_t> Vec2l;
typedef Vector<3, int64_t> Vec3l;
typedef Vector<4, int64_t> Vec4l;
typedef Vector<2, uint64_t> Vec2ul;
typedef Vector<3, uint64_t> Vec3ul;
typedef Vector<4, uint64_t> Vec4ul;
typedef Vector<2, float> Vec2f;
typedef Vector<3, float> Vec3f;
typedef Vector<4, float> Vec4f;
typedef Vector<2, double> Vec2d;
typedef Vector<3, double> Vec3d;
typedef Vector<4, double> Vec4d;

#undef VECTOR_OP_SCALAR
#undef VECTOR_OP_EQUAL_SCALAR
#undef VECTOR_OP
#undef VECTOR_OP_EQUAL


#endif // SLUAGH_VECTOR_H
