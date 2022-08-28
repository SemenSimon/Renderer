#include "linalg.h"
#include <iostream>


/*
* Produces vectors which span a hyperplane for a given normal vector.
*/
template<class parent>
subspace<parent> hyperplane(class parent::elem normal) {
	int dim = parent::dim;
	using vec = class parent::elem;
	using F = typename parent::field;
	
	vector<vec> plane(dim-1);

	for (int i = 1; i < dim; i++) {
		vector<F> u(dim);
		for (int j = 0; j < dim; j++) {
			u[j] = 0;
		}

		u[0] = (-1)*( normal[i][0] / normal[0][0] );
		u[i] = 1;

		plane[i - 1] = vec(u);
	}

	return subspace<parent>(plane);
}	



int main() {
	using num_type = long double;
	using K3 = coord_space<num_type, 3>;
	using vec = K3::elem;
	using mat = matrix<num_type>;

	vec u = {1,2,3};

	subspace<K3> plane = hyperplane<K3>(u);

	vec v = plane.get_basis()[0];
	vec w = plane.get_basis()[1];

	mat projection = mat(plane.get_basis()).t();

	vec k = { 6,2,3 };

	std::cout << (projection * k).print();


	return 0;
} 
