#include "vector.h"
#include "objects.h"
#include "utils.h"
#include "renderer.h" // g_plane_test
#include <math.h> // round, abs
#include <stdlib.h>
#include <stdbool.h> // bool
#include <stddef.h> // size_t
#include <stdio.h> // FILE, open, fclose, printf
#include <ctype.h> // isempty
#include <string.h> // strtok
#include <assert.h> // assert


// perpendicular 2D vector, i.e. rotated by 90 degrees ccw
#define VEC_PERP(src) (      \
{                            \
    __typeof__ (src) _ret;   \
    _ret.x = -src.y;         \
    _ret.y = src.x;          \
    _ret.z = 0;              \
    _ret;                    \
}                            \
)

#define VEC_PERP_DOT_PROD(a, b) a.x*b.y - a.y*b.x

static char conn_letters[] = {
#define X(a, b, c) a,
    CONN_TABLE
#undef X
};

static int conn_names[NUM_CONNECTIONS] = {
#define X(a, b, c) b,
    CONN_TABLE
#undef X
};

//----------------------------------------------------------------------------------------------------------
// Static functions
//----------------------------------------------------------------------------------------------------------
static inline bool obj__starts_with(const char* buffer, char first) {
    return buffer[0] == first;
}

static inline bool obj__line_is_comment(const char* buffer) {
    return buffer[0] == '#';
}

bool obj__line_is_empty(const char *s)
{
  while (*s) {
    if (!isspace(*s))
      return false;
    s++;
  }
  return true;
}

static inline void obj__mesh_update_bbox(mesh_t* mesh, int width, int height, int depth) {
    const int m = sqrt(width*width + height*height + depth*depth);
    mesh->bounding_box.x0 = mesh->center->x - m/2;
    mesh->bounding_box.y0 = mesh->center->y - m/2;
    mesh->bounding_box.z0 = mesh->center->z - m/2;
    mesh->bounding_box.x1 = mesh->center->x + m/2;
    mesh->bounding_box.y1 = mesh->center->y + m/2;
    mesh->bounding_box.z1 = mesh->center->z + m/2;
}
//----------------------------------------------------------------------------------------------------------
// Renderable shapes
//----------------------------------------------------------------------------------------------------------
mesh_t* obj_mesh_from_file(const char* fpath, int cx, int cy, int cz, unsigned width, unsigned height, unsigned depth) {
    FILE* file;
    file = fopen(fpath, "r");
    if (file == NULL) {
        printf("Fatal error: Cannot open file %s\n. Exiting...", fpath);
        exit(1);
    }
    char buffer[128];
    size_t n_verts = 0, n_surfs = 0;
    //// read numbers of vertices and surfaces
    while((fgets (buffer, 128, file))!= NULL) {
        if (obj__starts_with(buffer, 'v'))
            n_verts++;
        else if (obj__starts_with(buffer, 'f'))
            n_surfs++;
    }
    //// allocate data and prepare for reading
    mesh_t* new = malloc(sizeof(mesh_t));
    new->center = vec_vec3i_new(cx, cy, cz);
        new->n_vertices = n_verts;
    new->n_faces = n_surfs;
    new->vertices = (vec3i_t**) malloc(sizeof(vec3i_t*) * n_verts);
    new->vertices_backup = (vec3i_t**) malloc(sizeof(vec3i_t*) * n_verts);
    obj__mesh_update_bbox(new, width, height, depth);
    // allocate 2D array that indicates how vertices are connected at each surface
    new->connections = malloc(new->n_faces * sizeof(int*));
    for (int i = 0; i < new->n_faces; ++i)
        new->connections[i] = malloc(6 * sizeof(int));

    //// set vertices and surfaces
    // go back to beginning of the file
    fseek(file, 0, SEEK_SET);
    size_t ivert = 0, isurf = 0;
    while((fgets (buffer, 128, file)) != NULL) {
        char* pch = strtok (buffer, " vf");
        if (obj__starts_with(buffer, 'v')) {
            const float x = atof(pch);
            pch = strtok (NULL, " ");
            const float y = atof(pch);
            pch = strtok (NULL, " ");
            const float z = atof(pch);
            new->vertices[ivert++] = vec_vec3i_new(round(width/2*x), round(height/2*y), round(depth/2*z));
        } else if (obj__starts_with(buffer, 'f')) {
            assert(atoi(pch) <= new->n_vertices);
            new->connections[isurf][0] = atoi(pch);
            pch = strtok (NULL, " ");
            new->connections[isurf][1] = atoi(pch);
            pch = strtok (NULL, " ");
            new->connections[isurf][2] = atoi(pch);
            pch = strtok (NULL, " ");
            new->connections[isurf][3] = atoi(pch);
            pch = strtok (NULL, " ");
                        for (int i = 0; i < NUM_CONNECTIONS; ++i) {
                if (*pch == conn_letters[i])
                    new->connections[isurf][4] = conn_names[i];
            }
            pch = strtok (NULL, " ");
            new->connections[isurf][5] = *pch;
            isurf++;
        }
    }
    fclose(file);
    //// shift them to center and back them up
    for (int i = 0; i < new->n_vertices; ++i) {
        *new->vertices[i] = vec_vec3i_add(new->vertices[i], new->center);
        new->vertices_backup[i] = vec_vec3i_new(0, 0, 0);
        vec_vec3i_copy(new->vertices_backup[i], new->vertices[i]);
    }
    return new;
}

mesh_t* obj_triangle_new(vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, color_t color) {
    mesh_t* new = malloc(sizeof(mesh_t));
    new->center = malloc(sizeof(vec3i_t));
    new->center->x = (p0->x + p1->x + p2->x)/3;
    new->center->y = (p0->y + p1->y + p2->y)/3;
    new->center->z = (p0->z + p1->z + p2->z)/3;
    new->n_vertices = 3;
    new->n_faces = 1;
    new->vertices = (vec3i_t**) malloc(sizeof(vec3i_t*) * new->n_vertices);
    new->vertices_backup = (vec3i_t**) malloc(sizeof(vec3i_t*) * new->n_vertices);
    unsigned width = UT_MAX( UT_MAX(abs(p0->x - p1->x), abs(p0->x - p2->x)),
                             UT_MAX(abs(p0->x - p1->x), abs(p1->x - p2->x)));
    unsigned height = UT_MAX(UT_MAX(abs(p0->y - p1->y), abs(p0->y - p2->y)),
                             UT_MAX(abs(p0->y - p1->y), abs(p1->y - p2->y)));
    new->vertices[0] = vec_vec3i_new(p0->x, p0->y, p0->z);
    new->vertices[1] = vec_vec3i_new(p1->x, p1->y, p1->z);
    new->vertices[2] = vec_vec3i_new(p2->x, p2->y, p2->z);
    obj__mesh_update_bbox(new, width, height, 1);

    // allocate 2D array that indicates how vertices are connected at each surface
    new->connections = malloc(new->n_faces * sizeof(int*));
    for (int i = 0; i < new->n_faces; ++i)
        new->connections[i] = malloc(6 * sizeof(int));
    // define surfaces
    new->connections[0][0] = 0;
    new->connections[0][1] = 1;
    new->connections[0][2] = 2;
    new->connections[0][3] = 0;
    new->connections[0][4] = CONNECTION_TRIANGLE;
    new->connections[0][5] = color;

    // finish creating the vertices - shift the to the mesh's origin, back them up
    for (int i = 0; i < new->n_vertices; ++i) {
        *new->vertices[i] = vec_vec3i_add(new->vertices[i], new->center);
        new->vertices_backup[i] = vec_vec3i_new(0, 0, 0);
        vec_vec3i_copy(new->vertices_backup[i], new->vertices[i]);
    }
    return new;
}

void obj_mesh_rotate (mesh_t* mesh, float angle_x_rad, float angle_y_rad, float angle_z_rad) {
    for (size_t i = 0; i < mesh->n_vertices; ++i) {
        // first, reset each vertex so no floating point error is accumulated
        vec_vec3i_copy(mesh->vertices[i], mesh->vertices_backup[i]);

        // point to rotate about
        int x0 = mesh->center->x, y0 = mesh->center->y, z0 = mesh->center->z;
        // rotate around x axis, then y, then z
        // We rotate as follows (* denotes matrix product, C the mesh's origin):
        // v = v - C, v = Rz*Ry*Rx*v, v = v + C
        vec_vec3i_rotate(mesh->vertices[i], angle_x_rad, angle_y_rad, angle_z_rad, x0, y0, z0);
    }
}

void obj_mesh_translate(mesh_t* mesh, float dx, float dy, float dz) {
    // to update bounding box after the translation
    const unsigned width = abs(mesh->bounding_box.x0 - mesh->bounding_box.x1);
    const unsigned height = abs(mesh->bounding_box.y0 - mesh->bounding_box.y1);
    const unsigned depth = abs(mesh->bounding_box.z0 - mesh->bounding_box.z1);
    vec3i_t translation = {round(dx), round(dy), round(dz)};
    *mesh->center = vec_vec3i_add(mesh->center, &translation);
    for (size_t i = 0; i < mesh->n_vertices; ++i) {
        *mesh->vertices[i] = vec_vec3i_add(mesh->vertices[i], &translation);
        vec_vec3i_copy(mesh->vertices_backup[i], mesh->vertices[i]);
    }

    obj__mesh_update_bbox(mesh, width, height, depth);
}

void obj_mesh_free(mesh_t* mesh) {
    // free the data of the vertices first
    for (size_t i = 0; i < mesh->n_vertices; ++i) {
        free(mesh->vertices[i]);
        free(mesh->vertices_backup[i]);
    }
    free(mesh->vertices);
    free(mesh->vertices_backup);
    for (int i = 0; i < mesh->n_faces; ++i)
        free(mesh->connections[i]);
    free(mesh->connections);
    free(mesh->center);
    free(mesh);
}

//----------------------------------------------------------------------------------------------------------
// Ray
//----------------------------------------------------------------------------------------------------------
ray_t* obj_ray_new(int x0, int y0, int z0, int x1, int y1, int z1) {
    ray_t* new = malloc(sizeof(ray_t));
    new->orig = vec_vec3i_new(x0, y0, z0);
    new->end = vec_vec3i_new(x1, y1, z1);
    return new;
}

void obj_ray_set(ray_t* ray, int x0, int y0, int z0, int x1, int y1, int z1) {
    ray->orig = vec_vec3i_new(x0, y0, z0);
    ray->end = vec_vec3i_new(x1, y1, z1);

}

void obj_ray_send(ray_t* ray, int x, int y, int z) {
    ray->end->x = x;
    ray->end->y = y;
    ray->end->z = z;
}

void obj_ray_free(ray_t* ray) {
    free(ray->orig);
    free(ray->end);
    free(ray);
}

//-------------------------------------------------------------------------------------------------------------
// Camera
//-------------------------------------------------------------------------------------------------------------
camera_t* obj_camera_new(int cam_x0, int cam_y0, float focal_length) {
    camera_t* new = malloc(sizeof(camera_t));
    new->x0 = cam_x0;
    new->y0 = cam_y0;
    new->focal_length = focal_length;
    return new;
}

void obj_camera_set(camera_t* camera, int cam_x0, int cam_y0, float focal_length) {
    camera->x0 = cam_x0;
    camera->y0 = cam_y0;
    camera->focal_length = focal_length;
}


//----------------------------------------------------------------------------------------------------------
// Plane
//----------------------------------------------------------------------------------------------------------

plane_t* obj_plane_new (vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
    /*
    * Determine the plane through 3 3D points p0, p1, p2 by determining:
    *     1. the normal vector
    *     2. the offset
    *
    *                                     normal
    *                                       ^
    *                                      /
    *                    +----------------/-----------+
    *                   /     *p0        /           /
    *                  /       <_       /           /
    *                 /          \__   /           /
    *                /              \ /           /
    *               /               *p1          /
    *              /             _/             /
    *             /           _/               /
    *            /           <                /
    *           /         p2*                /
    *          /                            /
    *         +----------------------------+
    * If p0, p1, p2 are co-planar, then the normal through p1 is
    * perpendicular to both * p1p2 = p2 - p1 and p1p0 = p0 - p1.
    * Thererefore it's determined as the cross product of the two:
    * normal = p1p2 x p1p0 = (p2 - p1) x (p0 - p1)
    *
    *                                    normal
    *                                    ^
    *                                   /
    *                  +---------------/-----------+
    *                 /               /           /
    *                /               /           /
    *               /              *p1          /
    *              /              /            /
    *             /           ___/            /
    *            /           /               /
    *           /       * <_/               /
    *          /        x                  /
    *         +---------------------------+
    * If x = (x,y,z) is any point on the plane, then the normal
    * through p1 is perpendicular to p1x = x - p1 therefore their
    * dot product is zero:
    * n.(x - p1) = 0 =>
    * n.x - n.p1 = 0
    * -n.p1 is the offset from the origin
    */
    plane_t* new = malloc(sizeof(plane_t));
    new->normal = malloc(sizeof(vec3_t));
    vec3i_t p1p2 = vec_vec3i_sub(p2, p1);
    vec3i_t p1p0 = vec_vec3i_sub(p0, p1);
    *new->normal = vec_vec3i_crossprod(&p1p2, &p1p0);
    new->offset = -vec_vec3i_dotprod(new->normal, p1);
    return new;
}



void obj_plane_set(plane_t* plane, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
    // reset plane's normal and offset like obj_plane_new function computes them
    vec3i_t p1p2 = vec_vec3i_sub(p2, p1);
    vec3i_t p1p0 = vec_vec3i_sub(p0, p1);
    *plane->normal = vec_vec3i_crossprod(&p1p2, &p1p0);
    plane->offset = -vec_vec3i_dotprod(plane->normal, p1);
}


// Whether a point m is inside a triangle (a, b, c)
bool obj_is_point_in_triangle(vec3i_t* m, vec3i_t* a, vec3i_t* b, vec3i_t* c) {
/*
 * To test whether a point is inside a triangle,    | a_perp(-a_y, a,x)
 * we use the concept of perpendicular (perp)       | ^                     <----
 * vectors and perpendicular dot product. Perp      |  \                        |
 * dot product (pdot) formulates whether vector b is|  |                        |
 * clockwise (cw) or counterclockwise (ccw) of a.   |   \
 * Given vector a(a_x, a_y), its perp vector a_perp |    \                  a(a_x, a_y)
 * is defined as the same vector rotated by 90      |     \             ---->
 * degrees ccw:                                     |     |   ---------/
 * a_perp = (-a_y, a_x)                             |      \-/
 *                                                  |
 * The dot product (.) alone doesn't tell us whether|
 * b is (c)cw of a. We need the pdot for that.      |  ^ a_perp        b cw from a
 * As shown in the sketch on the right half:        |  |               angle(a, b) > 90
 *                                                  |   \              a_perp . b < 0
 * a_perp . b < 0 when b is cw from a and the       |   |       ----->
 * angle between a, b is obtuse and                 |    |-----/     a
 * a_perp . b < 0 when b is cw from a and the       |    |
 * angle between a, b is acute.                     |    |
 *                                                  |    v b
 * Therefore a_perp . b < 0 when b is cw from a.    |
 * Similarly, a_perp . b > 0 when b is ccw from a.  |  ^ a_perp         b cw from a
 * .                                               .|   \               angle(a, b) < 90
 * .                                               .|   |       ----->  a_perp . b < 0
 * .                                               .|   -------/     a
 * .                                               .|    \
 * .               (cont'ed)                       .|     \-
 * .                                               .|       \
 * .                                               .|        > b
 * .                                               .|
 * The scematic below shows that for point M to be  | For M to be inside triangle (ABC),
 * inside triangle (ABC) the following condition    | MB needs to be (c)cw from MA, MC
 * must be satisfied:                               | (c)cw from MB and MA (c)cw from MC
 *                                                  |                   A
 * (MB ccw from MA) => MA_perp . MB > 0 and         |                  _+
 * (MC ccw from MB) => MB_perp . MC > 0 and         |                 / ^\_
 * (MA ccw from MC) => MC_perp . MA > 0 and         |               _/ /   \
 * or                                               |              /   |    \_
 * (MB cw from MA) => MA_perp . MB < 0 and          |             /   /       \
 * (MC cw from MB) => MB_perp . MC < 0 and          |           _/  M*------   \_
 * (MA cw from MC) => MC_perp . MA < 0 and          |          /  --/       \---->
 * .                                               .|         / -/     ______/   +
 * .                                               .|       _--/______/           C
 * .                                               .|      </_/
 * .                                               .|      +
 * .                                               .|      B
 */
    const vec3i_t ma = vec_vec3i_sub(m, a);
    const vec3i_t mb = vec_vec3i_sub(m, b);
    const vec3i_t mc = vec_vec3i_sub(m, c);
    // cw = clockwise, ccw = counter-clockwise
    const bool are_all_cw =  ((VEC_PERP_DOT_PROD(ma, mb) < 0) &&
                              (VEC_PERP_DOT_PROD(mb, mc) < 0) &&
                              (VEC_PERP_DOT_PROD(mc, ma) < 0));
    const bool are_all_ccw = ((VEC_PERP_DOT_PROD(ma, mb) > 0) &&
                              (VEC_PERP_DOT_PROD(mb, mc) > 0) &&
                              (VEC_PERP_DOT_PROD(mc, ma) > 0));
    return are_all_cw || are_all_ccw;
}

bool obj_is_point_in_rect(vec3i_t* m, vec3i_t* a, vec3i_t* b, vec3i_t* c, vec3i_t* d) {
   /*
    * The diagram below visualises the conditions for M to be inside rectangle ABCD:
    *
    *                  A        (AM.AB).unit(AB)     B    AM.AB > 0
    *                  +---------->-----------------+     AM.AB < AB.AB
    *                  |          .                 |
    *                  |          .                 |
    *                  |          .                 |
    *                  |          .                 |     AM.AD > 0
    * (AD.AM).unit(AD) v. . . . . *M                |     AM.AD < AD.AD
    *                  |                            |
    *                  |                            |
    *                  |                            |
    *                  +----------------------------+
    *                  D                            C
    *
    */
    vec3i_t ab = vec_vec3i_sub(a, b);
    vec3i_t ad = vec_vec3i_sub(a, d);
    vec3i_t am = vec_vec3i_sub(a, m);
    return (0 < vec_vec3i_dotprod(&am, &ab)) &&
           (vec_vec3i_dotprod(&am, &ab) < vec_vec3i_dotprod(&ab, &ab)) &&
           (0 < vec_vec3i_dotprod(&am, &ad)) &&
           (vec_vec3i_dotprod(&am, &ad) < vec_vec3i_dotprod(&ad, &ad));
}

vec3i_t render__ray_plane_intersection(plane_t* plane, ray_t* ray) {
   /*
    * The parametric line of a ray from from the origin O through
    * point B ('end' of the ray) is:
    * R(t) = O + t(B - O) = tB
    * This ray meets the plane for some t=t0 such that:
    * R(t0) = B*t0
    * Therefore R(t0) validates the equation of the plane.
    * For the plane we know the normal vector n and the offset
    * from the origin d. Any point X on the plane validates its
    * equation, which is:
    * n.X = d
    * Since R(t0) lies on the plane:
    * n.R(t0) = d =>
    * n.B*t0 = d =>
    * t0 = d/(n.B)
    * Finally, the ray meets the plane at point
    * R(t0) = (d/(n.B))*B
    * This is what this function returns.
    */
    float t0 = (float)plane->offset / vec_vec3i_dotprod(plane->normal, ray->end);
    // only interested in intersections along the positive direction
    t0 = (t0 < 0.0) ? -t0 : t0;
    vec3i_t ray_at_intersection = vec_vec3i_mul_scalar(ray->end, t0);
    return ray_at_intersection;
}

bool obj_ray_hits_rectangle(ray_t* ray, vec3i_t** points) {
    // find the intersection between the ray and the plane segment
    // defined by p0, p1, p2, p3 and if the intersection is whithin
    // that segment, return true
    vec3i_t* p0 = points[0];
    vec3i_t* p1 = points[1];
    vec3i_t* p2 = points[2];
    vec3i_t* p3 = points[3];
    obj_plane_set(g_plane_test, p0, p1, p2);
    vec3i_t ray_plane_intersection = render__ray_plane_intersection(g_plane_test, ray);
    return obj_is_point_in_rect(&ray_plane_intersection, p0, p1, p2, p3);
}

bool obj_ray_hits_triangle(ray_t* ray, vec3i_t** points) {
    // Find the intersection between the ray and the triangle (p0, p1, p2).
    // Return whether the intersection is whithin that triangle
    vec3i_t* p0 = points[0];
    vec3i_t* p1 = points[1];
    vec3i_t* p2 = points[2];
    obj_plane_set(g_plane_test, p0, p1, p2);
    vec3i_t ray_plane_intersection = render__ray_plane_intersection(g_plane_test, ray);
    return obj_is_point_in_triangle(&ray_plane_intersection, p0, p1, p2);
}


void obj_plane_free (plane_t* plane) {
    free(plane->normal);
    free(plane);
}
