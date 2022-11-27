#include "IKSolver.h"

float IKSolver::Mfwd[3][3];
float IKSolver::Minv[3][3];

#define _max(a,b)    (((a) > (b)) ? (a) : (b))
#define _min(a,b)    (((a) < (b)) ? (a) : (b))



float IKSolver::findD(float a, float b, float c)
{
      return _max(0, _min(a, (c + (a*a-b*b)/c) / 2));
}

float IKSolver::findE(float a, float d)
{ 
	return sqrt(a*a-d*d);
}

bool IKSolver::solve(float A, float B, float const P[], float const D[], float Q[])
{
	float R[3];
	defineM(P,D);
	rot(Minv,P,R);
	float d = findD(A,B,length(R));
	float e = findE(A,d);
	float S[3] = {d,e,0};
	rot(Mfwd,S,Q);
	return d > 0 && d < A;
}

void IKSolver::defineM(float const P[], float const D[])
{
    float *X = Minv[0], *Y = Minv[1], *Z = Minv[2];

// Minv defines a coordinate system whose x axis contains P, so X = unit(P).
    int i;
      for (i = 0 ; i < 3 ; i++)
         X[i] = P[i];
      normalize(X);

// Its y axis is perpendicular to P, so Y = unit( E - X(E·X) ).

      float dDOTx = dot(D,X);
      for (i = 0 ; i < 3 ; i++)
         Y[i] = D[i] - dDOTx * X[i];
      normalize(Y);

// Its z axis is perpendicular to both X and Y, so Z = X×Y.

      cross(X,Y,Z);

// Mfwd = (Minv)T, since transposing inverts a rotation matrix.

      for (i = 0 ; i < 3 ; i++)
			{
         Mfwd[i][0] = Minv[0][i];
         Mfwd[i][1] = Minv[1][i];
         Mfwd[i][2] = Minv[2][i];
      }
}

//------------ GENERAL VECTOR MATH SUPPORT -----------

float IKSolver::dot(float const a[], float const b[])
{ 
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

float IKSolver::length(float const v[])
{ 
	return sqrt( dot(v,v) );
}

void IKSolver::normalize(float v[])
{
	float norm = length(v);
	for (int i = 0 ; i < 3 ; i++)
			v[i] /= norm;
}

void IKSolver::cross(float const a[], float const b[], float c[])
{
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
}

void IKSolver::rot(float const M[3][3], float const src[], float dst[])
{
      for (int i = 0 ; i < 3 ; i++)
         dst[i] = dot(M[i],src);
}

