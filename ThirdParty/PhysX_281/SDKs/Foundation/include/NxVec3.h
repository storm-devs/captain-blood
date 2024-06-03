#ifndef NX_FOUNDATION_NXVEC3
#define NX_FOUNDATION_NXVEC3
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



#include "Nxf.h"
#include "NxMath.h"

class  NxMat33;


/**
\brief Enum to classify an axis.
*/
	enum NxAxisType
	{
		NX_AXIS_PLUS_X,
		NX_AXIS_MINUS_X,
		NX_AXIS_PLUS_Y,
		NX_AXIS_MINUS_Y,
		NX_AXIS_PLUS_Z,
		NX_AXIS_MINUS_Z,
		NX_AXIS_ARBITRARY
	};


class NxVec3;

/** \cond Exclude from documentation */
typedef struct _Nx3F32
{
	NxReal x, y, z;
	
	NX_INLINE const _Nx3F32& operator=(const NxVec3& d);
} Nx3F32;
/** \endcond */

/**
\brief 3 Element vector class.

This is a vector class with public data members.
This is not nice but it has become such a standard that hiding the xyz data members
makes it difficult to reuse external code that assumes that these are public in the library.
The vector class can be made to use float or double precision by appropriately defining NxReal.
This has been chosen as a cleaner alternative to a template class.
*/
class NxVec3
	{
	public:
	//!Constructors

	/**
	\brief default constructor leaves data uninitialized.
	*/
	NX_INLINE NxVec3();

	/**
	\brief Assigns scalar parameter to all elements.
	
	Useful to initialize to zero or one.

	\param[in] a Value to assign to elements.
	*/
	explicit NX_INLINE NxVec3(NxReal a);

	/**
	\brief Initializes from 3 scalar parameters.

	\param[in] nx Value to initialize X component.
	\param[in] ny Value to initialize Y component.
	\param[in] nz Value to initialize Z component.
	*/
	NX_INLINE NxVec3(NxReal nx, NxReal ny, NxReal nz);
	
	/**
	\brief Initializes from Nx3F32 data type.

	\param[in] a Value to initialize with.
	*/
	NX_INLINE NxVec3(const Nx3F32 &a);

	/**
	\brief Initializes from an array of scalar parameters.

	\param[in] v Value to initialize with.
	*/
	NX_INLINE NxVec3(const NxReal v[]);

	/**
	\brief Copy constructor.
	*/
	NX_INLINE NxVec3(const NxVec3& v);

	/**
	\brief Assignment operator.
	*/
	NX_INLINE const NxVec3& operator=(const NxVec3&);

	/**
	\brief Assignment operator.
	*/
	NX_INLINE const NxVec3& operator=(const Nx3F32&);

	/**
	\brief Access the data as an array.

	\return Array of 3 floats.
	*/
	NX_INLINE const NxReal *get() const;
	
	/**
	\brief Access the data as an array.

	\return Array of 3 floats.
	*/
	NX_INLINE NxReal* get();

	/**
	\brief writes out the 3 values to dest.

	\param[out] dest Array to write elements to.
	*/
	NX_INLINE void get(NxF32 * dest) const;

	/**
	\brief writes out the 3 values to dest.
	*/
	NX_INLINE void get(NxF64 * dest) const;

	NX_INLINE NxReal& operator[](int index);
	NX_INLINE NxReal  operator[](int index) const;

	//Operators
	/**
	\brief true if all the members are smaller.
	*/
	NX_INLINE bool operator< (const NxVec3&) const;
	/**
	\brief returns true if the two vectors are exactly equal.

	use equal() to test with a tolerance.
	*/
	NX_INLINE bool operator==(const NxVec3&) const;
	/**
	\brief returns true if the two vectors are exactly unequal.

	use !equal() to test with a tolerance.
	*/
	NX_INLINE bool operator!=(const NxVec3&) const;

/*	NX_INLINE const NxVec3 &operator +=(const NxVec3 &);
	NX_INLINE const NxVec3 &operator -=(const NxVec3 &);
	NX_INLINE const NxVec3 &operator *=(NxReal);
	NX_INLINE const NxVec3 &operator /=(NxReal);
*/
//Methods
	NX_INLINE void  set(const NxVec3 &);

//legacy methods:
	NX_INLINE void setx(const NxReal & d);
	NX_INLINE void sety(const NxReal & d);
	NX_INLINE void setz(const NxReal & d);

	/**
	\brief this = -a
	*/
	NX_INLINE void  setNegative(const NxVec3 &a);

	/**
	\brief this = -this
	*/
	NX_INLINE void  setNegative();

	/**
	\brief reads 3 consecutive values from the ptr passed
	*/
	NX_INLINE void  set(const NxF32 *);

	/**
	\brief reads 3 consecutive values from the ptr passed
	*/
	NX_INLINE void  set(const NxF64 *);
	NX_INLINE void  set(NxReal, NxReal, NxReal);
	NX_INLINE void  set(NxReal);

	NX_INLINE void  zero();
	
	/**
	\brief tests for exact zero vector
	*/
	NX_INLINE NX_BOOL isZero()	const
		{
		if((x != 0.0f) || (y != 0.0f) || (z != 0.0f))	return NX_FALSE;
		return NX_TRUE;
		}

	NX_INLINE void  setPlusInfinity();
	NX_INLINE void  setMinusInfinity();

	/**
	\brief this = element wise min(this,other)
	*/
	NX_INLINE void min(const NxVec3 &);
	/**
	\brief this = element wise max(this,other)
	*/
	NX_INLINE void max(const NxVec3 &);

	/**
	\brief this = a + b
	*/
	NX_INLINE void  add(const NxVec3 & a, const NxVec3 & b);
	/**
	\brief this = a - b
	*/
	NX_INLINE void  subtract(const NxVec3 &a, const NxVec3 &b);
	/**
	\brief this = s * a;
	*/
	NX_INLINE void  multiply(NxReal s,  const NxVec3 & a);

	/**
	\brief this[i] = a[i] * b[i], for all i.
	*/
	NX_INLINE void  arrayMultiply(const NxVec3 &a, const NxVec3 &b);


	/**
	\brief this = s * a + b;
	*/
	NX_INLINE void  multiplyAdd(NxReal s, const NxVec3 & a, const NxVec3 & b);

	/**
	\brief normalizes the vector
	*/
	NX_INLINE NxReal normalize();

	/**
	\brief sets the vector's magnitude
	*/
	NX_INLINE void	setMagnitude(NxReal);

	/**
	\brief snaps to closest axis
	*/
	NX_INLINE NxU32			closestAxis()	const;

	/**
	\brief snaps to closest axis
	*/
	NX_INLINE NxAxisType	snapToClosestAxis();

//const methods
	/**
	\brief returns true if all 3 elems of the vector are finite (not NAN or INF, etc.)
	*/
	NX_INLINE bool isFinite() const;

	/**
	\brief returns the scalar product of this and other.
	*/
	NX_INLINE NxReal dot(const NxVec3 &other) const;

	/**
	\brief compares orientations (more readable, user-friendly function)
	*/
	NX_INLINE bool sameDirection(const NxVec3 &) const;

	/**
	\brief returns the magnitude
	*/
	NX_INLINE NxReal magnitude() const;

	/**
	\brief returns the squared magnitude
	
	Avoids calling sqrt()!
	*/
	NX_INLINE NxReal magnitudeSquared() const;

	/**
	\brief returns (this - other).magnitude();
	*/
	NX_INLINE NxReal distance(const NxVec3 &) const;

	/**
	\brief returns (this - other).magnitudeSquared();
	*/
	NX_INLINE NxReal distanceSquared(const NxVec3 &v) const;

	/**
	\brief this = left x right
	*/
	NX_INLINE void cross(const NxVec3 &left, const NxVec3 & right);

	/**
	\brief Stuff magic values in the point, marking it as explicitly not used.
	*/
	NX_INLINE void setNotUsed();

	/**
	\brief Checks the point is marked as not used
	*/
	NX_BOOL isNotUsed() const;

	/**
	\brief returns true if this and arg's elems are within epsilon of each other.
	*/
	NX_INLINE bool equals(const NxVec3 &, NxReal epsilon) const;

	/**
	\brief negation
	*/
	NxVec3 operator -() const;
	/**
	\brief vector addition
	*/
	NxVec3 operator +(const NxVec3 & v) const;
	/**
	\brief vector difference
	*/
	NxVec3 operator -(const NxVec3 & v) const;
	/**
	\brief scalar post-multiplication
	*/
	NxVec3 operator *(NxReal f) const;
	/**
	\brief scalar division
	*/
	NxVec3 operator /(NxReal f) const;
	/**
	\brief vector addition
	*/
	NxVec3&operator +=(const NxVec3& v);
	/**
	\brief vector difference
	*/
	NxVec3&operator -=(const NxVec3& v);
	/**
	\brief scalar multiplication
	*/
	NxVec3&operator *=(NxReal f);
	/**
	\brief scalar division
	*/
	NxVec3&operator /=(NxReal f);
	/**
	\brief cross product
	*/
	NxVec3 cross(const NxVec3& v) const;

	/**
	\brief cross product
	*/
	NxVec3 operator^(const NxVec3& v) const;
	/**
	\brief dot product
	*/
	NxReal      operator|(const NxVec3& v) const;

	NxReal x,y,z;
	};

/** \cond Exclude from documentation */
NX_INLINE const _Nx3F32& Nx3F32::operator=(const NxVec3& d)
	{
	x=d.x;
	y=d.y;
	z=d.z;
	return *this;
	}
/** \endcond */

//NX_INLINE implementations:
NX_INLINE NxVec3::NxVec3(NxReal v) : x(v), y(v), z(v)
	{
	}

NX_INLINE NxVec3::NxVec3(NxReal _x, NxReal _y, NxReal _z) : x(_x), y(_y), z(_z)
	{
	}

NX_INLINE NxVec3::NxVec3(const Nx3F32 &d) : x(d.x), y(d.y), z(d.z)
	{
	}

NX_INLINE NxVec3::NxVec3(const NxReal v[]) : x(v[0]), y(v[1]), z(v[2])
	{
	}


NX_INLINE NxVec3::NxVec3(const NxVec3 &v) : x(v.x), y(v.y), z(v.z)
	{
	}


NX_INLINE NxVec3::NxVec3()
	{
	//default constructor leaves data uninitialized.
	}


NX_INLINE const NxVec3& NxVec3::operator=(const NxVec3& v)
	{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
	}

NX_INLINE const NxVec3& NxVec3::operator=(const Nx3F32& d)
	{
	x = d.x;
	y = d.y;
	z = d.z;
	return *this;
	}

// Access the data as an array.

NX_INLINE const NxReal* NxVec3::get() const
	{
	return &x;
	}


NX_INLINE NxReal* NxVec3::get()
	{
	return &x;
	}

 
NX_INLINE void  NxVec3::get(NxF32 * v) const
	{
	v[0] = (NxF32)x;
	v[1] = (NxF32)y;
	v[2] = (NxF32)z;
	}

 
NX_INLINE void  NxVec3::get(NxF64 * v) const
	{
	v[0] = (NxF64)x;
	v[1] = (NxF64)y;
	v[2] = (NxF64)z;
	}


NX_INLINE NxReal& NxVec3::operator[](int index)
	{
	NX_ASSERT(index>=0 && index<=2);
	return (&x)[index];
	}


NX_INLINE NxReal  NxVec3::operator[](int index) const
	{
	NX_ASSERT(index>=0 && index<=2);
	return (&x)[index];
	}

 
NX_INLINE void NxVec3::setx(const NxReal & d) 
	{ 
	x = d; 
	}

 
NX_INLINE void NxVec3::sety(const NxReal & d) 
	{ 
	y = d; 
	}

 
NX_INLINE void NxVec3::setz(const NxReal & d) 
	{ 
	z = d; 
	}

//Operators
 
NX_INLINE bool NxVec3::operator< (const NxVec3&v) const
	{
	return ((x < v.x)&&(y < v.y)&&(z < v.z));
	}

 
NX_INLINE bool NxVec3::operator==(const NxVec3& v) const
	{
	return ((x == v.x)&&(y == v.y)&&(z == v.z));
	}

 
NX_INLINE bool NxVec3::operator!=(const NxVec3& v) const
	{
	return ((x != v.x)||(y != v.y)||(z != v.z));
	}

//Methods
 
NX_INLINE void  NxVec3::set(const NxVec3 & v)
	{
	x = v.x;
	y = v.y;
	z = v.z;
	}

 
NX_INLINE void  NxVec3::setNegative(const NxVec3 & v)
	{
	x = -v.x;
	y = -v.y;
	z = -v.z;
	}

 
NX_INLINE void  NxVec3::setNegative()
	{
	x = -x;
	y = -y;
	z = -z;
	}


 
NX_INLINE void  NxVec3::set(const NxF32 * v)
	{
	x = (NxReal)v[0];
	y = (NxReal)v[1];
	z = (NxReal)v[2];
	}

 
NX_INLINE void  NxVec3::set(const NxF64 * v)
	{
	x = (NxReal)v[0];
	y = (NxReal)v[1];
	z = (NxReal)v[2];
	}


 
NX_INLINE void  NxVec3::set(NxReal _x, NxReal _y, NxReal _z)
	{
	this->x = _x;
	this->y = _y;
	this->z = _z;
	}

 
NX_INLINE void NxVec3::set(NxReal v)
	{
	x = v;
	y = v;
	z = v;
	}

 
NX_INLINE void  NxVec3::zero()
	{
	x = y = z = 0.0;
	}

 
NX_INLINE void  NxVec3::setPlusInfinity()
	{
	x = y = z = NX_MAX_F32; //TODO: this may be double too, but here we can't tell!
	}

 
NX_INLINE void  NxVec3::setMinusInfinity()
	{
	x = y = z = NX_MIN_F32; //TODO: this may be double too, but here we can't tell!
	}

 
NX_INLINE void NxVec3::max(const NxVec3 & v)
	{
	x = NxMath::max(x, v.x);
	y = NxMath::max(y, v.y);
	z = NxMath::max(z, v.z);
	}

 
NX_INLINE void NxVec3::min(const NxVec3 & v)
	{
	x = NxMath::min(x, v.x);
	y = NxMath::min(y, v.y);
	z = NxMath::min(z, v.z);
	}




NX_INLINE void  NxVec3::add(const NxVec3 & a, const NxVec3 & b)
	{
	x = a.x + b.x;
	y = a.y + b.y;
	z = a.z + b.z;
	}


NX_INLINE void  NxVec3::subtract(const NxVec3 &a, const NxVec3 &b)
	{
	x = a.x - b.x;
	y = a.y - b.y;
	z = a.z - b.z;
	}


NX_INLINE void  NxVec3::arrayMultiply(const NxVec3 &a, const NxVec3 &b)
	{
	x = a.x * b.x;
	y = a.y * b.y;
	z = a.z * b.z;
	}


NX_INLINE void  NxVec3::multiply(NxReal s,  const NxVec3 & a)
	{
	x = a.x * s;
	y = a.y * s;
	z = a.z * s;
	}


NX_INLINE void  NxVec3::multiplyAdd(NxReal s, const NxVec3 & a, const NxVec3 & b)
	{
	x = s * a.x + b.x;
	y = s * a.y + b.y;
	z = s * a.z + b.z;
	}

 
NX_INLINE NxReal NxVec3::normalize()
	{
	NxReal m = magnitude();
	if (m)
		{
		const NxReal il =  NxReal(1.0) / m;
		x *= il;
		y *= il;
		z *= il;
		}
	return m;
	}

 
NX_INLINE void NxVec3::setMagnitude(NxReal length)
	{
	NxReal m = magnitude();
	if(m)
		{
		NxReal newLength = length / m;
		x *= newLength;
		y *= newLength;
		z *= newLength;
		}
	}

 
NX_INLINE NxAxisType NxVec3::snapToClosestAxis()
	{
	const NxReal almostOne = 0.999999f;
			if(x >=  almostOne) { set( 1.0f,  0.0f,  0.0f);	return NX_AXIS_PLUS_X ; }
	else	if(x <= -almostOne) { set(-1.0f,  0.0f,  0.0f);	return NX_AXIS_MINUS_X; }
	else	if(y >=  almostOne) { set( 0.0f,  1.0f,  0.0f);	return NX_AXIS_PLUS_Y ; }
	else	if(y <= -almostOne) { set( 0.0f, -1.0f,  0.0f);	return NX_AXIS_MINUS_Y; }
	else	if(z >=  almostOne) { set( 0.0f,  0.0f,  1.0f);	return NX_AXIS_PLUS_Z ; }
	else	if(z <= -almostOne) { set( 0.0f,  0.0f, -1.0f);	return NX_AXIS_MINUS_Z; }
	else													return NX_AXIS_ARBITRARY;
	}


NX_INLINE NxU32 NxVec3::closestAxis() const
	{
	const NxF32* vals = &x;
	NxU32 m = 0;
	if(NxMath::abs(vals[1]) > NxMath::abs(vals[m])) m = 1;
	if(NxMath::abs(vals[2]) > NxMath::abs(vals[m])) m = 2;
	return m;
	}


//const methods
 
NX_INLINE bool NxVec3::isFinite() const
	{
	return NxMath::isFinite(x) && NxMath::isFinite(y) && NxMath::isFinite(z);
	}

 
NX_INLINE NxReal NxVec3::dot(const NxVec3 &v) const
	{
	return x * v.x + y * v.y + z * v.z;
	}

 
NX_INLINE bool NxVec3::sameDirection(const NxVec3 &v) const
	{
	return x*v.x + y*v.y + z*v.z >= 0.0f;
	}

 
NX_INLINE NxReal NxVec3::magnitude() const
	{
	return NxMath::sqrt(x * x + y * y + z * z);
	}

 
NX_INLINE NxReal NxVec3::magnitudeSquared() const
	{
	return x * x + y * y + z * z;
	}

 
NX_INLINE NxReal NxVec3::distance(const NxVec3 & v) const
	{
	NxReal dx = x - v.x;
	NxReal dy = y - v.y;
	NxReal dz = z - v.z;
	return NxMath::sqrt(dx * dx + dy * dy + dz * dz);
	}

 
NX_INLINE NxReal NxVec3::distanceSquared(const NxVec3 &v) const
	{
	NxReal dx = x - v.x;
	NxReal dy = y - v.y;
	NxReal dz = z - v.z;
	return dx * dx + dy * dy + dz * dz;
	}

 
NX_INLINE void NxVec3::cross(const NxVec3 &left, const NxVec3 & right)	//prefered version, w/o temp object.
	{
	// temps needed in case left or right is this.
	NxReal a = (left.y * right.z) - (left.z * right.y);
	NxReal b = (left.z * right.x) - (left.x * right.z);
	NxReal c = (left.x * right.y) - (left.y * right.x);

	x = a;
	y = b;
	z = c;
	}

 
NX_INLINE bool NxVec3::equals(const NxVec3 & v, NxReal epsilon) const
	{
	return 
		NxMath::equals(x, v.x, epsilon) &&
		NxMath::equals(y, v.y, epsilon) &&
		NxMath::equals(z, v.z, epsilon);
	}


 
NX_INLINE NxVec3 NxVec3::operator -() const
	{
	return NxVec3(-x, -y, -z);
	}

 
NX_INLINE NxVec3 NxVec3::operator +(const NxVec3 & v) const
	{
	return NxVec3(x + v.x, y + v.y, z + v.z);	// RVO version
	}

 
NX_INLINE NxVec3 NxVec3::operator -(const NxVec3 & v) const
	{
	return NxVec3(x - v.x, y - v.y, z - v.z);	// RVO version
	}



NX_INLINE NxVec3 NxVec3::operator *(NxReal f) const
	{
	return NxVec3(x * f, y * f, z * f);	// RVO version
	}


NX_INLINE NxVec3 NxVec3::operator /(NxReal f) const
	{
		f = NxReal(1.0) / f; return NxVec3(x * f, y * f, z * f);
	}


NX_INLINE NxVec3& NxVec3::operator +=(const NxVec3& v)
	{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
	}


NX_INLINE NxVec3& NxVec3::operator -=(const NxVec3& v)
	{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
	}


NX_INLINE NxVec3& NxVec3::operator *=(NxReal f)
	{
	x *= f;
	y *= f;
	z *= f;
	return *this;
	}


NX_INLINE NxVec3& NxVec3::operator /=(NxReal f)
	{
	f = 1.0f/f;
	x *= f;
	y *= f;
	z *= f;
	return *this;
	}


NX_INLINE NxVec3 NxVec3::cross(const NxVec3& v) const
	{
	NxVec3 temp;
	temp.cross(*this,v);
	return temp;
	}


NX_INLINE NxVec3 NxVec3::operator^(const NxVec3& v) const
	{
	NxVec3 temp;
	temp.cross(*this,v);
	return temp;
	}


NX_INLINE NxReal NxVec3::operator|(const NxVec3& v) const
	{
	return x * v.x + y * v.y + z * v.z;
	}

/**
scalar pre-multiplication
*/

NX_INLINE NxVec3 operator *(NxReal f, const NxVec3& v)
	{
	return NxVec3(f * v.x, f * v.y, f * v.z);
	}

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
