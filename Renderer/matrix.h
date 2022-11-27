#pragma once
#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <string>
#include <complex>

using namespace std;

/*
* Class representing an (mxn)-matrix.
*/
template<typename F = double> class matrix
{
protected:
	int rows;
	int cols;
	std::vector<std::vector<F>> arr;

public:
	//constructors
	matrix<F>();
	matrix<F>(const matrix& A);
	matrix<F>(const std::vector<std::vector<F>>& x);

	matrix<F>(const initializer_list<F>& v);
	matrix<F>(const vector<F>& v);

	matrix<F>(const std::initializer_list<matrix>& col_list);
	matrix<F>(std::vector<matrix> col_list);

	//operator overloads
	matrix operator += (matrix const& other) const;
 	matrix operator + (matrix const& other) const;
	matrix operator - (matrix const& other) const;
	matrix operator * (matrix const& other) const;
	matrix operator * (F const& c) const;
	inline std::vector<F> operator [] (int const& index) { return arr[index]; }
	bool operator == (matrix const& other);

	//static functions
	static matrix id(int n);
	static matrix zero(int m, int n);
	static matrix unit(int m, int n, int x, int y);
	static matrix std_basis(int dim, int j);
	std::string print();

	//get fields
	int get_rows() { return rows; }
	int get_cols() { return cols; }
	std::vector<std::vector<F>> get_arr() { return arr; }

	//shit
	void set(int i, int j, F val) { this->arr[i][j] = val; }
	matrix comp(int i, int j = 0);
	matrix row(int i);
	matrix col(int i);
	matrix row_add(int r1, F c, int r2);
	matrix row_mult(int r, F c);
	matrix row_swap(int r1, int r2);
	matrix t();
	matrix submatrix(int m, int n);
	
};

#include "matrix.hpp"

#endif


