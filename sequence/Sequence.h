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

		static constexpr std::size_t first_index = 0ull;

		static constexpr std::size_t first_index = 0;
	public:
		//type names
		using type            = Sequence<T>;
		using value_type      = T;
		using pointer         = T*;
		using const_pointer   = const T*;
		using reference       = T&;
		using const_reference = const T&;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		using iterator        = Iterator;
		using const_iterator  = Const_Iterator;
		//#################################################

	private:
		//the important shit
		void tryReAllocate(size_type count) {
			pointer moved = static_cast<pointer>(::operator new(count * sizeof(value_type)));
			pointer initialized = moved;
			pointer begin = raw_begin();
			pointer end = raw_end();
			
			try {
				if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
					initialized = std::uninitialized_move(begin, end, moved);
				}
				else {
					initialized = std::uninitialized_copy(begin, end, moved);
				}
			}
			catch (...) {
				//this case should never happen but will only happen if T constructors themselves are faulty.
				//i can't save you if your move semantis are declared noexcept but do actually hick up. in which case nothing is indeed thrown. the whole thing gets shut down
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
				array = std::exchange(tempMem, nullptr);
				mSize = count;
				cap = count;
			}
			catch (...) {
				std::destroy(tempMem, initalizedTail);
				::operator delete(tempMem);
				throw;
			}
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
		Sequence(size_type count) requires std::default_initializable<value_type> {
			if (count == 0) return;
			tryElemConstructAlloc(count, [](pointer p, size_type n) {
				return std::uninitialized_default_construct_n(p, n);
				}
			);
		}
		Sequence(size_type count, const_reference value) requires std::copyable<value_type> {
			if (count == 0)
				return;
			tryElemConstructAlloc(count, [&value](pointer p, size_type n) {
				return std::uninitialized_fill_n(p, n, value);
				}
			);
		}
		Sequence(std::initializer_list<value_type> init) requires std::copyable<value_type> {
			if (init.size() == 0)
				return;
			tryElemConstructAlloc(init.size(), [init](pointer p, size_type n) {
				return std::uninitialized_copy(init.begin(), init.end(), p);
				}
			);
		}
		Sequence(const Sequence<value_type>& rhs) requires std::copyable<value_type> {
			if (!rhs.isValid())
				return;
			tryElemConstructAlloc(rhs.size(), [&rhs](pointer p, size_type n) {
				return std::uninitialized_copy(rhs.raw_begin(), rhs.raw_end(), p);
				}
			);
		}
		constexpr Sequence(Sequence<value_type>&& rhs)noexcept {//shallow copy theft, no need for requirements
			array = std::exchange(rhs.array, nullptr);
			mSize = std::exchange(rhs.mSize, 0);
			cap = std::exchange(rhs.cap, 0);
		}
		Sequence& operator=(Sequence<value_type> rhs)noexcept {//AND THE LORD SAID LET THERE BE LIGHT
			rhs.swap(*this);
			return *this;
		}
		Sequence& operator=(std::initializer_list<value_type> ilist) requires std::copyable<value_type>{
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
		constexpr reference front() {
			assert(mSize > first_index);
			return array[first_index];
		}
		constexpr const_reference front()const {
			assert(mSize > first_index);
			return array[first_index];
		}
		constexpr reference back() {
			assert(mSize > first_index);
			return array[mSize - 1];
		}
		constexpr const_reference back()const {
			assert(mSize > first_index);
			return array[mSize - 1];
		}
		constexpr reference operator[](size_type index) {
			assert(index < mSize);
			return array[index];
		}
		constexpr const_reference operator[](size_type index)const {
			assert(index < mSize);
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
		template <typename U>
		void push_back(U&& value) requires std::convertible_to<U, value_type>&& std::constructible_from<value_type, U&&> {//implicit copy or move requirement
			if (mSize == cap)
				tryReAllocate(growthFactor(cap));
			std::construct_at(raw_end(), std::forward<U>(value));
			++mSize;
		}
		template<typename... Args> void emplace_back(Args&&... args) requires std::constructible_from<value_type, Args&&...>{
			if (mSize == cap)
				tryReAllocate(growthFactor(cap));
			std::construct_at(raw_end(), std::forward<Args>(args)...);//safe to throw on fail, array not modified
			++mSize;
		}
		void pop_back()noexcept {
			if (mSize > 0) {
				std::destroy_at(raw_end() - 1);
				--mSize;
			}
		}
		iterator erase(iterator pos)requires strong_movable<value_type> {
			pointer eraseElem = pos.base();
			pointer arrayBegin = raw_begin();
			pointer arrayEnd = raw_end();
			if (eraseElem < arrayBegin || eraseElem > arrayEnd) throw std::out_of_range("position out of range");
			if (eraseElem == arrayEnd) return arrayEnd;

			std::move(eraseElem + 1, arrayEnd, eraseElem);//from -> till -> destination == eraseElem gets overwriten then destroyed
			--arrayEnd;//last valid element
			std::destroy_at(arrayEnd);//destroy the last valid element, oldEnd is now one off the end
			--mSize;
			
			return (eraseElem == arrayEnd) ? arrayEnd : eraseElem;
		}
		iterator erase(iterator first, iterator last)requires strong_movable<value_type> {
			pointer eraseFirst = first.base();
			pointer eraseLast = last.base();
			pointer arrayBegin = raw_begin();
			pointer arrayEnd = raw_end();

			if (eraseFirst < arrayBegin || eraseLast > arrayEnd) throw std::out_of_range("position out of range");
			if (eraseFirst == eraseLast) return eraseLast;//if first and last are same element, then return

			if (eraseLast == arrayEnd) {//if last is end, then it means we can simply erase the tail end
				std::destroy(eraseFirst, eraseLast);
				mSize = eraseFirst - arrayBegin;
				return raw_end();
			}
			//else
			std::move(eraseLast, arrayEnd, eraseFirst);
			//math: size 25: destroy elem 5 to 10:: 5+(25-10) = 20, destroy 20 to 25. then mSize-=(25-20), reduce by 5
			pointer destroyBegin = eraseFirst + (arrayEnd - eraseLast);
			std::destroy(destroyBegin, arrayEnd);//remove the dangling tail end
			mSize -= (arrayEnd - destroyBegin);

			return eraseFirst;
		}
		iterator remove(iterator pos)requires strong_movable<value_type> {
			pointer removeElem = pos.base();
			pointer arrayBegin = raw_begin();
			pointer arrayEnd = raw_end();
			
			if (removeElem < arrayBegin || removeElem > arrayEnd) throw std::out_of_range("position out of range");
			if (removeElem == arrayEnd) return arrayEnd;
			
			--arrayEnd;
			*removeElem = std::move(*arrayEnd);//slightly less op than swap
			std::destroy_at(arrayEnd);
			--mSize;
			return (removeElem == arrayEnd) ? arrayEnd : removeElem;
		}
		template <typename UnaryPred>
		iterator remove(UnaryPred predicate) requires strong_movable<value_type>  && std::predicate<UnaryPred, const_reference>{
	
			pointer arrayBegin = raw_begin();
			pointer arrayEnd = raw_end();
			pointer current = raw_begin();
			pointer validLast = raw_end() - 1;

			while (current <= validLast) {
				if (predicate(*current)) {
					*current = std::move(*validLast);
					--validLast;
				}
				else {
					++current;
				}
			}

			pointer realOnePastEnd = validLast + 1;
			size_type newSize = realOnePastEnd - arrayBegin;

			if (newSize < size()) {
				std::destroy(realOnePastEnd, arrayEnd);
				mSize = newSize;
			}

			return realOnePastEnd;
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

		void resize_shrink(size_type count)noexcept {
			if (count >= mSize) return;//when shrinking if count is more, simply no OP

			std::destroy(raw_begin() + count, raw_end());
			mSize = count;
		}
		void resize_grow(size_type count)requires std::default_initializable<value_type> && strong_movable<value_type> {
			if (count <= mSize) return;//if count is less than current size, then logically no grow no op

			if (count > cap) {
				tryReAllocate(growthFactor(count));
			}

			pointer startAt = raw_end();
			pointer ifFailurePosition = startAt;
			size_type amount = count - mSize;

			try {
				ifFailurePosition = std::uninitialized_default_construct_n(startAt, amount);
				mSize = count;
			}
			catch (...) {
				std::destroy(startAt, ifFailurePosition);
				throw;
			}
		}
		void reserve(size_type newCap) requires strong_movable<value_type> {
			if (newCap > cap) {
				tryReAllocate(newCap);
			}
		}
		void shrinkToFit() requires strong_movable<value_type> {
			if (cap > mSize) {
				tryReAllocate(size());
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
