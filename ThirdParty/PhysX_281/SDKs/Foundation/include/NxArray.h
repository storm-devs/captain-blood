#ifndef NX_FOUNDATION_NXARRAY
#define NX_FOUNDATION_NXARRAY
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/
/** \addtogroup foundation
  @{
*/

#include "Nx.h"
#include "NxAllocatorDefault.h"
//#include "NxUserAllocatorAccess.h"
//#include "Allocateable.h"

/**
 \brief Simple 'std::vector' style template container.

 if no NxUserAllocator is specified, the NxDefaultAllocator is used.  

 Note: the methods of this template are implemented inline in order to avoid the not yet very cross-compileable 'typename' keyword.
*/
template<class ElemType, class AllocType = NxAllocatorDefault>
class NxArray// : public Allocateable
	{
	public:
	typedef NxArray<ElemType,AllocType> MyType;
	typedef ElemType * Iterator;
	typedef const ElemType * ConstIterator;
	/*
	Same as above with STL compliant syntax.  Deprecated.
	*/
	typedef Iterator iterator;
	typedef ConstIterator const_iterator;

	/**
	creates an empty array
	*/
	NX_INLINE NxArray() : first(0), last(0), memEnd(0) 
																		{
																		//nothing
																		}

	/**
	creates an array of n elements with value x.

	\param n Number of elements to allocate
	\param v Value to set elements to.
	*/
	NX_INLINE NxArray(unsigned n, const ElemType& v = ElemType() )
																		{
																		first = allocate(n);
																		fill(first, n, v);
																		memEnd = last = first + n;
																		}

	/**
	copy constructor

	\param other Array to copy from.
	*/
	NX_INLINE NxArray(const MyType & other)
																		{
																		first = allocate(other.size());
																		memEnd = last = copy(other.begin(), other.end(), first);
																		}

	/**
	destructor.  Remember that if ElemType is a pointer type, then
	the actual objects pointed to are not destroyed.
	*/
	NX_INLINE ~NxArray()
																		{
																		destroy(first, last);
																		deallocate(first);
																		first = 0;
																		last = 0;
																		memEnd = 0;
																		}

	/**
	assignment

	\param other Array to copy from.
	*/
	NX_INLINE MyType& operator=(const MyType& other)
																		{
																		if (this != &other)
																			{
																			if (other.size() <= size())
																				{
																				Iterator s = copy(other.begin(), other.end(), first);
																				destroy(s, last);
																				last = first + other.size(); 
																				}
																			else if (other.size() <= capacity())
																				{
																				ConstIterator s = other.begin() + size();
																				copy(other.begin(), s, first);
																				copy(s, other.end(), last);
																				last = first + other.size();
																				}
																			else
																				{
																				destroy(first, last);
																				deallocate(first);
																				first = allocate(other.size());
																				last = copy(other.begin(), other.end(), first);
																				memEnd = last; 
																				}
																			}
																		return *this;
																		}

	/**
	The member function ensures that capacity() henceforth returns at least n.

	\param n Capacity to increase to.
	*/
	NX_INLINE void reserve(unsigned n)
																		{
																		if (capacity() < n)
																			{
																			
																			iterator s = allocate(n);
																			copy(first, last, s);
																			destroy(first, last);
																			deallocate(first);
																			//if we knew we just have atomics we could just do this:
																			//first = reallocate(n, first);
																			memEnd = s + n;
																			last = s + size();
																			first = s; 
																			}
																		}

	/**
	The member function returns the storage currently allocated to hold the controlled sequence, a value at least as large as size().
	*/
	NX_INLINE unsigned capacity() const
																		{
																		return (unsigned)(first == 0 ? 0 : memEnd - first);
																		}

	/**
	The member function ensures that size() henceforth returns n. 
	If it must lengthen the controlled sequence, it appends elements with value x.
	The capacity of the vector is set to fit the size.

	\param n New size.
	\param v Value to fill new elements with.
	*/
	NX_INLINE void resize(unsigned n, const ElemType& v = ElemType())
																		{
																		if (size() < n)
																			insert(end(), n - size(), v);
																		else if (n < size())
																			erase(begin() + n, end());
																		
																		if (first == last) 
																			{
																			deallocate(first);
																			first = 0;
																			last = 0;
																			memEnd = 0;
																			}

																		if (memEnd > last) //release unused memory
																			{
																			size_t s = (size_t)(last - first);
																			first = reallocate(size(), first);
																			memEnd = last  = first + s;
																			}
																		}

	/**
	The member function returns the length of the controlled sequence.
	*/
	NX_INLINE unsigned size() const
																		{
																		return (unsigned)(last - first);
																		}

	/**
	The member function returns true for an empty controlled sequence.
	*/
	NX_INLINE bool isEmpty() const
																		{
																		return (size() == 0);
																		}

	/**
	Same as above with STL compliant syntax.  Deprecated.
	*/
	NX_INLINE bool empty() const
																		{
																		return isEmpty();
																		}

	/**
	The member function inserts an element with value x at the end of the controlled sequence.

	NOTE: This calls ElemType operator = if its defined, on an uninitialized object of type ElemType.
	
	\param x Element to add.
	*/
	NX_INLINE void pushBack(const ElemType& x)
																		{
																		//insert(end(), x);
																		if (memEnd <= last)
																			{
																			reserve((1 + size()) * 2);
																			}
																		*last = x;
																		last ++;
																		}

	/**
	The member function inserts an uninitialized element at the end of the controlled sequence.  This element is returned so that the user can initialize it.
	*/
	NX_INLINE ElemType & pushBack()
																		{
																		if (memEnd <= last)
																			{
																			reserve((1 + size()) * 2);
																			}
																		last ++;
																		return *(last-1);
																		}
	/**
	Same as above with STL compliant syntax.  Deprecated.

	\param x Element to add.
	*/
	NX_INLINE void push_back(const ElemType& x)
																		{
																		pushBack(x);
																		}

	/**
	The member function removes the last element of the controlled sequence, which must be non-empty.
	*/
	NX_INLINE void popBack()
																		{
																		--last;
																		last->~ElemType();
																		}

	/**
	Same as above with STL compliant syntax.  Deprecated.
	*/
	NX_INLINE void pop_back()
																		{
																		popBack();
																		}

	/**
	Deletes all elements in the sequence.  Does not release memory (capacity).
	*/
	NX_INLINE void clear()
																		{
																		destroy(begin(), end());
																		last = first;
																		}

	/**
	Removes a given element from the array, replaces it with the last element.

	\param position position to replace with the last element.
	*/
	NX_INLINE void replaceWithLast(unsigned position)
																		{
																		NX_ASSERT(position<size());
																		if (position!=size()-1)
																			{
																			(*this)[position].~ElemType();
																			ElemType& elem = back();
																			(*this)[position] = elem;
																			}
																		popBack();
																		}

	/**
	Finds a given element in the array, removes it and replaces it with the last element. Does
	nothing if the element is not found.

	\param x element to replace with the last element.
	\return true if the element was found, false otherwise.
	*/
	NX_INLINE bool deleteEntry(const ElemType& x)
																		{
																		NxU32 s = size();
																		for(NxU32 i=0;i<s;i++)
																			{
																			if((*this)[i] == x)
																				{
																				replaceWithLast(i);
																				return true;	// Element found in the array
																				}
																			}
																		return false;	// Element not found in the array
																		}

	/**
	Returns a constant reference to an element in the sequence.
	*/
	NX_INLINE const ElemType & operator[](unsigned n) const
																		{
																		return *(first + n);
																		}

	/**
	Returns a reference to an element in the sequence.
	*/
	NX_INLINE ElemType & operator[](unsigned n)
																		{
																		return *(first + n);
																		}

	//! iteration

	/**
	The member function returns a random-access iterator that points at the first element of the sequence (or just beyond the end of an empty sequence).
	*/
	NX_INLINE Iterator begin()
																		{
																		return first;
																		}
	NX_INLINE ConstIterator begin() const
																		{
																		return first;
																		}

	/**
	The member function returns a random-access iterator that points just beyond the end of the sequence.
	*/
	NX_INLINE Iterator end()
																		{
																		return last;
																		}
	NX_INLINE ConstIterator end() const
																		{
																		return last;
																		}

	/**
	The member function returns a reference to the first element of the sequence, only valid if at least one element is in the sequence.
	*/
	NX_INLINE ElemType & front()
																		{
																		NX_ASSERT(last!=first);
																		return *first;
																		}
	NX_INLINE const ElemType & front() const
																		{
																		NX_ASSERT(last!=first);
																		return *first;
																		}

	/**
	The member function returns a reference to the last element of the sequence, only valid if at least one element is in the sequence.
	*/
	NX_INLINE ElemType & back()
																		{
																		NX_ASSERT(last!=first);
																		return *(last-1);
																		}
	NX_INLINE const ElemType & back() const
																		{
																		NX_ASSERT(last!=first);
																		return *(last-1);
																		}

	/**
	Insert n elements at where, set these elements to x

	\param where Position to insert elements.
	\param n Number of elements to insert.
	\param x Value to set elements to.
	*/
	NX_INLINE void insert(Iterator where, unsigned n, const ElemType & x)
																		{	
																		// ElemType tmp = x;

																		if (n == 0) return;
																		if (capacity() < size() + n) 
																			{
	    																	size_t pos = where - first;
																			reserve((n + size()) * 2);
																			where = first + pos;
																			}
																		iterator stop = where + n; 
																		copy(where,last,stop);
																		fill(where,n,x);
																		last = last + n;
																		}

	/**
	Erase elements from from to to.  Does not call destructors!
	
	\param from Start of range to erase.
	\param to End of range to erase.
	*/
	NX_INLINE void erase(Iterator from, Iterator to)
																		{	
																		if (to < last)
																			{
																			copy(to, last, from);
																			}
																		last = last - (to - from);
																		}

	/**
	Erase elements from from to end.  Does not call destructors!

	\param from Start of range to erase.
	*/
	NX_INLINE void erase(Iterator from)
																		{	
																		last = from;
																		}

	private:
	NX_INLINE void fill(Iterator f, unsigned n, const ElemType & x)
																		{
																		for (; 0 < n; --n, ++f)
																			*f = x;
																		}

	NX_INLINE Iterator copy(ConstIterator f, ConstIterator l,	Iterator p)
																		{
																		for (; f != l; ++p, ++f)
																			*p = *f;
																		return p; 
																		}

	NX_INLINE ElemType * allocate(size_t n)
																		{
#ifdef _DEBUG
																		return (ElemType *)allocator.mallocDEBUG(n * sizeof(ElemType), (const char *)__FILE__, __LINE__, "ElemType", NX_MEMORY_Generic_Array_Container);
#else
																		return (ElemType *)allocator.malloc(n * sizeof(ElemType), NX_MEMORY_Generic_Array_Container);
#endif
																		}

	NX_INLINE ElemType * reallocate(size_t n, ElemType *old)
																		{
																		return (ElemType *)allocator.realloc(old, n * sizeof(ElemType));
																		}

	NX_INLINE void deallocate(ElemType *p)
																		{
																		if(p)	allocator.free(p);
																		}

	NX_INLINE void destroy(Iterator f, Iterator l)
																		{
																		for (; f != l; ++f)
																			{
																			f->~ElemType();
																			}
																		}

	Iterator first, last, memEnd;

	AllocType allocator;
	};



 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
