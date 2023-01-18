#include "vector.h"
#include "objects.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define SQRT_TWO 1.414213
#define HALF_SQRT_TWO 0.7071065 


static inline bool is_point_in_rec(vec3i_t* m, vec3i_t* a, vec3i_t* b, vec3i_t* c, vec3i_t* d) {
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
    vec3i_t ab = (vec3i_t) {b->x - a->x, b->y - a->y, b->z - a->z};
    vec3i_t ad = (vec3i_t) {d->x - a->x, d->y - a->y, d->z - a->z};
    vec3i_t am = (vec3i_t) {m->x - a->x, m->y - a->y, m->z - a->z};
    return (0 < vec_vec3i_dotprod(&am, &ab)) && (vec_vec3i_dotprod(&am, &ab) < vec_vec3i_dotprod(&ab, &ab)) &&
           (0 < vec_vec3i_dotprod(&am, &ad)) && (vec_vec3i_dotprod(&am, &ad) < vec_vec3i_dotprod(&ad, &ad));
}


//----------------------------------------------------------------------------------------------------------
// Cube
//----------------------------------------------------------------------------------------------------------
cube_t* obj_cube_new(int cx, int cy, int cz, int side) {
/*
 *          p3                  p2 
 *           +-------------------+
 *           | \                 | \
 *           |    \              |    \                  ^y
 *           |      \  p7        |       \               |
 *           |         +-------------------+ p6          |
 *           |         |         .         |             |
 *           |         |*(cx,xy,cz)        |             o-------> x
 *           |         |         .         |              \
 *           |         |         .         |               \
 *           |         |         .         |                v z
 *        p0 +---------|.........+ p1      |
 *            \        |           .       |
 *              \      |             .     |
 *                 \   |                .  |
 *                    \+-------------------+
 *                     p4                   p5
 */
        cube_t* new = malloc(sizeof(cube_t));
        new->center = vec_vec3i_new(cx, cy, cz);
        new->vertices = (vec3i_t**) malloc(sizeof(vec3i_t*) * 8);
        new->vertices_backup = (vec3i_t**) malloc(sizeof(vec3i_t*) * 8);
        int diag = round(HALF_SQRT_TWO * side); 
        new->vertices[0] = vec_vec3i_new(cx-diag, cy-diag, cz-diag);
        new->vertices[1] = vec_vec3i_new(cx+diag, cy-diag, cz-diag);
        new->vertices[2] = vec_vec3i_new(cx+diag, cy+diag, cz-diag);
        new->vertices[3] = vec_vec3i_new(cx-diag, cy+diag, cz-diag);
        new->vertices[4] = vec_vec3i_new(cx-diag, cy-diag, cz+diag);
        new->vertices[5] = vec_vec3i_new(cx+diag, cy-diag, cz+diag);
        new->vertices[6] = vec_vec3i_new(cx+diag, cy+diag, cz+diag);
        new->vertices[7] = vec_vec3i_new(cx-diag, cy+diag, cz+diag);
        // back them up
        new->vertices_backup[0] = vec_vec3i_new(cx-diag, cy-diag, cz-diag);
        new->vertices_backup[1] = vec_vec3i_new(cx+diag, cy-diag, cz-diag);
        new->vertices_backup[2] = vec_vec3i_new(cx+diag, cy+diag, cz-diag);
        new->vertices_backup[3] = vec_vec3i_new(cx-diag, cy+diag, cz-diag);
        new->vertices_backup[4] = vec_vec3i_new(cx-diag, cy-diag, cz+diag);
        new->vertices_backup[5] = vec_vec3i_new(cx+diag, cy-diag, cz+diag);
        new->vertices_backup[6] = vec_vec3i_new(cx+diag, cy+diag, cz+diag);
        new->vertices_backup[7] = vec_vec3i_new(cx-diag, cy+diag, cz+diag);
        return new;
}

void obj_cube_rotate (cube_t* cube, float angle_x_rad, float angle_y_rad, float angle_z_rad) {
    // first, reset the vertices so no floating point error is accumulated
    for (int i = 0; i < 8; ++i)
        vec_vec3i_copy(cube->vertices[i], cube->vertices_backup[i]);

    float a = angle_x_rad, b = angle_y_rad, c = angle_z_rad;
    float ca = cos(a), cb = cos(b), cc = cos(c);
    float sa = sin(a), sb = sin(b), sc = sin(c);
    float matrix_rotx[3][3] = {
        {1, 0,  0  },
        {0, ca, -sa},
        {0, sa, ca },
    };
    float matrix_roty[3][3] = {
        {cb,  0, sb},
        {0,   1, 0},
        {-sb, 0, cb},
    };
    float matrix_rotz[3][3] = {
        {cc, -sc, 0},
        {sc, cc,  0},
        {0,  0,   1},
    };
    for (int i = 0; i < 8; ++i) {
        // -(cx, cy, cz)
        cube->vertices[i]->x -= cube->center->x;
        cube->vertices[i]->y -= cube->center->y;
        cube->vertices[i]->z -= cube->center->z;

        // Rx
        int x = cube->vertices[i]->x;
        int y = cube->vertices[i]->y;
        int z = cube->vertices[i]->z;
        cube->vertices[i]->x = round(matrix_rotx[0][0]*x + matrix_rotx[0][1]*y + matrix_rotx[0][2]*z);
        cube->vertices[i]->y = round(matrix_rotx[1][0]*x + matrix_rotx[1][1]*y + matrix_rotx[1][2]*z);
        cube->vertices[i]->z = round(matrix_rotx[2][0]*x + matrix_rotx[2][1]*y + matrix_rotx[2][2]*z);
        // Ry
        x = cube->vertices[i]->x;
        y = cube->vertices[i]->y;
        z = cube->vertices[i]->z;
        cube->vertices[i]->x = round(matrix_roty[0][0]*x + matrix_roty[0][1]*y + matrix_roty[0][2]*z);
        cube->vertices[i]->y = round(matrix_roty[1][0]*x + matrix_roty[1][1]*y + matrix_roty[1][2]*z);
        cube->vertices[i]->z = round(matrix_roty[2][0]*x + matrix_roty[2][1]*y + matrix_roty[2][2]*z);
        // Rz
        x = cube->vertices[i]->x;
        y = cube->vertices[i]->y;
        z = cube->vertices[i]->z;
        cube->vertices[i]->x = round(matrix_rotz[0][0]*x + matrix_rotz[0][1]*y + matrix_rotz[0][2]*z);
        cube->vertices[i]->y = round(matrix_rotz[1][0]*x + matrix_rotz[1][1]*y + matrix_rotz[1][2]*z);
        cube->vertices[i]->z = round(matrix_rotz[2][0]*x + matrix_rotz[2][1]*y + matrix_rotz[2][2]*z);
        // +(cx, cy, cz)
        cube->vertices[i]->x += cube->center->x;
        cube->vertices[i]->y += cube->center->y;
        cube->vertices[i]->z += cube->center->z;
    }
}


void obj_cube_free(cube_t* cube) {
    // free the data of the vertices first
    for (int i = 0; i < 8; ++i) {
        free(cube->vertices[i]);
        free(cube->vertices_backup[i]);
    }
    free(cube->vertices);
    free(cube->vertices_backup);
    free(cube->center);
    free(cube);
}

//----------------------------------------------------------------------------------------------------------
// Ray
//----------------------------------------------------------------------------------------------------------
ray_t* obj_ray_new(int x, int y, int z) {
    ray_t* new = malloc(sizeof(ray_t));
    new->orig = vec_vec3i_new(0, 0, 0);
    new->end = vec_vec3i_new(x, y, z);
    return new;
}

void obj_ray_set_color(ray_t* ray, char color) {
    ray->color = color;
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
    vec3i_t p1p2;
    vec3i_t p1p0;
    vec_vec3i_sub(&p1p2, p2, p1);
    vec_vec3i_sub(&p1p0, p0, p1);
    vec_vec3i_crossprod(new->normal, &p1p2, &p1p0);
    new->offset = -vec_vec3i_dotprod(new->normal, p1);
    return new;
}


vec3i_t obj_ray_plane_intersection(plane_t* plane, ray_t* ray) {
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
    int nornmal_dot_rayend = vec_vec3i_dotprod(plane->normal, ray->end);
    float t0 = ((float)plane->offset/nornmal_dot_rayend);
    // only interested in intersections along the positive direction
    t0 = (t0 < 0.0) ? -t0 : t0 ;
    //if (t0 < 0)
    //    t0 = -t0;
    vec3i_t ray_at_intersection;
    ray_at_intersection.x = round(t0*ray->end->x);
    ray_at_intersection.y = round(t0*ray->end->y);
    ray_at_intersection.z = round(t0*ray->end->z);
    return ray_at_intersection;
}

bool obj_ray_hits_rectangle(ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3) {
    // find the intersection between the ray and the plane segment
    // defined by p0, p1, p2, p3 and if the intersection is whithin
    // that segment, return true
    plane_t* plane = obj_plane_new(p0, p1, p2);
    vec3i_t ray_plane_intersection = obj_ray_plane_intersection(plane, ray);
    bool ret = false;
    if (is_point_in_rec(&ray_plane_intersection, p0, p1, p2, p3))
        ret = true;;
    obj_plane_free(plane);
    return ret;
}

extern inline int obj_plane_z_at_xy (plane_t* plane, int x, int y) {
    // solve for z in plane's eq/n: n.x*x + n.y*y + n.z*z + offset = 0
    vec3i_t coeffs = (vec3i_t) {plane->normal->x, plane->normal->y, plane->offset};
    vec3i_t xyz = (vec3i_t) {x, y, 1};
    return round(1.0/plane->normal->z*(-vec_vec3i_dotprod(&coeffs, &xyz)));
}


void obj_plane_set(plane_t* plane, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
    // reset plane's normal and offset like obj_plane_new function computes them
    vec3i_t p1p2;
    vec3i_t p1p0;
    vec_vec3i_sub(&p1p2, p2, p1);
    vec_vec3i_sub(&p1p0, p0, p1);
    vec_vec3i_crossprod(plane->normal, &p1p2, &p1p0);
    plane->offset = -vec_vec3i_dotprod(plane->normal, p1);
}

void obj_plane_free (plane_t* plane) {
    free(plane->normal);
    free(plane); 
}
