#include "vertex_shader.h"
#define PI 3.14159265358979323846  /* pi */


//WIREFRAME
wiremesh::wiremesh(vertex& v) {
    v.copy_to(this->mesh_begin,this->mesh_end);
    this->mesh_size = mesh_end - mesh_end;
}

void wiremesh::set_pos(vec new_pos) {
    vec pos_current = this->get_pos();
    vec pos_relative = new_pos - pos_current;

    vertex* v = this->mesh_begin;
    while (v != this->mesh_end) {
        v->data = v->data + pos_relative;
        v++;
    }  
    this->pos = new_pos;
}

inline vec wiremesh::get_pos() {
    double size = (this->mesh_end-this->mesh_begin);
    vertex* v = this->mesh_begin;
    vec centroid = R3::zero();

    while (v != this->mesh_end) {
        centroid = centroid + v->data;
        v++;
    }
    return centroid * (static_cast<double>(1) / static_cast<double>(size));
}

inline wiremesh wiremesh::operator += (const vec& w) {
     for (vertex* v = this->mesh_begin; v != this->mesh_end; v++) {
        v->data = v->data + w;
    }
    return *this;
}

/*
* Subtracts a vector from every point in the mesh
*/
inline wiremesh wiremesh::operator -= (const vec& w) {
    for (vertex* v = this->mesh_begin; v != this->mesh_end; v++) {
        v->data = v->data - w;  
    }
    return *this;
}

/*
* Transforms every point in the mesh with a matrix
*/
wiremesh wiremesh::operator *= (mat T) {
    vertex* v = this->mesh_begin;
    while ( v != this->mesh_end) {
        v->data = T * v->data;
        v++;
    }
    return *this;
}

//RENDER FRAME

render_frame::render_frame(draw_device& ddev, camera& cam) { 
this->ddev = &ddev; 
this->cam = &cam; 
}

inline void render_frame::draw_line(vec v1, vec v2, u32 color) {

    camera& cam = *(this->cam);
    draw_device& ddev = *(this->ddev);
    
    vec normal = cam.get_normal();
    vec pos = cam.get_pos();
    vec clip_point = cam.get_focal_point() + normal;
    realnum foc_dist = cam.get_foc_dist();

    bool v1_behind = R3::ip(v1 - clip_point, normal) <= 0;
    bool v2_behind = R3::ip(v2 - clip_point, normal) <= 0;
   
    if (v1_behind && v2_behind) {
        return;
    }
    
    if (v1_behind) {
        v1 = R3::line_plane_intersect(v1, v2 - v1, clip_point, normal);
    }

    if (v2_behind) {
        v2 = R3::line_plane_intersect(v2, v2 - v1, clip_point, normal);
    }

    vec pt1 = cam.proj(v1);
    vec pt2 = cam.proj(v2);
      
    ddev.draw_line(pt1, pt2, color);
}

void render_frame::draw(vertex& v, u32 color)
{
    auto f = [&](vertex* n1, vertex* n2) { this->draw_line(n1->data, n2->data, color); };
    v.execute_func(f);
}

//SHAPES
void obj_3d::set_pos(vec new_pos) { 
    mesh.set_pos(new_pos); 
    this->pos = new_pos;
}

void obj_3d::set_scale(double scale){
    scale = abs(scale);
    vertex* v = this->mesh.mesh_begin;
    while (v != this->mesh.mesh_end) {
        v->data = v->data * og_scale * scale;
        v++;
    }
    og_scale = 1 / scale;
    
}

void obj_3d::transform(mat T){
    this->mesh *= T;
}


//SURFACE
surface::surface(int size, realnum spacing) {
    mesh_grid_gen(this->mesh.mesh_begin, size, spacing);
    this->mesh.mesh_end = this->mesh.mesh_begin + static_cast<int>(pow(size,2));
    this->pos = (this->mesh.mesh_begin + size * size / 2)->data;
    this->size = size;
}

void surface::mesh_grid_gen(vertex*& start, int size, realnum spacing) {
    //don't look at this madness
    const vec zero = mat::zero(3, 1);
    const vec e[3] = { mat::std_basis(3,0) , mat::std_basis(3,1), mat::std_basis(3,2) };

    start = new vertex[pow(size + 1, 2)];

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            vertex& O = start[i * size + j];
            O.data = zero + e[0] * spacing * i + e[1] * spacing * j;

            for (int k = 0; k < 4; k++) {
                int sgn = pow(-1, k / 2);
                int dx = sgn * (k % 2); int dy = sgn * ((k + 1) % 2);
                int new_x = i + dx; int new_y = j + dy;

                if ((new_x >= 0 && new_x < size && new_y >= 0 && new_y < size)) {
                    vertex& P = start[new_x * size + new_y];
                    O.add_node(P);
                }
            }
        }
    }
};

//CUBE
cube::cube(realnum side_length, vec pos){
    vec zero = vec::zero(3, 1);
    vec e[3] = { mat::std_basis(3,0) , mat::std_basis(3,1), mat::std_basis(3,2) };

    matrix<int> adj = matrix<int>({
        {0,1,1,1,0,0,0,0},
        {1,0,0,0,1,0,1,0},
        {1,0,0,0,1,1,0,0},
        {1,0,0,0,0,1,1,0},
        {0,1,1,0,0,0,0,1},
        {0,0,1,1,0,0,0,1},
        {0,1,0,1,0,0,0,1},
        {0,0,0,0,1,1,1,0}
    }).t();

    int V = adj.get_rows();
    this->mesh.mesh_begin = new vertex[V];
    this->mesh.mesh_end = this->mesh.mesh_begin + V;
    
    for (int i = 0; i < V; i++) {
        vertex* v = this->mesh.mesh_begin + i;
        for (int j = i; j < V; j++) {
            if (adj[i][j]) {
                v->add_node(*(this->mesh.mesh_begin + j));
            }
        }
    }
    this->mesh.mesh_begin->data = zero;
    for (int i = 0; i < V-1; i++) {
        (this->mesh.mesh_begin + i + 1)->data =
            e[(i) % 3] * side_length + 
            e[(i+1) % 3] * (i >= 3) * side_length +
            e[(i + 2) % 3] * (i >= 6) * side_length
        ;
    }

    this->mesh.set_pos(pos);
    this->pos = pos;
    this->side_length = side_length;
}

//SPHERE
sphere::sphere(realnum r, int res, vec pos) {
    this->r = r;
    this->pos = pos;

    int size = res * 4; // amount of points per semicircle

    realnum theta = PI / (2 * res);
    vec axis = vec::std_basis(3, 2)*r;

    points.push_back(axis + pos);
    for (int i = 0; i < res*4; i++) {
        for (int j = 1; j < res*2; j++) {
            vec point = R3::rotate_intr(theta * i, theta * j, 0) * axis + pos;
            points.push_back(point);
        }
    }
    points.push_back(axis*R3::rotate_intr(0,PI,0) + pos);

    int mesh_size = points.size();
    this->mesh.mesh_begin = new vertex[mesh_size];
    this->mesh.mesh_end = this->mesh.mesh_begin + mesh_size;

    for (int i = 0; i < mesh_size; i++) {
        mesh[i]->data = points[i];
    }
    
    for (int i = 0; i < res * 4; i++) {
        for (int j = 0; j < res * 2; j++) {
            if (j == 0) {
                mesh[0]->add_node(*mesh[1 + i*(res*2 - 1) + j]);
            }
            else if (j == res * 2 - 1) {
               mesh[-1]->add_node(*mesh[i * (res * 2 - 1) + j]);
            }
            else {
                mesh[i * (res * 2 - 1) + j]->add_node(*mesh[i * (res * 2 - 1) + j + 1]);
                
            }

            bool in_range = (i + 1) * (res * 2 - 1) + j < mesh_size;
            if (in_range) {
                mesh[i * (res * 2 - 1) + j]->add_node(*mesh[(i + 1) * (res * 2 - 1) + j]);
            }
            else {
                mesh[i * (res * 2 - 1) + j]->add_node(*mesh[j]);
            }
        }
    }

}

void sphere::draw_points(draw_device ddev, camera cam)
{
    for (vec p : this->points) {
        ddev.draw_circ(cam.proj(p),3.4,0x00FFFF);
    }
    return;
}
