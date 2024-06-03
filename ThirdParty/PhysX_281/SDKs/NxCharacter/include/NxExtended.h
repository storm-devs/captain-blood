#ifndef NX_PHYSICS_NXBIG
#define NX_PHYSICS_NXBIG
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

// This needs to be included in Foundation just for the debug renderer

#include "Nxf.h"
#include "NxVec3.h"
#include "NxQuat.h"
#include "NxMat33.h"
#include "NxMat34.h"

// This has to be done here since it also changes the top-level "Nx" API (as well as "Nv" and "Np" ones)
#define NX_BIG_WORLDS

#ifdef NX_BIG_WORLDS
	typedef	double	Extended;
	#define	NX_MAX_EXTENDED	NX_MAX_F64
	#define	NX_MIN_EXTENDED	NX_MIN_F64
	#define ExtendedAbs(x)	fabs(x)

	struct NxExtendedVec3
	{
	NX_INLINE	NxExtendedVec3()															{}
	NX_INLINE	NxExtendedVec3(Extended _x, Extended _y, Extended _z) : x(_x), y(_y), z(_z)	{}

	NX_INLINE	NX_BOOL isZero()	const
		{
		if(x!=0.0 || y!=0.0 || z!=0.0)	return NX_FALSE;
		return NX_TRUE;
		}

	NX_INLINE Extended	dot(const NxVec3& v) const
		{
		return x * v.x + y * v.y + z * v.z;
		}

	NX_INLINE Extended	distance(const NxExtendedVec3& v) const
		{
		Extended dx = x - v.x;
		Extended dy = y - v.y;
		Extended dz = z - v.z;
		return NxMath::sqrt(dx * dx + dy * dy + dz * dz);
		}

	NX_INLINE	Extended distanceSquared(const NxExtendedVec3& v) const
		{
		Extended dx = x - v.x;
		Extended dy = y - v.y;
		Extended dz = z - v.z;
		return dx * dx + dy * dy + dz * dz;
		}

	NX_INLINE Extended magnitudeSquared() const
		{
		return x * x + y * y + z * z;
		}

	NX_INLINE Extended magnitude() const
		{
		return NxMath::sqrt(x * x + y * y + z * z);
		}

	NX_INLINE	Extended	normalize()
		{
		Extended m = magnitude();
		if (m)
			{
			const Extended il =  Extended(1.0) / m;
			x *= il;
			y *= il;
			z *= il;
			}
		return m;
		}

	NX_INLINE	bool isFinite()	const
		{
		return NxMath::isFinite(x) && NxMath::isFinite(y) && NxMath::isFinite(z);
		}

	NX_INLINE	void max(const NxExtendedVec3& v)
		{
		if (x < v.x) x = v.x;
		if (y < v.y) y = v.y;
		if (z < v.z) z = v.z;
		}

	 
	NX_INLINE	void min(const NxExtendedVec3& v)
		{
		if (x > v.x) x = v.x;
		if (y > v.y) y = v.y;
		if (z > v.z) z = v.z;
		}

	NX_INLINE	void	set(Extended x, Extended y, Extended z)
		{
		this->x = x;
		this->y = y;
		this->z = z;
		}

	NX_INLINE void	zero()
		{
		x = y = z = 0.0;
		}

	NX_INLINE void	setPlusInfinity()
		{
		x = y = z = NX_MAX_EXTENDED;
		}
	 
	NX_INLINE void	setMinusInfinity()
		{
		x = y = z = NX_MIN_EXTENDED;
		}

	NX_INLINE void	cross(const NxExtendedVec3& left, const NxVec3& right)
		{
		// temps needed in case left or right is this.
		Extended a = (left.y * right.z) - (left.z * right.y);
		Extended b = (left.z * right.x) - (left.x * right.z);
		Extended c = (left.x * right.y) - (left.y * right.x);

		x = a;
		y = b;
		z = c;
		}

	NX_INLINE void	cross(const NxExtendedVec3& left, const NxExtendedVec3& right)
		{
		// temps needed in case left or right is this.
		Extended a = (left.y * right.z) - (left.z * right.y);
		Extended b = (left.z * right.x) - (left.x * right.z);
		Extended c = (left.x * right.y) - (left.y * right.x);

		x = a;
		y = b;
		z = c;
		}

	NX_INLINE NxExtendedVec3 cross(const NxExtendedVec3& v) const
		{
		NxExtendedVec3 temp;
		temp.cross(*this,v);
		return temp;
		}

	NX_INLINE void	cross(const NxVec3& left, const NxExtendedVec3& right)
		{
		// temps needed in case left or right is this.
		Extended a = (left.y * right.z) - (left.z * right.y);
		Extended b = (left.z * right.x) - (left.x * right.z);
		Extended c = (left.x * right.y) - (left.y * right.x);

		x = a;
		y = b;
		z = c;
		}

	NX_INLINE	NxExtendedVec3		operator-()		const
		{
		return NxExtendedVec3(-x, -y, -z);
		}

	NX_INLINE	NxExtendedVec3&		operator+=(const NxExtendedVec3& v)
		{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
		}

	NX_INLINE	NxExtendedVec3&		operator-=(const NxExtendedVec3& v)
		{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
		}

	NX_INLINE	NxExtendedVec3&		operator+=(const NxVec3& v)
		{
		x += Extended(v.x);
		y += Extended(v.y);
		z += Extended(v.z);
		return *this;
		}

	NX_INLINE	NxExtendedVec3&		operator-=(const NxVec3& v)
		{
		x -= Extended(v.x);
		y -= Extended(v.y);
		z -= Extended(v.z);
		return *this;
		}

	NX_INLINE	NxExtendedVec3&		operator*=(const NxReal& s)
		{
		x *= Extended(s);
		y *= Extended(s);
		z *= Extended(s);
		return *this;
		}

	NX_INLINE	NxExtendedVec3		operator+(const NxExtendedVec3& v)	const
		{
		return NxExtendedVec3(x + v.x, y + v.y, z + v.z);
		}

	NX_INLINE	NxVec3			operator-(const NxExtendedVec3& v)	const
		{
		return NxVec3(NxReal(x - v.x), NxReal(y - v.y), NxReal(z - v.z));
		}

	NX_INLINE	Extended&			operator[](int index)
		{
		NX_ASSERT(index>=0 && index<=2);
		return (&x)[index];
		}


	NX_INLINE	Extended			operator[](int index) const
		{
		NX_ASSERT(index>=0 && index<=2);
		return (&x)[index];
		}

		Extended x,y,z;
	};


	#include "Nx9F32.h"
	typedef Nx9Real Mat33DataType;
	class NxMat33Shadow
	{
	public:
	Mat33DataType data;
	};

	NX_INLINE void NxMat33_multiply(const NxMat33& m_, const NxExtendedVec3& src, NxExtendedVec3& dst)
		{
		const NxMat33Shadow& m = (const NxMat33Shadow&)m_;	// That's what happens when you use OOP in stupid places

		Extended x,y,z;	//so it works if src == dst
		x = (Extended)m.data.s._11 * src.x + (Extended)m.data.s._12 * src.y + (Extended)m.data.s._13 * src.z;
		y = (Extended)m.data.s._21 * src.x + (Extended)m.data.s._22 * src.y + (Extended)m.data.s._23 * src.z;
		z = (Extended)m.data.s._31 * src.x + (Extended)m.data.s._32 * src.y + (Extended)m.data.s._33 * src.z;

		dst.x = x;
		dst.y = y;
		dst.z = z;	
		}

	NX_INLINE void NxMat33_multiplyByTranspose(const NxMat33& m_, const NxExtendedVec3& src, NxExtendedVec3& dst)
		{
		const NxMat33Shadow& m = (const NxMat33Shadow&)m_;	// That's what happens when you use OOP in stupid places

		Extended x,y,z;	//so it works if src == dst
		x = (Extended)m.data.s._11 * src.x + (Extended)m.data.s._21 * src.y + (Extended)m.data.s._31 * src.z;
		y = (Extended)m.data.s._12 * src.x + (Extended)m.data.s._22 * src.y + (Extended)m.data.s._32 * src.z;
		z = (Extended)m.data.s._13 * src.x + (Extended)m.data.s._23 * src.y + (Extended)m.data.s._33 * src.z;

		dst.x = x;
		dst.y = y;
		dst.z = z;	
		}
#ifdef OBSOLETE
	class Nx9Extended
		{
		
		public:
			struct S
				{
	#ifndef TRANSPOSED_MAT33
				Extended      _11, _12, _13;
				Extended      _21, _22, _23;
				Extended      _31, _32, _33;
	#else
				Extended      _11, _21, _31;
				Extended      _12, _22, _32;
				Extended      _13, _23, _33;
	#endif
				};
		
		union 
			{
			S s;
			Extended m[3][3];
			};
		};

	typedef Nx9Extended Mat33DataType2;	// We don't want the same type here!!
	class NxExtendedMat33
		{
		public:

		NX_INLINE void setColumn(int col, const NxExtendedVec3& v)
			{
		#ifndef TRANSPOSED_MAT33
			data.m[0][col] = v.x;
			data.m[1][col] = v.y;
			data.m[2][col] = v.z;
		#else
			data.m[col][0] = v.x;
			data.m[col][1] = v.y;
			data.m[col][2] = v.z;
		#endif
			}

		NX_INLINE const NxExtendedMat33& operator +=(const NxExtendedMat33& d)
			{
			data.s._11 += d.data.s._11;
			data.s._12 += d.data.s._12;
			data.s._13 += d.data.s._13;

			data.s._21 += d.data.s._21;
			data.s._22 += d.data.s._22;
			data.s._23 += d.data.s._23;

			data.s._31 += d.data.s._31;
			data.s._32 += d.data.s._32;
			data.s._33 += d.data.s._33;
			return *this;
			}

		NX_INLINE void	multiply(const NxVec3& src, NxVec3& dst) const
			{
			Extended x,y,z;	//so it works if src == dst
			x = data.s._11 * src.x + data.s._12 * src.y + data.s._13 * src.z;
			y = data.s._21 * src.x + data.s._22 * src.y + data.s._23 * src.z;
			z = data.s._31 * src.x + data.s._32 * src.y + data.s._33 * src.z;

			dst.x = x;
			dst.y = y;
			dst.z = z;	
			}

		NX_INLINE void zero()
			{
			data.s._11 = Extended(0.0);
			data.s._12 = Extended(0.0);
			data.s._13 = Extended(0.0);

			data.s._21 = Extended(0.0);
			data.s._22 = Extended(0.0);
			data.s._23 = Extended(0.0);

			data.s._31 = Extended(0.0);
			data.s._32 = Extended(0.0);
			data.s._33 = Extended(0.0);
			}

		NX_INLINE void id()
			{
			data.s._11 = Extended(1.0);
			data.s._12 = Extended(0.0);
			data.s._13 = Extended(0.0);

			data.s._21 = Extended(0.0);
			data.s._22 = Extended(1.0);
			data.s._23 = Extended(0.0);

			data.s._31 = Extended(0.0);
			data.s._32 = Extended(0.0);
			data.s._33 = Extended(1.0);
			}

		bool getInverse(NxExtendedMat33& dest) const
			{
			Extended b00,b01,b02,b10,b11,b12,b20,b21,b22;

			b00 = data.s._22*data.s._33-data.s._23*data.s._32;	b01 = data.s._13*data.s._32-data.s._12*data.s._33;	b02 = data.s._12*data.s._23-data.s._13*data.s._22;
			b10 = data.s._23*data.s._31-data.s._21*data.s._33;	b11 = data.s._11*data.s._33-data.s._13*data.s._31;	b12 = data.s._13*data.s._21-data.s._11*data.s._23;
			b20 = data.s._21*data.s._32-data.s._22*data.s._31;	b21 = data.s._12*data.s._31-data.s._11*data.s._32;	b22 = data.s._11*data.s._22-data.s._12*data.s._21;
			


			/*
			compute determinant: 
			NxReal d =   a00*a11*a22 + a01*a12*a20 + a02*a10*a21	- a02*a11*a20 - a01*a10*a22 - a00*a12*a21;
						0				1			2			3				4			5

			this is a subset of the multiplies done above:

			NxReal d = b00*a00				+		b01*a10						 + b02 * a20;
			NxReal d = (a11*a22-a12*a21)*a00 +		(a02*a21-a01*a22)*a10		 + (a01*a12-a02*a11) * a20;

			NxReal d = a11*a22*a00-a12*a21*a00 +		a02*a21*a10-a01*a22*a10		 + a01*a12*a20-a02*a11*a20;
					0			5					2			4					1			3
			*/

			Extended d = b00*data.s._11		+		b01*data.s._21				 + b02 * data.s._31;
			
			if (d == Extended(0.0))		//singular?
				{
				dest.id();
				return false;
				}
			
			d = Extended(1.0)/d;

			//only do assignment at the end, in case dest == this:


			dest.data.s._11 = b00*d; dest.data.s._12 = b01*d; dest.data.s._13 = b02*d;
			dest.data.s._21 = b10*d; dest.data.s._22 = b11*d; dest.data.s._23 = b12*d;
			dest.data.s._31 = b20*d; dest.data.s._32 = b21*d; dest.data.s._33 = b22*d;

			return true;
			}

		Mat33DataType2 data;
		};
#endif

	struct NxExtendedMat34
	{
		NX_INLINE	NxExtendedMat34()														{}
		NX_INLINE	NxExtendedMat34(const NxMat33& M_, const NxExtendedVec3& t_) : M(M_), t(t_)	{}
		NX_INLINE	explicit NxExtendedMat34(bool init/* = true*/)
			{
			if (init)
				{
				t.zero();
				M.id();
				}
			}

		NX_INLINE bool	isFinite() const
			{
			if(!M.isFinite())	return false;
			if(!t.isFinite())	return false;
			return true;
			}

		NX_INLINE	void multiply(const NxVec3& src, NxExtendedVec3& dst) const
			{
			dst = t;
			dst += M * src;
			}

		NX_INLINE	void multiply(const NxExtendedVec3& src, NxExtendedVec3& dst) const
			{
//			dst = t;
//			dst += M * src;
			NxMat33_multiply(M, src, dst);
			dst += t;
			}

		NX_INLINE	void multiply(const NxExtendedMat34& left, const NxExtendedMat34& right)
			{
			//[aR at] * [bR bt] = [aR * bR		aR * bt + at]  NOTE: order of operations important so it works when this ?= left ?= right.
//			t = left.M * right.t + left.t;
			NxMat33_multiply(left.M, right.t, t);
			t+=left.t;
			M.multiply(left.M, right.M);
			}

		NX_INLINE	void multiply(const NxExtendedMat34& left, const NxMat34& right)
			{
			//[aR at] * [bR bt] = [aR * bR		aR * bt + at]  NOTE: order of operations important so it works when this ?= left ?= right.
			NxVec3 tmp = left.M * right.t;
			t = NxExtendedVec3(tmp.x, tmp.y, tmp.z) + left.t;
			M.multiply(left.M, right.M);
			}

		NX_INLINE	void multiplyByInverseRT(const NxExtendedVec3& src, NxVec3& dst) const
			{
			//dst = M' * src - M' * t = M' * (src - t)
			M.multiplyByTranspose(src - t, dst);
			}

		NX_INLINE	void multiplyByInverseRT(const NxExtendedVec3& src, NxExtendedVec3& dst) const
			{
			//dst = M' * src - M' * t = M' * (src - t)
//			M.multiplyByTranspose(src - t, dst);
			NxExtendedVec3 tmp;	// PT: this one might be not needed (ie could be NxVec3?)
			tmp.x = src.x - t.x;
			tmp.y = src.y - t.y;
			tmp.z = src.z - t.z;
			NxMat33_multiplyByTranspose(M, tmp, dst);
			}

		NX_INLINE	void multiplyInverseRTRight(const NxExtendedMat34& left, const NxMat34& right)
			{
			//[aR at] * [bR' -bR'*bt] = [aR * bR'		-aR * bR' * bt + at]	NOTE: order of operations important so it works when this ?= left ?= right.
			M.multiplyTransposeRight(left.M, right.M);
			NxVec3 tmp = M * right.t;
			t = left.t;
			t -= NxExtendedVec3(tmp.x, tmp.y, tmp.z);
			}

		NX_INLINE	void multiplyInverseRTLeft(const NxExtendedMat34& left, const NxExtendedMat34& right)
			{
			//[aR' -aR'*at] * [bR bt] = [aR' * bR		aR' * bt  - aR'*at]	//aR' ( bt  - at )	NOTE: order of operations important so it works when this ?= left ?= right.
			NxExtendedVec3 tmp = right.t;
			tmp -= left.t;
			NxMat33_multiplyByTranspose(left.M, tmp, t);
//			t = left.M % (right.t - left.t);
			M.multiplyTransposeLeft(left.M, right.M);
			}

		NX_INLINE	void id()
			{
			M.id();
			t.zero();
			}

		NX_INLINE NxExtendedMat34 operator*  (const NxExtendedMat34& right) const
			{
			NxExtendedMat34 dest;
			dest.multiply(*this, right);
			return dest;
			}

		NX_INLINE NxExtendedMat34 operator*  (const NxMat34& right) const
			{
			NxExtendedMat34 dest;
			dest.multiply(*this, right);
			return dest;
			}

		NX_INLINE NxExtendedVec3 operator*  (const NxExtendedVec3& src) const
			{
			NxExtendedVec3 dest;
			multiply(src, dest);
			return dest;
			}

		NX_INLINE NxVec3 operator%  (const NxExtendedVec3& src) const
			{
			NxVec3 dest;
			multiplyByInverseRT(src, dest);
			return dest;
			}

		NX_INLINE NxExtendedVec3 operator_percent(const NxExtendedVec3& src) const
			{
			NxExtendedVec3 dest;
			multiplyByInverseRT(src, dest);
			return dest;
			}

		NxMat33		M;
		NxExtendedVec3	t;
	};

	NX_INLINE NxExtendedVec3 NxMat34_multiplyByInverseRT(const NxMat34& m, const NxExtendedVec3& src)
		{
		NxExtendedVec3 dest;
//		multiplyByInverseRT(src, dest);
		NxExtendedVec3 tmp = src;
		tmp -= m.t;
		NxMat33_multiplyByTranspose(m.M, tmp, dest);
		return dest;
		}

	class NxExtendedBox
	{
		public:
		NX_INLINE	NxExtendedBox()	{}
		NX_INLINE	NxExtendedBox(const NxExtendedVec3& _center, const NxVec3& _extents, const NxMat33& _rot) : center(_center), extents(_extents), rot(_rot){}
		NX_INLINE	~NxExtendedBox()	{}

		NxExtendedVec3	center;
		NxVec3		extents;
		NxMat33		rot;
	};

	class NxExtendedSphere
	{
		public:
		NX_INLINE NxExtendedSphere()																				{}
		NX_INLINE ~NxExtendedSphere()																			{}
		NX_INLINE NxExtendedSphere(const NxExtendedVec3& _center, NxF32 _radius) : center(_center), radius(_radius)	{}
		NX_INLINE NxExtendedSphere(const NxExtendedSphere& sphere) : center(sphere.center), radius(sphere.radius)		{}

		NxExtendedVec3	center;		//!< Sphere's center
		NxF32		radius;		//!< Sphere's radius
	};

	struct NxExtendedSegment
	{
		NX_INLINE const NxExtendedVec3& getOrigin() const
			{
			return p0;
			}

		NX_INLINE void computeDirection(NxVec3& dir) const
			{
			dir = p1 - p0;
			}

	NX_INLINE void computePoint(NxExtendedVec3& pt, Extended t) const
		{
		pt.x = p0.x + t * (p1.x - p0.x);
		pt.y = p0.y + t * (p1.y - p0.y);
		pt.z = p0.z + t * (p1.z - p0.z);
		}

		NxExtendedVec3	p0;		//!< Start of segment
		NxExtendedVec3	p1;		//!< End of segment
	};

	struct NxExtendedCapsule : public NxExtendedSegment
	{
		NxReal	radius;
	};

	struct NxExtendedBounds3
	{
		NX_INLINE NxExtendedBounds3()
			{
			}

		NX_INLINE void setEmpty()
			{
			// We know use this particular pattern for empty boxes
			set(NX_MAX_EXTENDED, NX_MAX_EXTENDED, NX_MAX_EXTENDED,
				NX_MIN_EXTENDED, NX_MIN_EXTENDED, NX_MIN_EXTENDED);
			}

		NX_INLINE void	set(Extended minx, Extended miny, Extended minz, Extended maxx, Extended maxy, Extended maxz)
			{
			min.set(minx, miny, minz);
			max.set(maxx, maxy, maxz);
			}

		NX_INLINE void	set(const NxExtendedVec3& _min, const NxExtendedVec3& _max)
			{
			min = _min;
			max = _max;
			}

		NX_INLINE void	setCenterExtents(const NxExtendedVec3& c, const NxVec3& e)
			{
			min = c;	min -= e;
			max = c;	max += e;
			}

		NX_INLINE void	getCenter(NxExtendedVec3& center) const
			{
			center = min + max;
			center *= 0.5;
			}

		NX_INLINE Extended	getCenter(NxU32 axis)	const
			{
			return (max[axis] + min[axis])*0.5;
			}

		NX_INLINE void	getExtents(NxVec3& extents) const
			{
			extents = max - min;
			extents *= NxReal(0.5);
			}

		NX_INLINE Extended	getExtents(NxU32 axis)	const
			{
			return (max[axis] - min[axis])*0.5;
			}

		NX_INLINE bool	intersect(const NxExtendedBounds3& b) const
			{
			if ((b.min.x > max.x) || (min.x > b.max.x)) return false;
			if ((b.min.y > max.y) || (min.y > b.max.y)) return false;
			if ((b.min.z > max.z) || (min.z > b.max.z)) return false;
			return true;
			}

		NX_INLINE void	boundsOfOBB(const NxMat33& orientation, const NxExtendedVec3& translation, const NxVec3& halfDims)
			{
			NxReal dimx = halfDims[0];
			NxReal dimy = halfDims[1];
			NxReal dimz = halfDims[2];

			NxReal x = NxMath::abs(orientation(0,0) * dimx) + NxMath::abs(orientation(0,1) * dimy) + NxMath::abs(orientation(0,2) * dimz);
			NxReal y = NxMath::abs(orientation(1,0) * dimx) + NxMath::abs(orientation(1,1) * dimy) + NxMath::abs(orientation(1,2) * dimz);
			NxReal z = NxMath::abs(orientation(2,0) * dimx) + NxMath::abs(orientation(2,1) * dimy) + NxMath::abs(orientation(2,2) * dimz);

			set(Extended(-x) + translation[0], Extended(-y) + translation[1], Extended(-z) + translation[2], Extended(x) + translation[0], Extended(y) + translation[1], Extended(z) + translation[2]);
			}

		NX_INLINE void	transform(const NxMat33& orientation, const NxExtendedVec3& translation)
			{
			// convert to center and extents form
			NxExtendedVec3 center;
			NxVec3 extents;
			getCenter(center);
			getExtents(extents);

//			center = orientation * center + translation;
			NxMat33_multiply(orientation, center, center);
			center += translation;

			boundsOfOBB(orientation, center, extents);
			}

		NX_INLINE void	add(const NxExtendedBounds3& b2)
			{
			// - if we're empty, min = MAX,MAX,MAX => min will be b2 in all cases => it will copy b2, ok
			// - if b2 is empty, the opposite happens => keep us unchanged => ok
			// => same behaviour as before, automatically
			min.min(b2.min);
			max.max(b2.max);
			}

		NX_INLINE bool	isInside(const NxExtendedBounds3& box) const
			{
			if(box.min.x > min.x)	return false;
			if(box.min.y > min.y)	return false;
			if(box.min.z > min.z)	return false;
			if(box.max.x < max.x)	return false;
			if(box.max.y < max.y)	return false;
			if(box.max.z < max.z)	return false;
			return true;
			}

		NX_INLINE void	scale(NxF32 scale)
			{
			NxExtendedVec3 center;	getCenter(center);
			NxVec3 extents;	getExtents(extents);
			setCenterExtents(center, extents * scale);
			}

		NxExtendedVec3 min, max;
	};

	struct NxExtendedRay
	{
		NxExtendedVec3	orig;	//!< Ray origin
		NxVec3		dir;	//!< Normalized direction
	};

	class NxExtendedPlane
	{
	public:
	NX_INLINE NxExtendedPlane()	{}
	NX_INLINE ~NxExtendedPlane()	{}

	NX_INLINE Extended distance(const NxExtendedVec3& p) const
		{
		// Valid for plane equation a*x + b*y + c*z + d = 0
		return p.dot(normal) + d;
		}

	NX_INLINE NxExtendedVec3 pointInPlane() const
		{
		// Project origin (0,0,0) to plane:
		// (0) - normal * distance(0) = - normal * ((p|(0)) + d) = -normal*d
		return NxExtendedVec3(-normal.x * d, -normal.y * d, -normal.z * d);
//		return normal * d;
		}

	NxVec3		normal;		//!< The normal to the plane
	Extended	d;			//!< The distance from the origin
	};


	NX_INLINE void NxGetColumn(const NxMat33& m, int col, NxExtendedVec3& v)
		{
		NxVec3 tmp;
		m.getColumn(col, tmp);
		v.x = tmp.x;
		v.y = tmp.y;
		v.z = tmp.z;
		}
#else
	// Big worlds not defined

	typedef	NxBox		NxExtendedBox;
	typedef	NxSphere	NxExtendedSphere;
	typedef NxSegment	NxExtendedSegment;
	typedef NxCapsule	NxExtendedCapsule;
	typedef	NxBounds3	NxExtendedBounds3;
	typedef NxPlane		NxExtendedPlane;
	typedef	NxMat34		NxExtendedMat34;
	typedef	NxVec3		NxExtendedVec3;
	typedef	NxReal		Extended;
	#define	NX_MAX_EXTENDED	NX_MAX_F32
	#define	NX_MIN_EXTENDED	NX_MIN_F32
	#define ExtendedAbs(x)	fabsf(x)
#endif

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
