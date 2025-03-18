/* Commonly used variable types throughout the whole project. */

#ifndef TYPEDEF_H
#define TYPEDEF_H

#include<array>
#include<vector>
#include<string>
#include<complex>

typedef std::string str; //Classical std string.
typedef std::vector<str> strvec; //Vector of n std strings.

typedef std::vector<bool> bvec; //Vector of n booleans.

typedef std::complex<double> dcomplex; //Complex double (double + i*double).
typedef std::vector<dcomplex> dcomplexvec; //Vector of n complex doubles.

typedef std::array<double, 3> dvec3; //Vector of 3 doubles.
typedef std::array<double, 4> dvec4; //Vector of 4 doubles.
typedef std::array<double, 6> dvec6; //Vector of 6 doubles.
typedef std::array<int, 3> ivec3; //Vector of 3 ints.
typedef std::array<int, 4> ivec4; //Vector of 4 ints.
typedef std::array<int, 6> ivec6; //Vector of 6 ints.
typedef std::array<unsigned int, 2> uvec2; //Vector of 2 unsigned ints.
typedef std::array<unsigned int, 3> uvec3; //Vector of 3 unsigned ints.
typedef std::array<unsigned int, 4> uvec4; //Vector of 4 unsigned ints.
typedef std::vector<double> dvec; //Vector of n doubles.
typedef std::vector<int> ivec; //Vector of n ints.

typedef std::vector<ivec3> imatnx3; //nx3 matrix of ints.
typedef std::vector<ivec4> imatnx4; //nx4 matrix of ints.
typedef std::vector<ivec6> imatnx6; //nx6 matrix of ints.
typedef std::vector<uvec2> umatnx2; //nx2 matrix of unsigned ints.
typedef std::vector<uvec3> umatnx3; //nx3 matrix of unsigned ints.
typedef std::vector<uvec4> umatnx4; //nx4 matrix of unsigned ints.
typedef std::array<dvec3, 3> dmat3; //3x3 matrix of doubles.
typedef std::vector<dvec3> dmatnx3; //nx3 matrix of doubles.
typedef std::vector<dvec> dmat; //nxn matrix of doubles.
typedef std::vector<ivec> imat; //nxn matrix of ints.

typedef std::vector<dmat3> dtensnx3x3; //nx3x3 tensor of doubles (basically n floors of 3x3 matrices of doubles).
typedef std::vector<dmat> dtens; //nxnxn tensor of doubles (T[(0)-(n-1)][(0)-(n-1)][(0)-(n-1)]).

#endif