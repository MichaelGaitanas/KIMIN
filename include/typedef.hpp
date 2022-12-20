#ifndef TYPEDEF_HPP
#define TYPEDEF_HPP

#include<complex>
#include<array>
#include<vector>
#include<string>

typedef std::string str; //classical std string

typedef std::array<double, 3> dvec3; //vector of 3 doubles
typedef std::array<double, 4> dvec4; //vector of 4 doubles
typedef std::array<double, 6> dvec6; //vector of 6 doubles

typedef std::array<int, 3> ivec3; //vector of 3 ints
typedef std::array<int, 4> ivec4; //vector of 4 ints
typedef std::array<int, 6> ivec6; //vector of 6 ints

typedef std::vector<double> dvec; //vector of n doubles
typedef std::vector<int> ivec; //vector of n ints
typedef std::vector<str> strvec; //vector of n std strings

typedef std::vector<ivec3> imatnx3; //nx3 matrix of ints
typedef std::vector<ivec4> imatnx4; //nx4 matrix of ints
typedef std::vector<ivec6> imatnx6; //nx6 matrix of ints

typedef std::array<dvec3, 3> dmat3; //3x3 matrix of doubles
typedef std::vector<dvec3> dmatnx3; //nx3 matrix of doubles

typedef std::vector<dvec> dmat; //nxn matrix of doubles
typedef std::vector<ivec> imat; //nxn matrix of ints

typedef std::vector<dmat3> dtensnx3x3; //nx3x3 tensor of doubles (basically n floors of 3x3 matrices of doubles)
typedef std::vector<dmat> dtens; // nxnxn tensor of doubles

typedef std::complex<double> dcomplex; //complex double (double + i*double)
typedef std::vector<dcomplex> dcomplexvec; //vector of n complex doubles

typedef std::vector<bool> bvec; //vector of n complex doubles

#endif