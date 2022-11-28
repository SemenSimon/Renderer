#include "draw_device.h"
#include <math.h>
#include <unordered_map>

using namespace linalg;

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
        this->y_slope = ((double)1 / slope) * !is_vert*!is_horz;

        this->sgn = sgn;
        this->len = len;
        this->reflected = reflect_line;
        this->O = O;
        this->P = P;
    }

    inline pt eval(double t) {
        int x = sgn * t;
        int y = (int)( (x_slope*!reflected + y_slope*reflected) * x);

        return pt(!reflected * (x + O.x) + (reflected) * (y + O.x),
                  !reflected * (y + O.y) + (reflected) * (x + O.y));
    }

    inline pt eval_by_x(double t) {
        int x = t;
        int y = (int)(x_slope * x);

        return pt((x + O.x),(y + O.y) );
    }

    inline pt eval_by_y(double t) {
        int y = t;
        int x = (int)(y_slope * y);

        return pt((x + O.x),(y + O.y));
    }

    inline bool operator == (const line& other) {
        return ((this->O == other.P && this->P == other.O)  ||
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

int comp_pts_by_y(const void* a, const void* b) {
    pt arg1 = *(const pt*)a;
    pt arg2 = *(const pt*)b;

    return (arg1.y > arg2.y) - (arg1.y < arg2.y);
}


struct pt_hash {
    std::size_t operator () (const pt& P) const {
        auto x = static_cast<unsigned long long> (P.x*10000);
        auto y = static_cast<unsigned long long> (P.y*10000);
        return std::hash<size_t>()(x) ^ std::hash<size_t>()(y);
    }
};

struct pt_eq {
    bool operator () (const pt& A, const pt& B) const {
       return A.x == B.x && A.y == B.y;
    }
};

//DRAW DEVICE

draw_device::draw_device()
{
    pMem = nullptr;
    DISPLAY_WIDTH = 0;
    DISPLAY_HEIGHT = 0;
    scale = 1;
    DISPLAY_CENTER = pt();
}

draw_device::draw_device(uint32_t* pMem_in, int width, int height,realnum scale) {
    scale = 1;
    pMem = pMem_in;
    DISPLAY_WIDTH = width;
    DISPLAY_HEIGHT = height;
    DISPLAY_CENTER = pt(width/2,height/2);
    this->scale = scale;
}

matrix<realnum> draw_device::get_center()
{
    matrix<realnum> fake_center( { (realnum)DISPLAY_CENTER.x / scale, (realnum)DISPLAY_CENTER.y / scale });
    return fake_center;
}

void draw_device::set_scale(realnum scale) {
    this->scale = scale;
}

inline void draw_device::draw_pixel(int x, int y, u32 color){
    *(pMem + (y * DISPLAY_WIDTH + x)* !(y > DISPLAY_HEIGHT - 1 || y < 0 || x > DISPLAY_WIDTH - 1 || x < 0)) = color;
}

void draw_device::set_color(u32 color)
{
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        *(pMem + i) = color;
    }
}

void draw_device::draw_line(matrix<realnum> O, matrix<realnum> P, u32 color)
{

    pt O2 = DISPLAY_CENTER + pt(O * scale); 
    pt P2 = DISPLAY_CENTER + pt(P * scale);

    draw_line_raw(O2, P2, color);
}

void draw_device::draw_circ(matrix<realnum> O, realnum r, u32 color) {
    int x_center = DISPLAY_CENTER.x + O[0][0]*scale; int y_center = DISPLAY_CENTER.y + O[1][0] * scale; r *= scale;
    draw_circ_raw(x_center, y_center, (int)r, color);
}

inline void draw_device::draw_line_raw(pt O, pt P, u32 color)
{
    line L(O, P);

    int len = min(L.len, DISPLAY_WIDTH)*!L.reflected + min(L.len,DISPLAY_HEIGHT)*L.reflected;

    for (int i = 0; i < L.len; i++) 
    {
        pt X = L.eval(i);
        draw_pixel(X.x,X.y, color);
    }
}

void draw_device::draw_triangle(matrix<realnum> A, matrix<realnum> B, matrix<realnum> C, u32 color)
{
    pt A2 = DISPLAY_CENTER + pt(A * scale);
    pt B2 = DISPLAY_CENTER + pt(B * scale);
    pt C2 = DISPLAY_CENTER + pt(C * scale);

    draw_triangle_raw(A2, B2, C2, color);
}

int comp(const void* a, const void* b) {
    const pt l1 = *static_cast<const pt*>(a); 
    const pt l2 = *static_cast<const pt*>(b);

    return l1.y - l2.y;
};

void draw_device::draw_triangle_raw(pt A, pt B, pt C, u32 color)
{

    pt* points = new pt[3]{ A, B, C };
    qsort(points, 3, sizeof(pt), comp);

    int height = points[2].y - points[0].y;

    line bot_mid(points[0], points[1]);
    line bot_top(points[0], points[2]);
    line mid_top(points[2], points[1]);

    for (int y = 0; y < height; y++) {
        bool below = (points[0].y + y < points[1].y);

        pt P1, P2;
        
        P1 = points[0] + pt(bot_top.y_slope * y, y);

        if (below) {
            P2 = points[0] + pt(bot_mid.y_slope * y, y );
        }
        else {
            int offset_y = y - abs(points[0].y - points[1].y);
            P2 = points[1] + pt(mid_top.y_slope * offset_y, offset_y);
        }

        draw_line_raw(P1, P2, color);
        
    }
}

void draw_device::draw_quadrilateral_raw(matrix<int> adjacency, pt* pts, u32 color) {
    pt pts_sorted_by_y[4];

    for (int i = 0; i < 4; i++) {
        pts_sorted_by_y[i] = *(pts + i);
    }
    qsort(pts_sorted_by_y, 4, sizeof(pt), comp_pts_by_y);
    
    std::unordered_map<pt, line*, pt_hash,pt_eq> lines_for_pts;

    //allocates memory for two lines corresponding to each point.
    for (int i = 0; i < 4; i++) {
        line* lines_i = new line[2];
        lines_for_pts.insert({ pts_sorted_by_y[i], lines_i });
    }

    for (int i = 0; i < 4; i++) {
        for (int j = i; j < 4; j++) {
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

    int h_max = pts_sorted_by_y[3].y - pts_sorted_by_y[0].y;

    for (int h = pts_sorted_by_y[0].y; h <= pts_sorted_by_y[3].y; h++) {
        pt P1 = L1.eval_by_y(h-L1.O.y);
        pt P2 = L2.eval_by_y(h-L2.O.y);

        int x_i = min(P1.x, P2.x);
        int x_f = max(P1.x, P2.x);

        for (int k = x_i; k < x_f; k++) {
            draw_pixel(k, h);
        }

        if (h >= y_next && i < 2) {
            i++;
            lines = lines_for_pts.at(pts_sorted_by_y[i]);
            y_next = pts_sorted_by_y[i+1].y;

            for (bool j = 0; j <= 1; j++) {
                line other = lines[(int)!j];

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

inline void draw_device::draw_line_raw_dotted(pt O, pt P, u32 color)
{
    //don't look at this

    int dx = P.x - O.x; int dy = P.y - O.y;
    realnum slope = static_cast<realnum>(dy) / static_cast<realnum>(dx);

    int sgn;                          //sgn is so we know if the function 
                                      //is increasing or decreasing
    int dot_width = 10;
    if (slope * sign(slope) < 1) {
        sgn = sign(P.x - O.x);

        for (int c = 0; c < abs(P.x - O.x); c += 2*dot_width)
            for (int i = c; i < min(c + dot_width, abs(P.x - O.x)); i++) {
                int j = sgn * i;
                int y = (int)(slope * j);
                draw_pixel(j + O.x, y + O.y, color = color);
            }
        }
    else {
        slope = static_cast<realnum>(1) / slope;
        sgn = sign(P.y - O.y);

        for (int c = 0; c < abs(P.y - O.y); c += 2 * dot_width) {
            for (int i = c; i < min(c + dot_width, abs(P.y - O.y)); i++) {
                int j = sgn * i;
                int x = (int)(slope * j);
                draw_pixel(x + O.x, j + O.y, color = color);
            }
        }
    }
} 

inline void draw_device::draw_circ_raw(int x_center, int y_center, int r, u32 color)
{
    int d = 2 * (int)r;

    auto f = [r](realnum x) {
        return sqrt(pow(r, 2) - pow(x, 2));
    };

    for (int x = -r; x < r; x++) {
        int y = f(x);
        int xPos = x_center + x;
        int yPos = y_center + x;
        draw_pixel(xPos, y_center - y,color);
        draw_pixel(xPos, y_center + y,color);
        draw_pixel(x_center + y, yPos,color);
        draw_pixel(x_center - y, yPos,color);
    }
}

