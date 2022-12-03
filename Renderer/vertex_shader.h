#pragma once
#include "draw_device.h"
#include "linked_node.h"
#include "camera.h"
#include <unordered_map>
#include <tuple>


const mat proj_xy = {
            {1,0,0},
            {0,1,0},
            {0,0,0} };


struct edge {
    edge() {}

    edge(vec v1, vec v2, double dist_squared = 0, u32 color = 0xFFFFFF) {
        this->v1 = v1;
        this->v2 = v2;
        this->color = color;
        this->dist_squared = dist_squared;
    }

    vec v1;
    vec v2;
    u32 color;
    double dist_squared;
};

struct face {
    face() {}

    face(vector<vec> vertices, matrix<int> adjacency, double dist_squared, u32 color = 0xFFFFFF) {
        for (vec p : vertices) {
           this->vertices.push_back(p);
        }
        this->nvertices = vertices.size();

        this->color = color;
        this->dist_squared = dist_squared;
        this->adjacency = adjacency;
    }

    u32 color;
    double dist_squared;
    matrix<int> adjacency;
    vector<vec> vertices;
    int nvertices;
};

/*Face comprised of n vertices, to be used for reference inside a mesh object.*/
struct face_internal {
    face_internal() {}
    face_internal(const face_internal& other);

    face_internal(int* vertex_indices,int num_vertices, matrix<int> adjacency) {
        this->vertex_indices = new int[num_vertices];

        for (int i = 0; i < num_vertices; i++) {
            this->vertex_indices[i] = vertex_indices[i];
        }

        this->num_vertices = num_vertices;
        this->adjacency = adjacency;
    }

    matrix<int> adjacency;
    int* vertex_indices;
    int num_vertices;
};

class wiremesh {
public:
    wiremesh() {}
    wiremesh(vector<vec> vertices, matrix<int> adjacency_matrix);

    wiremesh operator += (const vec& v);
    wiremesh operator -= (const vec& v);
    wiremesh operator *= (mat T);

    vec get_pos() { return this->pos; }
    void set_pos(vec v) { *this += v - this->pos; }
    int size() { return this->vertices.size(); };

    matrix<int> adjacency_matrix;
    vector<vec> vertices;
    vector< std::pair<int,int> > edges;
    vector<face_internal> faces;

    u32 color = 0xAA10FF;
private:
    vec pos;
};

class light {
public:
    light() {}
    light(vec pos, double strength) {
        this->pos = pos;
        this->strength = strength;
    }

    /*return light ray from source to an arbitrary point
    * @param contact_point [in]
    * @param dist [out] gives distance from light source to contact point
    */
    vec process_ray(vec contact_point, double* dist) {
        vec ray = contact_point - pos;
        *dist = R3::norm(ray);
        return ray * (1 / (*dist));
    }

    vec pos;
    double strength;

};

class vertex_shader {

public:
    //constructors
    vertex_shader() {}
    vertex_shader(draw_device& ddev, camera& cam);


    /* ---------- RENDERING PIPELINE OPERATIONS ----------- */
    edge process_edge(vec v1, vec v2, double dist_squared, u32 color = 0xFFFFFF);
    void process_meshes();

    /* ---------- OTHER ---------- */
    void add_mesh(wiremesh* mesh) { this->meshes.push_back(mesh); }
    void add_light(light* source) { this->lights.push_back(source); }
    void draw_line(vec v1, vec v2, u32 color = 0xFFFFFF);

private:
    
    vector<wiremesh*> meshes; 
    vector<light*> lights;

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
    void affine_transform(mat T);
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
    void draw_vertices(draw_device ddev,camera cam);
private:
    realnum r;
};

template<typename func>
inline void surface::eval(func f, realnum scale) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            realnum x = (i - size / 2) * scale;
            realnum y = (j - size / 2) * scale;
            vec point = this->mesh.vertices[i * size + j];

            this->mesh.vertices[i * size + j] = {point[0][0],point[1][0],f(x,y)};
        }
    }
}

template<typename func>
void obj_3d::transform(func F) {
    vec pos = this->get_pos();
    for (vec& v : this->mesh.vertices) {
        vec temp = v - pos;
        v = pos + F(temp);
    }
}

