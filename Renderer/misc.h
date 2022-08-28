#pragma once
#include "matrix.h"
#define PI 3.14159265358979323846  /* pi */


struct pt {
    pt() { x = 0; y = 0; }
    pt(int a, int b) { x = a; y = b; }
    template<typename K>
    pt(matrix<K> mat) {
        x = static_cast<int>(mat[0][0]);
        y = static_cast<int>(mat[1][0]);
    }

    pt operator + (const pt pt2) {
        return pt(this->x + pt2.x, this->y + pt2.y);
    }

    pt operator - (const pt& other) {
        return pt(this->x - other.x, this->y - other.y);
    }

    int x;
    int y;
  
};

/* Returns the sign of a number.
* @param x
* @return -1 if if is negative, 1 otherwise.
*/
inline int sign(double x) {
    bool neg = x < 0;
    return 0 + (-1) * neg + !neg;
}

