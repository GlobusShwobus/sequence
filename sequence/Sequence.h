#pragma once

#include <assert.h>
#include <stdexcept>
#include <memory>
#include <concepts>

namespace seq {
	template <typename T>
	concept strong_movable = std::movable<T>&& std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_move_assignable_v<T>;

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
		//#################################################

	private:
		//the important shit
		void moveAlloc(size_type count) {
			pointer moved = static_cast<pointer>(::operator new(count * sizeof(value_type)));
			pointer begin = raw_begin();
			pointer end = raw_end();
			pointer initialized = moved;
			
			try {
				if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
					initialized = std::uninitialized_move(begin, end, moved);
				}
				else {
					initialized = std::uninitialized_copy(begin, end, moved);
				}
			}
			catch (...) {
				std::destroy(moved, initialized);
				::operator delete(moved);
				throw;
			}
			
			std::destroy(begin, end);
			::operator delete(array);

			array = moved;
			cap = count;
			//mSize unchanged
		}
		template <typename Construct>
		void tryElemConstructAlloc(size_type count, Construct construct) {
			pointer tempMem = static_cast<pointer>(::operator new(count * sizeof(value_type)));
			pointer initalizedTail = tempMem;

			try {
				initalizedTail = construct(tempMem, count);
			}
			catch (...) {
				std::destroy(tempMem, initalizedTail);
				::operator delete(tempMem);
				throw;
			}
			array = std::exchange(tempMem, nullptr);
			mSize = count;
			cap = count;
		}
		//#################################################
		
		//the convenience
		void memFree()noexcept {
			if (array) {
				::operator delete(array);
				array = nullptr;
				cap = 0;
			}
		}
		void objDestroyAll()noexcept { 
			if (mSize > 0) {
				std::destroy(raw_begin(), raw_end());
				mSize = 0;
			}
		}
		constexpr pointer raw_begin()const                     { return array; }
		constexpr pointer raw_end()const                       { return array + mSize; }
		constexpr size_type growthFactor(size_type input)const { return input + (input / 2) + 1; }
		//#################################################
	public:
		//CONSTRUCTORS
		constexpr Sequence()noexcept = default;
		constexpr Sequence(size_type count) requires std::default_initializable<value_type> {
			if (count == 0) return;
			tryElemConstructAlloc(count, [](pointer p, size_type n) {
				return std::uninitialized_default_construct_n(p, n);
				}
			);
		}
		constexpr Sequence(size_type count, const_reference value) requires std::copyable<value_type> {
			if (count == 0)
				return;
			tryElemConstructAlloc(count, [&value](pointer p, size_type n) {
				return std::uninitialized_fill_n(p, n, value);
				}
			);
		}
		constexpr Sequence(std::initializer_list<value_type> init) requires std::copyable<value_type> {
			if (init.size() == 0)
				return;
			tryElemConstructAlloc(init.size(), [init](pointer p, size_type n) {
				return std::uninitialized_copy(init.begin(), init.end(), p);
				}
			);
		}
		constexpr Sequence(const Sequence& rhs) requires std::copyable<value_type> {
			if (!rhs.isValid())
				return;
			tryElemConstructAlloc(rhs.size(), [&rhs](pointer p, size_type n) {
				return std::uninitialized_copy(rhs.raw_begin(), rhs.raw_end(), p);
				}
			);
		}
		constexpr Sequence(Sequence&& rhs)noexcept {//shallow copy theft, no need for requirements
			array = std::exchange(rhs.array, nullptr);
			mSize = std::exchange(rhs.mSize, 0);
			cap = std::exchange(rhs.cap, 0);
		}
		constexpr Sequence& operator=(Sequence rhs)noexcept {//AND THE LORD SAID LET THERE BE LIGHT
			rhs.swap(*this);
			return *this;
		}
		constexpr Sequence& operator=(std::initializer_list<value_type> ilist) requires std::copyable<value_type>{
			if (ilist.size() == 0) {
				objDestroyAll();
				//memFree(); for the sake of future allocation, just don't free mem
				return *this;
			}
			Sequence temp(ilist);
			temp.swap(*this);
			return *this;
		}
		constexpr ~Sequence()noexcept {//putting a requirement here will cause a misleading error, noexcept is implict anyway but fuck it
			objDestroyAll();
			memFree();
		}
		//#################################################

		//ACCESS
		constexpr reference operator[](size_type index) {
			assert(index < mSize && "out of range position");
			return array[index];
		}
		constexpr const_reference operator[](size_type index)const {
			assert(index < mSize && "out of range position");
			return array[index]; 
		}
		constexpr reference at(size_type pos) {
			if (pos >= size())
				throw std::out_of_range("position out of range");
			return array[pos];
		}
		constexpr const_reference at(size_type pos)const {
			if (pos >= size())
				throw std::out_of_range("position out of range");
			return array[pos];
		}
		constexpr pointer        data()noexcept           { return   array; }
		constexpr const_pointer  data()const noexcept     { return   array; }

		constexpr iterator       begin()                  { return { array }; }
		constexpr iterator       end()                    { return { array + mSize }; }
		constexpr const_iterator begin()const             { return { array };  }
		constexpr const_iterator end()const               { return { array + mSize };  }
		constexpr const_iterator cbegin()const            { return { array };  }
		constexpr const_iterator cend()const              { return { array + mSize }; }
		//#################################################

		//MODIFICATION
		constexpr void push_back(const_reference value) requires std::copyable<value_type> {
			if (mSize == cap)
				moveAlloc(growthFactor(cap));
			std::construct_at(raw_end(), value);//safe to throw on fail, array not modified
			++mSize;
		}
		constexpr void push_back(T&& value) requires strong_movable<value_type> {
			if (mSize == cap)
				moveAlloc(growthFactor(cap));
			std::construct_at(raw_end(), std::move(value));//safe to throw on fail, array not modified
			++mSize;
		}
		constexpr void emplace_back(T&& value) requires strong_movable<value_type> {
			push_back(std::move(value));
		}
		template<typename... Args> constexpr void emplace_back(Args&&... args) requires std::constructible_from<value_type, Args&&...>{
			if (mSize == cap)
				moveAlloc(growthFactor(cap));
			std::construct_at(raw_end(), std::forward<Args>(args)...);//safe to throw on fail, array not modified
			++mSize;
		}
		void pop_back()noexcept {
			if (mSize > 0) {
				array[mSize - 1].~value_type();
				--mSize;
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
			if constexpr (std::default_initializable<value_type>) {
				if (count > cap) {
					static_assert(std::move_constructible<value_type>, "T must be move constructible");
					moveAlloc(growthFactor(count));
				}

				size_type diff = count - mSize;
				pointer newElemStart = raw_end();
				pointer newElemEnd = newElemStart;

				try {
					newElemEnd = std::uninitialized_default_construct_n(newElemStart, diff);
				}
				catch (...) {
					std::destroy(newElemStart, newElemEnd);
					throw;
				}
				mSize = count;
			}
			else {
				static_assert(std::default_initializable<value_type>, "T must be default initializable");
			}
		}
		iterator erase(iterator& pos)requires strong_movable<value_type> {
			if (pos < begin() || pos >= end()) {
				throw std::out_of_range("position out of range");
			}

			pointer pBase = pos.base();
			std::move(pBase + 1, raw_end(), pBase);//move all elements 1 space
			array[mSize - 1].~value_type();//end is dangling, delete it
			--mSize;

			return (pBase == raw_end()) ? raw_end() : pBase;
		}
		iterator erase(iterator first, iterator last)requires strong_movable<value_type> {
			if (first < begin() || first >= last || last >= end()) {
				throw std::out_of_range("position out of range");
			}
			pointer pfirst = first.base();
			pointer plast = last.base();

			if (pfirst == plast)//if first and last is same element, then no op
				return pfirst;

			pointer endPtr = raw_end();
			if (plast == endPtr) {//if last is one off the end, then delete start to end
				std::destroy(pfirst, endPtr);
				mSize = pfirst - raw_begin();
				return raw_end();
			}
			//else if last is not one off the end
			std::move(plast, endPtr, pfirst);//move all elements
			//math: size 25: destroy elem 5 to 10:: 5+(25-10) = 20, destroy 20 to 25. then mSize-=(25-20), reduce by 5
			pointer destroyBegin = pfirst + (endPtr - plast);
			std::destroy(destroyBegin, endPtr);//remove the dangling tail end
			mSize -= (endPtr - destroyBegin);

			return (pfirst == raw_end()) ? raw_end() : pfirst;
		}
		iterator remove(iterator pos)requires strong_movable<value_type> {
			if (pos < begin() || pos >= end()) {
				throw std::out_of_range("position out of range");
			}
			pointer dest = pos.base();
			pointer last = raw_end() - 1;

			if (dest != last) //only move if they're not the same elem
				*dest = std::move(*last);//switch places with the last element, don't shift everything
			std::destroy_at(last);//delete the end
			--mSize;

			return dest;
		}
		template <typename UnaryPred>
		iterator remove(UnaryPred predicate) requires strong_movable<value_type> {
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
		constexpr bool      isEmpty()const noexcept  { return mSize == 0; }
		constexpr bool      isValid()const noexcept  { return array; }
		constexpr size_type size()const noexcept     { return mSize; }
		constexpr size_type capacity()const noexcept { return cap; }

		void reserve(size_type newCap) requires std::movable<value_type> {
			if (newCap > cap) {
				moveAlloc(newCap);
			}
		}
		void shrinkToFit() requires std::movable<value_type> {
			if (cap > mSize) {
				moveAlloc(size());
			}
		}
		void clear() noexcept {
			objDestroyAll();
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

		constexpr Iterator(pointer p) :ptr(p) { assert(p != nullptr && "Iterator constructed from nullptr!"); }
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

		constexpr Const_Iterator(pointer p) :ptr(p) { assert(ptr != nullptr && "Iterator constructed from nullptr!");  }
		constexpr Const_Iterator(const Iterator& rp) :ptr(rp.base()) { assert(ptr != nullptr && "Iterator constructed from nullptr!"); }
		constexpr pointer base()const noexcept { return ptr; }
	private:
		pointer ptr = nullptr;
	};
}
