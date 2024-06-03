#ifndef NX_FOUNDATION_NxQuatT
#define NX_FOUNDATION_NxQuatT
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
#include "NxVec3.h"

/**
\brief This is a quaternion class. For more information on quaternion mathematics
consult a mathematics source on complex numbers.
 
*/

class NxQuat
	{
	public:
	/**
	\brief Default constructor, does not do any initialization.
	*/
	NX_INLINE NxQuat();

	/**
	\brief Copy constructor.
	*/
	NX_INLINE NxQuat(const NxQuat&);

	/**
	\brief copies xyz elements from v, and scalar from w (defaults to 0).
	*/
	NX_INLINE NxQuat(const NxVec3& v, NxReal w = 0);

	/**
	\brief creates from angle-axis representation.

	note that if Angle > 360 the resulting rotation is Angle mod 360.
	
	<b>Unit:</b> Degrees
	*/
	NX_INLINE NxQuat(const NxReal angle, const NxVec3 & axis);

	/**
	\brief Creates from orientation matrix.

	\param[in] m Rotation matrix to extract quaternion from.
	*/
	NX_INLINE NxQuat(const class NxMat33 &m); /* defined in NxMat33.h */


	/**
	\brief Set the quaternion to the identity rotation.
	*/
	NX_INLINE void id();

	/**
	\brief Test if the quaternion is the identity rotation.
	*/
	NX_INLINE bool isIdentityRotation() const;

	//setting:

	/**
	\brief Set the members of the quaternion, in order WXYZ
	*/
	NX_INLINE void setWXYZ(NxReal w, NxReal x, NxReal y, NxReal z);

	/**
	\brief Set the members of the quaternion, in order XYZW
	*/
	NX_INLINE void setXYZW(NxReal x, NxReal y, NxReal z, NxReal w);

	/**
	\brief Set the members of the quaternion, in order WXYZ
	*/
	NX_INLINE void setWXYZ(const NxReal *);

	/**
	\brief Set the members of the quaternion, in order XYZW
	*/
	NX_INLINE void setXYZW(const NxReal *);

	NX_INLINE NxQuat& operator=  (const NxQuat&);

	/**
	\brief Implicitly extends vector by a 0 w element.
	*/
	NX_INLINE NxQuat& operator=  (const NxVec3&);

	NX_INLINE void setx(const NxReal& d);
	NX_INLINE void sety(const NxReal& d);
	NX_INLINE void setz(const NxReal& d);
	NX_INLINE void setw(const NxReal& d);

	NX_INLINE void getWXYZ(NxF32 *) const;
	NX_INLINE void getXYZW(NxF32 *) const;

	NX_INLINE void getWXYZ(NxF64 *) const;
	NX_INLINE void getXYZW(NxF64 *) const;

	/**
	\brief returns true if all elements are finite (not NAN or INF, etc.)
	*/
	NX_INLINE bool isFinite() const;

	/**
	\brief sets to the quat [0,0,0,1]
	*/
	NX_INLINE void zero();

	/**
	\brief creates a random unit quaternion.
	*/
	NX_INLINE void random();
	/**
	\brief creates from angle-axis representation.

	Note that if Angle > 360 the resulting rotation is Angle mod 360.
	
	<b>Unit:</b> Degrees
	*/
	NX_INLINE void fromAngleAxis(NxReal angle, const NxVec3 & axis);

	/**
	\brief Creates from angle-axis representation.

	Axis must be normalized!
	
	<b>Unit:</b> Radians
	*/
	NX_INLINE void fromAngleAxisFast(NxReal AngleRadians, const NxVec3 & axis);

	/**
	\brief Sets this to the opposite rotation of this.
	*/
	NX_INLINE void invert();

	/**
	\brief Fetches the Angle/axis given by the NxQuat.

	<b>Unit:</b> Degrees
	*/
	NX_INLINE void getAngleAxis(NxReal& Angle, NxVec3 & axis) const;

	/**
	\brief Gets the angle between this quat and the identity quaternion.

	<b>Unit:</b> Degrees
	*/
	NX_INLINE NxReal getAngle() const;

	/**
	\brief Gets the angle between this quat and the argument

	<b>Unit:</b> Degrees
	*/
	NX_INLINE NxReal getAngle(const NxQuat &) const;

	/**
	\brief This is the squared 4D vector length, should be 1 for unit quaternions.
	*/
	NX_INLINE NxReal magnitudeSquared() const;

	/**
	\brief returns the scalar product of this and other.
	*/
	NX_INLINE NxReal dot(const NxQuat &other) const;

	//modifiers:
	/**
	\brief maps to the closest unit quaternion.
	*/
	NX_INLINE void normalize();

	/*
	\brief assigns its own conjugate to itself.

	\note for unit quaternions, this is the inverse.
	*/
	NX_INLINE void conjugate();

	/**
	this = a * b
	*/
	NX_INLINE void multiply(const NxQuat& a, const NxQuat& b);

	/**
	this = a * v
	v is interpreted as quat [xyz0]
	*/
	NX_INLINE void multiply(const NxQuat& a, const NxVec3& v);

	/**
	this = slerp(t, a, b)
	*/
	NX_INLINE void slerp(const NxReal t, const NxQuat& a, const NxQuat& b);

	/**
	rotates passed vec by rot expressed by unit quaternion.  overwrites arg with the result.
	*/
	NX_INLINE void rotate(NxVec3 &) const;

	/**
	rotates passed vec by this (assumed unitary)
	*/
	NX_INLINE const NxVec3 rot(const NxVec3 &) const;

	/**
	inverse rotates passed vec by this (assumed unitary)
	*/
	NX_INLINE const NxVec3 invRot(const NxVec3 &) const;

	/**
	transform passed vec by this rotation (assumed unitary) and translation p
	*/
	NX_INLINE const NxVec3 transform(const NxVec3 &v, const NxVec3 &p) const;

	/**
	inverse rotates passed vec by this (assumed unitary)
	*/
	NX_INLINE const NxVec3 invTransform(const NxVec3 &v, const NxVec3 &p) const;


	/**
	rotates passed vec by opposite of rot expressed by unit quaternion.  overwrites arg with the result.
	*/
	NX_INLINE void inverseRotate(NxVec3 &) const;



	/**
	negates all the elements of the quat.  q and -q represent the same rotation.
	*/
	NX_INLINE void negate();
	NX_INLINE NxQuat operator -() const; 

	NX_INLINE NxQuat& operator*= (const NxQuat&);
	NX_INLINE NxQuat& operator+= (const NxQuat&);
	NX_INLINE NxQuat& operator-= (const NxQuat&);
	NX_INLINE NxQuat& operator*= (const NxReal s);

	/** the quaternion elements */
    NxReal x,y,z,w;

	/** quaternion multiplication */
	NX_INLINE NxQuat operator *(const NxQuat &) const; 

	/** quaternion addition */
	NX_INLINE NxQuat operator +(const NxQuat &) const; 

	/** quaternion subtraction */
	NX_INLINE NxQuat operator -(const NxQuat &) const; 

	/** quaternion conjugate */
	NX_INLINE NxQuat operator !() const; 

    /* 
	ops we decided not to implement:
	bool  operator== (const NxQuat&) const;
	NxVec3  operator^  (const NxQuat& r_h_s) const;//same as normal quat rot, but casts itself into a vector.  (doesn't compute w term)
	NxQuat  operator*  (const NxVec3& v) const;//implicitly extends vector by a 0 w element.
	NxQuat  operator*  (const NxReal Scale) const;
	*/

	friend class NxMat33;
	private:
		NX_INLINE NxQuat(NxReal ix, NxReal iy, NxReal iz, NxReal iw);
	};




NX_INLINE NxQuat::NxQuat()
	{
	//nothing
	}


NX_INLINE NxQuat::NxQuat(const NxQuat& q) : x(q.x), y(q.y), z(q.z), w(q.w)
	{
	}


NX_INLINE NxQuat::NxQuat(const NxVec3& v, NxReal s)						// copy constructor, assumes w=0 
	{
	x = v.x;
	y = v.y;
	z = v.z;
	w = s;
	}


NX_INLINE NxQuat::NxQuat(const NxReal angle, const NxVec3 & axis)				// creates a NxQuat from an Angle axis -- note that if Angle > 360 the resulting rotation is Angle mod 360
	{
	fromAngleAxis(angle,axis);
	}


NX_INLINE void NxQuat::id()
	{
	x = NxReal(0);
	y = NxReal(0);
	z = NxReal(0);
	w = NxReal(1);
	}

NX_INLINE  bool NxQuat::isIdentityRotation() const
{
	return x==0 && y==0 && z==0 && fabsf(w)==1;
}


NX_INLINE void NxQuat::setWXYZ(NxReal sw, NxReal sx, NxReal sy, NxReal sz)
	{
	x = sx;
	y = sy;
	z = sz;
	w = sw;
	}


NX_INLINE void NxQuat::setXYZW(NxReal sx, NxReal sy, NxReal sz, NxReal sw)
	{
	x = sx;
	y = sy;
	z = sz;
	w = sw;
	}


NX_INLINE void NxQuat::setWXYZ(const NxReal * d)
	{
	x = d[1];
	y = d[2];
	z = d[3];
	w = d[0];
	}


NX_INLINE void NxQuat::setXYZW(const NxReal * d)
	{
	x = d[0];
	y = d[1];
	z = d[2];
	w = d[3];
	}


NX_INLINE void NxQuat::getWXYZ(NxF32 *d) const
	{
	d[1] = (NxF32)x;
	d[2] = (NxF32)y;
	d[3] = (NxF32)z;
	d[0] = (NxF32)w;
	}


NX_INLINE void NxQuat::getXYZW(NxF32 *d) const
	{
	d[0] = (NxF32)x;
	d[1] = (NxF32)y;
	d[2] = (NxF32)z;
	d[3] = (NxF32)w;
	}


NX_INLINE void NxQuat::getWXYZ(NxF64 *d) const
	{
	d[1] = (NxF64)x;
	d[2] = (NxF64)y;
	d[3] = (NxF64)z;
	d[0] = (NxF64)w;
	}


NX_INLINE void NxQuat::getXYZW(NxF64 *d) const
	{
	d[0] = (NxF64)x;
	d[1] = (NxF64)y;
	d[2] = (NxF64)z;
	d[3] = (NxF64)w;
	}

//const methods
 
NX_INLINE bool NxQuat::isFinite() const
	{
	return NxMath::isFinite(x) 
		&& NxMath::isFinite(y) 
		&& NxMath::isFinite(z)
		&& NxMath::isFinite(w);
	}



NX_INLINE void NxQuat::zero()
	{
	x = NxReal(0.0);
	y = NxReal(0.0);
	z = NxReal(0.0);
	w = NxReal(1.0);
	}


NX_INLINE void NxQuat::negate()
	{
	x = -x;
	y = -y;
	z = -z;
	w = -w;
	}

NX_INLINE NxQuat NxQuat::operator-() const
	{
	return NxQuat(-x,-y,-z,-w);
	}


NX_INLINE void NxQuat::random()
	{
	x = NxMath::rand(NxReal(0.0),NxReal(1.0));
	y = NxMath::rand(NxReal(0.0),NxReal(1.0));
	z = NxMath::rand(NxReal(0.0),NxReal(1.0));
	w = NxMath::rand(NxReal(0.0),NxReal(1.0));
	normalize();
	}


NX_INLINE void NxQuat::fromAngleAxis(NxReal Angle, const NxVec3 & axis)			// set the NxQuat by Angle-axis (see AA constructor)
	{
	x = axis.x;
	y = axis.y;
	z = axis.z;

	// required: Normalize the axis

	const NxReal i_length =  NxReal(1.0) / NxMath::sqrt( x*x + y*y + z*z );
	
	x = x * i_length;
	y = y * i_length;
	z = z * i_length;

	// now make a clQuaternionernion out of it
	NxReal Half = NxMath::degToRad(Angle * NxReal(0.5));

	w = NxMath::cos(Half);//this used to be w/o deg to rad.
	const NxReal sin_theta_over_two = NxMath::sin(Half );
	x = x * sin_theta_over_two;
	y = y * sin_theta_over_two;
	z = z * sin_theta_over_two;
	}

NX_INLINE void NxQuat::fromAngleAxisFast(NxReal AngleRadians, const NxVec3 & axis)
	{
	NxReal s;
	NxMath::sinCos(AngleRadians * 0.5f, s, w);
	x = axis.x * s;
	y = axis.y * s;
	z = axis.z * s;
	}

NX_INLINE void NxQuat::invert()
	{
	x = -x;
	y = -y;
	z = -z;
	}

NX_INLINE void NxQuat::setx(const NxReal& d) 
	{ 
	x = d;
	}


NX_INLINE void NxQuat::sety(const NxReal& d) 
	{ 
	y = d;
	}


NX_INLINE void NxQuat::setz(const NxReal& d) 
	{ 
	z = d;
	}


NX_INLINE void NxQuat::setw(const NxReal& d) 
	{ 
	w = d;
	}


NX_INLINE void NxQuat::getAngleAxis(NxReal& angle, NxVec3 & axis) const
	{
	//return axis and angle of rotation of quaternion
    angle = NxMath::acos(w) * NxReal(2.0);		//this is getAngle()
    NxReal sa = NxMath::sqrt(NxReal(1.0) - w*w);
	if (sa)
		{
		axis.set(x/sa,y/sa,z/sa);
		angle = NxMath::radToDeg(angle);
		}
	else
		axis.set(NxReal(1.0),NxReal(0.0),NxReal(0.0));

	}



NX_INLINE NxReal NxQuat::getAngle() const
	{
	return NxMath::acos(w) * NxReal(2.0);
	}



NX_INLINE NxReal NxQuat::getAngle(const NxQuat & q) const
	{
	return NxMath::acos(dot(q)) * NxReal(2.0);
	}


NX_INLINE NxReal NxQuat::magnitudeSquared() const

//modifiers:
	{
	return x*x + y*y + z*z + w*w;
	}


NX_INLINE NxReal NxQuat::dot(const NxQuat &v) const
	{
	return x * v.x + y * v.y + z * v.z  + w * v.w;
	}


NX_INLINE void NxQuat::normalize()											// convert this NxQuat to a unit clQuaternionernion
	{
	const NxReal mag = NxMath::sqrt(magnitudeSquared());
	if (mag)
		{
		const NxReal imag = NxReal(1.0) / mag;
		
		x *= imag;
		y *= imag;
		z *= imag;
		w *= imag;
		}
	}


NX_INLINE void NxQuat::conjugate()											// convert this NxQuat to a unit clQuaternionernion
	{
	x = -x;
	y = -y;
	z = -z;
	}


NX_INLINE void NxQuat::multiply(const NxQuat& left, const NxQuat& right)		// this = a * b
	{
	NxReal a,b,c,d;

	a =left.w*right.w - left.x*right.x - left.y*right.y - left.z*right.z;
	b =left.w*right.x + right.w*left.x + left.y*right.z - right.y*left.z;
	c =left.w*right.y + right.w*left.y + left.z*right.x - right.z*left.x;
	d =left.w*right.z + right.w*left.z + left.x*right.y - right.x*left.y;

	w = a;
	x = b;
	y = c;
	z = d;
	}


NX_INLINE void NxQuat::multiply(const NxQuat& left, const NxVec3& right)		// this = a * b
	{
	NxReal a,b,c,d;

	a = - left.x*right.x - left.y*right.y - left.z *right.z;
	b =   left.w*right.x + left.y*right.z - right.y*left.z;
	c =   left.w*right.y + left.z*right.x - right.z*left.x;
	d =   left.w*right.z + left.x*right.y - right.x*left.y;

	w = a;
	x = b;
	y = c;
	z = d;
	}

NX_INLINE void NxQuat::slerp(const NxReal t, const NxQuat& left, const NxQuat& right) // this = slerp(t, a, b)
	{
	const NxReal	quatEpsilon = (NxReal(1.0e-8f));

	*this = left;

	NxReal cosine = 
		x * right.x + 
		y * right.y + 
		z * right.z + 
		w * right.w;		//this is left.dot(right)

	NxReal sign = NxReal(1);
	if (cosine < 0)
		{
		cosine = - cosine;
		sign = NxReal(-1);
		}

	NxReal Sin = NxReal(1) - cosine*cosine;

	if(Sin>=quatEpsilon*quatEpsilon)	
		{
		Sin = NxMath::sqrt(Sin);
		const NxReal angle = NxMath::atan2(Sin, cosine);
		const NxReal i_sin_angle = NxReal(1) / Sin;



		NxReal lower_weight = NxMath::sin(angle*(NxReal(1)-t)) * i_sin_angle;
		NxReal upper_weight = NxMath::sin(angle * t) * i_sin_angle * sign;

		w = (w * (lower_weight)) + (right.w * (upper_weight));
		x = (x * (lower_weight)) + (right.x * (upper_weight));
		y = (y * (lower_weight)) + (right.y * (upper_weight));
		z = (z * (lower_weight)) + (right.z * (upper_weight));
		}
	}


NX_INLINE void NxQuat::rotate(NxVec3 & v) const						//rotates passed vec by rot expressed by quaternion.  overwrites arg ith the result.
	{
	//NxReal msq = NxReal(1.0)/magnitudeSquared();	//assume unit quat!
	NxQuat myInverse;
	myInverse.x = -x;//*msq;
	myInverse.y = -y;//*msq;
	myInverse.z = -z;//*msq;
	myInverse.w =  w;//*msq;

	//v = ((*this) * v) ^ myInverse;

	NxQuat left;
	left.multiply(*this,v);
	v.x =left.w*myInverse.x + myInverse.w*left.x + left.y*myInverse.z - myInverse.y*left.z;
	v.y =left.w*myInverse.y + myInverse.w*left.y + left.z*myInverse.x - myInverse.z*left.x;
	v.z =left.w*myInverse.z + myInverse.w*left.z + left.x*myInverse.y - myInverse.x*left.y;
	}


NX_INLINE void NxQuat::inverseRotate(NxVec3 & v) const				//rotates passed vec by opposite of rot expressed by quaternion.  overwrites arg ith the result.
	{
	//NxReal msq = NxReal(1.0)/magnitudeSquared();	//assume unit quat!
	NxQuat myInverse;
	myInverse.x = -x;//*msq;
	myInverse.y = -y;//*msq;
	myInverse.z = -z;//*msq;
	myInverse.w =  w;//*msq;

	//v = (myInverse * v) ^ (*this);
	NxQuat left;
	left.multiply(myInverse,v);
	v.x =left.w*x + w*left.x + left.y*z - y*left.z;
	v.y =left.w*y + w*left.y + left.z*x - z*left.x;
	v.z =left.w*z + w*left.z + left.x*y - x*left.y;
	}


NX_INLINE NxQuat& NxQuat::operator=  (const NxQuat& q)
	{
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;
	return *this;
	}

#if 0
NX_INLINE NxQuat& NxQuat::operator=  (const NxVec3& v)		//implicitly extends vector by a 0 w element.
	{
	x = v.x;
	y = v.y;
	z = v.z;
	w = NxReal(1.0);
	return *this;
	}
#endif

NX_INLINE NxQuat& NxQuat::operator*= (const NxQuat& q)
	{
	NxReal xx[4]; //working Quaternion
	xx[0] = w*q.w - q.x*x - y*q.y - q.z*z;
	xx[1] = w*q.x + q.w*x + y*q.z - q.y*z;
	xx[2] = w*q.y + q.w*y + z*q.x - q.z*x;
	z=w*q.z + q.w*z + x*q.y - q.x*y;

	w = xx[0];
	x = xx[1];
	y = xx[2];
	return *this;
	}


NX_INLINE NxQuat& NxQuat::operator+= (const NxQuat& q)
	{
	x+=q.x;
	y+=q.y;
	z+=q.z;
	w+=q.w;
	return *this;
	}


NX_INLINE NxQuat& NxQuat::operator-= (const NxQuat& q)
	{
	x-=q.x;
	y-=q.y;
	z-=q.z;
	w-=q.w;
	return *this;
	}


NX_INLINE NxQuat& NxQuat::operator*= (const NxReal s)
	{
	x*=s;
	y*=s;
	z*=s;
	w*=s;
	return *this;
	}

NX_INLINE NxQuat::NxQuat(NxReal ix, NxReal iy, NxReal iz, NxReal iw)
{
	x = ix;
	y = iy;
	z = iz;
	w = iw;
}

NX_INLINE NxQuat NxQuat::operator*(const NxQuat &q) const
{
	return NxQuat(w*q.x + q.w*x + y*q.z - q.y*z,
				  w*q.y + q.w*y + z*q.x - q.z*x,
				  w*q.z + q.w*z + x*q.y - q.x*y,
				  w*q.w - x*q.x - y*q.y - z*q.z);
}

NX_INLINE NxQuat NxQuat::operator+(const NxQuat &q) const
{
	return NxQuat(x+q.x,y+q.y,z+q.z,w+q.w);
}

NX_INLINE NxQuat NxQuat::operator-(const NxQuat &q) const
{
	return NxQuat(x-q.x,y-q.y,z-q.z,w-q.w);
}

NX_INLINE NxQuat NxQuat::operator!() const
{
	return NxQuat(-x,-y,-z,w);
}



NX_INLINE const NxVec3 NxQuat::rot(const NxVec3 &v) const
    {
	NxVec3 qv(x,y,z);

	return (v*(w*w-0.5f) + (qv^v)*w + qv*(qv|v))*2;
    }

NX_INLINE const NxVec3 NxQuat::invRot(const NxVec3 &v) const
    {
	NxVec3 qv(x,y,z);

	return (v*(w*w-0.5f) - (qv^v)*w + qv*(qv|v))*2;
    }



NX_INLINE const NxVec3 NxQuat::transform(const NxVec3 &v, const NxVec3 &p) const
    {
	return rot(v)+p;
    }

NX_INLINE const NxVec3 NxQuat::invTransform(const NxVec3 &v, const NxVec3 &p) const
    {
	return invRot(v-p);
    }

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
