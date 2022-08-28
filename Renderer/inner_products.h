#pragma once
#include "matrix.h"
/*Various inner products for different vector spaces.
* 
* If an inner product can have any extra parameters, make sure to store all the
* data for that parameter in a single type, and create atypedef for that type
* called "ex_input".
* 
*/
namespace inner_products {

	template<typename F,int dim>
	class std_coord {
		using mat = matrix<F>;

	public:
		typedef matrix<F> ex_input;

		inline std_coord() {
			this->weight = matrix<F>::id(dim);
		}
		std_coord(mat  weight) {
			this->weight = weight;
		}
	
		inline F operator () (mat  v, mat  w) const {	
			return (v.t() *  w)[0][0];
		}

	private:
		mat weight;
		
	};
}