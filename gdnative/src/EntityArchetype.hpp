#pragma once

#include <tuple>
#include <vector>

template <typename... Args>
class EntityArchetype {

private:

	std::tuple<std::vector<Args>...> components;

public:

	static_assert(sizeof...(Args) > 0, "Entity Archetype must have atleast 1 component");

	const size_t componentCount = sizeof...(Args);

	template <typename T> 
	inline T* get_component() const {
		return (T*)std::get<std::vector<T>>(components).data();
	}

	template <typename T>
	inline T& get_component(size_t index) const {
		return ((T*)std::get<std::vector<T>>(components).data())[index];
	}

	void remove_swap(size_t index) { //Fast O(1) removal, at the expense of not preserving insertion order

		std::apply([&](auto& ...vector) {

		if (size() > 0) {
			(..., (vector[index] = vector.back()));
			(..., vector.pop_back());
		}
		else {
			(..., vector.clear());
		}
			
		}, components);
	}

	template <typename F>
	void remove_batch_if(const F&& checkFunction) { //Fast multiple element O(n) removal, while preserving insertion order

		std::apply([&](auto& ...vector) {

		int32_t last = 0;

		for (int32_t i = 0; i < size(); ++i) {
			bool remove = checkFunction(i);
			if (remove)
				continue;

			(..., (vector[last] = vector[i]));
			last++;
		}

		(..., (vector.resize(last)));

		}, components);
	}

	void erase(size_t index) {

		std::apply([&](auto& ...vector) { 
			
		(..., vector.erase(vector.begin() + index));

		}, components);
	}

	void erase(size_t begin, size_t end) {

		std::apply([&](auto& ...vector) {

		(..., vector.erase(vector.begin() + begin, vector.begin() + end));

		}, components);
	}

	void resize(size_t newSize) {

		std::apply([&](auto& ...vector) {

		(..., vector.resize(newSize));

		}, components);
	}

	inline size_t size() const {
		return std::get<0>(components).size();
	}

	inline size_t capacity() const {
		return std::get<0>(components).capacity();
	}

};
