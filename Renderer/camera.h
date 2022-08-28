#pragma once
#include "linalg.h"

using namespace linalg;
using mat = matrix<R3::field>;
using vec = R3::elem;

class camera {

private:
	vec normal;
	vec pos;
	vec focal_point;
	realnum focal_dist;
	realnum FOV = 90;

	hyperplane<R3> cam_plane;
public:
	camera() { this->focal_dist = 0; }

	camera( vec normal, vec pos = mat::zero(3, 1), realnum focal_dist = 1);

	void set_facing(vec normal); 

	/*
	* Rotates camera upwards by angle vert, and clockwise about the xy-axis by angle horz
	* @return rotation matrix of camera rotation;
	*/
	void rotate(double horz, double vert);
	mat cam_rotation(double horz, double vert);
	void set_focus(realnum focus);
	void set_pos(vec v);

	vec get_pos() { return  this->pos; }
	vec get_focal_point() { return this->focal_point; }
	vec get_normal() { return this->normal; }
	realnum get_foc_dist() { return this->focal_dist; }

	hyperplane<R3> get_plane() {
		return this->cam_plane;
	}


	/*
	* Finds the intersection of the camera plane and the line from the focal point to v, then
	* maps the point to R2 using the camera position as the origin.
	*/
	inline vec proj(vec v) {
		return  cam_plane.map_lower_dim() * (R3::line_plane_intersect(focal_point, v - focal_point, pos, normal) - pos);
	}

};	