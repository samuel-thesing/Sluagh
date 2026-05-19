#ifndef SLUAGH_MATRIX_H
#define SLUAGH_MATRIX_H
#include <cmath>
#include <cstring>
#include <type_traits>

#include "sluagh/utils/Vector.h"

#define MATRIX_OP_SCALAR(op) \
	template <typename U> requires supportsArithmetics<T, U>		\
	Type operator op(U const scalar) {								\
		auto result = Type();										\
		for (int i = 0; i < Rows * Cols; ++i) {						\
			result.data[i] = static_cast<T>(data[i] op scalar);		\
		}															\
		return result;												\
	}																\
																	\
	template <typename U> requires supportsArithmetics<T, U>		\
	friend Type operator op(U const scalar, const Type& m) {		\
		auto result = Type();										\
		for (int i = 0; i < Rows * Cols; ++i) {						\
			result.data[i] = static_cast<T>(scalar op m.data[i]);	\
		}															\
		return result;												\
	}

#define MATRIX_OP_SCALAR_EQUAL(op) \
	template <typename U> requires supportsArithmetics<T, U>	\
	Type operator op##=(U const scalar) {							\
		for (int i = 0; i < Rows * Cols; ++i) {					\
			data[i] = static_cast<T>(data[i] op scalar);		\
		}														\
		return *this;											\
	}

#define MATRIX_OP(op) \
	Type operator op(Type const& o) const {							\
		auto result = Type();										\
		for (int i = 0; i < Rows * Cols; ++i) {						\
			result.data[i] = static_cast<T>(data[i] op o.data[i]);	\
		}															\
		return result;												\
	}

#define MATRIX_OP_EQUAL(op) \
	Type& operator op##=(Type const& o) {					\
		for (int i = 0; i < Rows * Cols; ++i) {				\
			data[i] = static_cast<T>(data[i] op o.data[i]);	\
		}													\
		return *this;										\
	}

template <typename T>
concept isQuad = requires(int Rows, int Cols) {
	Rows == Cols;
};

template <int Rows, int Cols, typename T> requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
class Matrix {
public:
	typedef Matrix<Rows, 1, T> ColumnType;
	typedef Matrix<1, Cols, T> RowType;
	typedef Matrix<Cols, Rows, T> TransposeType;
	typedef Matrix<Rows, Cols, T> Type;

private:
	T data[Rows * Cols];

public:
	Matrix() : data{static_cast<T>(0)} {}

	template <typename... Ts> requires ((std::is_convertible_v<Ts, T> && ...) && (sizeof...(Ts) == Rows * Cols))
	Matrix(Ts... elements)
		: data{} {
		int col = 0;
		int row = 0;
		([&] {
			++col;
			if (col >= Cols) {
				col = 0;
				++row;
			}
			data[col * Rows + row] = static_cast<T>(elements);
		}(), ...);
	}

	explicit Matrix(Type const& o) : data{} {
		std::memcpy(data, o.data, sizeof(T) * Rows * Cols);
	}

	template <typename U> requires (std::is_convertible_v<U, T> && !std::is_same_v<U, T>)
	Matrix(const Matrix<Rows, Cols, U>& o) : data{} {
		for (int i = 0; i < Rows * Cols; ++i) {
			data[i] = static_cast<T>(o.data[i]);
		}
	}

public:
	T operator[](int row, int col) const { return data[col * Rows + row]; }
	T& operator[](int row, int col) { return data[col * Rows + row]; }

	const T* valuePtr() const { return data; }

	bool operator==(Type const& o) const {
		for (int i = 0; i < Rows * Cols; ++i) {
			if (data[i] != o.data[i]) {
				return false;
			}
		}
		return true;
	}

	Type& operator=(Type const& o) {
		std::memcpy(data, o.data, sizeof(T) * Rows * Cols);
		return *this;
	}

	template <typename U> requires supportsArithmetics<T, U>
	Type operator=(U const scalar) {
		auto result = Type();
		for (int i = 0; i < Rows * Cols; ++i) {
			result.data[i] = static_cast<T>(data[i] + scalar);
		}
		return result;
	}

	MATRIX_OP_SCALAR(+)
	MATRIX_OP_SCALAR(-)
	MATRIX_OP_SCALAR(*)
	MATRIX_OP_SCALAR(/)

	MATRIX_OP_SCALAR_EQUAL(+)
	MATRIX_OP_SCALAR_EQUAL(-)
	MATRIX_OP_SCALAR_EQUAL(*)
	MATRIX_OP_SCALAR_EQUAL(/)

	MATRIX_OP(+)
	MATRIX_OP(-)
	MATRIX_OP_EQUAL(+)
	MATRIX_OP_EQUAL(-)

	T det() const requires (Rows == Cols && Rows == 1) {
		return data[0];
	}

	T det() const requires (Rows == Cols && Rows == 2) {
		return data[0] * data[3] - data[1] * data[2];
	}

	T det() const requires (Rows == Cols && Rows == 3) {
		return data[0] * data[4] * data[8] + data[1] * data[5] * data[6] + data[2] * data[3] * data[7]
			- data[2] * data[4] * data[6] - data[0] * data[5] * data[7] - data[1] * data[3] * data[8];
	}

	TransposeType transpose() const {
		auto result = TransposeType();
		for (int r = 0; r < Rows; ++r) {
			for (int c = 0; c < Cols; ++c) {
				result[c, r] = *this[r, c];
			}
		}
		return result;
	}

	template <int Cols2>
	Matrix<Rows, Cols2, T> operator*(const Matrix<Cols, Cols2, T>& o) {
		auto result = Matrix<Rows, Cols2, T>();
		for (int r = 0; r < Rows; ++r) {
			for (int c2 = 0; c2 < Cols2; ++c2) {
				auto sum = static_cast<T>(0);
				for (int c = 0; c < Cols; ++c) {
					sum += static_cast<T>(*this[r, c] * o[c, c2]);
				}
				result[r, c2] = sum;
			}
		}
		return result;
	}

	Vector<Rows, T> operator*(const Vector<Cols, T>& o) {
		auto result = Vector<Rows, T>();
		for (int r = 0; r < Rows; ++r) {
			auto sum = static_cast<T>(0);
			for (int c = 0; c < Cols; ++c) {
				sum += static_cast<T>(*this[r, c] * o[c]);
			}
			result[r] = sum;
		}
		return result;
	}

	static Type identity() {
		auto result = Type();
		for (int i = 0; i < Rows && i < Cols; ++i) {
			result[i, i] = static_cast<T>(1);
		}
		return result;
	}
};

typedef Matrix<2, 2, float> Mat2f;
typedef Matrix<3, 3, float> Mat3f;
typedef Matrix<4, 4, float> Mat4f;
typedef Matrix<2, 2, double> Mat2d;
typedef Matrix<3, 3, double> Mat3d;
typedef Matrix<4, 4, double> Mat4d;


#endif // SLUAGH_MATRIX_H
