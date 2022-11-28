#pragma once
#include "draw_device.h"
#include "linked_node.h"
#include "camera.h"
#include "wiremesh.h"
#include <unordered_map>

class vertex_shader {

public:
    //constructors
    vertex_shader() {}
    vertex_shader(draw_device& ddev, camera& cam);

    //custom data types
    struct edge {
        edge() {}
        edge(vec v1, vec v2, double dist_sq = 0, u32 color = 0xFFFFFF) {
            this->v1 = v1; 
            this->v2 = v2; 
            this->color = color;
            this->dist_sq = dist_sq;
        }

        vec v1;
        vec v2;
        u32 color;
        double dist_sq;
    };

    /* ---------- RENDERING PIPELINE OPERATIONS ----------- */
    edge process_edge(vec v1, vec v2, u32 color = 0xFFFFFF);
    void process_meshes();

    /* ---------- OTHER ---------- */
    void add_mesh(wiremesh* mesh) { this->meshes.push_back(mesh); }
    void draw_line(vec v1, vec v2, u32 color = 0xFFFFFF);

private:
    
    vector<wiremesh*> meshes; 
    draw_device* ddev;
    camera* cam;

};

class obj_3d {

protected: 
    vec pos;
public:
    //constructors
    obj_3d(){}
    
    //member functions
    inline vec get_pos() {return mesh.get_pos(); }
    void set_pos(vec new_pos);
    void set_scale(double scale);
    void transform(mat T);
    template<typename func>
    void transform(func F);

    //fields
    wiremesh mesh;
};

//grid
class surface : public obj_3d {
public:
    surface(){}
    surface(int size, realnum spacing);

    template<typename func>
    void eval(func f, realnum scale = 1);
private:
    int size;
    realnum spacing;
    void mesh_grid_gen(vertex*& start, int size, realnum spacing = 1);
};

//cube 
class cube : public obj_3d {
public:
    cube(){}
    cube(realnum side_length, vec pos = { 0,0,0 });
    bool is_inside(vec P);
    realnum get_side_length() { return this->side_length;};

private:
    realnum side_length;
};

//sphere
class sphere : public obj_3d {
public:
    sphere(realnum r, int res, vec pos = { 0,0,0 });
    void draw_points(draw_device ddev,camera cam);
private:
    realnum r;
};

template<typename func>
inline void surface::eval(func f, realnum scale) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            realnum x = (i - size / 2) * scale;
            realnum y = (j - size / 2) * scale;

            vec* point = &(this->mesh.points[i * size + j]);
            *point = { point[0][0],point[1][0], f(x,y) };
        }
    }
}

template<typename func>
void obj_3d::transform(func F) {
    vec pos = this->get_pos();
    for (vec* pv : this->mesh.points) {
        vec temp = *pv - pos;
        *pv = pos + F(temp);
    }
}

