#ifndef MATRIXUTILS_HPP
#define MATRIXUTILS_HPP

#include <iostream>
#include <assert.h>
#include <cassert>
#include <vector>
#include "matrix.h"
#include "matrixutils.h"

using namespace std;

/*
* Computes determinant of matrix via laplacian expansion.
*/
template<typename F> 
F det(matrix<F> A) {
	int size = A.get_rows();

	if (size == 1) { return A[0][0]; }

	F value = (F)0;
	for (int i = 0; i < size; i++) {
		value += pow(-1,i) * (A[0][i] * det(A.submatrix(0, i)));
	}

	return value;
}

/*
* Computes adjucate matrix.
*/
template<typename F> 
matrix<F> adj(matrix<F> A) {
	using vec = std::vector<F>;

	int rows = A.get_rows();
	int cols = A.get_cols();

	if (rows != cols) {
		return matrix<F>();
	}

	int size = rows;
	if (size == 1) {		
		return matrix<F>(vec({ (A[0][0] != 0)* (F)1 }));
	}

	vector<vector<F>> cofactor(size, vector<F>(size));
	int c = 0;
	for (int i = 0; i < size; i++) {
		c = i;
		for (int j = 0; j < size; j++) {
			cofactor[i][j] = pow(-1,c) * det(A.submatrix(i, j));
			c++;
		}
	}
	return matrix<F>(cofactor).t();

}

/*
* Computes reduced row echelon form of matrix.
*/
template<typename F> 
matrix<F> rref(matrix<F> A, matrix<F> b) {
	bool is_eq = b.get_cols() * b.get_rows();
	matrix<F> mat = matrix<F>(A);
	int rows = A.get_rows(); 
	int cols = A.get_cols();
	int k = 0; //number of leading ones.

	auto fix_zero = [](F x) {
		if (x < pow(10, -10)) {
			return (F)0;
		}
		return x;
	};

	for (int j = 0; j < cols; j++) {
		for (int i = k; i < rows && k < rows; i++) {

			if (abs(fix_zero(mat[i][j])) != 0) {
				if (is_eq) {
					b = (b.row_mult(i, 1 / mat[i][j])).row_swap(k, i);
				}
				mat = (mat.row_mult(i, 1 / mat[i][j])).row_swap(k, i);
				//std::cout << aug(mat, b).to_string() << "\n";
				
					for (int l = 0; l < rows; l++) { // get zeroes in other entries of j-th col.
						if (l != k) {
							if (is_eq) {
								b = b.row_add(i, -mat[l][j], l);
							}
							mat = mat.row_add(i, -mat[l][j], l);								
						}
						//std::cout << aug(mat, b).to_string() << "\n";
					}
				k++;
				break;
			}
		}
	}
	if (is_eq) {
		return aug(mat, b);
	}
	return mat;
}

/*
* Computes kernel of given matrix
* 
* @param A matrix to compute kernel of
* @return std::vector containing (nx1) column-vector matrices which lie
*	in the kernel of A
*/
template<typename F> 
vector<matrix<F>> ker(matrix<F> A) {
	int rows = A.get_rows();
	int cols = A.get_cols();
	matrix<F> mat = rref(A);

	auto leading_ones = [](matrix<F> A) {
		int rows = A.get_rows();
		int cols = A.get_cols();
		set<int> l_ones;

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				if (A[i][j] == 1) {
					l_ones.insert(j);
					break;
				}
			}
		}
		return l_ones;
	};
	set<int> ones = leading_ones(mat);

	vector<matrix<F>> kernel;
	
	for (int j = 0; j < cols; j++) { //loop through cols
		if (ones.find(j) == ones.end()) { // if col has no leading one, construct vector
											// in kernel
			vector<vector<F>> v(cols,vector<F>(1)); 
			for (int i = 0; i < cols; i++) { v[i] = { 0 }; } // set v as all zero

			v[j] = { 1 };
			int k = 0; //interator variable for rows
			for (int i : ones) {
				v[i] = { -mat[k][j] };
				k++;
			}
			kernel.push_back(matrix<F>(v));
		}
	}

	return kernel;
}

/*
* Augments matrix with another matrix
*/
template<typename F> 
matrix<F> aug(matrix<F> A, matrix<F> b)
{
	assert(b.get_rows() == A.get_rows());

	vector<vector<F>> mat = A.get_arr();
	for (int i = 0; i < b.get_rows(); i++) {
		for (int j = 0; j < b.get_cols(); j++) {
			mat[i].push_back(b.get_arr()[i][j]);
		}
	}
	return mat;
}

template<typename F>
inline matrix<F> proj_matrix(const initializer_list<matrix<F>>& span)
{
	matrix<F> A = matrix<F>(span);
	return A * (inv(A.t() * A)) * (A.t());
}

template<typename F>
matrix<F> inv(matrix<F> A)
{
	F d = det(A);

	if (d == (F)0) {
		return matrix<F>::MAT_NULL();
	} 

	return adj(A)*((F)1/d);
}

template<typename F> 
matrix<F> solve(matrix<F> A,matrix<F> b) {
	using mat = matrix<F>;
	int rows = A.get_rows();
	mat system = rref(A, b);
	mat A_reduced = rref(A);

	mat solution; {
		mat temp;
		for (int i = 0; i < rows; i++) {
			temp[i] = system[i][system.get_cols() - 1];
		}
		 solution = mat(temp);
	}

	for (int i = 0; i < rows; i++) {
		if ( A_reduced.row(i) == mat::zero( 1, A.get_cols() ) && solution[i] != 0) {
			return mat();
		}
	}

	if (rref(A) == matrix<F>::id(rows)) {
		return mat();
	} 

	return mat();
}



#endif



