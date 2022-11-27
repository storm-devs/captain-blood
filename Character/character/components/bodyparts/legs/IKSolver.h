#ifndef KEN_PERLIN_IK_SOLVER
#define KEN_PERLIN_IK_SOLVER

#include <math.h>
#include <stdio.h>
#include <stdlib.h>



class IKSolver
{
public:
//-------- SOLVE TWO LINK INVERSE KINEMATICS -------------
// Author: Ken Perlin
//
// Given a two link joint from [0,0,0] to end effector position P,
// let link lengths be a and b, and let norm |P| = c.  Clearly a+b >= c.
//
// Problem: find a "knee" position Q such that |Q| = a and |P-Q| = b.
//
// In the case of a point on the x axis R = [c,0,0], there is a
// closed form solution S = [d,e,0], where |S| = a and |R-S| = b:
//
//    d2+e2 = a2                  -- because |S| = a
//    (c-d)2+e2 = b2              -- because |R-S| = b
//
//    c2-2cd+d2+e2 = b2           -- combine the two equations
//    c2-2cd = b2 - a2
//    c-2d = (b2-a2)/c
//    d - c/2 = (a2-b2)/c / 2
//
//    d = (c + (a2-b2/c) / 2      -- to solve for d and e.
//    e = sqrt(a2-d2)

   static float findD(float a, float b, float c);
   static float findE(float a, float d);

// This leads to a solution to the more general problem:
//
//   (1) R = Mfwd(P)         -- rotate P onto the x axis
//   (2) Solve for S
//   (3) Q = Minv(S)         -- rotate back again

   static float Mfwd[3][3];
   static float Minv[3][3];

   static bool solve(float A, float B, float const P[], float const D[], float Q[]);

// If "knee" position Q needs to be as close as possible to some point D,
// then choose M such that M(D) is in the y>0 half of the z=0 plane.
//
// Given that constraint, define the forward and inverse of M as follows:

   static void defineM(float const P[], float const D[]);

//------------ GENERAL VECTOR MATH SUPPORT -----------

   static float dot(float const a[], float const b[]);

   static float length(float const v[]);

   static void normalize(float v[]);

   static void cross(float const a[], float const b[], float c[]);

   static void rot(float const M[3][3], float const src[], float dst[]);
};


#endif