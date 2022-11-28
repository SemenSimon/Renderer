#pragma once
#include "misc.h"
#include <math.h>
#include <stdint.h>
#include <vector>
#include "linalg.h"


#define u32 uint32_t

#define GRAY 0x333333 
#define RED 0xFF00000
#define BLUE 0x0000FF
#define GREEN 0x00FF00

using namespace linalg;

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

    void draw_quadrilateral_raw(
        matrix<int> adjacency,
        pt* pts,
        u32 color = 0xFFFFFF
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
