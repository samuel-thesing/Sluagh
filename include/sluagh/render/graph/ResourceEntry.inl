#ifndef SLUAGH_RESOURCEENTRY_INL
#define SLUAGH_RESOURCEENTRY_INL

#include "ResourceEntry.h"

template<typename T>
T& ResourceEntry::get() {
	return getModel<T>()->resource;
}

template<typename T>
const T::Desc& ResourceEntry::getDescriptor() const {
	return getModel<T>()->descriptor;
}

template<typename T>
ResourceEntry::Model<T>* ResourceEntry::getModel() const {
	auto *model = dynamic_cast<Model<T>*>(concept_ptr.get());
	return model;
}

template<typename T>
ResourceEntry::ResourceEntry(Type type, uint32_t id, const typename T::Desc &descriptor, T &&resource)
	: type{type}, id{id}, version{initial_version},
		concept_ptr{std::make_unique<Model<T>>(descriptor, std::forward<T>(resource))} {}

inline void ResourceEntry::create(void *allocator) {
	concept_ptr->create(allocator);
}

inline void ResourceEntry::destroy(void *allocator) {
	concept_ptr->destroy(allocator);
}

inline void ResourceEntry::preRead(void *context) {
	concept_ptr->preRead(context);
}

inline void ResourceEntry::preWrite(void *context) {
	concept_ptr->preWrite(context);
}

template<typename T>
ResourceEntry::Model<T>::Model(const typename T::Desc &descriptor, T &&resource)
	: descriptor{descriptor}, resource{std::move(resource)} {}

template<typename T>
void ResourceEntry::Model<T>::create(void *allocator) {
	resource.create(descriptor, allocator);
}

template<typename T>
void ResourceEntry::Model<T>::destroy(void *allocator) {
	resource.destroy(descriptor, allocator);
}

template<typename T>
void ResourceEntry::Model<T>::preRead(void *context) {
	if constexpr (has_preRead<T>) {
		resource.preRead(context);
	}
}

template<typename T>
void ResourceEntry::Model<T>::preWrite(void *context) {
	if constexpr (has_preWrite<T>) {
		resource.preWrite(context);
	}
}

template<typename T>
std::string ResourceEntry::Model<T>::toString() const {
	if constexpr (has_toString<T>) {
		return T::toString(descriptor);
	}
	return "";
}

#endif // SLUAGH_RESOURCEENTRY_INL
