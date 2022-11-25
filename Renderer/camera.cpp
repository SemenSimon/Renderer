#include "camera.h"
#include <iostream>
#include <math.h>

camera::camera(vec normal, vec pos, realnum focal_dist)
{
	normal = R3::unitize(normal);
	this->pos = pos;
	this->normal = normal;
	this->focal_point = pos - normal * focal_dist;
	this->focal_dist = focal_dist;

	this->cam_plane = hyperplane<R3>(normal);
}

void camera::set_facing(vec normal) {
	this->normal = R3::unitize(normal);
	this->pos = focal_point + normal * focal_dist;
	this->cam_plane = hyperplane<R3>(normal);
}

void camera::rotate(double horz, double vert) {
	//planevert and planehorz are a basis for the plane subspace
	vec planenorm = this->normal;
	
	/*
	* rotates the normal vector and vertical part of the plane basis by angle
	* 'vert' around the vector orthogonal to the normal vector and parallel to the
	* xy plane. All vectors are then rotated clockwise with respect to the xy plane 
	* by angle 'horz.'
	*/
	planenorm = R3::rotatex(-horz) * R3::rot_axis(this->cam_plane[0], vert) *planenorm;

	this->normal = planenorm;

	//this makes it so the focal point serves as the 'joint' for rotation
	this->pos = focal_point + planenorm*focal_dist;
	this->cam_plane = hyperplane<R3>(planenorm);

}

mat camera::cam_rotation(double horz, double vert)
{
	return R3::rotatex(-horz) * R3::rot_axis(this->cam_plane[0], vert);
}

void camera::set_focus(realnum focus) {
	this->focal_dist = focus;
	this->pos = focal_point + normal * focus;;
}

void camera::set_pos(vec v){
	this->pos = v + normal * focal_dist;
	this->focal_point = v;
}
