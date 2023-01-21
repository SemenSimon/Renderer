#pragma once
#ifndef DRAW_DEVICE_H
#define DRAW_DEVICE_H

#include "misc.h"
#include <math.h>
#include <stdint.h>
#include <vector>
#include <math.h>
#include <unordered_map>
#include "linalg.h"


#define u32 uint32_t

#define GRAY 0x333333 
#define RED 0xFF00000
#define BLUE 0x0000FF
#define GREEN 0x00FF00

using namespace linalg;

static u32 get_RGB(u32 color,u32 color_select) {
    u32 C = 0;
    u32 mask;
    int shift;

    switch (color_select) {
        case RED: {
            mask = 0x00010000;
            shift = 16;
            break;
        }   
        case GREEN: {
            mask = 0x00000100;
            shift = 8;
            break;
        }
        default: {
            mask = 1;
            shift = 0;
        }
    }
    for (int i = 0; i < 8; i++) {
        u32 mask_i = (mask << i) & color;
        C = C | (mask_i >> shift);
    }

    return C;
}


/*
* parameterization of line between two points in R2
*/
struct line {

    line() {
        //this is useful for telling us if the line was created by the default constructor.
        this->sgn = 0;
    }

    line(pt O, pt P) {
        int dx = P.x - O.x; int dy = P.y - O.y;
        bool is_horz = (dy == 0);
        bool is_vert = (dx == 0);
        double slope = (static_cast<double>(dy + is_horz) / static_cast<double>(dx + is_vert));

        bool reflect_line = (abs(slope) > 1) * !is_horz;

        //slope = !is_horz * !is_vert * (
        //    !reflect_line * slope +
        //    reflect_line * (double)(1) / slope + is_horz + is_vert);

        //sgn is so we know if the function is increasing or decreasing
        int sgn = (!reflect_line) * sign(dx) + (reflect_line)*sign(dy);

        //if the line is reflected, it gets evaluated on the y axis instead.
        int len = (!reflect_line) * abs(dx) + (reflect_line)*abs(dy);


        this->x_slope = slope * !is_horz * !is_vert;
        this->y_slope = ((double)1 / slope) * !is_vert * !is_horz;

        this->sgn = sgn;
        this->len = len;
        this->reflected = reflect_line;
        this->O = O;
        this->P = P;
    }

    inline pt eval(double t) {
        int x = sgn * t;
        int y = (int)((x_slope * !reflected + y_slope * reflected) * x);

        return pt(!reflected * (x + O.x) + (reflected) * (y + O.x),
            !reflected * (y + O.y) + (reflected) * (x + O.y));
    }

    inline pt eval_by_x(double t) {
        int x = t;
        int y = (int)(x_slope * x);

        return pt((x + O.x), (y + O.y));
    }

    inline pt eval_by_y(double t) {
        int y = t;
        int x = (int)(y_slope * y);

        return pt((x + O.x), (y + O.y));
    }

    inline bool operator == (const line& other) {
        return ((this->O == other.P && this->P == other.O) ||
            (this->O == other.O && this->P == other.P)) &&
            (this->sgn == -other.sgn);
    }

    line flip() {
        this->sgn *= -1;
        pt temp1 = this->O;
        pt temp2 = this->P;
        this->O = temp2;
        this->P = temp1;
        return *this;
    }

    pt O, P;
    double x_slope;
    double y_slope;
    int len;
    int sgn;
    bool reflected;
};


struct pt_hash {
    std::size_t operator () (const pt& P) const {
        auto x = static_cast<unsigned long long> (P.x * 10000);
        auto y = static_cast<unsigned long long> (P.y * 10000);
        return std::hash<size_t>()(x) ^ std::hash<size_t>()(y);
    }
};

struct pt_eq {
    bool operator () (const pt& A, const pt& B) const {
        return A.x == B.x && A.y == B.y;
    }
};

class draw_device {

public:
    draw_device();
    draw_device(uint32_t* pMem_in, int width, int height, realnum scale =1) ;

    matrix<realnum> get_center();
    realnum get_width() { return this->DISPLAY_WIDTH / scale; }
    realnum get_height() { return this->DISPLAY_HEIGHT / scale; }
    pt get_center_raw() { return this->DISPLAY_CENTER; }

    void set_scale(
        realnum scale
    );

    void set_color(
        u32 color
    );

    void draw_pixel(
        int x,
        int y,
        u32 color = 0xFFFFFF); 

    void draw_line(
        matrix<realnum> O,
        matrix<realnum> P,
        u32 color = 0xFFFFFF
    );

    void draw_line_raw(
        pt O,
        pt P,
        u32 color = 0xFFFFFF
    );

    void draw_triangle(
        matrix<realnum> A,
        matrix<realnum> B,
        matrix<realnum> C,
        u32 color = 0xFFFFFF
    );

    void draw_triangle_raw(
        pt A,
        pt B,
        pt C,
        u32 color = 0xFFFFFF
    );

    template<typename func>
    void draw_quadrilateral(
        matrix<int> adjacency,
        vector<matrix<realnum>> vertices,
        func s,
        u32 color = 0xFFFFFF
    );

    template<typename func>
    void draw_quadrilateral_raw(
        matrix<int> adjacency,
        int npts,
        pt* pts,
        func s
    );

    void draw_line_raw_dotted(
        pt O,
        pt P,
        u32 color = 0xFFFFFF
    );

    void draw_circ(
        matrix<realnum> O,
        realnum r,
        u32 color = 0xFFFFFF
    );

    void draw_circ_raw(
        int x_center,
        int y_center,
        int r,
        u32 color = 0xFFFFFF
    );

private:
    u32* pMem;
    int DISPLAY_WIDTH;
    int DISPLAY_HEIGHT;
    pt DISPLAY_CENTER;
    realnum scale;
};



template<typename func>
void draw_device::draw_quadrilateral(
    matrix<int> adjacency,
    vector<matrix<realnum>> vertices,
    func s,
    u32 color
)
{
    int npts = vertices.size();
    pt* pts = new pt[npts];

    for (int i = 0; i < npts; i++) {
        pts[i] = DISPLAY_CENTER + pt(vertices[i] * scale);
        if (pts[i] == DISPLAY_CENTER) {
            return;
        }
    }

    draw_quadrilateral_raw(adjacency, npts, pts, s);
}

template<typename func>
void draw_device::draw_quadrilateral_raw(
    matrix<int> adjacency,
    int npts,
    pt* pts,
    func s
)
{
    pt* pts_sorted_by_y = new pt[npts];

    for (int i = 0; i < npts; i++) {
        pts_sorted_by_y[i] = *(pts + i);
    }

    auto comp_pts_by_y = [](const void* a, const void* b) {
        pt arg1 = *(const pt*)a;
        pt arg2 = *(const pt*)b;

        return (arg1.y > arg2.y) - (arg1.y < arg2.y);
    };

    qsort(pts_sorted_by_y, npts, sizeof(pt), comp_pts_by_y);

    std::unordered_map<pt, line*, pt_hash, pt_eq> lines_for_pts;

    //allocates memory for two lines corresponding to each point.
    for (int i = 0; i < npts; i++) {
        line* lines_i = new line[2];
        lines_for_pts.insert({ pts_sorted_by_y[i], lines_i });
    }

    for (int i = 0; i < npts; i++) {
        for (int j = i; j < npts; j++) {
            //this means the points are connected.
            if (adjacency[i][j]) {
                line* lines_i = lines_for_pts.at(pts[i]);
                line* lines_j = lines_for_pts.at(pts[j]);

                //sgn is zero for lines made by the default constructor. This lets
                //us fill both spots in lines_i and lines_j.
                *(lines_i + (lines_i[0].sgn != 0)) = line(pts[i], pts[j]);
                *(lines_j + (lines_j[0].sgn != 0)) = line(pts[j], pts[i]);
                int a = 1;
            }
        }
    }

    line* lines = lines_for_pts.at(pts_sorted_by_y[0]);
    int y_next = pts_sorted_by_y[1].y;
    int i = 0;

    line L1 = lines[0];
    line L2 = lines[1];

    int h_max = pts_sorted_by_y[npts - 1].y - pts_sorted_by_y[0].y;

    for (int h = pts_sorted_by_y[0].y; h <= pts_sorted_by_y[3].y; h++) {
        pt P1 = L1.eval_by_y(h - L1.O.y);
        pt P2 = L2.eval_by_y(h - L2.O.y);

        int x_i = min(P1.x, P2.x);
        int x_f = max(P1.x, P2.x);

        for (int k = x_i; k < x_f; k++) {
            pt pos = pt(k, h) - DISPLAY_CENTER;
            draw_pixel(k, h, s(pos.x,pos.y));
        }

        if (h >= y_next && i < 2) {
            i++;
            lines = lines_for_pts.at(pts_sorted_by_y[i]);
            y_next = pts_sorted_by_y[i + 1].y;

            for (int j = 0; j <= 1; j++) {
                line other = lines[(j == 0)];

                if (L1 == lines[j]) {
                    L1 = other;
                    break;
                }
                if (L2 == lines[j]) {
                    L2 = other;
                    break;
                }
            }
        }
    }
}



#endif // !DRAW_DEVICE_H