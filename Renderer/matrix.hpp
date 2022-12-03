#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "matrix.h"
#include <string>
#include <assert.h>
#include <cassert>
#include <complex>
#include <iostream>

using namespace std;

/*
template<typename F>
matrix<F>::matrix(F* p, int m, int n) {
	rows = m;
	cols = n;
	arr = vector<vector<F>>(m, vector<F>(n));

	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			arr[i][j] = (*(p + i*n + j));
		}
	}
}
*/

//CONSTRUCTORS

/*
* Default constructor. Creates matrix of size zero.
*/
template<typename F> 
matrix<F>::matrix() {
	rows = 0;
	cols = 0;
}

/*
* Copy constructor.
*/
template<typename F>
matrix<F>::matrix(const matrix<F>& A) {
	arr = A.arr;
	rows = A.rows;
	cols = A.cols;
	debug_setup();
}

/*
* Initializes matrix as (nx1) column vector from vector;
*
* @param v F list
*/
template<typename F> 
matrix<F>::matrix(const std::initializer_list<F>& v)
{
	this->cols = 1;

	int n = v.size();
	this->rows = n;
	this->arr = vector<vector<F>>(n);

	int i = 0;
	for (F x : v) {
		this->arr[i] = vector<F>({ x });
		i++;
	}
	debug_setup();
}

/*
* Initializes matrix as (nx1) column vector from vector;
*
* @param v F list
*/
template<typename F> 
matrix<F>::matrix(const vector<F>& v)
{
	int n = v.size();
	this->cols = 1;
	this->rows = n;
	this->arr = vector<vector<F>>(n);

	for (int i = 0; i < n; i ++) { this->arr[i] = vector<F>({v[i]}); }
	debug_setup();
}

/*
* Initializes matrix from two dimensional vector.
* @param x two dimensional vector.
*/
template<typename F> 
matrix<F>::matrix(const vector<vector<F>>& x) {
	rows = x.size();
	cols = x[0].size();
	arr = x;
	debug_setup();
}

/*
* Initializes matrix from list of matrices.  Places them side by side.
*/
template<typename F>
matrix<F>::matrix(const std::initializer_list<matrix>& col_list)
{
	for (matrix v : col_list) {
		arr.push_back(v.t().arr[0]);
	}
	arr = matrix(arr).t().arr;
	cols = arr[0].size();
	rows = arr.size();
	debug_setup();
}

/*
* Initializes matrix from list of matrices.  Places them side by side.
*/
template<typename F>
matrix<F>::matrix(std::vector<matrix> col_list)
{
	for (matrix v : col_list) {
		arr.push_back(v.t().arr[0]);
	}
	arr = matrix(arr).t().arr;
	cols = arr[0].size();
	rows = arr.size();
	debug_setup();
}

//OPERATORS
template<typename F>
inline matrix<F> matrix<F>::operator += (matrix const& other) const
{
	auto sum = *this + other;
	return sum;
}

template<typename F>
inline matrix<F> matrix<F>::operator + (matrix const& other) const {
	std::vector<vector<F>> temp = arr;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			temp[i][j] += (F)other.arr[i][j];
		}
	}
	return matrix(temp);
}

template<typename F> 
inline matrix<F> matrix<F>::operator - (matrix const& other) const{
	vector<vector<F>> temp = arr;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			temp[i][j] -= (F)other.arr[i][j];
		}
	}	
	return matrix(temp);

}

/*
* Multiplication operator for scalars.
*/
template<typename F> 
inline matrix<F> matrix<F>::operator * (F const& c) const {
	vector<vector<F>> temp = vector<vector<F>>(arr);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			temp[i][j] *= (F)c;
		}
	}

	return matrix(temp);
}

/*
* Multiplication operator for matrices.
*/
template<typename F> 
inline matrix<F> matrix<F>::operator * (matrix const& other) const {

	vector<vector<F>> temp(rows,vector<F>(other.cols));
	//F* arr_begin = this->arr.data;

	for (int j = 0; j < other.cols; j++) {
		for (int i = 0; i < rows; i++) {
			for (int k = 0; k < cols; k++) {
				temp[i][j] += arr[i][k]*(F)other.arr[k][j];
			}
		}
	}

	return matrix(temp);
}

template<typename F> 
bool matrix<F>::operator == (matrix const& other) {
	if (arr == other.arr) {
		return true;
	}
	return false;
}

/*
* Returns string representation of matrix
*/
template<typename F>  
string matrix<F>::print() {
	for (std::vector<F> v : arr) {
		for (F x : v) {
			std::cout << x << ", ";
		}
		std::cout << "\n";
	}
	return "\n";
}

//STATIC FUNCTIONS

/*
* Returns zero matrix of specified size.
* 
* @param m,n row and column dimensions
* @return (mxn)-matrix with zero in all entries
*/
template<typename F> 
matrix<F> matrix<F>::zero(int m, int n) {
	vector<vector<F>> mat;

	for (int i = 0; i < m; i++) {
		vector<F> row;
		for (int j = 0; j < n; j++) {
			row.push_back(0);
		}
		mat.push_back(row);
	}
	return matrix(mat);
}

/*
* Returns identity matrix of specified size.
* 
* @param n size
* @return identity matrix of size n
*/
template<typename F> 
matrix<F> matrix<F>::id(int n) {
	vector<vector<F>> mat;

	for (int i = 0; i < n; i++) {
		vector<F> row;
		for (int j = 0; j < n; j++) {
			if (i == j) {
				row.push_back(1);
			}
			else {
				row.push_back(0);
			}		
		}
		mat.push_back(row);
	}
	return matrix(mat);
}

/*
* Returns matrix of specified size with 1 in a single entry.
* 
* @param m,n row and column dimensions of matrix
* @param x,y row and column index to have 1
* @return (mxn)-matrix with 1 in (x,y)-entry and zeroes elsewhere
*/
template<typename F> 
matrix<F> matrix<F>::unit(int m, int n, int x, int y) {
	vector<vector<F>> mat(m, vector<F>(n));

	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			if (i == x && j == y) {
				mat[i][j] = 1;
			}
			else {
				mat[i][j] = 0;
			}
		}
	}
	return matrix(mat);
}

template<typename F> 
matrix<F> matrix<F>::std_basis(int dim, int j)
{
	return unit(dim,1,j,0);
}

//SHIT
/*
template<typename F>
matrix<F> matrix<F>::aug(matrix& B, int* side)
{
	int rows_new = this->rows + side[1]*side[1] * B.rows; //squaring side[n] avoids abs()
	int cols_new = this->cols + side[0]*side[0] * B.cols;
	F* aug_mat = new F[rows_new][cols_new];

	return matrix();
}
*/

/*
* Returns (1xn)-matrix of i-th row.
* 
* @param i row index
* @return ith row of matrix
*/
template<typename F>
matrix<F> matrix<F>::comp(int i, int j)
{
	return unit(rows, cols, i, j) * this->arr[i][j];
}

template<typename F>
matrix<F> matrix<F>::row(int i) {
	return matrix({arr[i]});
}

/*
* Returns (mx1)-matrix of j-th column.
* 
* @param j column index
* @return j-th row of matrix
*/
template<typename F> 
matrix<F> matrix<F>::col(int j) {
	vector<vector<F>> mat(rows, vector<F>(1));
	for (int i = 0; i < rows; i++) {
		mat[i][0] = arr[i][j];
	}
	return matrix(mat);
}

/*
* Row add operation with scalar mult.
* 
* @param r1 row to add to other row
* @param c scalar to multiply r1 by when adding
* @param r2 row to be added to
* @return matrix with c*r1 added to r2
*/
template<typename F> 
matrix<F> matrix<F>::row_add(int r1, F c, int r2) {
	vector<vector<F>> mat = arr;
	mat[r2] = (matrix({ arr[r2] }) + matrix({ arr[r1] }) * c).arr[0];
	return matrix(mat);
}

/*
* Scalar mult operation on row.
*
* @param r row to multiply
* @param c scalar to multiply row by
* @return matrix with row r multiplied by c
*/
template<typename F> 
matrix<F> matrix<F>::row_mult(int r, F c) {
	vector<vector<F>> mat = arr;
	mat[r] =  (matrix({ arr[r] } ) * c).arr[0];
	return matrix(mat);
}

/*
* Row swap operation.
*		
* @param r1,r2 row indices to swap
* @return matrix with rows r1 and r2 swapped
*/
template<typename F> 
matrix<F> matrix<F>::row_swap(int r1,int r2) {
	vector<vector<F>> mat = arr;
	mat[r1] = arr[r2];
	mat[r2] = arr[r1];
	return matrix(mat);
}

/*
* Returns transpose of matrix.  Switches row and column indices of entries
* 
* @return transpose matrix
*/
template<typename F> 
matrix<F> matrix<F>::t() {
	vector<vector<F>> mat(this->cols,vector<F>(this->rows));

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			mat[j][i] = arr[i][j];
		}
	}
	return matrix(mat);
}

/*
* Returns submatrix with m-th row and n-th column removed.
* 
* @param m,n row and column indices
* @return (m-1 x n-1)-matrix which is the original matrix 
*	with mth row and nth col removed
*/
template<typename F> 
matrix<F> matrix<F>::submatrix(int m, int n) {
	assert(m >= 0 && n >= 0 && m <= rows && n <= cols);

	vector<vector<F>> temp(rows - 1, vector<F>(cols - 1));

	int x = 0; 
	for (int i = 0; i < rows; i++) {
		if (i != m) {
			int y = 0;
			for (int j = 0; j < cols; j++) {
				if (j != n) {					
					temp[x][y] = arr[i][j];
					y++;
				}
			}
			x++;
		}
	}
	return matrix(temp);
}

template<typename F>
matrix<F> matrix<F>::select(int* indices, int num_indices) {
	matrix<F> mat_new = matrix::zero(num_indices, num_indices);

	for (int i = 0; i < num_indices; i++) {
		for (int j = 0; j < num_indices; j++) {
			mat_new.set(i, j,(*this)[indices[i]][indices[j]]);
		}
	}
	return mat_new;
}

#endif