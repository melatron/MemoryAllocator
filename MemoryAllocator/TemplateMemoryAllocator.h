//#pragma once
//
//#include <cstddef>
//#include <allocators>
//
//template <class T>
//class SimpleAllocator
//{
//public:
//	typedef T value_type;
//	SimpleAllocator();
//	template <class U> SimpleAllocator(const SimpleAllocator<U>& other);
//	T* allocate(std::size_t n);
//	void deallocate(T* p, std::size_t n);
//	
//	MemoryAllocator m_allocator;
//};
//
//template <class T, class U>
//bool operator==(const SimpleAllocator<T>&, const SimpleAllocator<U>&);
//template <class T, class U>
//bool operator!=(const SimpleAllocator<T>&, const SimpleAllocator<U>&);
//
//
//template<class T>
//inline SimpleAllocator<T>::SimpleAllocator()
//{
//}
//
//template<class T>
//T * SimpleAllocator<T>::allocate(std::size_t n)
//{
//	return static_cast<T*>(m_allocator.allocate(n));
//}
//
//template<class T>
//void SimpleAllocator<T>::deallocate(T * p, std::size_t n)
//{
//	m_allocator.deallocate(p);
//}
//
//template<class T>
//template<class U>
//inline SimpleAllocator<T>::SimpleAllocator(const SimpleAllocator<U>& other)
//{
//}
//
//template<class T, class U>
//inline bool operator==(const SimpleAllocator<T>&, const SimpleAllocator<U>&)
//{
//	return true;
//}
//
//template<class T, class U>
//inline bool operator!=(const SimpleAllocator<T>&, const SimpleAllocator<U>&)
//{
//	return true;
//}
//


#pragma once
#include "MemoryAllocator.h"

 //std::allocator compatible using plain malloc to avoid operator new
template <typename T>
struct MallocAllocator {
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;


	MemoryAllocator m_allocator;


	pointer allocate(size_type n, const void* = 0)
	{
		return static_cast<T*>(m_allocator.allocate(n * sizeof(T)));
	}

	void deallocate(pointer ptr, size_type) { 
		m_allocator.deallocate(ptr);
	}

	// boilerplate follows
	MallocAllocator() {}

	MallocAllocator(const MallocAllocator&) {}

	template <typename Other>
	MallocAllocator(const MallocAllocator<Other>&)
	{
	}

	MallocAllocator& operator=(const MallocAllocator&) { return *this; }

	template <class Other>
	MallocAllocator& operator=(const MallocAllocator<Other>&)
	{
		return *this;
	}

	template <typename Other>
	struct rebind {
		typedef MallocAllocator<Other> other;
	};

	size_type max_size() const throw()
	{
		// ugly size_t maximum but don't want to drag <numeric_limits> just because of this
		return std::size_t(-1) / sizeof(T);
	}

	pointer address(reference ref) const { return &ref; }

	const_pointer address(const_reference ref) const { return &ref; }

	void construct(pointer ptr, const value_type& val) { ::new(ptr) value_type(val); }

	void destroy(pointer ptr) { ptr->~value_type(); }
};

template <typename T, typename U>
inline bool operator==(const MallocAllocator<T>&, const MallocAllocator<U>&)
{
	return true;
}

template <typename T, typename U>
inline bool operator!=(const MallocAllocator<T>& a, const MallocAllocator<U>& b)
{
	return !(a == b);
}
