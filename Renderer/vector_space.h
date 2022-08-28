#pragma once
#include "matrix.h"
#include "misc.h"
#include "inner_products.h"
typedef long double realnum;

/*
* Base vector space class. 
* @tparam F - scalar field 
* @tparam vec - type of vector object
*/
template<typename F,typename vec>
class vector_space {

public:
	typedef vec elem;
	typedef F field;
private:
};

/* 
* Inner product space.
* @tparam F - scalar field 
* @tparam vec - type of vector object
* @tparam product - function from underlying vector space to F
*/
template<typename F,typename vec,typename product>
class ip_space : public vector_space<F,vec> {

public:
	static inline F ip(vec v, vec w, typename product::ex_input* ex_arg = nullptr) { 
		product op;
		return op(v, w);
		if (ex_arg != nullptr) { 
			op = product(*ex_arg);
		}	
	}
	static inline F norm(vec v) { return sqrt(ip(v, v)); }
	static inline vec unitize(vec v) { return v*(1/norm(v)); }
};

/*
* n-dimensional coordiante space.
* @tparam F - scalar field 
* @tparam dim - dimension
* @tparam product - dot product by default, or get creative if you want.
*/
template<typename F,int dim, typename product = inner_products::std_coord<F, dim>>
class coord_space : public ip_space<F, matrix<F>, product> {
public:
	static const int dim = dim;
	static matrix<F> zero() {
		return matrix<F>::zero(dim, 1);
	}
};

class R3 : public coord_space<realnum, 3, inner_products::std_coord<realnum, 3>> {
public:
	static inline realnum ip(elem v, elem w) {
		return (v.t() * w)[0][0];//v[0][0] * w[0][0] +
			   //v[1][0] * w[1][0] + 
			   //v[2][0] * w[2][0];
	}

	static inline realnum norm(elem v) { return sqrt(ip(v, v)); }

	static inline matrix<realnum> cross_prod_matrix(elem v) {
		return matrix<realnum>({
			{ 0		     , v[2][0]*-1 , v[1][0]    },
			{ v[2][0]    , 0          , v[0][0]*-1 },
			{ v[1][0]*-1 , v[0][0]    , 0          }
			});
	}

	static inline elem cross_prod(elem v, elem w) {
		return cross_prod_matrix(v) * w;
	}

	/*
	* rotation matrix about a vector k by angle theta in R3.
	* @param v - 3x1 matrix.
	* @param k - 3x1 matrix.
	* @param theta - rotation angle.
	*/
	static inline elem rot_axis(elem k, double theta) {
		k = R3::unitize(k);
		return matrix<realnum>::id(3)*cos(theta) + cross_prod_matrix(k)*sin(theta) + (k*(k.t()))*(1-cos(theta));
	}

	/*
	* @param line_start - point on line.
	* @param line_dir - direction vector of line.
	* @param plane_pos - point on plane.
	* @param plane_normal - normal vector of plane.
	* 
	* @returns - Intersection of a 1-dimensional affine subspace and 2-dimensional 
	* affine subspace in R3. If there is no intersection, or the intersection is 
	* the entire 1-dimensional space, returns the zero vector.
	*/
	static inline elem line_plane_intersect(elem line_start, elem line_dir, elem plane_pos, elem plane_normal) {
		field dot = ip(line_dir, plane_normal);
		bool is_orthogonal = (dot == 0);
		return (line_start + line_dir * (ip(plane_pos - line_start, plane_normal) / (dot + is_orthogonal)))*(!is_orthogonal);
	}
	
	/* 
	* Rotation matrix in R3 about x-axis.
	*/
	inline static matrix<realnum> rotatex(double angle) {
		return matrix<realnum>({
			{ cos(angle), -sin(angle), 0},
			{ sin(angle), cos(angle) , 0 },
			{ 0         ,           0, 1 }
			});
	}

	/*
	* Rotation matrix in R3 about y-axis.
	*/
	inline static matrix<realnum> rotatey(double angle) {
		return matrix<realnum>({
			{cos(angle), 0, -sin(angle)},
			{0, 1, 0},
			{ sin(angle), 0, cos(angle) }
			});
	}

	/*
	* Rotation matrix in R3 about y-axis.
	*/
	inline static matrix<realnum> rotatez(double angle) {
		matrix<realnum> rot = matrix<realnum>({
			{1, 0, 0},
			{ 0, cos(angle), -sin(angle) },
			{ 0, sin(angle), cos(angle), }
			});
		return rot;
	}

	/*
	* Intrinsic rotation by Euler angles alpha, beta, and gamma
	* @param alpha - yaw
	* @param beta - pitch
	* @param gamma - roll
	*/
	inline static matrix<realnum> rotate_intr(double alpha, double beta, double gamma) {
		return rotatex(alpha) * rotatey(beta) * rotatez(gamma);
	}

	static double stereographic_proj_R2(double x, double y) {
		bool zero = (x == 0)*(y == 0);
		double r = sqrt(pow(x, 2) + pow(y, 2));
		return sign(x) * r *  ( asin(  (y - r) / sqrt( pow(x, 2) + pow(y - r, 2) ) )  + PI / 2.0  )*!zero;
	}

	static elem stereographic_proj_R3(elem v) {
		double p1 = stereographic_proj_R2(v[0][0], v[2][0]);
		double p2 = stereographic_proj_R2(v[1][0], v[2][0]);
		return elem({ p1, p2,0 });
	}
};

/*
* Uses gram-schmidt process to orthonormalize a set of vectors.
* @param start - element of "basis" list to start algorithm from.
* @returns - std::vector containing new set of vector.
*/
template<typename parent_space>
vector<class parent_space::elem> orthonormalize(vector<class parent_space::elem> vecs) {
	using vec = class parent_space::elem;

	//GRAM SCHMIDT PROCESS
	vector<vec> new_vecs;
	for (vec w : vecs) {
		w = w * (1/parent_space::norm(w));

		vec w_new = vec(w);

		for (vec u : new_vecs) {
			w_new = w_new - u * parent_space::ip(u, w);
		}
		w_new = w_new * (1/parent_space::norm(w_new));
		new_vecs.push_back(w_new);
	}
	return new_vecs;
}


