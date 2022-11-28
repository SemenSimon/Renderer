#include "vertex_shader.h"
#define PI 3.14159265358979323846  /* pi */


static u32 darken(u32 color,double factor) {
    u32 R = 0, G = 0, B = 0;
    for (int i = 0; i < 8; i++) {
        u32 R_mask = (0x00010000 << i)&color;
        u32 G_mask = (0x00000100 << i)&color;
        u32 B_mask = (0x00000001 << i)&color;

        R = R | R_mask >> 16;
        G = G | G_mask >> 8;
        B = B | B_mask;
    }
    R = (int)(static_cast<double>(R) * factor) << 16;
    G = (int)(static_cast<double>(G) * factor) << 8;
    B = (int)(static_cast<double>(B) * factor);
    return R|G|B;
}

/* for an adjacency matrix A, links i-th and j-th nodes by setting (i,j)-th and
(j,i)-th entries to 1*/
void link(int i, int j, matrix<int>* A) {
    A->set(i, j, 1);
    A->set(j, i, 1);
}

const mat proj_xy = {
            {1,0,0},
            {0,1,0},
            {0,0,0} };

/* norm sqaured of a column vector v */
double nsq(vec v) {
    double x = 0;
    for (int i = 0; i < v.get_rows(); i++) {
        x += pow(v[i][0], 2);
    }
    return x;
}

int comp_edge(const void* E1, const void* E2) {
    return ((vertex_shader::edge*)E2)->dist_sq - ((vertex_shader::edge*)E1)->dist_sq;
}5

//RENDER FRAME
vertex_shader::vertex_shader(draw_device& ddev, camera& cam) { 
this->ddev = &ddev; 
this->cam = &cam; 
}

vertex_shader::edge vertex_shader::process_edge(vec v1, vec v2, u32 color)
{
    camera& cam = *(this->cam);
    draw_device& ddev = *(this->ddev);

    vec normal = cam.get_normal();
    vec clip_point = cam.get_focal_point() + normal;
    realnum foc_dist = cam.get_foc_dist();

    bool v1_behind = R3::ip(v1 - clip_point, normal) <= 0;
    bool v2_behind = R3::ip(v2 - clip_point, normal) <= 0;

    if (v1_behind && v2_behind) {
        return edge({0,0,0},{0,0,0},-1);
    }

    if (v1_behind) {
        v1 = R3::line_plane_intersect(v1, v2 - v1, clip_point, normal);
    }

    if (v2_behind) {
        v2 = R3::line_plane_intersect(v2, v2 - v1, clip_point, normal);
    }

    return edge(v1, v2, nsq(proj_xy*((v1 + v2) * 0.5 - this->cam->get_focal_point())), color);
}

void vertex_shader::process_meshes()
{
    double render_dist = 2000;
    unordered_map<wiremesh*, vector<edge>> edge_container;

    //this section adds each edge to a partition of edge_container for its
    //respective mesh
    for (wiremesh* pmesh: this->meshes) {
        vector<edge> edges;

        auto add_as_edge = [&](vec v1, vec v2) {
            edge E = process_edge(v1, v2,pmesh->color);

            //1 dist_sq of -1 is reserved for when the edge is off-camera
            if (E.dist_sq != -1 /* && E.dist_sq < pow(render_dist, 2)*/) {
                edges.push_back(E); 
            }
        };
     
        //looks at connections in the adjacency matrix and adds connected vertices 
        //as edges.
        for (int i = 0; i < pmesh->size(); i++) {
            vector<int> row = pmesh->adjacency_matrix[i];

            for (int j = i; j < pmesh->size(); j++) {
                if (row[j]) {
                    add_as_edge(pmesh->points[i],pmesh->points[j]);
                }
            }
        }

        edge_container.insert({ pmesh,edges });
    }

    //All edges in edge_container are then put into a single std::vector and sorted.
    vector<edge> all_edges;

    for (wiremesh* pmesh : meshes) {
        for (edge E : edge_container.at(pmesh)) {
            double x = exp(-(pow(E.dist_sq/pow(render_dist*0.6,2),3) ));
            E.color = darken(E.color,x);
            all_edges.push_back(E);
        }
    }

    qsort(all_edges.data(),all_edges.size(),sizeof(edge), comp_edge);

    for (edge E : all_edges) {
        ddev->draw_line(cam->proj(E.v1), cam->proj(E.v2), E.color);
    }
}

void vertex_shader::draw_line(vec v1, vec v2, u32 color)
{
    edge E = process_edge(v1, v2);
    ddev->draw_line(cam->proj(E.v1), cam->proj(E.v2),color);
}

//SHAPES
void obj_3d::set_pos(vec new_pos) { 
    mesh.set_pos(new_pos); 
    this->pos = new_pos;
}

void obj_3d::transform(mat T){
    vec pos = this->get_pos();
    for (vec& v : this->mesh.points) {
        vec temp = v - pos;
        v = pos + T * temp;
    }
}

//SURFACE
surface::surface(int size, realnum spacing) {
    //don't look at this madness
    const vec zero = mat::zero(3, 1);
    const vec e[3] = { mat::std_basis(3,0) , mat::std_basis(3,1), mat::std_basis(3,2) };

    int npoints = pow(size, 2);

    vector<vec> points(npoints);
    matrix<int> adjacency = matrix<int>::zero(npoints, npoints);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            vec O = zero + e[0] * spacing * i + e[1] * spacing * j;
            points[i * size + j] = O;

            for (int k = 0; k < 4; k++) {
                int sgn = pow(-1, k / 2);
                int dx = sgn * (k % 2); int dy = sgn * ((k + 1) % 2);
                int new_x = i + dx; int new_y = j + dy;

                if ((new_x >= 0 && new_x < size && new_y >= 0 && new_y < size)) {
                    vec P = points[new_x * size + new_y];
                    link(i * size + j, new_x * size + new_y, &adjacency);
                }
            }
        }
    }
    this->mesh = wiremesh(points, adjacency);
    this->pos = mesh.get_pos();
}

//CUBE
cube::cube(realnum side_length, vec pos){
    vec zero = vec::zero(3, 1);
    vec e[3] = { mat::std_basis(3,0) , mat::std_basis(3,1), mat::std_basis(3,2) };

    matrix<int> adjacency = matrix<int>({
        {0,1,1,1,0,0,0,0},
        {1,0,0,0,1,0,1,0},
        {1,0,0,0,1,1,0,0},
        {1,0,0,0,0,1,1,0},
        {0,1,1,0,0,0,0,1},
        {0,0,1,1,0,0,0,1},
        {0,1,0,1,0,0,0,1},
        {0,0,0,0,1,1,1,0}
    }).t();

    int npoints = adjacency.get_rows();
    vector<vec> points(npoints);

    points[0] = zero;
    for (int i = 0; i < npoints -1; i++) {
        points[i + 1] =
            e[(i) % 3] * side_length + 
            e[(i+1) % 3] * (i >= 3) * side_length +
            e[(i + 2) % 3] * (i >= 6) * side_length;
    }

    this->mesh = wiremesh(points, adjacency);
    this->mesh.set_pos(pos);
    this->pos = mesh.get_pos();
    this->side_length = side_length;
}

bool cube::is_inside(vec P)
{
    vec O = this->pos;
    vec OP = P - O;

    return max(max(abs(OP[0][0]),abs(OP[1][0])),abs(OP[2][0])) - 0.1 <= this->side_length / 2;
}

//SPHERE
sphere::sphere(realnum r, int res, vec pos) {
    this->r = r;
    this->pos = pos;

    vector<vec> points;

    int size = res * 4; // amount of points per halfcircle

    realnum theta = PI / (2 * res);
    vec axis = {0,0,r};

    points.push_back(axis + pos);
    for (int i = 0; i < size; i++) {
        for (int j = 1; j < size/2; j++) {
            vec point = R3::rotate_intr(theta * i, theta * j, 0) * axis + pos;
            points.push_back(point);
        }
    }
    points.push_back(axis*R3::rotate_intr(0,PI,0) + pos);
    int npoints = points.size();

    //this matrix tells us which vertices are connected
    matrix<int> adjacency = matrix<int>::zero(npoints,npoints);

    for (int i = 0; i < res * 4; i++) {
        for (int j = 0; j < res * 2; j++) {
            if (j == 0) {
                link(0,1 + i*(res*2 - 1) + j, &adjacency);
            }
            else if (j == res * 2 - 1) {
                link(-1,i * (res * 2 - 1) + j, &adjacency);
            }
            else {
                link(i * (res * 2 - 1) + j,i * (res * 2 - 1) + j + 1, &adjacency);
                
            }

            bool in_range = ((i + 1) * (res * 2 - 1) + j < npoints - 1) && abs(i) + abs(j) != 0;
            if (in_range) {
                link(i * (res * 2 - 1) + j,(i + 1) * (res * 2 - 1) + j, &adjacency);
            }
            else {
                link(i * (res * 2 - 1) + j,j, &adjacency);
            }
        }
    }
    
    this->mesh = wiremesh(points, adjacency);
    this->mesh.set_pos(pos);
    this->pos = mesh.get_pos();

}

void sphere::draw_points(draw_device ddev, camera cam)
{
    for (vec p : this->mesh.points) {
        ddev.draw_circ(cam.proj(p),3.4,0x00FFFF);
    }
    return;
}

