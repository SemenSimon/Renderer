#include "wiremesh.h"

inline wiremesh wiremesh::operator-=(const vec& v)
{
	for (vec& p : this->points) {
		p = p - v;
	}
	this->pos = this->pos - v;
	return *this;
}

inline wiremesh wiremesh::operator*=(mat T)
{
	for (vec& p : this->points) {
		p = T*p;
	}
	this->pos = T*this->pos;
	return *this;
}