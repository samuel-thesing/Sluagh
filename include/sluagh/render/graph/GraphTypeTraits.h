#ifndef SLUAGH_GRAPHTYPETRAITS_H
#define SLUAGH_GRAPHTYPETRAITS_H

#include <concepts>
#include <string>
#include <type_traits>

template<typename T>
concept has_Descriptor = requires(T t) {
	requires std::conjunction_v<
		std::is_default_constructible<T>,
		std::is_move_constructible<T>
	>;

	typename T::Desc;
	{ t.create(typename T::Desc{}, static_cast<void*>(nullptr)) } -> std::same_as<void>;
	{ t.destroy(typename T::Desc{}, static_cast<void*>(nullptr)) } -> std::same_as<void>;
};

template <typename T>
concept has_preRead = requires(T t) {
	{ t.preRead(typename T::Desc{}, static_cast<void*>(nullptr)) } -> std::same_as<void>;
};
template <typename T>
concept has_preWrite = requires(T t) {
	{ t.preWrite(typename T::Desc{}, static_cast<void*>(nullptr)) } -> std::same_as<void>;
};

template <typename T>
concept has_toString = requires() {
	{ T::toString(typename T::Desc{}) } -> std::convertible_to<std::string>;
};

#endif // SLUAGH_GRAPHTYPETRAITS_H
