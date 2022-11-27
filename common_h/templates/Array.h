// TEMPLATE ARRAY CLASS
#ifndef TEMPLATE_ARRAY_HPP
#define TEMPLATE_ARRAY_HPP

#include "tcommon.h"

//#pragma pack(push, 8)

#define INVALID_ARRAY_INDEX			0xFFFFFFFF

// Class dynamic array
template< class _Ty > class array
{
private:
	// Allocator object
	allocator< _Ty > _Al;

public:
	// This array type redefinition
	typedef array<_Ty>	_Myt;

	// Default constructor
	array(const char * pFileName, long iFileLine, dword _dwAdd = 8) : _Al(pFileName, iFileLine, _dwAdd) {}
	// Default destructor
	~array() {}

	// Setup filename and line for memory allocation 
	void SetFileLine(const char * pFileName, long iFileLine) { _Al.SetFileLine(pFileName, iFileLine); }

	// Delete all content with destructor's
	_Myt & DelAll() 
	{ 
		_Al.DelAll(); 
		return * this;
	} 

	// Delete all content with destructor's and free all memory
	_Myt & Free() 
	{ 
		_Al.DelAll(); 
		_Al.FreeMemory();
		return * this;
	} 

	// Delete all content with destructor's, and delete each element /* VERY DANGEROUS FUNCTION !!! */ 
	_Myt & DelAllWithPointers() 
	{ 
		for (dword i=0; i<Size(); i++)
			DELETE(_Al.pE[i]);

		_Al.DelAll(); 
		return * this;
	} 
	// Empty all content without destructor's and delete array buffer /* VERY DANGEROUS FUNCTION !!! */ 
	_Myt & Empty() { _Al.Empty(); return * this; }		
	// Empty all content without delete array buffer /* VERY DANGEROUS FUNCTION !!! */ 
	_Myt & EmptyWithDestructors() 
	{ 
		for (long i=0; i<Len(); i++) 
			_Al.Destroy(i);
		_Al.Empty(); 
		return * this; 
	}		
	// Set new add elements size
	_Myt & SetAddElements(dword dwReserveCount) { _Al.SetAddElements(dwReserveCount); return * this; }
	// Add to array and Construct elements
	_Myt & AddElements(dword dwElementsCount)
	{
		if(!dwElementsCount) return * this;
		//dword dwCurrentPos = (IsEmpty()) ? 0 : Last();
		dword dwCurrentPos = Last() + 1;
		_Al.Reserve(Size() + dwElementsCount);
		for (dword i=dwCurrentPos; i<dwCurrentPos + dwElementsCount; i++)
		{
			_Al.Construct(i);
			_Al.IncSize();
		}
		return * this;
	}
	// Reserve number of elements
	_Myt & Reserve(dword dwReserveCount)
	{ 
		if (dwReserveCount < Size()) return * this;
		_Al.Reserve(dwReserveCount); 
		return * this;
	}
	// Insert new element to index position with shift
	_Myt & Insert(dword dwIndex)
	{
		Assert(dwIndex <= Size());
		_Al.Reserve(Size() + 1);

		if (dwIndex != Size())
			for (dword i=Size(); i > dwIndex; i--)
			{
				_Al.Construct(i);
				GetElement(i) = GetElement(i - 1);
				_Al.Destroy(i - 1);
			}

		_Al.Construct(dwIndex);
		_Al.IncSize();
		return * this;
	}
	// Insert new element to index position with shift
	_Myt & Insert(const _Ty & _T, dword dwIndex)
	{ 
		Assert(dwIndex <= Size());
		Insert(dwIndex);
		_Al.pE[dwIndex] = _T; 
		return * this;
	}
	// Extract element with shift
	_Myt & Extract(dword dwIndex)
	{ 
		Assert(dwIndex < Size());
		
		if (dwIndex == Last())
			_Al.Destroy(dwIndex);
		else{ 
			for (dword i=dwIndex; i < Last(); i++)
			{
				_Al.Destroy(i);
				_Al.Construct(i);
				GetElement(i) = GetElement(i + 1);
			}
			_Al.Destroy(Last());
		}

		_Al.DecSize();
		return * this;
	}
	// Extract element and place last element to this element index position
	_Myt & ExtractNoShift(dword dwIndex)
	{ 
		Assert(dwIndex < Size());
		_Al.Destroy(dwIndex);
		if (Size() > 1 && dwIndex != Last()) 
		{
			_Al.Construct(dwIndex);
			_Al.pE[dwIndex] = _Al.pE[Last()];
			_Al.Destroy(Last());
		}
		_Al.DecSize();
		return * this;
	}
	// Add new element to end of array
	dword Add(const _Ty & _T)
	{	
		if (Size() + 1 >= _Al.GetMaxElements()) _Al.Reserve(Size() + 1);
		_Al.Construct(Size());
		_Al.pE[Size()] = _T; 
		_Al.IncSize(); 
		return Last(); 
	}
	// Add empty element to end of array, return index
	dword Add()
	{	
		if (Size() + 1 >= _Al.GetMaxElements()) _Al.Reserve(Size() + 1);
		_Al.Construct(Size());
		//Убрал конструктор и копирование лишнее, в Construct() все уже сделано
		_Al.pE[Size()] = _Ty();
		_Al.IncSize(); 
		return Last(); 
	}
	// return true if element exist
	bool IsExist(const _Ty & _T) const
	{
		return Find(_T) != INVALID_ARRAY_INDEX;
	}
	// Find element, return index
	dword Find(const _Ty & _T) const
	{	
		for (dword i=0; i<Size(); i++) if (_T == _Al.pE[i]) return i; 
		return INVALID_ARRAY_INDEX; 
	}
	// Delete element from index position with shift
	_Myt & DelIndex(dword dwIndex)
	{	
		Extract(dwIndex);
		return * this;
	}
	// Delete elements from index range with shift
	_Myt & DelRange(dword dwFirst, dword dwLast)
	{
		//dword i;
		Assert(dwFirst <= Last() && dwLast <= Last() && dwFirst <= dwLast);

		dword i, j;
		for(i = dwFirst, j = dwLast + 1; j <= Last(); i++, j++)
		{
			_Al.Destroy(i);
			_Al.Construct(i);
			GetElement(i) = GetElement(j);
		}
		while(Size() > i)
		{
			_Al.Destroy(Last());
			_Al.DecSize();
		}
		return * this;
	}
	// Delete element with shift
	_Myt & Del(const _Ty & _T)
	{	
		dword dwIndex = Find(_T); 
		if (dwIndex == INVALID_ARRAY_INDEX) return * this;
		Extract(dwIndex);
		return * this;
	}
	// Replace element with index dwIndexToReplace by new element
	_Myt & Replace(dword dwIndexToReplace, const _Ty & _T)
	{
		Assert(dwIndexToReplace < Size());
		_Al.Destroy(dwIndexToReplace);
		_Al.Construct(dwIndexToReplace);
		_Al.pE[dwIndexToReplace] = _T; 
		return * this;
	}
	// Find and delete element and replace by new element, or add new
	_Myt & AddDel(const _Ty & _Tdel, const _Ty & _Tadd)
	{	
		dword dwIndex = Find(_Tdel);
		if (dwIndex != INVALID_ARRAY_INDEX) 
		{
			_Al.Destroy(dwIndex);
			_Al.Construct(dwIndex);
			_Al.pE[dwIndex] = _Tadd; 
		}
		else 
			Add(_Tadd); 

		return * this;
	}
	// Return total size of array(all elements number multiply by size of element)
	__forceinline dword GetDataSize() const { return _Al.Size() * sizeof(_Ty); }
	// Return number of elements
	__forceinline dword Size() const { return _Al.Size(); } 
	// Return number of elements
	__forceinline long Len() const { return (long)_Al.Size(); } 
	// Return last element index
	__forceinline dword Last() const { return _Al.Size() - 1; }
	// Return first element index
	__forceinline dword First() const { return 0; }
	// Operator return number of elements
	__forceinline operator long () const { return long(_Al.Size()); }
	// Operator return number of elements
	__forceinline dword operator () (void) const { return _Al.Size(); }
	
	// Return element reference
	__forceinline _Ty & GetElement(dword dwIndex) {	return _Al.pE[dwIndex]; }
	// Return const element reference
	__forceinline const _Ty & operator [] (dword dwIndex) const	{ Assert(dwIndex < _Al.Size()); return _Al.pE[dwIndex]; }
	// Return element reference
	__forceinline _Ty & operator [] (dword dwIndex) { Assert(dwIndex < _Al.Size()); return _Al.pE[dwIndex]; }
	// Return pointer to array buffer
	__forceinline _Ty * GetBuffer() { return (Size()) ? &_Al.pE[0] : null; }
	// Return pointer to array buffer
	__forceinline const _Ty * GetBuffer() const { return (Size()) ? &_Al.pE[0] : null; }
	// Return first element
	__forceinline _Ty & FirstE() { Assert(Size()); return _Al.pE[0]; }
	// Return last element
	__forceinline _Ty & LastE() { Assert(Size()); return _Al.pE[Last()]; }
	
	// return true if array is empty
	__forceinline bool IsEmpty() const { return Len() == 0; }

	// Copy other array to this array
	_Myt & Copy(const _Myt & _X)
	{
		DelAll();
		Reserve(_X.Size());
		for (dword i=0; i<_X(); i++) Add(_X[i]);
		return * this;
	}
	// Copy operator 
	_Myt & operator = (const _Myt & _X)
	{
		if (this == &_X) return (* this);
		Copy(_X);
		return (* this);
	}
	// Swap two elements using indexes // Maybe FIX-ME ???
	__forceinline _Myt & Swap(dword i, dword j)
	{
		Assert(i < Size() && j < Size());
		_Swap(GetElement(i), GetElement(j));
		return * this;
	}
	// Swap array
	__forceinline _Myt & SwapArray()
	{
		for (dword i=0; i<Size() / 2; i++) 
			_Swap(GetElement(i), GetElement(Last() - i));
		return * this;
	}

	// Quick sort array, based on GMXQSort :)
	_Myt & QSort(bool (*compare_func)(const _Ty &, const _Ty &))
	{
		if(_Al.Size() <= 1) return *this;
		QSortRecursive(compare_func, &_Al.pE[0], &_Al.pE[_Al.Size() - 1]);
		return *this;
	}


	//Call:   array<myType>::QSort(Func, buffer, sizeOfBuffer);
	__forceinline static void QSort(bool (*compare_func)(const _Ty &, const _Ty &), _Ty * ptr, long size)
	{
		if(!ptr) return;
		if(size <= 1) return;
		QSortRecursive(compare_func, ptr, ptr + size - 1);
	}

private:

	array (const _Myt & _X) 
	{
		Assert(false);
	}


	static __forceinline void _Swap(_Ty & v1, _Ty & v2)
	{
		_Ty tmp = v1;
		v1 = v2;
		v2 = tmp;
	}

	static void QSortRecursive(bool (*compare_func)(const _Ty &, const _Ty &), _Ty * start, _Ty * end)
	{
		if(start >= end) return;
		_Ty * middle = start;
		for(_Ty * cur = start; cur < end; cur++)
		{
			//*cur < *end
			if(compare_func(*cur, *end))
			{
				_Swap(*middle, *cur);
				middle++;
			}
		}
		_Swap(*middle, *end);		
		QSortRecursive(compare_func, start, middle - 1);
		QSortRecursive(compare_func, middle + 1, end);
	}
	
};

//#pragma pack(pop)

#endif
