#pragma once

#include <cassert>
#include <algorithm>
#include <memory>

#include <compare>

template <typename T>
class Sequence {
private:
	class Iterator;
	class Const_Iterator;
public:
	using value_type = T;
	using pointer = T*;
	using reference = T&;
	using const_reference = const T&;
	using size_type = std::size_t;

	using iterator = Iterator;
	using const_iterator = Const_Iterator;

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

	pointer array = nullptr;
	size_type mSize;
	size_type cap;

	pointer memAlloc(size_type amount) {
		void* bytes = ::operator new(amount * sizeof(value_type));
		return static_cast<pointer>(bytes);
	}
	void memFree() {
		if (array) {
			::operator delete(array, cap * sizeof(value_type));
			array = nullptr;
			cap = 0;
		}
	}
	pointer raw_begin()const {
		return array;
	}
	pointer raw_end()const {
		return array + mSize;
	}
	size_type growthFactor(size_type input)const {
		return input + (input / 2) + 1;
	}

	void grow(size_type newCap) {
		pointer newArray = memAlloc(newCap);		
		std::uninitialized_move(raw_begin(), raw_end(), newArray);
		std::destroy(raw_begin(), raw_end());
		memFree();
		array = newArray;
		cap = newCap;
	}
	pointer eraseImpl(const_iterator pos) {
		assert(pos >= begin() && pos < end());
		pointer ppos = const_cast<pointer>(pos.base());
		std::move(ppos + 1, raw_end(), ppos);//shift all elements
		array[mSize - 1].~T();
		--mSize;

		return (ppos == raw_end()) ? raw_end() : ppos;//ppos was not shifted but they array did get smaller, so raw_end() is a new value at this point
	}
	pointer eraseRangeImpl(const_iterator first, const_iterator last) {
		assert(first >= begin() && first <= last && last <= end());

		pointer pfirst = const_cast<pointer>(first.base());
		pointer plast  = const_cast<pointer>(last.base());

		if (pfirst == plast)
			return pfirst;

		pointer end = raw_end();

		if (plast == end) {
			std::destroy(pfirst, end);
			mSize = pfirst - raw_begin();
			return raw_end();
		}
		//else
		std::move(plast, end, pfirst);//from plast, until end, into first

		pointer destroyBegin = pfirst + (end - plast);
		std::destroy(destroyBegin, end);
		mSize -= (end - destroyBegin);

		return pfirst;
	}
	pointer removeImpl(const_iterator pos) {
		assert(pos >= begin() && pos < end());

		pointer dest = const_cast<pointer>(pos.base());
		pointer last = raw_end() - 1;

		*dest = std::move(*last);
		std::destroy_at(last);

		--mSize;
		return dest;
	}
public:
	//CONSTRUCTORS
	Sequence() :array(nullptr), mSize(0), cap(0) {}
	Sequence(const Sequence& rhs) {
		pointer newArray = memAlloc(rhs.cap);
		std::uninitialized_copy(rhs.raw_begin(), rhs.raw_end(), newArray);
		mSize = rhs.mSize;
		cap = rhs.cap;
		array = newArray;
	}
	Sequence& operator=(const Sequence& rhs) {
		if (this == &rhs)
			return *this;
		clear();
		memFree();
		pointer newArray = memAlloc(rhs.cap);
		std::uninitialized_copy(rhs.raw_begin(), rhs.raw_end(), newArray);
		mSize = rhs.mSize;
		cap = rhs.cap;
		array = newArray;
		return *this;
	}
	~Sequence() {
		clear();
		memFree();
	}
	//#################################################

	bool      isEmpty()const  { return mSize == 0; }
	size_type size()const     { return mSize; }
	size_type capacity()const { return cap; }

	void add(const reference value) {
		if (mSize == cap)
			grow(growthFactor(cap));
		std::construct_at(raw_end(), value);
		++mSize;
	}
	void add(T&& value) {
		if (mSize == cap)
			grow(growthFactor(cap));
		std::construct_at(raw_end(), std::move(value));
		++mSize;
	}

	void pop_back() {
		assert(mSize > 0);
		array[mSize-1].~T();
		--mSize;
	}

	void clear() {
		if (mSize == 0)
			return;
		std::destroy(raw_begin(), raw_end());
		mSize = 0;
	}

	reference operator[](size_type index) {
		assert(index < mSize);
		return array[index];
	}
	const reference operator[](size_type index)const {
		assert(index < mSize);
		return array[index];
	}
	const_iterator remove(const_iterator pos) {
		return const_iterator(removeImpl(pos));
	}
	iterator remove(iterator pos) {
		return iterator(removeImpl(pos));
	}
	template <typename UnaryPred>
	iterator remove(UnaryPred predicate) {

		pointer last = raw_end() - 1;
		pointer current = raw_begin();

		//if predicate is true, move last element to current then move the last pointer
		while (current <= last) {

			if (predicate(*current)) {
				*current = std::move(*last);
				--last;
			}
			else {
				++current;
			}
		}
		size_type newSize = last - raw_begin() + 1;

		//bulk delete everything from last to end then apply new size
		if (newSize < mSize) {
			std::destroy(last + 1, raw_end());
			mSize = newSize;
		}

		return end();
	}
	const_iterator erase(const_iterator pos) {
		return const_iterator(eraseImpl(pos));
	}
	iterator erase(iterator pos) {
		return iterator(eraseImpl(pos));//implicit conversion to const_iterator should occur
	}
	iterator erase(const_iterator first, const_iterator last) {
		return const_iterator(eraseRangeImpl(first, last));
	}
	iterator erase(iterator first, iterator last) {
		return iterator(eraseRangeImpl(first, last));
	}


	void reserve(size_type newCap) {
		if (newCap > cap) {
			grow(newCap);
		}
	}
	void shrinkToFit() {
		if (cap > mSize) {
			grow(mSize);
		}
	}
	void resize(size_type count) {
		if (count == mSize) {
			return;
		}
		//if count is less than current capacity, then no need to reallocate
		if (count <= cap) {
			//if count is less than size, cut off the end, else add default constructed elements
			if (count < mSize) {
				std::destroy(raw_begin() + count, raw_end());
			}
			else {
				std::uninitialized_default_construct(raw_end(), raw_begin() + count);
			}
			mSize = count;
			return;
		}
		//need reallocation and since count > cap, its always greater than mSize
		size_type newCap = growthFactor(count);
		pointer newArray = memAlloc(newCap);

		std::uninitialized_move(raw_begin(), raw_end(), newArray);
		std::uninitialized_default_construct(newArray + mSize, newArray + count);

		std::destroy(raw_begin(), raw_end());
		memFree();
		array = newArray;
		cap = newCap;
		mSize = count;
	}


	/*
	TODO

	rule of 5 constructor + initalizer list
	static asserts to make sure object has default, copy and move constructors, otherwise == no bueno
		
	
	emplace_back - i have no idea how to even but need to know ( && ? )
	at
	data

	EXTRAS IF AT ALL:
	append range (add multiple in one go, initalizer list?)
	*/
};

template<typename T>
class Sequence<T>::Iterator {
public:
	using value_type = T;
	using pointer = T*;
	using reference = T&;
	using iterator_category = std::random_access_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using self_type = Iterator;

	reference operator*() { return *ptr; }
	pointer operator->() { return ptr; }
	reference operator[](difference_type n) { return ptr[n]; }
	const reference operator*() const { return *ptr; }
	const pointer operator->() const { return ptr; }
	const reference operator[](difference_type n) const { return ptr[n]; }

	self_type& operator++() { ++ptr; return *this; }
	self_type operator++(int) { self_type temp = *this; ++ptr; return temp; }
	self_type& operator--() { --ptr; return *this; }
	self_type operator--(int) { self_type temp = *this; --ptr; return temp; }

	self_type& operator+=(difference_type n) { ptr += n; return *this; }
	self_type& operator-=(difference_type n) { ptr -= n; return *this; }
	self_type operator+(difference_type n)const { return self_type(ptr + n); }
	self_type operator-(difference_type n)const { return self_type(ptr - n); }
	difference_type operator-(const self_type& rhs)const { return ptr - rhs.ptr; }

	bool operator==(const self_type& rhs)const noexcept { return ptr == rhs.ptr; }
	auto operator<=>(const self_type& rhs)const noexcept { return ptr <=> rhs.ptr; }

	//bool operator!=(const self_type& rhs)const noexcept { return ptr != rhs.ptr; }
	//bool operator<(const self_type& rhs)const noexcept { return ptr < rhs.ptr; }
	//bool operator>(const self_type& rhs)const noexcept { return ptr > rhs.ptr; }
	//bool operator<=(const self_type& rhs)const noexcept { return ptr <= rhs.ptr; }
	//bool operator>=(const self_type& rhs)const noexcept { return ptr >= rhs.ptr; }


	explicit Iterator(pointer p) :ptr(p) {}
	pointer base()const { return ptr; }
private:
	pointer ptr = nullptr;
};

template<typename T>
class Sequence<T>::Const_Iterator {
public:
	using value_type = const T;
	using pointer = const T*;
	using reference = const T&;
	using iterator_category = std::random_access_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using self_type = Const_Iterator;

	reference operator*()const { return *ptr; }
	pointer operator->()const { return ptr; }
	reference operator[](difference_type n)const { return ptr[n]; }

	self_type& operator++() { ++ptr; return *this; }
	self_type operator++(int) { self_type temp = *this; ++ptr; return temp; }
	self_type& operator--() { --ptr; return *this; }
	self_type operator--(int) { self_type temp = *this; --ptr; return temp; }

	self_type& operator+=(difference_type n) { ptr += n; return *this; }
	self_type& operator-=(difference_type n) { ptr -= n; return *this; }
	self_type operator+(difference_type n)const { return self_type(ptr + n); }
	self_type operator-(difference_type n)const { return self_type(ptr - n); }
	difference_type operator-(const self_type& rhs)const { return ptr - rhs.ptr; }

	bool operator==(const self_type& rhs)const noexcept { return ptr == rhs.ptr; }
	auto operator<=>(const self_type& rhs)const noexcept { return ptr <=> rhs.ptr; }

	//bool operator!=(const self_type& rhs)const noexcept { return ptr != rhs.ptr; }
	//bool operator<(const self_type& rhs)const noexcept { return ptr < rhs.ptr; }
	//bool operator>(const self_type& rhs)const noexcept { return ptr > rhs.ptr; }
	//bool operator<=(const self_type& rhs)const noexcept { return ptr <= rhs.ptr; }
	//bool operator>=(const self_type& rhs)const noexcept { return ptr >= rhs.ptr; }

	explicit Const_Iterator(pointer p) :ptr(p) {}
	Const_Iterator(const Iterator& rp) :ptr(rp.base()) {}
	pointer base()const { return ptr; }
private:
	pointer ptr = nullptr;
};
