#ifndef TYPEDEF_HPP
#define TYPEDEF_HPP

#include<complex>
#include<array>
#include<vector>
#include<string>

typedef std::string str; //Classical std string.

typedef std::array<double, 2> dvec2; //Vector of 2 doubles.
typedef std::array<double, 3> dvec3; //Vector of 3 doubles.
typedef std::array<double, 4> dvec4; //Vector of 4 doubles.
typedef std::array<double, 6> dvec6; //Vector of 6 doubles.

typedef std::array<int, 3> ivec3; //Vector of 3 ints.
typedef std::array<int, 4> ivec4; //Vector of 4 ints.
typedef std::array<int, 6> ivec6; //Vector of 6 ints.
typedef std::array<int, 9> ivec9; //Vector of 9 ints.

typedef std::vector<double> dvec; //Vector of n doubles.
typedef std::vector<int> ivec; //Vector of n ints.
typedef std::vector<str> strvec; //Vector of n std strings.

typedef std::vector<ivec3> imatnx3; //nx3 matrix of ints.
typedef std::vector<ivec4> imatnx4; //nx4 matrix of ints.
typedef std::vector<ivec6> imatnx6; //nx6 matrix of ints.
typedef std::vector<ivec9> imatnx9; //nx9 matrix of ints.

typedef std::array<dvec3, 3> dmat3; //3x3 matrix of doubles.
typedef std::vector<dvec2> dmatnx2; //nx2 matrix of doubles.
typedef std::vector<dvec3> dmatnx3; //nx3 matrix of doubles.

typedef std::vector<dvec> dmat; //nxn matrix of doubles.
typedef std::vector<ivec> imat; //nxn matrix of ints.

typedef std::vector<dmat3> dtensnx3x3; //nx3x3 tensor of doubles (basically n floors of 3x3 matrices of doubles, T[(0)-(n-1)][0-2][0-2]).
typedef std::vector<dmat> dtens; // nxnxn tensor of doubles (T[(0)-(n-1)][(0)-(n-1)][(0)-(n-1)]).

typedef std::complex<double> dcomplex; //Complex double (double + i*double).
typedef std::vector<dcomplex> dcomplexvec; //Vector of n complex doubles.

typedef std::vector<bool> bvec; //Vector of n booleans.

#endif