/*
* EXCEPTIONS::
*	copyAlloc and moveAlloc provide strong guarantees existing memory does not get fucked over
*	however if an allocation fails
* 
*/

#pragma once

#include <stdexcept>
#include <algorithm>
#include <memory>
#include <concepts>
#include <type_traits>

/*
IMPORTANT!!!
			eraseImpl, eraseRangeImpl, remove and removeImpl all use std::move which can fail
			consider how to communnicate failure (currently moveAlloc handles but fails silently)
*/
namespace seq {

	template <typename T>
	class Sequence {
	private:
		static_assert(std::is_object_v<T>, "T must be an object type");
		static_assert(std::destructible<T>, "T must be destructible");
		static_assert(!std::is_const_v<T>, "T cannot be const type");
		//forward declares
		class Iterator;
		class Const_Iterator;
	public:
		//type names
		using type = Sequence<T>;
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		using iterator = Iterator;
		using const_iterator = Const_Iterator;

	private:
		
		class CopyPredicate {
			const_pointer  srcBegin = nullptr;
			const_pointer  srcEnd = nullptr;
		public:
			CopyPredicate(const_pointer srcBegin, const_pointer srcEnd) :srcBegin(srcBegin), srcEnd(srcEnd) {}
			pointer operator()(pointer dest) {
				return std::uninitialized_copy(srcBegin, srcEnd, dest);
			}
		};
		class MovePredicate {
			const_pointer  srcBegin = nullptr;
			const_pointer  srcEnd = nullptr;
		public:
			MovePredicate(const_pointer srcBegin, const_pointer srcEnd) :srcBegin(srcBegin), srcEnd(srcEnd) {}
			pointer operator()(pointer dest) {
				return std::uninitialized_move(srcBegin, srcEnd, dest);
			}
		};
		/*
		* DEPRICATED, ALWAYS USE FILL INSTEAD, FUCK IT
		 
		class ValueConstructPredicate {
			size_type _count = 0;
		public:
			ValueConstructPredicate(size_type _count) :_count(_count) {}
			pointer operator()(pointer dest) {
				return std::uninitialized_value_construct_n(dest, _count);
			}
		};
		*/
		class FillConstructNPredicate {
			size_type _count = 0;
			const_reference _value;
		public:
			FillConstructNPredicate(size_type _count, const_reference _value) :_count(_count), _value(_value) {}
			pointer operator()(pointer dest) {
				return std::uninitialized_fill_n(dest, _count, _value);
			}
		};

		class Allocator {
			pointer memBegin = nullptr;
			pointer memInitCount = nullptr;

			void destroy() {
				if (memInitCount) {//implies memBegin is also non nullptr
					std::destroy(memBegin, memInitCount);
					memInitCount = nullptr;
				}
			}
			void dealloc() {
				if (memBegin) {
					::operator delete(memBegin);
					memBegin = nullptr;
				}
			}
		public:
			Allocator() = default;

			bool memAlloc(size_type count) {
				try {
					void* bytes = ::operator new(count * sizeof(value_type));
					memBegin = static_cast<pointer>(bytes);
				}
				catch (std::bad_alloc& badAlloc) {
					return false;//destructor cleans it up
				}
				return true;
			}
			template<typename Pred>
			bool memInit(Pred predicate) {
				try {
					memInitCount = predicate(memBegin);
				}
				catch (...) {
					return false;//destructor cleans it up
				}
				return true;
			}
			pointer release()noexcept {
				pointer temp = memBegin;
				memBegin = nullptr;
				memInitCount = nullptr;
				return temp;
			}
			~Allocator() {
				destroy();
				dealloc();
			}
		};
		void copyMoveAlloc(const_pointer begin, const_pointer end, size_type elemCount, size_type capCount, bool isMove) {
			Allocator newArray;
			
			if (!newArray.memAlloc(capCount)) {
				throw std::bad_alloc();
			}

			bool initOK = false;
			if (isMove && std::is_nothrow_move_constructible_v<T>) {
				initOK = newArray.memInit(MovePredicate{ begin, end });
			}
			else {
				initOK = newArray.memInit(CopyPredicate{ begin, end });
			}

			if (!initOK) {
				if (isMove) {
					throw std::runtime_error("failed to move elements");
				}
				else {
					throw std::runtime_error("failed to copy elements");
				}
			}

			//if we survived to this point
			clear();//no op if already empty
			memFree();//no op if already nullptr
			array = newArray.release();
			cap = capCount;
			mSize = elemCount;
		}
		void defaultAlloc(size_type elemCount, size_type capCount, const_reference value) {
			Allocator newArray;

			if (!newArray.memAlloc(capCount)) {
				throw std::bad_alloc();
			}
			bool initOK = newArray.memInit(FillConstructNPredicate{ elemCount, value });
			if (!initOK) {
				throw std::runtime_error("failed to default construct elements T");
			}
			//if we survived to this point
			clear();//no op if already empty
			memFree();//no op if already nullptr
			array = newArray.release();
			cap = capCount;
			mSize = elemCount; 
		}

		//private members
		void memFree() {
			if (array) {
				::operator delete(array, cap * sizeof(value_type));
				array = nullptr;
				cap = 0;
			}
		}
		constexpr pointer raw_begin()const {
			return array;
		}
		constexpr pointer raw_end()const {
			return array + mSize;
		}
		constexpr size_type growthFactor(size_type input)const {
			return input + (input / 2) + 1;
		}
	public:
		//CONSTRUCTORS
		Sequence()noexcept :array(nullptr), mSize(0), cap(0) {}
		Sequence(std::initializer_list<value_type> init) requires std::copyable<T> {
			if (init.size() > 0) {
				copyMoveAlloc(init.begin(), init.end(), init.size(), init.size(), false);
			}
		}
		explicit Sequence(size_type count) requires std::default_initializable<T>  {
			if (count > 0) {
				T _defaultT;
				defaultAlloc(count, count, _defaultT);
			}
		}
		Sequence(size_type count, const_reference value) requires std::copyable<T> {
			if (count > 0) {
				defaultAlloc(count, count, value);
			}
		}
		Sequence(const Sequence& rhs) requires std::copyable<T> {
			if (rhs.isValid()) {//if it is empty, copyMoveAlloc only assigns raw mem + it handles clearing/freeing current
				copyMoveAlloc(rhs.raw_begin(), rhs.raw_end(), rhs.size(), rhs.capacity(), false);
			}
		}
		constexpr Sequence(Sequence&& rhs)noexcept {
			array = std::exchange(rhs.array, nullptr);
			mSize = std::exchange(rhs.mSize, 0);
			cap = std::exchange(rhs.cap, 0);
		}
		Sequence& operator=(const Sequence& rhs)requires std::copyable<T> {
			if (this != &rhs) {

				if (rhs.isValid()) {//if it is empty, copyMoveAlloc only assigns raw mem + it handles clearing/freeing current
					copyMoveAlloc(rhs.raw_begin(), rhs.raw_end(), rhs.size(), rhs.capacity(), false);//
				}
				else {
					clear();
					memFree();
				}
			}
			return *this;
		}
		Sequence& operator=(Sequence&& rhs)noexcept {
			if (this != &rhs) {
				clear();
				memFree();

				array = std::exchange(rhs.array, nullptr);
				mSize = std::exchange(rhs.mSize, 0);
				cap = std::exchange(rhs.cap, 0);
			}
			return *this;
		}
		Sequence& operator=(std::initializer_list<value_type> ilist) requires std::copyable<T>{
			if (ilist.size() > 0) {
				copyMoveAlloc(ilist.begin(), ilist.end(), ilist.size(), ilist.size(), false);
			}
			else {
				clear();
				memFree();
			}
			return *this;
		}

		~Sequence() {//putting a requirement here will cause a misleading error, though would be more correct
			clear();
			memFree();
		}
		//#################################################

		//ACCESS
		constexpr reference operator[](size_type index) {
			return array[index];
		}
		constexpr const_reference operator[](size_type index)const {
			return array[index];
		}
		constexpr reference at(size_type pos) {
			if (pos >= size()) {
				throw std::out_of_range("position out of range");
			}
			return array[pos];
		}
		constexpr const_reference at(size_type pos)const {
			if (pos >= size()) {
				throw std::out_of_range("position out of range");
			}
			return array[pos];
		}
		constexpr pointer data()noexcept {
			return array;
		}
		constexpr const_pointer data()const noexcept {
			return array;
		}
		constexpr iterator       begin()noexcept        { return { array }; }
		constexpr iterator       end()noexcept          { return { array + mSize }; }
		constexpr const_iterator begin()const noexcept  { return { array }; }
		constexpr const_iterator end()const noexcept    { return { array + mSize }; }
		constexpr const_iterator cbegin()const noexcept { return { array }; }
		constexpr const_iterator cend()const noexcept   { return { array + mSize }; }
		//#################################################

		//MODIFICATION
		void push_back(const_reference value) requires std::copyable<T> {
			if (mSize == cap)
				copyMoveAlloc(raw_begin(), raw_end(), size(), growthFactor(cap), true);
			std::construct_at(raw_end(), value);//but this can and will fail as well if above failed because the capacity is not enough
			++mSize;
		}
		void push_back(T&& value) requires std::movable<T> {
			if (mSize == cap)
				copyMoveAlloc(raw_begin(), raw_end(), size(), growthFactor(cap), true);
			std::construct_at(raw_end(), std::move(value));
			++mSize;
		}
		void emplace_back(T&& value) requires std::movable<T>{
			push_back(std::move(value));
		}
		template<typename... Args>
		void emplace_back(Args&&... args) requires std::constructible_from<T, Args&&...>{
			if (mSize == cap)
				copyMoveAlloc(raw_begin(), raw_end(), size(), growthFactor(cap), true);
			std::construct_at(raw_end(), std::forward<Args>(args)...);
			++mSize;
		}
		void pop_back()noexcept(std::is_nothrow_destructible_v<T>) {
			if (mSize > 0) {
				array[mSize - 1].~T();
				--mSize;
			}
		}
		void clear() noexcept(std::is_nothrow_destructible_v<T>) {
			if (mSize > 0) {
				std::destroy(raw_begin(), raw_end());
				mSize = 0;
			}
		}
		void resize(size_type count) {
			if (count == mSize) {
				return;
			}
			//if count is less than size, cut off the tail
			if (count < mSize) {
				std::destroy(raw_begin() + count, raw_end());
				mSize = count;
				return;
			}
			// if T is default constructible, may need moveAlloc in which case need movable
			if constexpr (std::default_initializable<T>) {
				if (count > cap) {
					static_assert(std::move_constructible<T>, "T must be move constructible");
					asscockerror(raw_begin(), raw_end(), count);
				}
				std::uninitialized_default_construct(raw_end(), raw_begin() + count);
				mSize = count;
			}
			else {
				static_assert(std::default_initializable<T>, "T must be default initializable");
			}
		}
		iterator erase(const_iterator pos)requires std::movable<T> {
			if (pos < begin() || pos >= end()) {
				throw std::out_of_range("position out of range");
			}
			pointer pPos = const_cast<pointer>(pos.base());
			std::move(pPos + 1, raw_end(), pPos);//move all elements 1 space
			array[mSize - 1].~T();//end is dangling, delete it
			--mSize;
			
			return iterator((pPos == raw_end()) ? raw_end() : pPos);
		}
		iterator erase(iterator pos)requires std::movable<T> {
			return erase(const_iterator(pos));
		}
		iterator erase(const_iterator first, const_iterator last)requires std::movable<T> {
			if (first < begin() || first >= last || last >= end()) {
				throw std::out_of_range("position out of range");
			}
			pointer pfirst = const_cast<pointer>(first.base());
			pointer plast = const_cast<pointer>(last.base());
			if (pfirst == plast)//if first and last is same element, then no op
				return iterator(pfirst);

			pointer endPtr = raw_end();
			if (plast == endPtr) {//if last is one off the end, then delete start to end
				std::destroy(pfirst, endPtr);
				mSize = pfirst - raw_begin();
				return iterator(raw_end());
			}
			//else if last is not one off the end
			std::move(plast, endPtr, pfirst);//move all elements
			//math: size 25: destroy elem 5 to 10:: 5+(25-10) = 20, destroy 20 to 25. then mSize-=(25-20), reduce by 5
			pointer destroyBegin = pfirst + (endPtr - plast);
			std::destroy(destroyBegin, endPtr);//remove the dangling tail end
			mSize -= (endPtr - destroyBegin);
			
			return iterator((pfirst == raw_end()) ? raw_end() : pfirst);
		}
		iterator erase(iterator first, iterator last)requires std::movable<T> {
			return erase(const_iterator(first), const_iterator(last));
		}
		iterator remove(const_iterator pos)requires std::movable<T> {
			if (pos < begin() || pos >= end()) {
				throw std::out_of_range("position out of range");
			}
			pointer dest = const_cast<pointer>(pos.base());
			pointer last = raw_end() - 1;

			if (dest != last) //only move if they're not the same elem
				*dest = std::move(*last);//switch places with the last element, don't shift everything
			std::destroy_at(last);//delete the end

			--mSize;

			return iterator(dest);
		}
		iterator remove(iterator pos)requires std::movable<T> {
			return remove(const_iterator(pos));
		}
		template <typename UnaryPred>
		iterator remove(UnaryPred predicate) requires std::movable<T> {
			pointer last = raw_end() - 1;
			pointer current = raw_begin();

			//if predicate is true, move last element to current then move the last pointer
			while (current <= last) {
				if (predicate(*current)) {
					if (current == last) {
						--last;
						break;
					}
					else {
						*current = std::move(*last);
						--last;
					}
				}
				else {
					++current;
				}
			}
			size_type newSize = last - raw_begin() + 1;//last began at last element, not one off the last, so here add it back

			//bulk delete everything from last to end then apply new size
			if (newSize < mSize) {
				std::destroy(last + 1, raw_end());
				mSize = newSize;
			}

			return end();
		}

		constexpr void swap(Sequence& rhs)noexcept {
			pointer tempArr = array;
			array = rhs.array;
			rhs.array = tempArr;

			size_type tempSize = mSize;
			mSize = rhs.mSize;
			rhs.mSize = tempSize;

			size_type tempCap = cap;
			cap = rhs.cap;
			rhs.cap = tempCap;
		}
		//#################################################


		//CAPACITY
		constexpr bool      isEmpty()const noexcept{ return mSize == 0; }
		constexpr bool      isValid()const noexcept { return array; }
		constexpr size_type size()const noexcept{ return mSize; }
		constexpr size_type capacity()const noexcept{ return cap; }

		void reserve(size_type newCap) requires std::movable<T> {
			if (newCap > cap) {
				copyMoveAlloc(raw_begin(), raw_end(), size(), newCap, true);
			}
		}
		void shrinkToFit() requires std::movable<T> {
			if (cap > mSize) {//moveAlloc sets the cap only, so if this function is called twice in a raw, just skip it entirely, otherwise if it grows in between the situation changed
				copyMoveAlloc(raw_begin(), raw_end(), size(), size(), true);
			}
		}
		//#################################################
	private:
		//members
		pointer array = nullptr;
		size_type mSize = 0;
		size_type cap = 0;
	};

	template <typename SequenceType>
	constexpr bool operator==(const Sequence<SequenceType>& lhs, const Sequence<SequenceType>& rhs) {
		if (lhs.size != rhs.size) {
			return false;
		}
		return std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}
	template <typename SequenceType>
	constexpr bool operator!=(const Sequence<SequenceType>& lhs, const Sequence<SequenceType>& rhs) {
		return !(lhs == rhs);
	}
	template <typename SequenceType>
	constexpr bool operator<(const Sequence<SequenceType>& lhs, const Sequence<SequenceType>& rhs) {
		return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}
	template <typename SequenceType>
	constexpr bool operator>(const Sequence<SequenceType>& lhs, const Sequence<SequenceType>& rhs) {
		return rhs < lhs;
	}
	template <typename SequenceType>
	constexpr bool operator<=(const Sequence<SequenceType>& lhs, const Sequence<SequenceType>& rhs) {
		return !(rhs < lhs);
	}
	template <typename SequenceType>
	constexpr bool operator>=(const Sequence<SequenceType>& lhs, const Sequence<SequenceType>& rhs) {
		return !(lhs < rhs);
	}

	template<typename SequenceType>
	constexpr void swap(Sequence<SequenceType>& lhs, Sequence<SequenceType>& rhs)noexcept {
		lhs.swap(rhs);
	}

	template<typename T>
	class Sequence<T>::Iterator {
	public:
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using self_type = Iterator;

		constexpr reference operator*()noexcept { return *ptr; }
		constexpr pointer operator->()noexcept { return ptr; }
		constexpr reference operator[](difference_type n)noexcept { return ptr[n]; }
		constexpr const reference operator*() const noexcept { return *ptr; }
		constexpr const pointer operator->() const noexcept { return ptr; }
		constexpr const reference operator[](difference_type n) const noexcept { return ptr[n]; }

		constexpr self_type& operator++()noexcept { ++ptr; return *this; }
		constexpr self_type operator++(int)noexcept { self_type temp = *this; ++ptr; return temp; }
		constexpr self_type& operator--()noexcept { --ptr; return *this; }
		constexpr self_type operator--(int)noexcept { self_type temp = *this; --ptr; return temp; }

		constexpr self_type& operator+=(difference_type n)noexcept { ptr += n; return *this; }
		constexpr self_type& operator-=(difference_type n)noexcept { ptr -= n; return *this; }
		constexpr self_type operator+(difference_type n)const noexcept { return self_type(ptr + n); }
		constexpr self_type operator-(difference_type n)const noexcept { return self_type(ptr - n); }
		constexpr difference_type operator-(const self_type& rhs)const noexcept { return ptr - rhs.ptr; }

		constexpr bool operator==(const self_type& rhs)const noexcept { return ptr == rhs.ptr; }
		constexpr std::strong_ordering operator<=>(const self_type& rhs)const noexcept { return ptr <=> rhs.ptr; }

		constexpr Iterator(pointer p) noexcept :ptr(p) {}
		constexpr pointer base()const noexcept { return ptr; }
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

		constexpr reference operator*()const noexcept { return *ptr; }
		constexpr pointer operator->()const noexcept { return ptr; }
		constexpr reference operator[](difference_type n)const noexcept { return ptr[n]; }

		constexpr self_type& operator++() noexcept { ++ptr; return *this; }
		constexpr self_type operator++(int) noexcept { self_type temp = *this; ++ptr; return temp; }
		constexpr self_type& operator--() noexcept { --ptr; return *this; }
		constexpr self_type operator--(int) noexcept { self_type temp = *this; --ptr; return temp; }

		constexpr self_type& operator+=(difference_type n) noexcept { ptr += n; return *this; }
		constexpr self_type& operator-=(difference_type n) noexcept { ptr -= n; return *this; }
		constexpr self_type operator+(difference_type n)const noexcept { return self_type(ptr + n); }
		constexpr self_type operator-(difference_type n)const noexcept { return self_type(ptr - n); }
		constexpr difference_type operator-(const self_type& rhs)const noexcept { return ptr - rhs.ptr; }

		constexpr bool operator==(const self_type& rhs)const noexcept { return ptr == rhs.ptr; }
		constexpr std::strong_ordering operator<=>(const self_type& rhs)const noexcept { return ptr <=> rhs.ptr; }

		constexpr Const_Iterator(pointer p)noexcept :ptr(p) {}
		constexpr Const_Iterator(const Iterator& rp) noexcept :ptr(rp.base()) {}
		constexpr pointer base()const noexcept { return ptr; }
	private:
		pointer ptr = nullptr;
	};
}
