#pragma once

#include <cassert>
#include <algorithm>
#include <memory>


template <typename T>
class Sequence {

public:
	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using size_type = std::size_t;

	template<typename I>
	struct Iterator {
	public:
		using value_type = I;
		using pointer = I*;
		using reference = I&;
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using self_type = Iterator;


		//constructors
		friend class Sequence<T>;//iterator is dependant on sequence therefor T not I
		template <typename friendo> friend struct Iterator;
		Iterator(pointer p) :ptr(p) {}
		template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>//regular to const
		Iterator(const Iterator<U>& other) : ptr(other.ptr) {}
		template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>//regualar to const
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
		reference operator[](difference_type n)const {
			return ptr[n];
		}

		self_type& operator++() {
			++ptr;
			return *this;
		}
		self_type operator++(int) {
			self_type temp = *this;
			++ptr;
			return temp;
		}
		self_type& operator--() {
			--ptr;
			return *this;
		}
		self_type operator--(int) {
			self_type temp = *this;
			--ptr;
			return temp;
		}

		self_type& operator+=(difference_type n) {
			ptr += n;
			return *this;
		}
		self_type& operator-=(difference_type n) {
			ptr -= n;
			return *this;
		}
		self_type operator+(difference_type n)const {
			return self_type(ptr + n);
		}
		self_type operator-(difference_type n)const {
			return self_type(ptr - n);
		}
		difference_type operator-(const self_type& rhs)const {
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

	pointer array = nullptr;
	size_type mSize;
	size_type cap;

	void grow() {
		size_type newCap = (cap == 0) ? 1 : cap * 2;//assign new size
		
		pointer newArray = memAlloc(newCap);

		//void* bytes = ::operator new(newCap * sizeof(T));//allocate raw bytes
		//T* newArray = static_cast<T*>(bytes);//cast it to the actual type
		
		std::uninitialized_move(raw_begin(), raw_end(), newArray);//optimized??
		//vs
		//for (size_t i = 0; i < mSize; i++)
		//	new (newArray + i) T(std::move(array[i]));//use rvalue move from old sequence to new
			
		//::operator delete(array, cap * sizeof(T));//type T MUST have noexcept move constructor to ignore calling .~T(), thus we can get away with only deleting the memory
		memFree();

		array = newArray;
		cap = newCap;
	}
	/*
	* DEPRICATED
	void copyFormDirty(const Sequence& rhs) {
		//almost exactly the same as grow() except it makes deep copies
		mSize = rhs.mSize;
		cap = rhs.cap;
		//void* bytes = ::operator new(rhs.cap * sizeof(T));
		//T* newArray = static_cast<T*>(bytes);
		T* newArray = memAlloc(rhs.cap);

		std::uninitialized_copy(rhs.begin(), rhs.end(), newArray);
		////vs
		//for (size_t i = 0; i < mSize; i++) {
		//	new (newArray + i) T(rhs.array[i]);
		//}
		array = newArray;
	}
	*/
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
	pointer eraseImpl(pointer pos) {
		assert(pos >= array && pos < array + mSize);
		std::move(pos + 1, raw_end(), pos);//shift all elements
		array[mSize - 1].~T();
		--mSize;
		
		return (pos == raw_end()) ? raw_end() : pos;
	}
	pointer eraseRangeImpl(pointer first, pointer last) {
		assert(first >= array && first <= last && last <= (array + mSize));

		if (first == last) 
			return first;
		
		pointer end = raw_end();

		if (last == end) {
			std::destroy(first, end);
			mSize = first - array;
			return raw_end();
		}
		//else
		std::move(last, end, first);//shift all elements

		pointer destroyBegin = first + (end - last);
		std::destroy(destroyBegin, end);
		mSize -= (end - destroyBegin);

		//for (T* i = destroyBegin; i != end; ++i) {
		//	i->~T();
		//	--mSize;
		//}
		return first;
	}
	pointer raw_begin() {
		return array;
	}
	pointer raw_end() {
		return array + mSize;
	}

public:

	Sequence() :array(nullptr), mSize(0), cap(0) {}
	Sequence(const Sequence& rhs) {
		pointer newArray = memAlloc(rhs.cap);
		std::uninitialized_copy(rhs.begin(), rhs.end(), newArray);
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
		std::uninitialized_copy(rhs.begin(), rhs.end(), newArray);
		mSize = rhs.mSize;
		cap = rhs.cap;
		array = newArray;
		//copyFormDirty(rhs);
		return *this;
	}


	~Sequence() {
		clear();
		memFree();
	}

	bool isEmpty()const {
		return mSize == 0;
	}
	size_type size()const {
		return mSize;
	}
	size_type capacity()const {
		return cap;
	}
	void add(const_reference value) {
		if (mSize == cap)
			grow();
		std::construct_at(raw_end(), value);
		//new(array + mSize) T(value);//beginning of the sequence+ptr arithmetic index, place the value there
		++mSize;
	}
	void add(T&& value) {
		if (mSize == cap)
			grow();
		std::construct_at(raw_end(), std::move(value));
		//new(array + mSize) T(std::move(value));//beginning of the sequence+ptr arithmetic index, place the value there
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
		//for (size_t i = mSize; i-- > 0;) {
		//	array[i].~T();
		//}
		mSize = 0;
	}

	reference operator[](size_type index) {
		assert(index < mSize);
		return array[index];
	}
	const_reference operator[](size_type index)const {
		assert(index < mSize);
		return array[index];
	}

	void remove(const_iterator pos) {
		assert(pos >= begin() && pos < end());
		pointer dest = const_cast<pointer>(pos.ptr);
		std::destroy_at(dest);
		std::construct_at(dest, std::move(array[mSize - 1]));
		std::destroy_at(raw_end() - 1);
		//*dest = std::move(array[mSize - 1]);
		//*pos = std::move(array[mSize - 1]);
		//array[mSize - 1].~T();
		--mSize;
	}
	void remove(iterator pos) {
		remove(const_iterator(pos));
	}
	template <typename Pred>
	void remove(Pred predicate) {
		//LEFT OFF HERE FOR TESTING, BUT THIS IS SHIT ALREADY, REMOVE_IF DOES NOT DO WHAT I WANT IT
		auto it = std::remove_if(begin(), end(), predicate);
		erase(it, end());
	}

	const_iterator erase(const_iterator pos) {
		return const_iterator(eraseImpl(const_cast<pointer>(pos.ptr)));
	}
	iterator erase(iterator pos) {
		return iterator(eraseImpl(pos.ptr));
	}
	iterator erase(const_iterator first, const_iterator last) {
		return const_iterator(eraseRangeImpl(const_cast<pointer>(first.ptr), const_cast<pointer>(last.ptr)));
	}
	iterator erase(iterator first, iterator last) {
		return iterator(eraseRangeImpl(first.ptr, last.ptr));
	}


	/*
	TODO
	resize
	reserve
	rule of 5 constructor + initalizer list
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