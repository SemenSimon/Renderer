#pragma once
#include "matrix.h"
#include "matrixutils.h"
#include "misc_algebra.h"
#include <vector>
#include <initializer_list>

/*
* Linear subspace.
* @tparam parent - underlying vector space structure
*/
template <class parent>
class subspace {
	using K = typename parent::field;
	using vec = class parent::elem;
	using mat = matrix<K>;

public:
	subspace() { 
		dim = 0; 
	}

	/*
	* Initializes subspace from list of vectors.
	* Should be linearly independent.
	*/
	subspace(std::initializer_list<vec> list) { 
		dim = 0;
		for (vec v : list) {
			basis.push_back(v);
			dim++;
		}
		this->basis = orthonormalize<parent>(this->basis);
	} 

	/*
	* Initializes subspace from std::vector of vectors.
	*/
	subspace(std::vector<vec> list) {
		dim = 0;
		for (vec v : list) {
			basis.push_back(v);
			dim++;
		}
		this->basis = orthonormalize<parent>(this->basis);
	}

	/*
	* Orthogonal projection of vector onto subspace.
	* maps V to V
	*/
	vec proj(vec v) {
		vec w = parent::zero();
		for (vec u : basis) {
			w = w + u * ip(u, v);
		}
		return w;
	}

	inline vec operator [] (int const& index) {
		return basis[index];
	}

	inline std::vector<vec> get_basis() { return this-> basis; }
	inline int get_dim() { return this->dim; }

private:
	K ip(vec v, vec w) { return parent::ip(v, w); }
	K norm(vec v) { return parent::norm(v); }
	
protected:
	std::vector<vec> basis;
	int dim;

};	

template<typename parent>
class hyperplane : public subspace<parent> {
	using K = typename parent::field;
	using vec = class parent::elem;
	using mat = matrix<K>;

private:
	vec normal;
	mat lower_dim_proj;

public:
	hyperplane() {
		this->dim = 0;
	}

	hyperplane(vec normal, std::vector<mat> basis) {
		this->normal = normal;
		this->basis = basis;
		this->dim = parent::dim - 1;
		this->lower_dim_proj = mat(basis).t();
	}

	hyperplane(vec normal) {
		normal = R3::unitize(normal);
		this->normal = normal;
		int dim = parent::dim;
		this->dim = dim - 1;
		vector<vec> plane;
		auto coefficient = [&normal](int x) {
			return normal[x][0];
		};

		//this is basically all we need, since we mostly use 2d planes
		if (dim == 3) {
			vec p1 = R3::cross_prod(normal, mat({ 0,0,1 }));
			vec p2 = R3::cross_prod(p1, normal);
			plane = vector<vec>(2); 
			plane[0] = p1; plane[1] = p2;

			this->basis = plane;
			this->lower_dim_proj = mat(plane).t();
		}

		//if we need to generate a basis for an arbitrary hyperplane
		for (int i = 0; i < dim; i++) {
			if (coefficient(i) != 0) {
				for (int j = 0; j < dim; j++) {

					vector<K> u(dim); for (int j = 0; j < dim; j++) { u[j] = 0; } u[i] = 1;

					if (plane.size() >= dim - 1) { break; }

					if (i != j) {

						if (coefficient(j) == 0) {
							u[i] = 0;
							u[j] = 1;
						}
						else {
							u[j] = (-1) * (static_cast<double>(coefficient(i)) / static_cast<double>(coefficient(j)));
						}

						mat test = mat(u);
						if (!(test == normal)) {
							//u = (test * (-1)).t().get_arr()[0];
							plane.push_back(u);
							//i++;
						}
					}
				}
			}
		}
		plane = orthonormalize<parent>(plane);

		this->basis = plane;
		this->lower_dim_proj = mat(plane).t();
	}

		

	inline mat map_lower_dim() { return this->lower_dim_proj; }
};

