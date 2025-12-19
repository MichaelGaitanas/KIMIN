/* This header contains aliases for the commonly used variable types throughout the whole code. */

#ifndef TYPEDEF_H
#define TYPEDEF_H

#include<array>
#include<vector>
#include<complex>

typedef std::array<double, 2> dvec2; //Vector of 2 doubles.
typedef std::array<double, 3> dvec3; //Vector of 3 doubles.
typedef std::array<double, 4> dvec4; //Vector of 4 doubles.
typedef std::array<double, 6> dvec6; //Vector of 6 doubles.
typedef std::vector<double>   dvec;  //Vector of N doubles.

typedef std::array<unsigned, 2> uvec2; //Vector of 2 unsigned integers.
typedef std::array<unsigned, 3> uvec3; //Vector of 3 unsigned integers.

typedef std::vector<uvec2> umatnx2; //nx2 matrix of unsigned integers.
typedef std::vector<uvec3> umatnx3; //nx3 matrix of unsigned integers.

typedef std::array<dvec3, 3> dmat3; //3x3 matrix of doubles.
typedef std::vector<dvec3>   dmatnx3; //nx3 matrix of doubles.
typedef std::vector<dvec>    dmat; //nxn matrix of doubles.

typedef std::vector<dmat3> dtensnx3x3; //nx3x3 tensor of doubles (basically N floors of 3x3 matrices of doubles).
typedef std::vector<dmat>  dtens; //nxnxn tensor of doubles (T[(0)-(N-1)][(0)-(N-1)][(0)-(N-1)]).

typedef std::vector<bool> bvec; //Vector of N booleans.

typedef std::complex<double>  dcomplex; //Complex double (double + i*double).
typedef std::vector<dcomplex> dcomplexvec; //Vector of N complex doubles.

#endif