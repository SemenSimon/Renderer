#pragma once

#include "matrix.h"
#include "linalg.h"

using namespace linalg;
using mat = matrix<realnum>;
using vec = matrix<realnum>;

static vec centroid(vector<vec> points) {
    vec temp = points[0];
    for (int i = 1; i < points.size(); i++) {
        temp = temp + points[i];
    }

    return temp * pow((double)points.size(),-1);
}

class wiremesh {
public:
    wiremesh() {}
    wiremesh(vector<vec> points, matrix<int> adjacency_matrix) {
        this->points = points;
        this->adjacency_matrix = adjacency_matrix;
        this->pos = centroid(points);
    }

    wiremesh operator += (const vec& v) {
        for (vec& p : this->points) {
            p = p + v;
        }
        this->pos = this->pos + v;
        return *this;
    }
    wiremesh operator -= (const vec& v);
    wiremesh operator *= (mat T);

    vec get_pos() { return this->pos; }
    void set_pos(vec v) { *this += v - this->pos; }
    int size() { return this->points.size(); };

    vector<vec> points;
    matrix<int> adjacency_matrix;

    u32 color = 0xFFFFFF;

private:
    vec pos;
};