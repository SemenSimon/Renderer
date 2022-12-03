#include "vertex_shader.h"
#define PI 3.14159265358979323846  /* pi */

using bigint = long long int;


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

int comp_edge(const void* E1, const void* E2) {
    return ((edge*)E2)->dist_squared - ((edge*)E1)->dist_squared;
}

int comp_face(const void* F1, const void* F2) {
    return (*(face**)F2)->dist_squared - (*(face**)F1)->dist_squared;
}

static vec centroid(vector<vec> vertices) {
    vec temp = vertices[0];
    for (int i = 1; i < vertices.size(); i++) {
        temp = temp + vertices[i];
    }

    return temp * pow((double)vertices.size(), -1);
}



bigint factorial(int x) {
    bigint val = x;
    while (x > 1) {
        x--;
        val *= x;
    }
    return val;
}

bigint nCk(int n, int k) {
    bigint val = 1;
    for (int i = 1; i <= k; i++) {
        val *= (n - k + i);
    }
    return val / factorial(k);
}

int** generate_all_combinations(int n, int k) {

    bigint m_height = nCk(n, k);
    int** combos = new int* [m_height];
    int* nums = new int[k];

    for (int i = 0; i < m_height; i++) combos[i] = new int[k];
    for (int i = 0; i < k; i++) nums[i] = i + 1;
    for (int i = 0; i < k; i++)  combos[0][i] = nums[i] - 1;

    bigint combo_num = 1;

    while (nums[0] <= n - k) {

        nums[k - 1]++;

        for (int i = 0; i < k; i++) {
            if (nums[k - i - 1] > n) {

                int prev = nums[k - i - 2];

                for (int j = -1; j <= i; j++) {
                    nums[k - i - 1 + j] = prev + j + 2;
                }
            }
        }

        if (nums[k - 1] <= n) {
            for (int i = 0; i < k; i++)  combos[combo_num][i] = nums[i] - 1;
            combo_num++;
        }
    }

    return combos;
}

face_internal::face_internal(const face_internal& other) {
    this->adjacency = other.adjacency;
    this->num_vertices = other.num_vertices;
    this->vertex_indices = new int[num_vertices];

    for (int i = 0; i < num_vertices; i++) {
        vertex_indices[i] = other.vertex_indices[i];
    }
}

//MESH
wiremesh::wiremesh(vector<vec> vertices, matrix<int> adjacency_matrix) {

    this->vertices = vertices;
    this->adjacency_matrix = adjacency_matrix;
    this->pos = centroid(vertices);

    for (int i = 0; i < this->size(); i++) {
        vector<int> row = adjacency_matrix[i];
        //find edges
        for (int j = i; j < this->size(); j++) {
            if (row[j]) {
                this->edges.push_back({ i,j });
            }
        }
    }

    //find faces
    int vertices_per_face = 4;
    int ncombos = nCk(this->size(),vertices_per_face);
    int** vertex_combos = generate_all_combinations(this->size(), vertices_per_face);

    for (int i = 0; i < ncombos; i++) {
        //partition of adjacency matrix corresponding to a particular combination of vertices
        matrix<int> adjacency = this->adjacency_matrix.select(vertex_combos[i], vertices_per_face);

        //vector full of ones
        vector<int> ones(vertices_per_face); for (int& x : ones) x = 1;
        matrix<int> connections_per_vertex = adjacency * ones;

        for (int k = 0; k < vertices_per_face; k++) {
            if (connections_per_vertex[k][0] != 2) {
                break;
            }
            if (k == vertices_per_face - 1) {
                this->faces.push_back(face_internal(vertex_combos[i], vertices_per_face, adjacency));
            }
        }
    }
}

inline wiremesh wiremesh::operator +=(const vec& v) {
    for (vec& p : this->vertices) {
        p = p + v;
    }
    this->pos = this->pos + v;
    return *this;
}

inline wiremesh wiremesh::operator -=(const vec& v)
{
    for (vec& p : this->vertices) {
        p = p - v;
    }
    this->pos = this->pos - v;
    return *this;
}

inline wiremesh wiremesh::operator*=(mat T)
{
    for (vec& p : this->vertices) {
        p = T * p;
    }
    this->pos = T * this->pos;
    return *this;
}

//VERTEX SHADER
vertex_shader::vertex_shader(draw_device& ddev, camera& cam) { 
this->ddev = &ddev; 
this->cam = &cam; 
}

edge vertex_shader::process_edge(vec v1, vec v2, double dist_squared, u32 color){

    vec normal = this->cam->get_normal();
    vec clip_point = this->cam->get_focal_point() + normal;
    realnum foc_dist = this->cam->get_foc_dist();

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

    return edge(v1, v2, dist_squared, color);
}

void vertex_shader::process_meshes()
{
    double render_dist = 2000;
    vec focal_point = this->cam->get_focal_point();

    unordered_map<wiremesh*, vector<edge>> mesh_edge_map;
    unordered_map<wiremesh*, vector<face>> mesh_face_map;

    //this adds each edge to a partition of edge_container for its
    //respective mesh
    for (wiremesh* pmesh: this->meshes) {
        auto all_projected_vertices = pmesh->vertices;
        for (vec& v : all_projected_vertices) {
            v = cam->proj(v);
        }

        vector<edge> edges;
        vector<face> faces;

        for (auto vertex_pair : pmesh->edges) {

            vec& v1 = pmesh->vertices[vertex_pair.first];
            vec& v2 = pmesh->vertices[vertex_pair.second];

            //the midpoint of v1 and v2 relative to the camera is projected onto 
            // the xy-plane to get cylindrical distance.
            vec midpoint = proj_xy * ((v1 + v2) * 0.5 - focal_point);
            double dist_squared = R3::ip(midpoint, midpoint);

            //dist_squared of -1 is reserved for when the edge is off-camera. We 
            // only render edges which are in view
            if (dist_squared != -1){
                edges.push_back(process_edge(v1, v2, dist_squared, pmesh->color));
            }
        }

        vec n = cam->get_normal();

        for (face_internal facedata : pmesh->faces) {
            int npoints = facedata.num_vertices;

            //gather vertices for face
            vector<vec> projected_vertices(npoints);
            vector<vec> vertices(npoints);

            for (int i = 0; i < npoints; i++) {
                projected_vertices[i] = all_projected_vertices[facedata.vertex_indices[i]];
                vertices[i] = pmesh->vertices[facedata.vertex_indices[i]];
            }

            vec midpoint = centroid(vertices) - focal_point;
            double dist_squared = R3::ip(midpoint, midpoint);

            //handle shading
            vec v1 = vertices[1] - vertices[0];
            vec v2 = vertices[2] - vertices[0];

            vec surface_normal = R3::cross_prod(v1,v2);
            double brightness = 0;

            for (light* L : this->lights) {
                double dist;
                vec light_ray = L->process_ray(midpoint+focal_point, & dist);

                brightness +=( R3::ip(surface_normal, light_ray)*L->strength )/ (R3::norm(surface_normal)*pow(dist,2));
            }

            u32 color = darken(pmesh->color, abs(brightness));

            if (dist_squared != -1) {
                faces.push_back(face(projected_vertices,facedata.adjacency,dist_squared,color));
            }
        }
       
        mesh_face_map.insert({ pmesh,faces });
        mesh_edge_map.insert({ pmesh,edges });
    }

    //All edges in edge_container are then put into a single std::vector and sorted.
    int num_faces_total = 0;
    for (wiremesh* pmesh : meshes) {
        num_faces_total += mesh_face_map.at(pmesh).size();
    }
    vector<face*> all_faces(num_faces_total); {
        int i = 0;
        for (wiremesh* pmesh : meshes) {
            for (face& F : mesh_face_map.at(pmesh)) {
                all_faces[i] = &F;
                i++;
            }
        }
    }

    // we sort the edges by the square of their distance from the camera and then 
    // draw them in that order.
    qsort(all_faces.data(),all_faces.size(),sizeof(face*), comp_face);

    for (face* F : all_faces) {
        //vector<vec> projected_vertices(E.nvertices);
        //
        //for (int i = 0; i < E.nvertices; i++) {
        //    projected_vertices[i] = cam->proj(E.vertices[i]);
        //}
        //
        ddev->draw_quadrilateral(F->adjacency, F->vertices, F->color);

        //ddev->draw_line(cam->proj(E.vertices[0]), cam->proj(E.vertices[2]), E.color);
    }
}

void vertex_shader::draw_line(vec v1, vec v2, u32 color)
{
    edge E = process_edge(v1, v2,1);
    ddev->draw_line(cam->proj(E.v1), cam->proj(E.v2),color);
}

//SHAPES
void obj_3d::set_pos(vec new_pos) { 
    mesh.set_pos(new_pos); 
    this->pos = new_pos;
}

void obj_3d::transform(mat T){
    vec pos = this->get_pos();
    for (vec& v : this->mesh.vertices) {
        vec temp = v - pos;
        v = pos + T * temp;
    }
}

void obj_3d::affine_transform(mat T) {
    vec pos = this->get_pos();
    for (vec& v : this->mesh.vertices) {
        v = T * v;
    }
}

//SURFACE
surface::surface(int size, realnum spacing) {
    //don't look at this madness
    const vec zero = mat::zero(3, 1);
    const vec e[3] = { mat::std_basis(3,0) , mat::std_basis(3,1), mat::std_basis(3,2) };

    int nvertices = pow(size, 2);

    vector<vec> vertices(nvertices);
    matrix<int> adjacency = matrix<int>::zero(nvertices, nvertices);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            vec O = zero + e[0] * spacing * i + e[1] * spacing * j;
            vertices[i * size + j] = O;

            for (int k = 0; k < 4; k++) {
                int sgn = pow(-1, k / 2);
                int dx = sgn * (k % 2); int dy = sgn * ((k + 1) % 2);
                int new_x = i + dx; int new_y = j + dy;

                if ((new_x >= 0 && new_x < size && new_y >= 0 && new_y < size)) {
                    vec P = vertices[new_x * size + new_y];
                    link(i * size + j, new_x * size + new_y, &adjacency);
                }
            }
        }
    }
    this->size = size;
    this->mesh = wiremesh(vertices, adjacency);
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

    int nvertices = adjacency.get_rows();
    vector<vec> vertices(nvertices);

    vertices[0] = zero;
    for (int i = 0; i < nvertices -1; i++) {
        vertices[i + 1] =
            e[(i) % 3] * side_length + 
            e[(i+1) % 3] * (i >= 3) * side_length +
            e[(i + 2) % 3] * (i >= 6) * side_length;
    }

    this->mesh = wiremesh(vertices, adjacency);
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

    vector<vec> vertices;

    int size = res * 4; // amount of vertices per halfcircle

    realnum theta = PI / (2 * res);
    vec axis = {0,0,r};

    vertices.push_back(axis + pos);
    for (int i = 0; i < size; i++) {
        for (int j = 1; j < size/2; j++) {
            vec point = R3::rotate_intr(theta * i, theta * j, 0) * axis + pos;
            vertices.push_back(point);
        }
    }
    vec top = R3::rotate_intr(0, PI, 0) * axis  + pos;
    vertices.push_back(top);
    int nvertices = vertices.size();

    //this matrix tells us which vertices are connected
    matrix<int> adjacency = matrix<int>::zero(nvertices,nvertices);

    for (int i = 0; i < res * 4; i++) {
        for (int j = 0; j < res * 2; j++) {

            if (j == 0) {
                link(0,1 + i*(res*2 - 1) + j, &adjacency);
            }
            else if (j == res * 2 - 1) {
                link(nvertices-1,i * (res * 2 - 1) + j, &adjacency);
            }
            else {
                link(i * (res * 2 - 1) + j,i * (res * 2 - 1) + j + 1, &adjacency);          
            }

            bool in_range = ((i + 1) * (res * 2 - 1) + j < nvertices - 1) && abs(i) + abs(j) != 0;

            if (in_range) {
                link(i * (res * 2 - 1) + j,(i + 1) * (res * 2 - 1) + j, &adjacency);
            }
            else {
                link(i * (res * 2 - 1) + j,j, &adjacency);
            }
        }
    }
    
    this->mesh = wiremesh(vertices, adjacency);
    this->mesh.set_pos(pos);
    this->pos = mesh.get_pos();

}

void sphere::draw_vertices(draw_device ddev, camera cam)
{
    for (vec p : this->mesh.vertices) {
        ddev.draw_circ(cam.proj(p),3.4,0x00FFFF);
    }
    return;
}

