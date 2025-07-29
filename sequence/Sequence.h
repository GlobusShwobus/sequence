#pragma once

#include <cassert>
#include <algorithm>
#include <memory>


template <typename T>
class Sequence {

public:
	template<typename I>
	struct Iterator {
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = I;
		using difference_type = std::ptrdiff_t;
		using pointer = I*;
		using reference = I&;

		//constructors
		template <typename friendo> friend struct Iterator;
		Iterator(I* p) :ptr(p) {}
		template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
		Iterator(const Iterator<U>& other) : ptr(other.ptr) {}
		template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
		Iterator& operator=(const Iterator<U>& other) {
			ptr = other.ptr;
			return *this;
		}
		//########################

		reference operator*()const {
			return *ptr;
		}
		pointer operator->()const {
			return ptr;
		}
		reference operator[](long long n)const {
			return ptr[n];
		}

		Iterator& operator++() {
			++ptr;
			return *this;
		}
		Iterator operator++(int) {
			Iterator temp = *this;
			++ptr;
			return temp;
		}
		Iterator& operator--() {
			--ptr;
			return *this;
		}
		Iterator operator--(int) {
			Iterator temp = *this;
			--ptr;
			return temp;
		}

		Iterator& operator+=(difference_type n) {
			ptr += n;
			return *this;
		}
		Iterator& operator-=(difference_type n) {
			ptr -= n;
			return *this;
		}
		Iterator operator+(difference_type n)const {
			return Iterator(ptr + n);
		}
		Iterator operator-(difference_type n)const {
			return Iterator(ptr - n);
		}
		difference_type operator-(const Iterator& rhs)const {
			return ptr - rhs.ptr;
		}
		template<typename U>
		bool operator==(const Iterator<U>& rhs)const noexcept {
			return ptr == rhs.ptr;
		}
		template<typename U>
		bool operator!=(const Iterator<U>& rhs)const noexcept {
			return ptr != rhs.ptr;
		}
		template<typename U>
		bool operator<(const Iterator<U>& rhs)const noexcept {
			return ptr < rhs.ptr;
		}
		template<typename U>
		bool operator>(const Iterator<U>& rhs)const noexcept {
			return ptr > rhs.ptr;
		}
		template<typename U>
		bool operator<=(const Iterator<U>& rhs)const noexcept {
			return ptr <= rhs.ptr;
		}
		template<typename U>
		bool operator>=(const Iterator<U>& rhs)const noexcept {
			return ptr >= rhs.ptr;
		}
	private:
		pointer ptr = nullptr;
	};

	using iterator = Iterator<T>;
	using const_iterator = Iterator<const T>;

	iterator begin() {
		return iterator(array);
	}
	iterator end() {
		return iterator(array + mSize);
	}
	const_iterator begin()const {
		return const_iterator(array);
	}
	const_iterator end()const {
		return const_iterator(array + mSize);
	}
	const_iterator cbegin()const {
		return const_iterator(array);
	}
	const_iterator cend()const {
		return const_iterator(array + mSize);
	}

private:

	T* array = nullptr;
	size_t mSize;
	size_t cap;

	void grow() {
		size_t newCap = (cap == 0) ? 1 : cap * 2;//assign new size
		void* bytes = ::operator new(newCap * sizeof(T));//allocate raw bytes
		T* newArray = static_cast<T*>(bytes);//cast it to the actual type
		
		std::uninitialized_move(array, array + mSize, newArray);//optimized??
		//vs
		//for (size_t i = 0; i < mSize; i++)
		//	new (newArray + i) T(std::move(array[i]));//use rvalue move from old sequence to new
			
		::operator delete(array, cap * sizeof(T));//type T MUST have noexcept move constructor to ignore calling .~T(), thus we can get away with only deleting the memory

		array = newArray;
		cap = newCap;
	}
	void copyFormDirty(const Sequence& rhs) {
		//almost exactly the same as grow() except it makes deep copies
		mSize = rhs.mSize;
		cap = rhs.cap;
		void* bytes = ::operator new(rhs.cap * sizeof(T));
		T* newArray = static_cast<T*>(bytes);

		std::uninitialized_copy(rhs.begin(), rhs.end(), newArray);
		////vs
		//for (size_t i = 0; i < mSize; i++) {
		//	new (newArray + i) T(rhs.array[i]);
		//}
		array = newArray;
	}
	void clearAndDelete() {
		clear();
		::operator delete(array, cap * sizeof(T));
		array = nullptr;
	}
	T* eraseImpl(T* pos) {
		assert(pos >= array && pos < array + mSize);
		std::move(pos + 1, array + mSize, pos);//shift all elements
		array[mSize - 1].~T();
		--mSize;
		if (pos == (array + mSize))
			return array + mSize;
		return pos;
	}
	T* eraseRangeImpl(T* first, T* last) {
		assert(first >= array && first <= last && last <= (array + mSize));

		if (first == last) 
			return first;
		
		T* end = array + mSize;

		if (last == end) {
			for (T* i = first; i != end; ++i)
				i->~T();
			mSize = first - array;
			return (array + mSize);
		}
		//else
		std::move(last, end, first);//shift all elements
		T* destroyBegin = first + (end - last);

		for (T* i = destroyBegin; i != end; ++i) {
			i->~T();
			--mSize;
		}
		return first;
	}

public:
	Sequence() :array(nullptr), mSize(0), cap(0) {}
	Sequence(const Sequence& rhs) {
		copyFormDirty(rhs);
	}
	Sequence& operator=(const Sequence& rhs) {
		if (this == &rhs)
			return *this;
		clearAndDelete();
		copyFormDirty(rhs);
		return *this;
	}


	~Sequence() {
		clearAndDelete();
	}

	bool isEmpty()const {
		return mSize == 0;
	}
	size_t size()const {
		return mSize;
	}
	size_t capacity()const {
		return cap;
	}
	void add(const T& value) {
		if (mSize == cap)
			grow();
		new(array + mSize) T(value);//beginning of the sequence+ptr arithmetic index, place the value there
		++mSize;
	}
	void add(T&& value) {
		if (mSize == cap)
			grow();
		new(array + mSize) T(std::move(value));//beginning of the sequence+ptr arithmetic index, place the value there
		++mSize;
	}

	void pop_back() {
		assert(mSize != 0);
		array[mSize-1].~T();
		--mSize;
	}

	void clear() {
		if (mSize == 0)
			return;
		for (size_t i = mSize; i-- > 0;) {
			array[i].~T();
		}
		mSize = 0;
	}

	T& operator[](size_t index) {
		assert(index < mSize);
		return array[index];
	}
	const T& operator[](size_t index)const {
		assert(index < mSize);
		return array[index];
	}

	void remove(const_iterator pos) {
		assert(pos >= begin() && pos < end());
		*pos = std::move(array[mSize - 1]);
		array[mSize - 1].~T();
		--mSize;
	}
	void remove(iterator pos) {
		remove(const_iterator(pos));
	}
	template <typename Pred>
	void remove(Pred predicate) {
		auto it = std::remove_if(begin(), end(), predicate);
		erase(it, end());
	}

	const_iterator erase(const_iterator pos) {
		return const_iterator(eraseImpl(const_cast<T*>(&*pos)));//reference to the address casted to T
	}
	iterator erase(iterator pos) {
		return iterator(eraseImpl(&*pos));
	}
	iterator erase(const_iterator first, const_iterator last) {
		return const_iterator(eraseRangeImpl(const_cast<T*>(&*first), const_cast<T*>(&*last)));
	}
	iterator erase(iterator first, iterator last) {
		return iterator(eraseRangeImpl(&*first, &*last));
	}


	/*
	TODO
	resize
	reserve
	move constructor
	THEN CAN CLEAN THE FUCK UP BEFORE OTHER FLUFF

	emplace_back - i have no idea how to even but need to know ( && ? )
	append range (add multiple in one go, initalizer list?)
	shrink to fit

	IMPORTNAT:
		static asserts to make sure object has default, copy and move constructors, otherwise == no bueno
		*/
};




//template <typename T>
//SeqIterator<T> operator+(typename SeqIterator<T>::difference_type n, const SeqIterator<T>& it) {
//	return it + n;
//}
//template<typename T>
//ConstSeqIterator<T> operator+(typename ConstSeqIterator<T>::difference_type n, const ConstSeqIterator<T>& it) {
//	return it + n;
//}